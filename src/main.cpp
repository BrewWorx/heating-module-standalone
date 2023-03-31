// External libraries
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESPPubSubClientWrapper.h>
#include <Wire.h>
#include <LittleFS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

// Internal dependencies
#include <Vessel.h>

// Wireless Network Credentials
const char *ssid = "SSID";
const char *wifiPassword = "Password";

// TLS Certificates
#define CA_CERT_FILE "/ca.crt"
#define KEY_FILE "/client.key"
#define CERT_FILE "/client.crt"

char *ca_cert = nullptr;
char *client_cert = nullptr;
char *client_key = nullptr;

BearSSL::X509List *rootCert;
BearSSL::X509List *clientCert;
BearSSL::PrivateKey *clientKey;

std::vector<uint16_t> ciphers = {BR_TLS_RSA_WITH_AES_256_GCM_SHA384};

WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;
WiFiClientSecure wifiClient;

// Time config
WiFiUDP ntpClient;
NTPClient timeClient(ntpClient, "pool.ntp.org");

ESPPubSubClientWrapper mqClient("mqtt.panimo.org", 8883);

StaticJsonDocument<256> jsonControlMessage;

Vessel hlt(1, 12, 13); // Hot Liquor Tank
Vessel mlt(0, 14);     // Mash / Lauter Tun
Vessel bk(2, 16, 15);  // Boil Kettle

void setupCertificates();

void onMessage(char *topic, char *payload)
{
  if (payload)
  {
    DeserializationError error = deserializeJson(jsonControlMessage, payload);

    if (!error)
    {
      String vessel = jsonControlMessage["vessel"];
      double setpoint = jsonControlMessage["setpoint"];
      bool at = jsonControlMessage["at"];

      if (vessel.equals("HLT"))
      {
        Serial.println("Setting HLT setpoint");
        hlt.setTemperature(setpoint);
        hlt.toggleAutotune(at);
      }
      else if (vessel.equals("BK"))
      {
        bk.setTemperature(setpoint);
        bk.toggleAutotune(at);
      }
    }
    else
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
    }
  }
  else
    Serial.println("Payload is NULL.");
};

void setup()
{
  // Open serial communication for debugging
  Serial.begin(115200);
  Serial.println();

  wifiClient.setSSLVersion(BR_TLS12);
  wifiClient.allowSelfSignedCerts();
  wifiClient.setX509Time(timeClient.getEpochTime());

  setupCertificates();

  // LED Indicator for debugging
  pinMode(LED_BUILTIN, OUTPUT);

  mqClient.onConnect(
      [](uint16_t count)
      {
        Serial.println("MQTT Connected!");
      });

  mqClient.onConnectFail([](uint16_t count) -> bool
                         {
    Serial.println("Fuck");
    timeClient.update();
    Serial.println(wifiClient.getLastSSLError());
    return true; });

  mqClient.on("brewery/ctrl", onMessage, 0);

  gotIpEventHandler = WiFi.onStationModeGotIP(
      [](const WiFiEventStationModeGotIP &event)
      {
        Serial.print(F("Station connected, IP: "));
        Serial.println(WiFi.localIP());
      });

  disconnectedEventHandler = WiFi.onStationModeDisconnected(
      [](const WiFiEventStationModeDisconnected &event)
      {
        Serial.println(event.reason);
      });

  Serial.printf("Connecting to %s ...\n", ssid);
  WiFi.begin(ssid, wifiPassword);
}

void loop()
{
  mqClient.loop();

  hlt.compute();
  mlt.compute();
  bk.compute();

  char msg[256];
  serializeJson(hlt.getTelemetry(), msg);
  mqClient.publish("brewery/telemetry/hlt", msg);
  serializeJson(mlt.getTelemetry(), msg);
  mqClient.publish("brewery/telemetry/mlt", msg);
  serializeJson(bk.getTelemetry(), msg);
  mqClient.publish("brewery/telemetry/bk", msg);
}

void setupCertificates()
{
  // Get cert(s) from file system
  LittleFS.begin();

  File ca = LittleFS.open(CA_CERT_FILE, "r");
  if (!ca)
  {
    Serial.println("Couldn't load CA cert");
  }
  else
  {
    size_t certSize = ca.size();
    ca_cert = (char *)malloc(certSize);
    if (certSize != ca.readBytes(ca_cert, certSize))
    {
      Serial.println("Loading CA cert failed");
    }
    else
    {
      Serial.println("Loaded CA cert");
      rootCert = new BearSSL::X509List(ca_cert);
      wifiClient.setTrustAnchors(rootCert);
    }
    free(ca_cert);
    ca.close();
  }

  File key = LittleFS.open(KEY_FILE, "r");
  if (!key)
  {
    Serial.println("Couldn't load key");
  }
  else
  {
    size_t keySize = key.size();
    client_key = (char *)malloc(keySize);
    if (keySize != key.readBytes(client_key, keySize))
    {
      Serial.println("Loading key failed");
    }
    else
    {
      Serial.println("Loaded key");
      clientKey = new BearSSL::PrivateKey(client_key);
    }
    free(client_key);
    key.close();
  }

  File cert = LittleFS.open(CERT_FILE, "r");
  if (!cert)
  {
    Serial.println("Couldn't load cert");
  }
  else
  {
    size_t certSize = cert.size();
    client_cert = (char *)malloc(certSize);
    if (certSize != cert.readBytes(client_cert, certSize))
    {
      Serial.println("Loading client cert failed");
    }
    else
    {
      Serial.println("Loaded client cert");
      clientCert = new BearSSL::X509List(client_cert);
    }
    free(client_cert);
    cert.close();
  }

  wifiClient.setClientRSACert(clientCert, clientKey);
}