
#define BLYNK_TEMPLATE_ID "TMPL6R3sD_hHX"
#define BLYNK_TEMPLATE_NAME "SMART PLANT WATERING SYSTEM"
#define BLYNK_AUTH_TOKEN "14_S6EnI5sOjbUxS9ZFGALuTYyGe2izM"

// LIBRARIES
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// --- Pin Definitions ---
#define DHTPIN 14
#define DHTTYPE DHT11
#define SOIL_PIN 34
// The RELAY_PIN is now on the Arduino, not here.

// --- WiFi Credentials ---
char ssid[] = "SM-POKHARA1";
char pass[] = "6557165571";

// --- Sensor & Timer Setup ---
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

// --- State Variables ---
int mode = 0;           // 0 = Auto, 1 = Manual
int manualPump = 0;     // 1 = ON, 0 = OFF
bool isSoilDry = false;
bool pumpOn = false;

// --- Virtual Pins ---
#define PUMP_LED_VPIN V5

// --- Mode Switch (Auto / Manual) ---
BLYNK_WRITE(V4) {
  mode = param.asInt();
}

// --- Manual Pump Button ---
BLYNK_WRITE(V3) {
  manualPump = param.asInt();
}

void sendDataAndControlPump() {
  // --- Read Sensors ---
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int soilRaw = analogRead(SOIL_PIN);
  int soilPercent = map(soilRaw, 4095, 1700, 0, 100); // Using a generic but reasonable calibration
  soilPercent = constrain(soilPercent, 0, 100);

  // --- Send to Blynk ---
  Blynk.virtualWrite(V0, temp);
  Blynk.virtualWrite(V1, hum);
  Blynk.virtualWrite(V2, soilPercent);

  // --- Decide Pump State ---
  bool pumpShouldBeOn;
  if (mode == 0) { // Auto Mode
    pumpShouldBeOn = (soilPercent < 30); // Turn on if moisture is less than 30%
  } else { // Manual Mode
    pumpShouldBeOn = (manualPump == 1);
  }

  // --- SEND COMMAND TO ARDUINO ---
  // This is the key change. Instead of digitalWrite, we send a character.
  if (pumpShouldBeOn) {
    Serial2.print('1'); // Send '1' to Arduino to turn the pump ON
  } else {
    Serial2.print('0'); // Send '0' to Arduino to turn the pump OFF
  }

  // --- Pump LED Widget & Notifications ---
  Blynk.virtualWrite(PUMP_LED_VPIN, pumpShouldBeOn ? 255 : 0);

  if (pumpShouldBeOn != pumpOn) {
    pumpOn = pumpShouldBeOn;
    if (pumpOn) {
      Blynk.logEvent("pump_on", "Water Pump is ON.");
    } else {
      Blynk.logEvent("pump_off", "Water Pump is OFF.");
    }
  }

  // --- Soil Moisture Notification ---
  if (soilPercent < 30 && !isSoilDry) {
    Blynk.logEvent("dry_soil", "Soil is too dry! Pump may turn on.");
    isSoilDry = true;
  } else if (soilPercent >= 30 && isSoilDry) {
    Blynk.logEvent("wet_soil", "Soil is wet again.");
    isSoilDry = false;
  }
}

void setup() {
  Serial.begin(115200);
  // Initialize Serial2 for communication with the Arduino.
  // We specify the RX pin (16, unused) and the new TX pin (25).
  Serial2.begin(9600, SERIAL_8N1, 16, 25);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  dht.begin();
  timer.setInterval(3000L, sendDataAndControlPump);
}

void loop() {
  Blynk.run();
  timer.run();
}
