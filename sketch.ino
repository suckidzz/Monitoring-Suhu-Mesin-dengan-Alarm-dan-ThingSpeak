#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN 4
#define DHTTYPE DHT22
#define ALARM_PIN 12
#define BATAS_SUHU 60.0

const char* ssid = "Wokwi-GUEST";       // Untuk simulasi
const char* password = "";              // Kosong untuk Wokwi

const char* server = "http://api.thingspeak.com/update";
String apiKey = "DHYV80LXHTV5LS7T";     // API Key ThingSpeak kamu

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(ALARM_PIN, OUTPUT);
  digitalWrite(ALARM_PIN, LOW);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");

  WiFi.begin(ssid, password);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 10) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Wokwi-GUEST connected");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi CONNECTED");
  } else {
    Serial.println("\n❌ Gagal konek WiFi");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi GAGAL");
  }

  delay(2000);
}

void loop() {
  float suhu = dht.readTemperature();

  if (isnan(suhu)) {
    Serial.println("❌ Gagal baca suhu");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error");
    delay(2000);
    return;
  }

  Serial.print("🌡️ Suhu: ");
  Serial.print(suhu);
  Serial.println(" °C");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Suhu: ");
  lcd.print(suhu, 1);
  lcd.print(" C");

  if (suhu >= BATAS_SUHU) {
    digitalWrite(ALARM_PIN, HIGH);
    lcd.setCursor(0, 1);
    lcd.print("⚠️ Suhu Tinggi");
  } else {
    digitalWrite(ALARM_PIN, LOW);
    lcd.setCursor(0, 1);
    lcd.print("Status: Normal ");
  }

  // Kirim ke ThingSpeak
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(server) + "?api_key=" + apiKey + "&field1=" + String(suhu);
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      Serial.println("✅ Kirim ke ThingSpeak: Berhasil");
    } else {
      Serial.print("❌ Kirim gagal. Kode: ");
      Serial.println(httpCode);
    }

    http.end();
  } else {
    Serial.println("❌ WiFi tidak tersedia");
  }

  delay(15000);  // 15 detik sesuai rate limit ThingSpeak
}
