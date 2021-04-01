#include "Queue.h"
#include "CommunicationModule.h"
#include <stdlib.h>
#include <stdint.h>
#include <SoftwareSerial.h>
#include <DS3231.h>
#include <XBee.h>

#define STATE_IDLE 0
#define STATE_WAITING_FOR_GROUND_RESPONSE 1
#define STATE_WAITING_FOR_PAYLOAD_1_RESPONSE 2
#define STATE_WAITING_FOR_PAYLOAD_2_RESPONSE 3

#define SIMULATION_DISABLED 0
#define SIMULATION_ENABLED 1
#define SIMULATION_ACTIVATED 2

class ContainerCommunicationModule : CommunicationModule {
  public:
    uint8_t lastCommandEcho[10];

    uint8_t groundCommunicationState = 0;
    uint8_t payloadCommunicationState = 0;

    DS3231 rtc;

    XBee groundXBee;
    ZBRxResponse groundResponseObj = ZBRxResponse();
    ZBTxRequest groundRequestObj;
    ZBTxStatusResponse groundRequestStatusObj = ZBTxStatusResponse();

    XBee payloadsXBee;
    ZBRxResponse payloadsResponseObj = ZBRxResponse();
    ZBTxRequest payloadsRequestObj;
    ZBTxStatusResponse payloadsRequestStatusObj = ZBTxStatusResponse();

    PayloadCommandQueue payload1CommandQueue = PayloadCommandQueue();
    PayloadCommandQueue payload2CommandQueue = PayloadCommandQueue();
    TelemetryPacketQueue telemetryPacketQueue = TelemetryPacketQueue();

    void (*setContainerTelemetryActivated)(bool telemetryActivated);
    void (*setLatestSimulationPressureValue)(int pressureVal);
    void (*setContainerSimulationMode)(int newSimulationMode);

    void ContainerCommunicationModule::init(XBee groundXBeeDevice, XBee payloadsXBeeDevice, DS3231 RTC);

    void setRtcTimeFromPacket(uint8_t* packetData, uint8_t packetLength);
    void parseReceivedPacket(uint8_t* packetData, uint8_t packetLength);
    void parseCommandPacket(uint8_t* packetData, uint8_t packetLength);
    void parseTelemetryPacket(uint8_t* packetData, uint8_t packetLength);

    void sendNextPayload1Command();
    void sendNextPayload2Command();
    void sendNextTelemetryPacket();

    void manageGroundCommunication();
    void managePayloadsCommunication();

    void loop();
};
