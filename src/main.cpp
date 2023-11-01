#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>

// WiFiManager settings
char mqttServer[40] = "192.168.8.160"; // Default MQTT server
char mqttPort[6] = "1883"; // Default MQTT port
char mqttUser[40] = ""; // Default MQTT user (can be an empty string)
char mqttPassword[40] = ""; // Default MQTT password (can be an empty string)

const char* apName = "ESP8266 Relay";
const char* clientID = "esp8266-relay";
const char* controlTopic = "relay"; // Topic to control the relay
const int ledPin = LED_BUILTIN;

WiFiClient wlan;
PubSubClient mqtt(wlan);

void relay_control(byte relay, byte state) {
  byte control[] = {0xA0, relay, state, relay + 0xA0 + state};
  Serial.write(control, sizeof(control));
}

// WiFiManager callback function
void saveConfigCallback() {
  Serial.println("Configuration saved");
  strcpy(mqttServer, WiFiManagerParameter("mqtt_server").getValue());
  strcpy(mqttPort, WiFiManagerParameter("mqtt_port").getValue());
  strcpy(mqttUser, WiFiManagerParameter("mqtt_user").getValue());
  strcpy(mqttPassword, WiFiManagerParameter("mqtt_password").getValue());
}

void reconnect() {
  while (!mqtt.connected()) {
    Serial.println("Attempting to connect to the MQTT server...");
    if (mqtt.connect(clientID, mqttUser, mqttPassword)) {
      Serial.println("MQTT reconnected");
      mqtt.subscribe(controlTopic); // Subscribe to the control topic
    } else {
      Serial.print("MQTT connect failed, retrying...");
      delay(2000);
    }
  }
}

void onMessage(char* topic, byte* payload, unsigned int length) {
  // Handle received messages
  String payloadStr = "";
  for (unsigned int i = 0; i < length; i++) {
    payloadStr += (char)payload[i];
  }

  Serial.print("Received message: [");
  Serial.print(topic); // Print the received topic
  Serial.print("] ");
  Serial.println(payloadStr);

  if (strcmp(topic, controlTopic) == 0) { // Compare with the control topic
    // If a control message is received, perform the corresponding action
    if (payloadStr == "open1") {
      relay_control(1, 1); // Open the first relay
    } else if (payloadStr == "close1") {
      relay_control(1, 0); // Close the first relay
    } else if (payloadStr == "open2") {
      relay_control(2, 1); // Open the second relay
    } else if (payloadStr == "close2") {
      relay_control(2, 0); // Close the second relay
    } else if (payloadStr == "open3") {
      relay_control(3, 1); // Open the third relay
    } else if (payloadStr == "close3") {
      relay_control(3, 0); // Close the third relay
    } else if (payloadStr == "open4") {
      relay_control(4, 1); // Open the fourth relay
    } else if (payloadStr == "close4") {
      relay_control(4, 0); // Close the fourth relay
    }
  }
}

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);

  WiFiManager wifiManager;
  // Add custom parameters for configuring MQTT server information
  WiFiManagerParameter custom_mqtt_server("mqtt_server", "MQTT Server", mqttServer, 40);
  WiFiManagerParameter custom_mqtt_port("mqtt_port", "MQTT Port", mqttPort, 6);
  WiFiManagerParameter custom_mqtt_user("mqtt_user", "MQTT User", mqttUser, 40);
  WiFiManagerParameter custom_mqtt_password("mqtt_password", "MQTT Password", mqttPassword, 40);

  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_password);

  // Initialize WiFi connection and connect to a saved WiFi network or start AP mode for configuration
  wifiManager.autoConnect(apName);

  // Set the MQTT server and port
  mqtt.setServer(mqttServer, atoi(mqttPort));
  mqtt.setCallback(onMessage);
  mqtt.setClient(wlan);
}

void loop() {
  if (!mqtt.connected()) {
    reconnect();
  }
  mqtt.loop();
}
