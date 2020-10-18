
/********************************************************************
  Mega Relay

  The Arduino Relays/Switchs brain for Home Assistant
*********************************************************************/

// ==================================================================
// Includes
// ==================================================================

#include <Arduino.h>
#include <string.h>
#include <SPI.h>
#include <Ethernet.h>
#include <MQTT.h>

// ==================================================================
// Variables/Constants
// ==================================================================

byte mac[] = { 0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xFF };
IPAddress ip(192, 168, 1, 51);
EthernetClient ethClient;

#define SERVER_IP "192.168.1.50"
#define SERVER_PORT 1883
MQTTClient client;

const int RELAYS_MAX = 16;

int relayPin[RELAYS_MAX + 1] = {0, 23, 22, 25, 24, 27, 26, 29, 28, 31, 30, 33, 32, 35, 34, 37, 36};
bool relayState[RELAYS_MAX + 1];

// ==================================================================
// Relays
// ==================================================================

void setRelay(int id, bool value) {
    int pin = relayPin[id];

    if (relayState[id] != value) {
        relayState[id] = value;
        digitalWrite(pin, value ? LOW : HIGH);
    }
}

void setupRelays() {
    int pin;

    for (byte id = 1; id <= RELAYS_MAX; id++) {
        pin = relayPin[id];
        relayState[id] = false;

        digitalWrite(pin, HIGH);
        pinMode(pin, OUTPUT);        
        pin++;
    }
}

// ==================================================================
// MQTT
// ==================================================================

void messageReceived(String &topic, String &payload) {
    // Example: ha/relay/1
    String path = topic.substring(3);
    int pos = path.indexOf("/");
    
    String type = path.substring(0, pos);
    int id = path.substring(pos + 1).toInt();
    char state = (char)payload[0];

    Serial.println(type + " " + String(id) + " = " + state);
    
    if (type == "relay") {
        bool value = (state == '1');
        setRelay(id, value);
    }
}

void publishRelays() {
    String topic;
    String state;

    for (int id = 1; id <= RELAYS_MAX; id++) {
        topic = "ha/relay/" + String(id);
        state = String((int)relayState[id]);        
        client.publish(topic.c_str(), state.c_str());
        Serial.println(String(topic) + " => " + String(state));
    }
}

void reconnect() {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect("Mega", "hass", "hass")) {
        Serial.print("connected");
        //publishRelays();
        client.subscribe("ha/#");
    } else {
        Serial.print("failed, rc=");
        Serial.print(client.lastError());
        delay(1000);
    }

    Serial.println();
}

void setupMQTT() {
    client.begin(SERVER_IP, SERVER_PORT, ethClient);
    client.onMessage(messageReceived);
}

// ==================================================================
// Ethernet
// ==================================================================

void setupEthernet() {
    Ethernet.begin(mac, ip);
    Serial.println(Ethernet.localIP());
}

// ==================================================================
// Main
// ==================================================================

void setup() {
    Serial.begin(9600);
    setupRelays();
    setupEthernet();
    setupMQTT();
    delay(1500);
}

void loop() {  
  client.loop();
  delay(10);
  if (!client.connected()) reconnect();
}
