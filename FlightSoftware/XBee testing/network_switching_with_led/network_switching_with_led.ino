#include <XBee.h>
#include <SoftwareSerial.h>

int LED_PIN = 13;
unsigned long lastMillis;
uint8_t* NET_ID_GROUND = {'2', '7', '6', '4'};
uint8_t* NET_ID_PAYLOAD = {'2', '7', '6', '9'};
uint8_t* currentNetId = NET_ID_GROUND;

void setup() {
  lastMillis = millis();
  pinMode(LED_PIN, OUTPUT)
}

void loop() {
  if (millis() - lastMillis > 500) {
    lastMillis = millis();
    if (currentNetId == NET_ID_GROUND) {
      switchToNetId(NET_ID_PAYLOAD);
      ldigitalWrite(LED_PIN, LOW)
    } else {
      switchToNetId(NET_ID_GROUND);
      digitalWrite(LED_PIN, HIGH)
    }
  }
}