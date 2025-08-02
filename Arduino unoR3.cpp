
#define RELAY_PIN 7

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // LOW turns ON pump if relay is active-low
}

void loop() {
  // Relay stays ON continuously
}