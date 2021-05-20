#include "Queue.h"
#include "CommunicationModule.h"
#include <stdlib.h>
#include <stdint.h>

#define STATE_IDLE 0
#define STATE_WAITING_FOR_CONTAINER_RESPONSE 1


class PayloadCommunicationModule : CommunicationModule {
  public:
    uint16_t packetCount;
    uint8_t containerCommunicationState = 0;

    XBee containerXBee;
    ZBRxResponse containerResponseObj = ZBRxResponse();
    ZBTxRequest containerRequestObj;
    ZBTxStatusResponse containerRequestStatusObj = ZBTxStatusResponse();

    TelemetryPacketQueue telemetryPacketQueue = TelemetryPacketQueue();

    void (*setPayloadTelemetryActivated)(bool telemetryActivated);

    void init(XBee& xbeeDevice);

    void parseReceivedPacket(uint8_t* packetData, uint8_t packetLength);
    void parseCommandPacket(uint8_t* packetData, uint8_t packetLength);

    void sendNextTelemetryPacket();

    void loop();
};
