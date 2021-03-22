#include <stdlib.h>
#include <stdint.h>
#include <DS3231.h>
#include <XBee.h>

#define SIMULATION_DISABLED 0
#define SIMULATION_ENABLED 1
#define SIMULATION_ACTIVATED 2

/////    QUEUE.H START    /////
#define NULL 0

struct PayloadCommandQueueNode {
  uint8_t data;
  PayloadCommandQueueNode* next;
};

class PayloadCommandQueue {// '0': telemetry off, '1': telemetry on
  public:
  PayloadCommandQueueNode* head; //Payload telemetry example: 2764,00:01:32,100,S1,700.2,18.2,3.2
  PayloadCommandQueueNode* tail;
  uint8_t length = 0;

  void add(uint8_t nextCommand) {
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

  void removeHead() {
    if (isEmpty()){
      return;
    }
    PayloadCommandQueueNode* temp = head->next;
    delete head;
    head = temp;
    length--;
  }

  bool isEmpty(){
    return length == 0;
  }
};

//Container tlemetry example 2764,00:01:32,10,C,F,N,N,700.2,18.2,8.98,20:54:33,42.30402,34.30402,699.3,3,STARTUP,0,0,CXON

struct TelemetryPacketQueueNode {
  uint8_t* data;
  uint8_t dataLength;
  TelemetryPacketQueueNode* next;
};

class TelemetryPacketQueue {
  public:
  TelemetryPacketQueueNode* head; //Payload telemetry example: 2764,00:01:32,100,S1,700.2,18.2,3.2
  TelemetryPacketQueueNode* tail;
  uint8_t length = 0;

  void add(uint8_t nextCommand[], uint8_t nextCommandLength) {
    TelemetryPacketQueueNode *tmp = new TelemetryPacketQueueNode;
    tmp->data = nextCommand;
    tmp->dataLength = nextCommandLength;
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

  void removeHead() {
    if (isEmpty()){
      return;
    }
    TelemetryPacketQueueNode* temp = head->next;
    delete head;
    head = temp;
    length--;
  }

  bool isEmpty(){
    return length == 0;
  }
};


/////    QUEUE.H END    /////


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

  void init() {
    // Start the serial port
    Serial.begin(9600);
    // Tell XBee to use Hardware Serial. It's also possible to use SoftwareSerial
    xbee.setSerial(Serial);

  }

  void setRtcTimeFromPacket(uint8_t* packetData, uint8_t packetLength) {
    //Example command: CMD,2764,ST,13:35:59
    if (packetLength != 20 || packetData[9] != 'S' || packetData[10] != 'T'){
      // console.log('Rtc time set package is invalid!');
      return;
    }
    //console.log('Parsing time package');
    uint8_t buffer[3] = {0, 0, 0};

    memcpy(buffer, &packetData[12], 2);
    rtc.setHour(atoi(buffer)); 

    memcpy(buffer, &packetData[15], 2);
    rtc.setMinute(atoi(buffer)); 

    memcpy(buffer, &packetData[18], 2);
    rtc.setSecond(atoi(buffer)); 
  }

  void parseReceivedPacket(uint8_t* packetData, uint8_t packetLength) {
    if (packetData[0] == 'C') {
      parseCommandPacket(packetData, packetLength);
    } else {
      parseTelemetryPacket(packetData, packetLength);
    }
  }

  void parseCommandPacket(uint8_t* packetData, uint8_t packetLength) {
    /*
      Examples:
      CMD,2764,CX,ON activates Container telemetry
      CMD,2764,ST,13:35:59 sets the mission time to the value given (we already parse these commmands using setRtcTimeFromPacket)
      CMD,2764,SIM,ENABLE and CMD,1000,SIM,ACTIVATE commands are required to begin simulation mode.
      CMD,2764,SP1X,ON will trigger the Container to relay a command to theScience Payload 1 to begin telemetry transmissions.
      CMD,1000,SIMP,101325 provides a simulated pressure reading to the Container (101325 Pascals = approximately sea level). Note: this command is to be used only in simulation mode.
    */
    if (packetData[9] == 'C') { //CX command
      setContainerTelemetryActivated(packetData[12] == 'O' && packetData[13] == 'N');
    }
    else if (packetData[9] == 'S' && packetData[10] == 'I') { //SIM or SIMP command
      if (packetData[12] == ',') { //SIM command
        if (packetData[13] =='D') {
          setContainerSimulationMode(SIMULATION_DISABLED);
        }
        else if (packetData[13] =='E') {
          setContainerSimulationMode(SIMULATION_ENABLED);
        }
        else {
          setContainerSimulationMode(SIMULATION_ACTIVATED);
        }
      } else { //SIMP command
        uint8_t pressureValueBuffer[8];
        uint8_t i;
        for (i = 14; i < packetLength; i++) {
          pressureValueBuffer[i-14] = packetData[i];
        }
        pressureValueBuffer[i] = 0;
        setLatestSimulationPressureValue(atoi(pressureValueBuffer));
      }
    }
    else if (packetData[9] == 'S' && packetData[10] == 'P') { //SP1X or SP2X command
      if (packetData[14] == 'O' && packetData[15] == 'N'){
        if (packetData[11] == '1') {
          payload1CommandQueue.add('1');
        } else {
          payload2CommandQueue.add('1');
        }
      } else {
        if (packetData[11] == '1') {
          payload1CommandQueue.add('0');
        } else {
          payload2CommandQueue.add('0');
        }
      }
    }
    memcpy(lastCommandEcho, packetData, packetLength);
    lastCommandEcho[packetLength] = 0;
  }

  void parseTelemetryPacket(uint8_t* packetData, uint8_t packetLength) {
    uint8_t packetDataCopy[packetLength];
    memcpy(packetDataCopy, packetData, packetLength);
    telemetryPacketQueue.add(packetDataCopy, packetLength);
  }

  void switchToNetId(uint8_t netId[]){
    atCommandRequest.setCommandValue(netId);
    xbee.send(atCommandRequest);
  }

  int receivePackets() {
    xbee.readPacket();
    if (xbee.getResponse().isAvailable()) {
      if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {    
        xbee.getResponse().getZBRxResponse(responseObj);
        if (responseObj.getOption() == ZB_PACKET_ACKNOWLEDGED) {
          return ZB_RX_RESPONSE;
        } else {
          // we got it (obviously) but sender didn't get an ACK
        }
      } else if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
        xbee.getResponse().getZBTxStatusResponse(requestStatusObj);
        return ZB_TX_STATUS_RESPONSE;
        // if (txStatus.getDeliveryStatus() == SUCCESS) {
        //   return ZB_TX_STATUS_RESPONSE;
        // } else {
        //   // We sould resend the package
        // }
      } else if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
        xbee.getResponse().getAtCommandResponse(atCommandResponse);
        return AT_COMMAND_RESPONSE;
      }
    } else if (xbee.getResponse().isError()) {
      //console.log("Error reading packet.  Error code: ");  
      //console.log(xbee.getResponse().getErrorCode());
    }
    return false;
  }

  bool checkPacketsAck() {

  }

  void sendNextPayload1Command() {
    requestObj = ZBTxRequest(payload1Address, payload1CommandQueue.head->data, sizeof(uint8_t));
    xbee.send(requestObj);
  }

  void sendNextPayload2Command() {
    requestObj = ZBTxRequest(payload2Address, payload2CommandQueue.head->data, sizeof(uint8_t));
    xbee.send(requestObj);
  }

  void sendNextTelemetryPacket(){
    requestObj = ZBTxRequest(groundAddress, telemetryPacketQueue.head->data, telemetryPacketQueue.head->dataLength);
    xbee.send(requestObj);
  }

  void loop() {
    if (currentState == STATE_RTC_SETUP) {
      if (receivePackets() == ZB_RX_RESPONSE) {
        uint8_t* packetData = responseObj.getData();
        uint8_t dataLength = responseObj.getDataLength();
        setRtcTimeFromPacket(packetData, dataLength);
        if (responseObj.getOption() == ZB_PACKET_ACKNOWLEDGED) {
          //console.log('ST command package received and acknowledged');
        } else {
          //console.log('ST command package received but sender didnt get an ack');
        }
      }
      return;
    }

    manageStateSwitching();
    switch(currentState) {
      case STATE_P1_COMMUNICATION:
        if (receivePackets() == ZB_RX_RESPONSE) { //We received  telemetry
          uint8_t* packetData = responseObj.getData();
          uint8_t dataLength = responseObj.getDataLength();
          parseReceivedPacket(packetData, dataLength);
          if (responseObj.getOption() == ZB_PACKET_ACKNOWLEDGED) {
            //console.log('ST command package received and acknowledged');
          } else {
            //console.log('ST command package received but sender didnt get an ack');
          }
        } else if (receivePackets() == ZB_TX_STATUS_RESPONSE) {
          //We received an update on a previously sent command
          if (requestStatusObj.getDeliveryStatus() == SUCCESS) {
            payload1CommandQueue.removeHead(); //We should ideally verify whether this is actually a payload 1 command (although it would be unlikely for it to be a Payload 2 command)
          } else {
            //We didn't get an ack, so we should resend, right?
            // sendNextPayload1Command();
          }
        } else if (receivePackets() == AT_COMMAND_RESPONSE) {
          if (atCommandResponse.isOk()) {
            //console.log('Successfully switched to NET ID:');
          } 
          else {
            //console.log('Failed to switch to NET ID:');
          }
        }
        break;
      case STATE_P2_COMMUNICATION:
        if (receivePackets() == ZB_RX_RESPONSE) { //We received  telemetry
          uint8_t* packetData = responseObj.getData();
          uint8_t dataLength = responseObj.getDataLength();
          parseReceivedPacket(packetData, dataLength);
          if (responseObj.getOption() == ZB_PACKET_ACKNOWLEDGED) {
            //console.log('ST command package received and acknowledged');
          } else {
            //console.log('ST command package received but sender didnt get an ack');
          }
        } else if (receivePackets() == ZB_TX_STATUS_RESPONSE) {
          //We received an update on a previously sent command
          if (requestStatusObj.getDeliveryStatus() == SUCCESS) {
            payload2CommandQueue.removeHead(); //We should ideally verify whether this is actually a payload 2 command (although it would be unlikely for it to be a Payload 1 command)
          } else {
            //We didn't get an ack, so we should resend, right?
            // sendNextPayload1Command();
          }
        }
        break;
      case STATE_GROUND_COMMUNICATION_1:
        //check own telemetry data ack / resend
        //receive ground commands
        if (receivePackets() == ZB_RX_RESPONSE) { //We received  telemetry
          uint8_t* packetData = responseObj.getData();
          uint8_t dataLength = responseObj.getDataLength();
          parseReceivedPacket(packetData, dataLength);
          if (responseObj.getOption() == ZB_PACKET_ACKNOWLEDGED) {
            //console.log('ST command package received and acknowledged');
          } else {
            //console.log('ST command package received but sender didnt get an ack');
          }
        } else if (receivePackets() == ZB_TX_STATUS_RESPONSE) {
          //We received an update on a previously sent telemetry packet
          if (requestStatusObj.getDeliveryStatus() == SUCCESS) {
            telemetryPacketQueue.removeHead(); 
          } else {
            // We didn't get an ack, so we should resend, right?
            // sendNextTelemetryPacket();
          }
        } else if (receivePackets() == AT_COMMAND_RESPONSE) {
          if (atCommandResponse.isOk()) {
            //console.log('Successfully switched to NET ID:');
          } 
          else {
            //console.log('Failed to switch to NET ID:');
          }
        }
        break;
      case STATE_GROUND_COMMUNICATION_2:
        //check p1 telemetry data ack / resend
        //receive ground simp data
        if (receivePackets() == ZB_TX_STATUS_RESPONSE) {
          //We received an update on a previously sent telemetry packet
          if (requestStatusObj.getDeliveryStatus() == SUCCESS) {
            telemetryPacketQueue.removeHead(); 
          } else {
            // We didn't get an ack, so we should resend, right?
            // sendNextTelemetryPacket();
          }
        }
        break;
      case STATE_GROUND_COMMUNICATION_3:
        //check p2 telemetry data ack / resend
        if (receivePackets() == ZB_TX_STATUS_RESPONSE) {
          //We received an update on a previously sent telemetry packet
          if (requestStatusObj.getDeliveryStatus() == SUCCESS) {
            telemetryPacketQueue.removeHead(); 
          } else {
            // We didn't get an ack, so we should resend, right?
            // sendNextTelemetryPacket();
          }
        }
        break;
    }
}

  void switchToState(int8_t newState) {
    currentState = newState;
    switch(newState) {
      case STATE_P1_COMMUNICATION:
        if (!payload1CommandQueue.isEmpty()) {
          switchToNetId(PAYLOADS_NET_ID);
          sendNextPayload1Command();
        }
        break;
      case STATE_P2_COMMUNICATION:
        if (!payload2CommandQueue.isEmpty()) {
          sendNextPayload2Command();
        }
        break;
      case STATE_GROUND_COMMUNICATION_1:
        switchToNetId(GROUND_NET_ID);
        //Send own telemetry data
        if (!telemetryPacketQueue.isEmpty()){
          sendNextTelemetryPacket();
        }
        break;
      case STATE_GROUND_COMMUNICATION_2:
        //Send p1 telemetry data
        if (!telemetryPacketQueue.isEmpty()){
          sendNextTelemetryPacket();
        }
        break;
      case STATE_GROUND_COMMUNICATION_3:
        //Send p2 telemetry data
        if (!telemetryPacketQueue.isEmpty()){
          sendNextTelemetryPacket();
        }
        break;
    }
  }

  void manageStateSwitching() {
    uint8_t rtcSeconds = rtc.getSecond();
    if (rtcSeconds != currentSec) {
      switchToState(STATE_P1_COMMUNICATION);
      currentSec = rtcSeconds;
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
};
