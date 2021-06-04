#include "ContainerCommunicationModule.h"
#include "ElectromechanicalModule.h"
#include "SensorModule.h"
#include <stdint.h>
#include <stdio.h>
#include <Wire.h>
#include <EEPROM.h>
#include <DS3231.h>
#include <TimerOne.h>

//External Component pins

#define BEACON_PIN_NUMBER 10//buzzer to arduino pin 9

#define GROUND_XBEE_RX_PIN 3
#define GROUND_XBEE_TX_PIN 4
#define PAYLOADS_XBEE_RX_PIN 5
#define PAYLOADS_XBEE_TX_PIN 6

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
#define CURRENT_STATE_EEPROM_ADDR 0
#define SEND_TELEMETRY_EEPROM_ADDR 1
#define SIMULATION_MODE_EEPROM_ADDR 2
#define PACKET_COUNT_EEPROM_ADDR 4
#define SP1_PCOUNT_EEPROM_ADDR 6
#define SP2_PCOUNT_EEPROM_ADDR 8
#define SP1_DEPLOY_TIME_EEPROM_ADDR 10
#define SP2_DEPLOY_TIME_EEPROM_ADDR 18
#define HAS_REACHED_APOGEE_EEPROM_ADDR 26

#define TEAM_ID 2764

#define ALTITUDE_LIST_LENGTH 6

unsigned long t=366.3,k=512;// default 1000 μs (1000 Hz), meander, pulse duration is equal to duty cycle k = 512 (50%)

static const uint8_t startupStr[10] = {'_', 'S', 'T', 'A', 'R', 'T', 'U', 'P', '_', '_'};
static const uint8_t predeploStr[10] = {'P', 'R', 'E', '_', 'D', 'E', 'P', 'L', 'O', 'Y'};
static const uint8_t p1DeployedStr[10] = {'P', '1', 'D', 'E', 'P', 'L', 'O', 'Y', 'E', 'D'};
static const uint8_t p2DeployedStr[10] = {'P', '2', 'D', 'E', 'P', 'L', 'O', 'Y', 'E', 'D'};
static const uint8_t landedStr[10] = {'_', '_', 'L', 'A', 'N', 'D', 'E', 'D', '_', '_'};

uint8_t* STATE_STRING_ARRAY[5] = {startupStr, predeploStr, p1DeployedStr, p2DeployedStr, landedStr};

struct mission_time_t {
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
};

struct mission_time_t missionTime;

//External Components
DS3231 rtc;

// The serial connection to the GPS device
SoftwareSerial groundXBeeSerial(GROUND_XBEE_RX_PIN, GROUND_XBEE_TX_PIN);
SoftwareSerial payloadsXBeeSerial(PAYLOADS_XBEE_RX_PIN, PAYLOADS_XBEE_TX_PIN);

//State variables
uint8_t currentState = STATE_STARTUP;
bool sendTelemetry = true;
uint8_t simulationMode = SIMULATION_DISABLED;
mission_time_t sp1DeployTime = {};
mission_time_t sp2DeployTime = {};
uint8_t telPacketString[137];
uint8_t missionTimeStringBuffer[10];
uint8_t gpsTimeStringBuffer[8];
bool hasReachedApogee = false;

float latestAltitudes[ALTITUDE_LIST_LENGTH];
uint8_t latestAltitudesIndex = 0;

float latestSimulationPressureValue = -1;

//RTC DS3231 info variables
bool H12, PM, CENTURY;
uint8_t currentSec = 0;
unsigned long lastMilis = 0;

ContainerCommunicationModule communicationModule = ContainerCommunicationModule();
ElectromechanicalModule electromechanicalModule = ElectromechanicalModule();
SensorModule sensorModule = SensorModule();

void setContainerTelemetryActivated(bool telemetryActivated) {
  sendTelemetry = telemetryActivated;
//  EEPROM.put(PACKAGE_COUNT_EEPROM_ADDR, false);
//  Serial.println(sendTelemetry);
}

void setLatestSimulationPressureValue(float pressureVal) {
  latestSimulationPressureValue = pressureVal;
  //Serial.write(latestSimulationPressureValue);
}

void setContainerSimulationMode(uint8_t newSimulationMode) {
  simulationMode = newSimulationMode;
  if (newSimulationMode == SIMULATION_ACTIVATED)
    sensorModule.bmpBasePressureHPa = -1;
}

void setRtcTimeFromCommandPacket(uint8_t* packetData, uint8_t packetLength) {
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


//2764,00:01:32,10,C,F,N,N,700.2,18.2,8.98,20:54:33,42.30402,34.30402,699.3,3,STARTUP,0,0,CXON
uint8_t* createTelemetryPacketStr(float altitude, float temp, float voltage, double gpsLat, double gpsLng, float gpsAltitude, uint8_t gpsSats, uint8_t gpsHH, uint8_t gpsMI, uint8_t gpsSS) {
   
   uint8_t buffer[12];
   unsigned char precision = 1, voltagePrecision = 2, latLngPrecision = 5, bufferPadding;
   
   itoa(TEAM_ID, telPacketString, 10);
   telPacketString[4] = ',';
   generateMissionTimeString();
   memcpy(telPacketString + 5, missionTimeStringBuffer, 8);
   telPacketString[13] = ',';
   itoa(communicationModule.packetCount, buffer, 10);
   bufferPadding =  4 - strlen(buffer);
   memcpy(telPacketString + 14 + bufferPadding, buffer, strlen(buffer));
   telPacketString[18] = ',';
   telPacketString[19] = 'C';
   telPacketString[20] = ',';
   telPacketString[21] = simulationMode == SIMULATION_ACTIVATED ? 'S' : 'F';
   telPacketString[22] = ',';
   telPacketString[23] = currentState >= STATE_PAYLOAD_1_DEPLOY ? 'R' : 'N';
   telPacketString[24] = ',';
   telPacketString[25] = currentState >= STATE_PAYLOAD_2_DEPLOY ? 'R' : 'N';
   telPacketString[26] = ',';
   dtostrf(altitude, 6, precision, telPacketString + 27);
   telPacketString[33] = ',';
   dtostrf(temp, 4, precision, telPacketString + 34);
   telPacketString[38] = ',';
   dtostrf(voltage, 4, voltagePrecision, telPacketString + 39);
   telPacketString[43] = ',';
   memcpy(telPacketString + 44, getGpsTimeString(gpsHH, gpsMI, gpsSS), 8);
   telPacketString[53] = ',';
   dtostrf(gpsLat, 8, latLngPrecision, telPacketString + 54);
   telPacketString[62] = ',';
   dtostrf(gpsLng, 8, latLngPrecision, telPacketString + 63);
   telPacketString[71] = ',';
   dtostrf(gpsAltitude, 6, precision, telPacketString + 72);
   telPacketString[78] = ',';
   itoa(gpsSats, buffer, 10);
   bufferPadding =  4 - strlen(buffer);
   memcpy(telPacketString + 79 + bufferPadding, buffer, strlen(buffer));
   telPacketString[83] = ',';
   memcpy(telPacketString + 84, STATE_STRING_ARRAY[currentState], 10);
   telPacketString[94] = ',';
   itoa(1280, buffer, 10); //sp1PacketCount
   bufferPadding =  4 - strlen(buffer);
   memcpy(telPacketString + 95 + bufferPadding, buffer, strlen(buffer));
   telPacketString[99] = ',';
   itoa(69, buffer, 10); //sp2PacketCount
   bufferPadding =  4 - strlen(buffer);
   memcpy(telPacketString + 100 + bufferPadding, buffer, strlen(buffer));
   telPacketString[111] = ',';
   uint8_t i = 0;
   while (communicationModule.lastCommandEcho[i] != 0) telPacketString[112+i] = communicationModule.lastCommandEcho[i++];
   telPacketString[112+i+1] = 0;
   return telPacketString;
}

mission_time_t getMissionTime(uint8_t hh, uint8_t mi, uint8_t ss){
  missionTime.hours = hh;
  missionTime.minutes = mi;
  missionTime.seconds = ss;
  return missionTime;
}

uint8_t* generateMissionTimeString(){
  return getMissionTimeString(rtc.getHour(H12, PM), rtc.getMinute(), rtc.getSecond());
//    return getMissionTimeString(0, 0, 0);

}

uint8_t* getMissionTimeString(uint8_t hh, uint8_t mi, uint8_t ss) { // Format HH:MM:SS
  if (hh >= 10){
    itoa(hh, missionTimeStringBuffer, 10);
  } else {
    missionTimeStringBuffer[0] = '0';
    itoa(hh, missionTimeStringBuffer+1, 10);
  }
  missionTimeStringBuffer[2] = ':';
  if (mi >= 10){
    itoa(mi, missionTimeStringBuffer+3, 10);
  } else {
    missionTimeStringBuffer[3] = '0';
    itoa(mi, missionTimeStringBuffer+4, 10);
  }
  missionTimeStringBuffer[5] = ':';
  if (ss >= 10){
    itoa(ss, missionTimeStringBuffer+6, 10);
  } else {
    missionTimeStringBuffer[6] = '0';
    itoa(ss, missionTimeStringBuffer+7, 10);
  }
  return missionTimeStringBuffer;
}

uint8_t* getGpsTimeString(uint8_t hh, uint8_t mi, uint8_t ss) {
  if (hh >= 10){
    itoa(hh, gpsTimeStringBuffer, 10);
  } else {
    gpsTimeStringBuffer[0] = '0';
    itoa(hh, gpsTimeStringBuffer+1, 10);
  }
  gpsTimeStringBuffer[2] = ':';
  if (mi >= 10){
    itoa(mi, gpsTimeStringBuffer+3, 10);
  } else {
    gpsTimeStringBuffer[3] = '0';
    itoa(mi, gpsTimeStringBuffer+4, 10);
  }
  gpsTimeStringBuffer[5] = ':';
  if (ss >= 10){
    itoa(ss, gpsTimeStringBuffer+6, 10);
  } else {
    gpsTimeStringBuffer[6] = '0';
    itoa(ss, gpsTimeStringBuffer+7, 10);
  }
  return gpsTimeStringBuffer;
}

void setup() {
  Serial.begin(19200);
  Wire.begin();

  groundXBeeSerial.begin(19200);
//  payloadsXBeeSerial.begin(9600);

  sensorModule.init();

  pinMode(BEACON_PIN_NUMBER, OUTPUT);
  Timer1.initialize(t); // period
  
  //Retrieve state variables from EEPROM
//  EEPROM.get(SEND_TELEMETRY_EEPROM_ADDR, sendTelemetry);
//  EEPROM.get(SIMULATION_MODE_EEPROM_ADDR, simulationMode);
//  EEPROM.get(CURRENT_STATE_EEPROM_ADDR, currentState);
//  EEPROM.get(SP1_DEPLOY_TIME_EEPROM_ADDR, sp1DeployTime);
//  EEPROM.get(SP2_DEPLOY_TIME_EEPROM_ADDR, sp2DeployTime);
//  EEPROM.get(HAS_REACHED_APOGEE_EEPROM_ADDR, hasReachedApogee);
  if (hasReachedApogee == 255) hasReachedApogee = false;  
  

  Serial.println(sendTelemetry);
  Serial.println(simulationMode);
  Serial.println(currentState);
  Serial.println(hasReachedApogee);

  communicationModule.setContainerTelemetryActivated = &setContainerTelemetryActivated;
  communicationModule.setContainerSimulationMode = &setContainerSimulationMode;
  communicationModule.setLatestSimulationPressureValue = &setLatestSimulationPressureValue;
  communicationModule.setRtcTimeFromCommandPacket = &setRtcTimeFromCommandPacket;
  communicationModule.generateMissionTimeString = &generateMissionTimeString;
  communicationModule.init(groundXBeeSerial, payloadsXBeeSerial);

//  electromechanicalModule.init();

  currentSec = seconds();
  lastMilis = millis();

  //If there was no state saved in EEPROM, currentState will equal STATE_STARTUP
  if(currentState == STATE_STARTUP || currentState == 255) {
    Serial.write("Hola");
    switchToState(STATE_PRE_DEPLOY);
  } 
  else {
    Serial.write('0' + currentState);
    switchToState(currentState);
  }
}

uint8_t seconds(){
  //return millis() / 1000;
  return rtc.getSecond();
}

void takeMeasurementsAndSendTelemetry(float altitude){
  float temperature = sensorModule.readTemperature();
  int voltageSensorValue = analogRead(A7);
  float voltage = voltageSensorValue * (10.0 / 1023.0); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  createTelemetryPacketStr
  (altitude,
                          temperature,
                          voltage,
                          0,
                         0,
                          0,
                          0,
                         0,
                         0,
                          0);
//  Serial.write(telPacketString, 137);
//  Serial.write('\n');

  communicationModule.telemetryPacketQueue.add(telPacketString, 137);
}

bool constantLowAltitude(){
  float max = latestAltitudes[0];
  float min = latestAltitudes[0];
  for (int i = 1; i < ALTITUDE_LIST_LENGTH; i++) {
    if (latestAltitudes[i] < min) min = latestAltitudes[i];
    if (latestAltitudes[i] < max) max = latestAltitudes[i];
  }
  Serial.println("Max - min:");
  Serial.println(max - min);
  Serial.println("Max:");
  Serial.println(max);
  return max - min < 5 && max < 5;
}

void loop() {
  uint8_t rtcSeconds = seconds();
  unsigned long currMillis = millis();
  if (currMillis - lastMilis > 500) {
    communicationModule.loop();
    lastMilis = currMillis;
  }
  
//  sensorModule.loop();
  float altitude = simulationMode == SIMULATION_ACTIVATED ? sensorModule.getAltitudeFromPressure(latestSimulationPressureValue) : sensorModule.readAltitude();
  if (hasReachedApogee && abs(latestAltitudes[latestAltitudesIndex] - altitude) > 95 ) { //Check for sensor glitches, if the difference in altitudes is too big, we ignore the new altitude.
     altitude = latestAltitudes[latestAltitudesIndex];
     Serial.println(altitude);
  } else {
    latestAltitudes[latestAltitudesIndex++] = altitude;
    if (latestAltitudesIndex == ALTITUDE_LIST_LENGTH) latestAltitudesIndex = 0;
  }
  
  switch(currentState) {
    case STATE_STARTUP:
      break;
    case STATE_PRE_DEPLOY:
      if (sendTelemetry == true && rtcSeconds != currentSec) {
          currentSec = rtcSeconds;
          takeMeasurementsAndSendTelemetry(altitude);
      }
      if (!hasReachedApogee && altitude > 650){
            hasReachedApogee = true;
            Serial.println("Apogee reached");
      }
      else if (hasReachedApogee && altitude < 500)
        switchToState(STATE_PAYLOAD_1_DEPLOY);
      break;
    case STATE_PAYLOAD_1_DEPLOY:
      if (sendTelemetry == true && rtcSeconds != currentSec) {
          currentSec = rtcSeconds;
          takeMeasurementsAndSendTelemetry(altitude);
      }
     
      if (altitude < 400){
        Serial.println("Switching again :P");
        Serial.println(altitude);
        switchToState(STATE_PAYLOAD_2_DEPLOY);
      }

      break;
      
    case STATE_PAYLOAD_2_DEPLOY:
      if (sendTelemetry == true && rtcSeconds != currentSec) {
          currentSec = rtcSeconds;
          takeMeasurementsAndSendTelemetry(altitude);
      }
     
      if (constantLowAltitude())
        switchToState(STATE_LANDED);
  
      break;
      
    case STATE_LANDED:
      if (rtcSeconds % 2 == 0) {
        Timer1.pwm(BEACON_PIN_NUMBER, k,t); // k - fill factor 0-1023.
      }
      break;
  }
}

void switchToState(int8_t newState) {
  currentState = newState;
  Serial.println(newState);
  EEPROM.put(CURRENT_STATE_EEPROM_ADDR, currentState);
  switch(newState) {
    case STATE_PRE_DEPLOY:
      break;
    case STATE_PAYLOAD_1_DEPLOY:
      electromechanicalModule.releasePayload1();
      if (sp2DeployTime.hours == 0 && sp2DeployTime.minutes == 0 && sp2DeployTime.seconds == 0) {
        sp1DeployTime = getMissionTime(rtc.getHour(H12, PM), rtc.getMinute(), rtc.getSecond());
        EEPROM.put(SP1_DEPLOY_TIME_EEPROM_ADDR, sp1DeployTime);
      }
      break;
    case STATE_PAYLOAD_2_DEPLOY:
      electromechanicalModule.releasePayload2();
      if (sp2DeployTime.hours == 0 && sp2DeployTime.minutes == 0 && sp2DeployTime.seconds == 0) {
        sp2DeployTime = getMissionTime(rtc.getHour(H12, PM), rtc.getMinute(), rtc.getSecond());
        EEPROM.put(SP2_DEPLOY_TIME_EEPROM_ADDR, sp2DeployTime);
      }
      break;
    case STATE_LANDED:
      sendTelemetry = false;
      break;
  }
}
