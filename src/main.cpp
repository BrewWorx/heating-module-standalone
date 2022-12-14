// External libraries
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LittleFS.h>

// Internal dependencies
#include <Vessel.h>
#include <MessageService.h>

// Wireless Network Credentials
const char *ssid = "*";
const char *wifiPassword = "********";

WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;
WiFiClient wifiClient;

PubSubClient mqClient(wifiClient); // Obsolete, will be implemented in MessageService

const char username[] = "user";
const char password[] = "password";
const char serverUrl[] = "serverUrl";
const char rootInTopic[] = "inTopic";
const char rootOutTopic[] ="outTopic";

MessageService msgService(wifiClient, serverUrl, username, password, rootInTopic, rootOutTopic);

bool ledState;
long lastReconnectAttempt = 0;

// Async RTD registry
uint16_t rtdRegistry[3];

Vessel mlt("mlt", 14);                       // Mash / Lauter Tun
Vessel hlt("hlt", 12, &mlt.output, &mlt.at); // Hot Liquor Tank
Vessel bk("bk", 16);                        // Boil Kettle

// MQTT Message handler method
void handleMessage(char *topic, byte *payload, unsigned int length)
{
}

// MQTT Reconnecting method
boolean reconnect()
{
  if (mqClient.connect("esp8266Client"))
  {
    // Once connected, publish an announcement...
    mqClient.publish("outTopic", "hello world");
    // ... and resubscribe
    mqClient.subscribe("#");
  }
  return mqClient.connected();
}

void setup()
{
  // Open serial communication for debugging
  Serial.begin(115200);
  Serial.println();

  // LED Indicator for debugging
  pinMode(LED_BUILTIN, OUTPUT);

  gotIpEventHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP &event)
                                              {
    Serial.print(F("Station connected, IP: "));
    Serial.println(WiFi.localIP()); });

  disconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected &event)
                                                            { Serial.println("Station disconnected"); });

  Serial.printf("Connecting to %s ...\n", ssid);
  WiFi.begin(ssid, wifiPassword);

  mqClient.setServer("raspberrypi", 1883);
  mqClient.setCallback(handleMessage);

  lastReconnectAttempt = 0;
}

void loop()
{
  if (!mqClient.connected() && WiFi.status() == WL_CONNECTED)
  {
    long now = millis();
    if (now - lastReconnectAttempt > 5000)
    {
      lastReconnectAttempt = now;
      Serial.println("Attempting MQTT reconnect...");
      // Attempt to reconnect
      if (reconnect())
      {
        lastReconnectAttempt = 0;
        Serial.println("Connected!");
      }
      else
      {
        Serial.println("Unable to reconnect :(");
      }
    }
  }
  else
  {
    // Client connected
    mqClient.loop();
  }

  hlt.compute();
  mlt.compute();
  bk.compute();

  digitalWrite(LED_BUILTIN, ledState);
  ledState = !ledState;
}