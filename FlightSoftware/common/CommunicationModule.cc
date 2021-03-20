#include <cstdint>
#include <SoftwareSerial.h>
#include <RTC.h>
#include <XBee.h>

#define STATE_RTC_SETUP -1
#define STATE_P1_COMMUNICATION 0
#define STATE_P2_COMMUNICATION 1
#define STATE_GROUND_COMMUNICATION_1 2
#define STATE_GROUND_COMMUNICATION_2 3
#define STATE_GROUND_COMMUNICATION_3 4

// Pin receiving the one-pulse-per-second signal from the RTC.
// This should be an interrupt-capable pin.
#define RTC_INTERRUPT_PIN 2;


class CommunicationModule {
  SoftwareSerial XBee(2, 3);
  uint8_t currentState = STATE_IDLE;

  RTC_DS3231 rtc;
  unsigned long secStartMillis = 0;
  uint8_t currentSec = 0;

  bool hasMessage;
  bool isBusy;
  uint8_t buffer[256];
  uint16_t bufferIndex;

  uint16_t packageCount;

  void init() {
    // Start the serial port
    Serial.begin(9600);
    // Tell XBee to use Hardware Serial. It's also possible to use SoftwareSerial
    xbee.setSerial(Serial);

  }


  void setDestinationAddress(int newAddress) {

  }
 
  void setNetworkId(int newId) {

  }

  void sendPackage(uint8_t[] package) {

  }

  void loop() {

    manageStateSwitching()
  
    switch(currentState) {
      case STATE_RTC_SETUP:
        xbee.readPacket();
        
        break;
      case STATE_P1_COMMUNICATION:
        //check payload 1 command ack / resend
        //receive payload 1 telemetry
        break;
      case STATE_P2_COMMUNICATION:
        //check payload 2 command ack / resend
        //receive payload 2 telemetry
        break;
      case STATE_GROUND_COMMUNICATION_1:
        //check own telemetry data ack / resend
        //receive ground commands
        break;
      case STATE_GROUND_COMMUNICATION_2:
        //check p1 telemetry data ack / resend
        //receive ground simp data
        break;
      case STATE_GROUND_COMMUNICATION_3:
        //check p2 telemetry data ack / resend
        break;
    }
}

  void switchToState(int8_t newState) {
    currentState = newState;
    switch(newState) {
      case STATE_RTC_SETUP:
        break;
      case STATE_P1_COMMUNICATION:
        //Send payload 1 commands
        break;
      case STATE_P2_COMMUNICATION:
        //Send payload 2 commands
        break;
      case STATE_GROUND_COMMUNICATION_1:
        //Send own telemetry data
        break;
      case STATE_GROUND_COMMUNICATION_2:
        //Send p1 telemetry data
        break;
      case STATE_GROUND_COMMUNICATION_3:
        //Send p2 telemetry data
        break;
    }
  }

  void manageStateSwitching() {
    DateTime time = rtc.now();
    if (time.second() != currentSec) {
      switchToState(STATE_P1_COMMUNICATION);
      secs = time.second();
      secStartMillis = millis();
    }
    else {
      unsigned long ellapsedMillis = millis() - secStartMillis;
      if (ellapsedMillis < 250 && currentState != STATE_P1_COMMUNICATION) {
        switchToState(STATE_P1_COMMUNICATION);
      }
      else if (ellapsedMillis < 500 && currentState != STATE_P2_COMMUNICATION) {
        switchToState(STATE_P2_COMMUNICATION);
      }
      else if (ellapsedMillis < 667 && currentState != STATE_GROUND_COMMUNICATION_1) {
        switchToState(STATE_GROUND_COMMUNICATION_1);
      }
      else if (ellapsedMillis < 834 && currentState != STATE_GROUND_COMMUNICATION_2) {
        switchToState(STATE_GROUND_COMMUNICATION_2);
      }
      else if (currentState != STATE_GROUND_COMMUNICATION_3) {
        switchToState(STATE_GROUND_COMMUNICATION_3);
      }
    }
  }
}