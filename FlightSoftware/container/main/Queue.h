#include <stdint.h>
#define NULL 0

struct PayloadCommandQueueNode {
  uint8_t data;
  PayloadCommandQueueNode* next;
};

struct TelemetryPacketQueueNode {
  uint8_t* data;
  uint8_t dataLength;
  TelemetryPacketQueueNode* next;
};

class PayloadCommandQueue {// '0': telemetry off, '1': telemetry on
  public:
    PayloadCommandQueueNode* head; //Payload command example: CMD,SP1,ON??
    PayloadCommandQueueNode* tail;
    uint8_t length = 0;

    void add(uint8_t nextCommand);
    void removeHead();
    bool isEmpty();
};

class TelemetryPacketQueue {
  public:
  TelemetryPacketQueueNode* head; //Payload telemetry example: 2764,00:01:32,100,S1,700.2,18.2,3.2
  TelemetryPacketQueueNode* tail;
  uint8_t length = 0;

  void add(uint8_t nextPacket[], uint8_t nextPacketLength);
  void removeHead();
  bool isEmpty();
};