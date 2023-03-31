#include "MessageService.h"

MessageService::MessageService(WiFiClient client, const char *serverUrl, const char *username, const char *password, const char *rootInTopic, const char *rootOutTopic) : _mqClient(client)
{
    _serverUrl = serverUrl;
    _username = username;
    _password = password;
    _rootInTopic = rootInTopic;
    _rootOutTopic = rootOutTopic;

    _lastReconnectAttempt = 0;

    _mqClient.setServer(serverUrl, 1883);
    _mqClient.setCallback(handleMessage);
}

bool MessageService::reconnect()
{
    if (_mqClient.connect("BreweryHeatingModule", _username, _password, "/will", 1, false, "BreweryHeatingModule disconnected"))
    {
        _mqClient.subscribe(_rootInTopic);
    }
    return _mqClient.connected();
}

void MessageService::handleMessage(char *topic, byte *payload, unsigned int length)
{
}

bool MessageService::sendMessage(char *topic, char *payload)
{
    bool response = _mqClient.publish(topic, payload);
    return response;
}

void MessageService::loop()
{

    if (!_mqClient.connected() && WiFi.status() == WL_CONNECTED)
    {
        long now = millis();
        if (now - _lastReconnectAttempt > 5000)
        {
            _lastReconnectAttempt = now;
            Serial.println("Attempting MQTT reconnect...");
            // Attempt to reconnect
            if (reconnect())
            {
                _lastReconnectAttempt = 0;
                Serial.println("Connected!");
            }
            else
            {
                Serial.println("Unable to reconnect :(");
            }
        }
    }

}