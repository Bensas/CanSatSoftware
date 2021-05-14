#include "ContainerCommunicationModule.h"
#include "ElectromechanicalModule.h"
#include "SensorModule.h"
#include <stdint.h>
#include <stdio.h>
#include <Wire.h>
#include <EEPROM.h>
#include <DS3231.h>

//External Component pins

#define BEACON_PIN_NUMBER  9//buzzer to arduino pin 9

#define GROUND_XBEE_RX_PIN 2
#define GROUND_XBEE_TX_PIN 3
#define PAYLOADS_XBEE_RX_PIN 4
#define PAYLOADS_XBEE_TX_PIN 5

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
#define PACKAGE_COUNT_EEPROM_ADDR 4
#define SP1_PCOUNT_EEPROM_ADDR 6
#define SP2_PCOUNT_EEPROM_ADDR 8
#define SP1_DEPLOY_TIME_EEPROM_ADDR 10
#define SP2_DEPLOY_TIME_EEPROM_ADDR 18

#define TEAM_ID 2764


uint8_t startupStr[10] = {'_', 'S', 'T', 'A', 'R', 'T', 'U', 'P', '_', '_'};
uint8_t predeploStr[10] = {'P', 'R', 'E', '_', 'D', 'E', 'P', 'L', 'O', 'Y'};
uint8_t p1DeployedStr[10] = {'P', '1', 'D', 'E', 'P', 'L', 'O', 'Y', 'E', 'D'};
uint8_t p2DeployedStr[10] = {'P', '2', 'D', 'E', 'P', 'L', 'O', 'Y', 'E', 'D'};
uint8_t landedStr[10] = {'_', '_', 'L', 'A', 'N', 'D', 'E', 'D', '_', '_'};

uint8_t* STATE_STRING_ARRAY[5] = {startupStr, predeploStr, p1DeployedStr, p2DeployedStr, landedStr};
// Types

struct mission_time_t {
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
};

//External Components
DS3231 rtc;
XBee groundXBee = XBee();
XBee payloadsXBee = XBee();

// The serial connection to the GPS device
SoftwareSerial groundXBeeSerial(GROUND_XBEE_RX_PIN, GROUND_XBEE_TX_PIN);
SoftwareSerial payloadsXBeeSerial(PAYLOADS_XBEE_RX_PIN, PAYLOADS_XBEE_TX_PIN);

//State variables
uint8_t currentState = STATE_STARTUP;
bool sendTelemetry = false;
uint8_t simulationMode = SIMULATION_DISABLED;
uint16_t packageCount = 0;
uint16_t sp1PackageCount = 0;
uint16_t sp2PackageCount = 0;
mission_time_t sp1DeployTime = {};
mission_time_t sp2DeployTime = {};
uint8_t telPacketString[146];
uint8_t missionTimeStringBuffer[8];

float latestSimulationPressureValue = 0;

//RTC DS3231 info variables
bool H12, PM, CENTURY;
uint8_t currentSec = 0;
unsigned long lastMilis = 0;

ContainerCommunicationModule communicationModule = ContainerCommunicationModule();
ElectromechanicalModule electromechanicalModule = ElectromechanicalModule();
SensorModule sensorModule = SensorModule();

void setContainerTelemetryActivated(bool telemetryActivated) {
  sendTelemetry = telemetryActivated;
//  Serial.println(sendTelemetry);
}

void setLatestSimulationPressureValue(float pressureVal) {
  latestSimulationPressureValue = pressureVal;
  //Serial.write(latestSimulationPressureValue);
}

void setContainerSimulationMode(uint8_t newSimulationMode) {
  simulationMode = newSimulationMode;
}

void setRTCTime(mission_time_t time) {
  
}


//2764,00:01:32,10,C,F,N,N,700.2,18.2,8.98,20:54:33,42.30402,34.30402,699.3,3,STARTUP,0,0,CXON
uint8_t* createTelemetryPacketStr(float altitude, float temp, float voltage, double gpsLat, double gpsLng, float gpsAltitude, uint8_t gpsSats) {
   
   uint8_t buffer[12];
   unsigned char precision = 1, voltagePrecision = 2, latLngPrecision = 5, bufferPadding;
   
   itoa(TEAM_ID, telPacketString, 10);
   telPacketString[4] = ',';
   memcpy(telPacketString + 5, getMissionTimeString(12, 2, 3), 8);
   telPacketString[13] = ',';
   itoa(100, buffer, 10);
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
   dtostrf(voltage, 5, voltagePrecision, telPacketString + 39);
   telPacketString[40] = ',';
   memcpy(telPacketString + 41, getMissionTimeString(11, 3, 23), 8);
   telPacketString[49] = ',';
   dtostrf(gpsLat, 8, latLngPrecision, telPacketString + 50);
   telPacketString[58] = ',';
   dtostrf(gpsLng, 8, latLngPrecision, telPacketString + 59);
   telPacketString[67] = ',';
   dtostrf(gpsAltitude, 6, precision, telPacketString + 68);
   telPacketString[74] = ',';
   itoa(gpsSats, buffer, 10);
   bufferPadding =  4 - strlen(buffer);
   memcpy(telPacketString + 75 + bufferPadding, buffer, strlen(buffer));
   telPacketString[79] = ',';
   memcpy(telPacketString + 80, STATE_STRING_ARRAY[currentState], 10);
   telPacketString[90] = ',';
   itoa(1280, buffer, 10); //sp1PacketCount
   bufferPadding =  4 - strlen(buffer);
   memcpy(telPacketString + 91 + bufferPadding, buffer, strlen(buffer));
   telPacketString[95] = ',';
   itoa(69, buffer, 10); //sp2PacketCount
   bufferPadding =  4 - strlen(buffer);
   memcpy(telPacketString + 96 + bufferPadding, buffer, strlen(buffer));
   telPacketString[107] = ',';
   uint8_t i = 0;
   while (communicationModule.lastCommandEcho[i] != 0) telPacketString[108+i] = communicationModule.lastCommandEcho[i++];
   telPacketString[108+i+1] = 0;
   return telPacketString;
}
//CMD2764CXON0
//           i
//CMD2764cxon0

mission_time_t getMissionTime(uint8_t hh, uint8_t mi, uint8_t ss){
  struct mission_time_t missionTime = {
    hh,
    mi,
    ss
  };
  return missionTime;
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

void setup() {
  Serial.begin(19200);
  Wire.begin();

  groundXBeeSerial.begin(9600);
//  payloadsXBeeSerial.begin(9600);
  groundXBee.setSerial(groundXBeeSerial);
//  payloadsXBee.setSerial(payloadsXBeeSerial);

//  sensorModule.init();

  pinMode(BEACON_PIN_NUMBER, OUTPUT);
  
  //Retrieve state variables from EEPROM
  EEPROM.get(SEND_TELEMETRY_EEPROM_ADDR, sendTelemetry);
  EEPROM.get(SIMULATION_MODE_EEPROM_ADDR, simulationMode);
  EEPROM.get(PACKAGE_COUNT_EEPROM_ADDR, packageCount);
  EEPROM.get(SP1_PCOUNT_EEPROM_ADDR, sp1PackageCount);
  EEPROM.get(SP2_PCOUNT_EEPROM_ADDR, sp2PackageCount);
  EEPROM.get(CURRENT_STATE_EEPROM_ADDR, currentState);
  EEPROM.get(SP1_DEPLOY_TIME_EEPROM_ADDR, sp1DeployTime);
  EEPROM.get(SP2_DEPLOY_TIME_EEPROM_ADDR, sp2DeployTime);

  communicationModule.setContainerTelemetryActivated = &setContainerTelemetryActivated;
  communicationModule.setContainerSimulationMode = &setContainerSimulationMode;
  communicationModule.setLatestSimulationPressureValue = &setLatestSimulationPressureValue;
  communicationModule.init(groundXBee, payloadsXBee, rtc);

  electromechanicalModule.init();

  currentSec = seconds();
  lastMilis = millis();

  //If there was no state saved in EEPROM, currentState will equal STATE_STARTUP
  if(currentState == STATE_STARTUP || currentState == 255) {
    Serial.write("Startup...");
    switchToState(STATE_PRE_DEPLOY);
  } 
  else {
    Serial.write('0' + currentState);
    switchToState(currentState);
  }
}

uint16_t seconds(){
  return millis() / 1000;
}

void loop() {
  uint8_t rtcSeconds = seconds();
  unsigned long currMillis = millis();
  if (currMillis - lastMilis > 500) {
    communicationModule.loop();
    lastMilis = currMillis;
  }
  
  sensorModule.loop();
  switch(currentState) {
    case STATE_STARTUP:
      break;
    case STATE_PRE_DEPLOY:
      //si esto se repite en STATE_PAYLOAD_1_DEPLOY podriamos meterlo en una funcion y llamar eso directamente?
      float altitude = simulationMode == SIMULATION_ACTIVATED ? sensorModule.getAltitudeFromPressure(latestSimulationPressureValue) : 700;
      if (sendTelemetry == true && rtcSeconds != currentSec) {
                currentSec = rtcSeconds;
        float temperature = sensorModule.readTemperature();
        int voltageSensorValue = analogRead(A0);
        float voltage = voltageSensorValue * (5.0 / 1023.0); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
//        double gpsLat = sensorModule.gps.location.lat();
//        double gpsLng = sensorModule.gps.location.lng();
//        double gpsAltitude = sensorModule.gps.altitude.meters();
//        uint32_t gpsSatellites = sensorModule.gps.satellites.value();

        communicationModule.telemetryPacketQueue.add(createTelemetryPacketStr(altitude, temperature, voltage, 7567.5, 867556.665, 704, 4), 146);

      }
//      if (altitude < 500) {
//        switchToState(STATE_PAYLOAD_1_DEPLOY);
//      }
      break;
    case STATE_PAYLOAD_1_DEPLOY:
      //take all sensor measurements
      if (sendTelemetry == true && simulationMode == SIMULATION_ACTIVATED && rtcSeconds != currentSec) {
        currentSec = rtcSeconds;
        float temperature = sensorModule.readTemperature();
        float altitude = sensorModule.readAltitude();
        
        int voltageSensorValue = analogRead(A0);
        float voltage = voltageSensorValue * (5.0 / 1023.0); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
        
//        double gpsLat = sensorModule.gps.location.lat();
//        double gpsLng = sensorModule.gps.location.lng();
//        double gpsAltitude = sensorModule.gps.altitude.meters();
//        uint32_t gpsSatellites = sensorModule.gps.satellites.value();

        communicationModule.telemetryPacketQueue.add(createTelemetryPacketStr(altitude, temperature, voltage, 345435.5, 535345.5, 703, 3), 146);

      } else if (simulationMode == SIMULATION_ACTIVATED) {
        
      }
     
      if (altitude < 400)
        switchToState(STATE_PAYLOAD_2_DEPLOY);

      break;
      
    case STATE_PAYLOAD_2_DEPLOY:
      if (sendTelemetry == true && simulationMode == SIMULATION_ACTIVATED && rtcSeconds != currentSec) {
        currentSec = rtcSeconds;
        float temperatureInCelsius = sensorModule.readTemperature();
        float altitude = sensorModule.readAltitude();
        // me faltaria rotacion, con gps?
        packageCount++; // habria que ver si los recibe?
        // send telemetryPackage to ground
      } else if (simulationMode == SIMULATION_ACTIVATED) {
        float altitude = sensorModule.getAltitudeFromPressure(latestSimulationPressureValue);
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
  EEPROM.put(CURRENT_STATE_EEPROM_ADDR, currentState);
  switch(newState) {
    case STATE_PRE_DEPLOY:
      break;
    case STATE_PAYLOAD_1_DEPLOY:
      electromechanicalModule.movePayload1Servo(180);
      if (sp2DeployTime.hours == 0 && sp2DeployTime.minutes == 0 && sp2DeployTime.seconds == 0) {
        sp1DeployTime = getMissionTime(rtc.getHour(H12, PM), rtc.getMinute(), rtc.getSecond());
        EEPROM.put(SP1_DEPLOY_TIME_EEPROM_ADDR, sp1DeployTime);
      }
      break;
    case STATE_PAYLOAD_2_DEPLOY:
      electromechanicalModule.movePayload2Servo(180);
      if (sp2DeployTime.hours == 0 && sp2DeployTime.minutes == 0 && sp2DeployTime.seconds == 0) {
        sp2DeployTime = getMissionTime(rtc.getHour(H12, PM), rtc.getMinute(), rtc.getSecond());
        EEPROM.put(SP2_DEPLOY_TIME_EEPROM_ADDR, sp2DeployTime);
      }
      break;
    case STATE_LANDED:
      sendTelemetry = false;
      tone(BEACON_PIN_NUMBER, 1000); // Send 1KHz sound signal...
      break;
  }
}
