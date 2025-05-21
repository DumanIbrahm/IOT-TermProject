#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// WiFi & MQTT
#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASS     "YOUR_WIFI_PASSWORD"
#define AIO_USERNAME  "YOUR_ADAFRUIT_USERNAME"
#define AIO_KEY       "YOUR_ADAFRUIT_KEY"
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish soilMoistureFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/soil-moisture");
Adafruit_MQTT_Publish pumpStatusFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/pump-status");
Adafruit_MQTT_Subscribe manualModeFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/manual-mode");
Adafruit_MQTT_Subscribe manualPumpFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/manual-pump");

// Pinler ve Ekran
#define RELAY_PIN         25
#define SOIL_MOISTURE_PIN 32
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Durum Değişkenleri
int soilMoistureValue = 0;
bool pumpStatus = false;
bool manualMode = false;
bool manualPumpControl = false;
const int thresholdValue = 3000;

unsigned long lastMQTTSend = 0;
const unsigned long mqttInterval = 20000;

void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("🔌 WiFi bağlanıyor...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println(" ✅ Bağlandı!");
}

void connectToMQTT() {
  Serial.print("🌐 MQTT'ye bağlanılıyor...");
  int8_t ret;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    mqtt.disconnect();
    delay(5000);
  }
  Serial.println("✅ MQTT bağlı.");
}

void setupDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("❌ OLED başlatılamadı");
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}

void readMQTTCommands() {
  mqtt.processPackets(10);
  mqtt.ping();

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &manualModeFeed && manualModeFeed.lastread != nullptr) {
      String modeValue = String((char *)manualModeFeed.lastread);
      manualMode = (modeValue == "1" || modeValue == "on" || modeValue == "ON");
      Serial.print("📨 manual-mode: "); Serial.println(modeValue);
    }
    if (subscription == &manualPumpFeed && manualPumpFeed.lastread != nullptr) {
      String pumpValue = String((char *)manualPumpFeed.lastread);
      manualPumpControl = (pumpValue == "ON" || pumpValue == "on" || pumpValue == "1" || pumpValue == "true");
      Serial.print("📨 manual-pump: "); Serial.println(pumpValue);
    }
  }

  // Röle kontrolü MQTT verileri okunduktan hemen sonra yapılır
  if (manualMode) {
    digitalWrite(RELAY_PIN, manualPumpControl ? LOW : HIGH);
    pumpStatus = manualPumpControl;
    Serial.println("🧭 Mod: MANUEL");
  } else {
    bool shouldPump = (soilMoistureValue > thresholdValue);
    digitalWrite(RELAY_PIN, shouldPump ? LOW : HIGH);
    pumpStatus = shouldPump;
    Serial.println("🧪 Mod: OTOMATIK");
  }

  Serial.print("🚰 Pompa: ");
  Serial.println(pumpStatus ? "AÇIK" : "KAPALI");
}

void readSoilMoisture() {
  soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);
  Serial.print("🌱 Nem: "); Serial.println(soilMoistureValue);
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Toprak Nem:");
  display.setCursor(0, 10);
  display.print(soilMoistureValue);
  display.setCursor(0, 30);
  display.println(manualMode ? "MOD: MANUEL" : "MOD: OTOMATIK");
  display.setCursor(0, 50);
  display.print("Pompa: ");
  display.println(pumpStatus ? "ACIK" : "KAPALI");
  display.display();
}

void publishMQTTData() {
  unsigned long now = millis();
  if (now - lastMQTTSend >= mqttInterval) {
    lastMQTTSend = now;
    soilMoistureFeed.publish(soilMoistureValue);
    pumpStatusFeed.publish(pumpStatus ? "1" : "0");
    Serial.println("📡 MQTT verileri gönderildi.");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Başlangıçta pompa kapalı
  setupDisplay();
  connectToWiFi();
  mqtt.subscribe(&manualModeFeed);
  mqtt.subscribe(&manualPumpFeed);
  connectToMQTT();
}

void loop() {
  if (!mqtt.connected()) connectToMQTT();

  readMQTTCommands();
  readSoilMoisture();
  updateDisplay();
  publishMQTTData();

  delay(1000); // Dengeli çalışma için 1 sn bekleme
}
