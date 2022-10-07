#ifndef FileSystemService_h
#define FileSystemService_h

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

class MessageService
{
public:
    // Constructors
    MessageService(WiFiClient client, const char *serverUrl, const char *username, const char *password, const char *rootInTopic, const char *rootOutTopic);

    /**
     * Sum numbers in a vector.
     *
     * @param msg Message to send via MQTT.
     * @return Boolean of message delivery status
     *
     * @exceptsafe This function does not throw exceptions.
     */
    bool sendMessage(char *topic, char *payload);

private:
    PubSubClient _mqClient;

    const char *_serverUrl;
    const char *_username;
    const char *_password;
    const char *_rootInTopic;
    const char *_rootOutTopic;

    bool reconnect();

    static void handleMessage(char *topic, byte *payload, unsigned int length);
};

#endif