///// COMMUNICATION MODULE /////
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


///// COMMUNICATION MODULE /////


#include <stdint.h>
#include <time.h>
//#include "CommunicationModule.h"
#include <Wire.h>
#include <EEPROM.h>
#include <DS3231.h>

#include <SoftwareSerial.h> 
#include "TinyGPS++.h"

#include <Servo.h> 

#include <SPI.h>
#include <Adafruit_BMP280.h>

//External Component pins
#define BMP_SCK  13
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS   10

#define SERVO_PIN 2

#define BEACON_PIN_NUMBER  9//buzzer to arduino pin 9

//Enums
#define STATE_STARTUP 0
#define STATE_PRE_DEPLOY 1
#define STATE_PAYLOAD_1_DEPLOY 2
#define STATE_PAYLOAD_2_DEPLOY 3
#define STATE_LANDED 4

#define SIMULATION_DISABLED 0
#define SIMULATION_ENABLED 1
#define SIMULATION_ACTIVATED 2

//EEPROM Memory addresses
#define CURRENT_STATE_EEPROM_ADD 0
#define SEND_TELEMETRY_EEPROM_ADD 1
#define SIMULATION_MODE_EEPROM_ADD 2
#define PACKAGE_COUNT_EEPROM_ADD 4
#define SP1_PCOUNT_EEPROM_ADD 6
#define SP2_PCOUNT_EEPROM_ADD 8
#define SP1_DEPLOY_TIME 10
#define SP2_DEPLOY_TIME 18

#define TEAM_ID 2764

// Types
struct ContainerTelemetryPackage {
    uint16_t teamID; // 2 bytes
    time_t missionTime; // 1 sec reslution
    uint16_t packetCount;
    uint8_t packetType[2]; // 1 byte
    bool mode; // 1 byte TRUE = F, FALSE = S
    bool sp1Released;
    bool sp2Released;
    float altitude;
    float temp;
    float voltage;
    time_t gpsTime; // 1 sec reslution
    double gpsLat;
    double gpsLong; //8 bytes
    float gpsAltitude; //4 bytes
    uint8_t gpsSats;
    uint8_t softwareState;
    uint16_t sp1PackageCount;
    uint16_t sp2PackageCount;
    uint8_t cmdEcho[8];
};

struct PayloadTelemetryPackage {
    uint16_t teamID; // 2 bytes
    time_t missionTime; // 1 sec reslution
    uint16_t packetCount;
    uint8_t packetType[2]; 
    float spAltitude;
    float spTemp;
    float spRotationRate;
};

//External Components
DS3231 rtc;
SoftwareSerial gpsSerial(3, 4);
TinyGPSPlus gps;
Adafruit_BMP280 bmp;
Servo servo;

//State variables
bool sendTelemetry = false;
uint8_t simulationMode = SIMULATION_DISABLED;
uint16_t packageCount = 0;
uint16_t sp1PackageCount = 0;
uint16_t sp2PackageCount = 0;
uint8_t currentState = STATE_STARTUP;
time_t sp1DeployTime = 0;
time_t sp2DeployTime = 0;

float latestSimulationPressureValue = 0;

unsigned long actualTime, lastTime;

//RTC DS3231 info variables
bool H12, PM, CENTURY;

ContainerCommunicationModule communicationModule = ContainerCommunicationModule();

void setContainerTelemetryActivated(bool telemetryActivated) {
  sendTelemetry = telemetryActivated;
}

void setLatestSimulationPressureValue(int pressureVal) {
  latestSimulationPressureValue = pressureVal;
}

void setContainerSimulationMode(uint8_t newSimulationMode) {
  simulationMode = newSimulationMode;
}

float readAltitude(float seaLevelhPa, float currentPa) {
  float altitude;

  float pressure = currentPa; // in Si units for Pascal
  pressure /= 100;

  altitude = 44330 * (1.0 - pow(pressure / seaLevelhPa, 0.1903));

  return altitude;
}

struct ContainerTelemetryPackage createTelemetryPackage(float altitude, float temp, float voltage, time_t gpsTime, double gpsLat, double gpsLong, float gpsAltitude, uint8_t gpsSats) {
  struct ContainerTelemetryPackage ret = {TEAM_ID, getActualUnix(rtc.getYear(), rtc.getMonth(CENTURY), 
                                          rtc.getDate(), rtc.getHour(H12, PM), rtc.getMinute(), 
                                          rtc.getSecond()), packageCount, "C\0", 
                                          simulationMode != SIMULATION_ACTIVATED, 
                                          currentState >= STATE_PAYLOAD_1_DEPLOY, currentState >= STATE_PAYLOAD_2_DEPLOY,
                                          altitude, temp, voltage, gpsTime, gpsLat, gpsLong, gpsAltitude, gpsSats, 
                                          currentState, sp1PackageCount, sp2PackageCount, communicationModule.lastCommandEcho};
  return ret;
}

struct PayloadTelemetryPackage createPayloadTelemetryPackage(uint16_t packetCount, uint8_t packetType[2], float spAltitude, float spTemp, float spRotationRate) {
  struct PayloadTelemetryPackage ret = {TEAM_ID, getActualUnix(rtc.getYear(), rtc.getMonth(CENTURY), rtc.getDate(), rtc.getHour(H12, PM), 
                                        rtc.getMinute(), rtc.getSecond()), packetCount, packetType, spAltitude, spTemp, spRotationRate};
  return ret;
}

time_t getActualUnix(uint8_t yy, uint8_t mm, uint8_t dd, uint8_t hh, uint8_t mi, uint8_t ss){
  struct tm breakdown;
  breakdown.tm_year = yy - 1900; /* years since 1900 */
  breakdown.tm_mon = mm - 1;
  breakdown.tm_mday = dd;
  breakdown.tm_hour = hh;
  breakdown.tm_min = mi;
  breakdown.tm_sec = ss;
  return mktime(&breakdown);
}

void setup() {
  Wire.begin();

  gpsSerial.begin(9600);

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                Adafruit_BMP280::STANDBY_MS_500);

  servo.attach(SERVO_PIN); 

  pinMode(BEACON_PIN_NUMBER, OUTPUT);
  
  //Retrieve state variables from EEPROM
  EEPROM.get(SEND_TELEMETRY_EEPROM_ADD, sendTelemetry);
  EEPROM.get(SIMULATION_MODE_EEPROM_ADD, simulationMode);
  EEPROM.get(PACKAGE_COUNT_EEPROM_ADD, packageCount);
  EEPROM.get(SP1_PCOUNT_EEPROM_ADD, sp1PackageCount);
  EEPROM.get(SP2_PCOUNT_EEPROM_ADD, sp2PackageCount);
  EEPROM.get(CURRENT_STATE_EEPROM_ADD, currentState);
  EEPROM.get(SP1_DEPLOY_TIME, sp1DeployTime);
  EEPROM.get(SP2_DEPLOY_TIME, sp2DeployTime);

  communicationModule.setContainerTelemetryActivated = &setContainerTelemetryActivated;
  communicationModule.setContainerSimulationMode = &setContainerSimulationMode;
  communicationModule.setLatestSimulationPressureValue = &setLatestSimulationPressureValue;

  //If there was no state saved in EEPROM, currentState will equal STATE_STARTUP
  if(currentState == STATE_STARTUP) {
    switchToState(STATE_PRE_DEPLOY);
  } 
  else {
    switchToState(currentState);
  }
}

void loop() {
  switch(currentState) {
    case STATE_STARTUP:
      //ejecutar stateLoop
      break;
    case STATE_PRE_DEPLOY:
      //si esto se repite en STATE_PAYLOAD_1_DEPLOY podriamos meterlo en una funcion y llamar eso directamente?
      float altitude = bmp.readAltitude(1013.25);
      actualTime = millis();
      if (sendTelemetry == true && simulationMode == SIMULATION_ACTIVATED && actualTime - lastTime > 1000) {
        lastTime = actualTime;
        float pressure = bmp.readPressure();
        float altitude = bmp.readAltitude(1013.25);
        int sensorValue = analogRead(A0);
        float voltage = sensorValue * (5.0 / 1023.0);
        //creo otro telemetry package?
      }
      if (altitude < 500) {
        switchToState(STATE_PAYLOAD_1_DEPLOY);
      }
      break;
    case STATE_PAYLOAD_1_DEPLOY:
            
      //take all sensor measurements
      unsigned long actualTime = millis();
      if (sendTelemetry == true && simulationMode == SIMULATION_ACTIVATED && actualTime - lastTime > 1000) {
        lastTime = actualTime;
        float temperatureInCelsius = bmp.readTemperature();
        float pressure = bmp.readPressure();
        float altitude = bmp.readAltitude(1013.25);
        
        int sensorValue = analogRead(A0);
        float voltage = sensorValue * (5.0 / 1023.0); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
        
        // me faltaria rotacion, con gps?
        //if (gps.time.isUpdated()) { que pasa si "no esta updated"?
          time_t gpsTime = getActualUnix(gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second());
        //}
        //if (gps.location.isUpdated()) { que pasa si "no esta updated"?
          double lat = gps.location.lat();
          double lng = gps.location.lng();
        //}
        //if (gps.altitude.isUpdated()) que pasa si "no esta updated"?
          double altitudeMeters = gps.altitude.meters();
        //if (gps.satellites.isUpdated()) que pasa si "no esta updated"?
          uint32_t satelites = gps.satellites.value(); // Number of satellites in use (u32)

        // send telemetryPackage to ground
      } else if (simulationMode == SIMULATION_ACTIVATED) {
        
      }
     
      if (altitude < 400)
        switchToState(STATE_PAYLOAD_2_DEPLOY);

      break;
      
    case STATE_PAYLOAD_2_DEPLOY:
      
      //take all sensor measurements
      actualTime = millis();
      if (sendTelemetry == true && simulationMode == SIMULATION_ACTIVATED && actualTime - lastTime > 1000) {
        lastTime = actualTime;
        float temperatureInCelsius = bmp.readTemperature();
        float pressure = bmp.readPressure();
        float altitude = bmp.readAltitude(1013.25);
        // me faltaria rotacion, con gps?
        packageCount++; // habria que ver si los recibe?
        // send telemetryPackage to ground
      } else if (simulationMode == SIMULATION_ACTIVATED) {
        // if (variable de SIMP activada, recibi comando) {
        //   float temperatureInCelsius = bmp.readTemperature();
        //   float pressure = valor del SIMP;
        //   float altitude = readAltitude(1013.25, pressure);
        //   packageCount++;
        //   send telemetryPackage to ground
        // }
      }
     
//      if (altitude es constante) // guardar una altitud previa y ver delta?
        switchToState(STATE_LANDED);
  
      break;
      
    case STATE_LANDED:
      //tone(buzzer, 1000); // Send 1KHz sound signal...
      //delay(1000);        // Habria que hacer esto?
      //noTone(buzzer);     // Los delays son malos, no se como deberia sonar el buzzer, si es constante usamos lo del setup, sino hacemos con millis algo.
      
      break;
  }
}

void switchToState(int8_t newState) {
  currentState = newState;
  EEPROM.put(CURRENT_STATE_EEPROM_ADD, currentState);
  switch(newState) {
    case STATE_PRE_DEPLOY:
      break;
    case STATE_PAYLOAD_1_DEPLOY:
      servo.write(180); // Cuantos grados?
      if (sp1DeployTime == 0) {
        sp1DeployTime = getActualUnix(rtc.getYear(), rtc.getMonth(CENTURY), 
                                      rtc.getDate(), rtc.getHour(H12, PM), rtc.getMinute(), 
                                      rtc.getSecond());
        EEPROM.put(SP1_DEPLOY_TIME, sp1DeployTime);
      }
      break;
    case STATE_PAYLOAD_2_DEPLOY:
      servo.write(180); // Cuantos grados?
      if (sp2DeployTime == 0) {
        sp2DeployTime = getActualUnix(rtc.getYear(), rtc.getMonth(CENTURY), 
                                      rtc.getDate(), rtc.getHour(H12, PM), rtc.getMinute(), 
                                      rtc.getSecond());
        EEPROM.put(SP2_DEPLOY_TIME, sp2DeployTime);
      }
      break;
    case STATE_LANDED:
      sendTelemetry = false;
      tone(BEACON_PIN_NUMBER, 1000); // Send 1KHz sound signal...
      break;
  }
}