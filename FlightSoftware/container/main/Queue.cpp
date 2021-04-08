#include "Queue.h"

void PayloadCommandQueue::add(uint8_t nextCommand) {
  PayloadCommandQueueNode *tmp = new PayloadCommandQueueNode;
  tmp->data = nextCommand;
  tmp->next = NULL;
  if(head == NULL)
  {
      head = tmp;
      tail = tmp;
  }
  else
  {
      tail->next = tmp;
      tail = tail->next;
  }
  length++;
}
void PayloadCommandQueue::removeHead() {
  if (isEmpty()){
    return;
  }
  PayloadCommandQueueNode* temp = head->next;
  delete head;
  head = temp;
  length--;
}
bool PayloadCommandQueue::isEmpty(){
  return length == 0;
}

//Container tlemetry example 2764,00:01:32,10,C,F,N,N,700.2,18.2,8.98,20:54:33,42.30402,34.30402,699.3,3,STARTUP,0,0,CXON
void TelemetryPacketQueue::add(uint8_t nextPacket[], uint8_t nextPacketLength) {
  TelemetryPacketQueueNode *tmp = new TelemetryPacketQueueNode;
  tmp->data = nextPacket;
  tmp->dataLength = nextPacketLength;
  tmp->next = NULL;
  if(head == NULL)
  {
      head = tmp;
      tail = tmp;
  }
  else
  {
      tail->next = tmp;
      tail = tail->next;
  }
  length++;
}
void TelemetryPacketQueue::removeHead() {
  if (isEmpty()){
    return;
  }
  TelemetryPacketQueueNode* temp = head->next;
  delete head;
  head = temp;
  length--;
}
bool TelemetryPacketQueue::isEmpty(){
  return length == 0;
}