#include "Queue.h"
#include <stdlib.h>
#include <stdint.h>
#include <DS3231.h>
#include <XBee.h>

#define SIMULATION_DISABLED 0
#define SIMULATION_ENABLED 1
#define SIMULATION_ACTIVATED 2

#define STATE_RTC_SETUP -1
#define STATE_P1_COMMUNICATION 0
#define STATE_P2_COMMUNICATION 1
#define STATE_GROUND_COMMUNICATION_1 2
#define STATE_GROUND_COMMUNICATION_2 3
#define STATE_GROUND_COMMUNICATION_3 4

// Pin receiving the one-pulse-per-second signal from the RTC.
// This should be an interrupt-capable pin.
#define RTC_INTERRUPT_PIN 2;

class ContainerCommunicationModule {
  public:
  uint8_t currentState = STATE_RTC_SETUP;

  DS3231 rtc;
  unsigned long secStartMillis = 0;
  uint8_t currentSec = 0;

  bool hasMessage;
  bool isBusy;
  uint8_t lastCommandEcho[10];

  uint16_t packageCount;


  XBee xbee = XBee();
  ZBRxResponse responseObj = ZBRxResponse();

  ZBTxRequest requestObj;
  ZBTxStatusResponse requestStatusObj = ZBTxStatusResponse();

  
  uint8_t GROUND_NET_ID[1] {0xACC}; // 2764
  uint8_t PAYLOADS_NET_ID[1] {0xAD1}; // 2769
  uint8_t ID_CMD[2] {'I','D'};
  AtCommandRequest atCommandRequest = AtCommandRequest(ID_CMD, GROUND_NET_ID, sizeof(GROUND_NET_ID));
  AtCommandResponse atCommandResponse = AtCommandResponse();


  PayloadCommandQueue payload1CommandQueue = PayloadCommandQueue();
  PayloadCommandQueue payload2CommandQueue = PayloadCommandQueue();
  TelemetryPacketQueue telemetryPacketQueue = TelemetryPacketQueue();

  // Specify the address of the remote XBee (this is the SH + SL)
  XBeeAddress64 payload1Address = XBeeAddress64(0x0013a200, 0x403e0f30);
  XBeeAddress64 payload2Address = XBeeAddress64(0x0013a200, 0x403e0f30);
  XBeeAddress64 groundAddress = XBeeAddress64(0x0013a200, 0x403e0f30);

  void (*setContainerTelemetryActivated)(bool telemetryActivated);
  void (*setLatestSimulationPressureValue)(int pressureVal);
  void (*setContainerSimulationMode)(int newSimulationMode);

  void init();

  void setRtcTimeFromPacket(uint8_t* packetData, uint8_t packetLength);

  void parseReceivedPacket(uint8_t* packetData, uint8_t packetLength);

  void parseCommandPacket(uint8_t* packetData, uint8_t packetLength);
  void parseTelemetryPacket(uint8_t* packetData, uint8_t packetLength);

  void switchToNetId(uint8_t netId[]);

  int receivePackets();

  void sendNextPayload1Command();

  void sendNextPayload2Command();

  void sendNextTelemetryPacket();
  void loop(uint8_t rtcSeconds);

  void switchToState(int8_t newState);

  void manageStateSwitching(uint8_t rtcSeconds);
};
