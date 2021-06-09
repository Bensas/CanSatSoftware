#include "Queue.h"
#include "CommunicationModule.h"
#include <stdlib.h>
#include <stdint.h>
#include <SoftwareSerial.h>
#include <XBee.h>

#define STATE_IDLE 0
#define STATE_WAITING_FOR_GROUND_RESPONSE 1
#define STATE_WAITING_FOR_PAYLOAD_1_RESPONSE 2
#define STATE_WAITING_FOR_PAYLOAD_2_RESPONSE 3

#define SIMULATION_DISABLED 0
#define SIMULATION_ENABLED 1
#define SIMULATION_ACTIVATED 2

#define PAYLOAD_MAX_PACKAGE_LENGTH 42

#define PACKET_COUNT_EEPROM_ADDR 4

class ContainerCommunicationModule : CommunicationModule {
  public:
    uint8_t lastCommandEcho[24];

    uint8_t groundCommunicationState = 0;
    uint8_t payloadCommunicationState = 0;
    uint16_t packetCount = 0;
    uint16_t payload1PacketCount = 0;
    uint16_t payload2PacketCount = 0;

    XBee groundXBee = XBee();;
    ZBRxResponse groundResponseObj = ZBRxResponse();
    ZBTxRequest groundRequestObj;
    ZBTxStatusResponse groundRequestStatusObj = ZBTxStatusResponse();

    XBee payloadsXBee = XBee();;
    ZBRxResponse payloadsResponseObj = ZBRxResponse();
    ZBTxRequest payloadsRequestObj;
    ZBTxStatusResponse payloadsRequestStatusObj = ZBTxStatusResponse();

    PayloadCommandQueue payload1CommandQueue = PayloadCommandQueue();
    uint8_t latestPayload1Packet[PAYLOAD_MAX_PACKAGE_LENGTH];
    PayloadCommandQueue payload2CommandQueue = PayloadCommandQueue();
    uint8_t latestPayload2Packet[PAYLOAD_MAX_PACKAGE_LENGTH];
    TelemetryPacketQueue telemetryPacketQueue = TelemetryPacketQueue();

    void (*setContainerTelemetryActivated)(bool telemetryActivated);
    void (*setLatestSimulationPressureValue)(float pressureVal);
    void (*setContainerSimulationMode)(int newSimulationMode);
    void (*setRtcTimeFromCommandPacket)(uint8_t* packetData, uint8_t packetLength);
    uint8_t (*generateMissionTimeString)();

    void ContainerCommunicationModule::init(Stream& groundXBeeSerial, Stream& payloadsXBeeSerial);

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
