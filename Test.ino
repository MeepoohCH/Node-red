#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define LED_PIN 4  // ขาที่ต่อกับ LED
#define DHTPIN 5   // ขาที่ต่อเซนเซอร์ DHT11
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
const char* ssid = "Mameemeepooh";
const char* password = "hello11111111";
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_Client = "Meepooh";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[100];

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqtt_Client)) {
      Serial.println("connected");
      client.subscribe("Meepooh/led/status");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  for (int i = 0; i < length; i++) {
    message += char(payload[i]);
  }
  Serial.println(message);
  if (String(topic) == "Meepooh/led/status") {
    if (message == "ON") {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("LED ON");
    } else if (message == "OFF") {
      digitalWrite(LED_PIN, LOW);
      Serial.println("LED OFF");
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  dht.begin();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {  // ส่งข้อมูลทุก 5 วินาที
    lastMsg = now;

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // ตรวจสอบค่าที่อ่านจาก DHT
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // สร้างข้อความ JSON
    snprintf(msg, 100, "{\"temperature\":%.2f,\"humidity\":%.2f}", t, h);
    
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("Meepooh/in1", msg);  // ส่งข้อมูลไปยัง MQTT
  }

  delay(100);  // ระยะเวลาสั้นๆ เพื่อให้การทำงานต่อไป
}
