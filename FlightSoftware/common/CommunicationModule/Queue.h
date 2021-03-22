#include <stdint.h>
// class PayloadCommandQueue;
// class TelemetryPacketQueue;
#define NULL 0

// class PayloadCommandQueue {// '0': telemetry off, '1': telemetry on
//   public:
//   uint8_t command[1]; 
//   PayloadCommandQueue* next;

//   void add(uint8_t nextCommand) {
//     if (empty()){
//       command[0] = nextCommand;
//     } else {
//       next = new PayloadCommandQueue();
//       next->command[0] = nextCommand;
//     }
//   }

//   bool empty(){
//     return command[0]  != '0' && command[0] != '1';
//   }
// };

// // class TelemetryPacketQueue {
// //   public:
// //   uint8_t* command; //Payload telemetry example: 2764,00:01:32,100,S1,700.2,18.2,3.2
// //   uint8_t commandLength;
// //   TelemetryPacketQueue* next;

// //   void add(uint8_t nextCommand[], uint8_t nextCommandLength) {
// //     if (empty()){
// //       command = nextCommand;
// //       commandLength = nextCommandLength;
// //     } else {
// //       next = new TelemetryPacketQueue();
// //       next->command = nextCommand;
// //       next->commandLength = nextCommandLength;
// //     }
// //   }

// //   bool empty(){
// //     return command == NULL;
// //   }
// // };

// //Container tlemetry example 2764,00:01:32,10,C,F,N,N,700.2,18.2,8.98,20:54:33,42.30402,34.30402,699.3,3,STARTUP,0,0,CXON

// struct QueueNode {
//   uint8_t* data;
//   uint8_t dataLength;
//   QueueNode* next;
// };

// class TelemetryPacketQueue {
//   public:
//   QueueNode* head; //Payload telemetry example: 2764,00:01:32,100,S1,700.2,18.2,3.2
//   QueueNode* tail;
//   uint8_t length = 0;

//   void add(uint8_t nextCommand[], uint8_t nextCommandLength) {
//     QueueNode *tmp = new QueueNode;
//     tmp->data = nextCommand;
//     tmp->dataLength = nextCommandLength;
//     tmp->next = NULL;
//     if(head == NULL)
//     {
//         head = tmp;
//         tail = tmp;
//     }
//     else
//     {
//         tail->next = tmp;
//         tail = tail->next;
//     }
//     length++;
//   }

//   void removeHead() {
//     if (isEmpty()){
//       return;
//     }
//     QueueNode* temp = head->next;
//     delete head;
//     head = temp;
//   }

//   bool isEmpty(){
//     return length == 0;
//   }
// };