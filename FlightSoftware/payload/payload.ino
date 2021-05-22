#include "PayloadCommunicationModule.h"
#include "PayloadSensorModule.h"
#include <SoftwareSerial.h>
#include <stdint.h>
#include <stdio.h>
#include <Wire.h>
#include <EEPROM.h>
#include <DS3231.h>

//External Component pins
#define XBEE_RX_PIN 3
#define XBEE_TX_PIN 4

//EEPROM Memory addressesele
#define SEND_TELEMETRY_EEPROM_ADDR 1
#define PACKET_COUNT_EEPROM_ADDR 4

#define TEAM_ID 2764

struct mission_time_t {
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
};

struct mission_time_t missionTime;

//External Components
XBee containerXBee = XBee();

// The serial connection to the GPS device
SoftwareSerial containerXBeeSerial(XBEE_RX_PIN, XBEE_TX_PIN);

//State variables
bool sendTelemetry = true;
uint8_t telPacketString[42];
uint8_t missionTimeStringBuffer[10];

//RTC DS3231 info variables
bool H12, PM, CENTURY;
uint8_t currentSec = 0;
unsigned long lastMilis = 0;

PayloadCommunicationModule communicationModule = PayloadCommunicationModule();
PayloadSensorModule sensorModule = PayloadSensorModule();

void setPayloadTelemetryActivated(bool telemetryActivated) {
  sendTelemetry = telemetryActivated;
//  EEPROM.put(PACKET_COUNT_EEPROM_ADDR, false);
//  Serial.println(sendTelemetry);
}


//2764,00:01:32,10,C,F,N,N,700.2,18.2,8.98,20:54:33,42.30402,34.30402,699.3,3,STARTUP,0,0,CXON
// 2764,20:02:03,1232,S1, 23.4,19.2,200
uint8_t* createTelemetryPacketStr(float altitude, float temp, int rpm) {   
   uint8_t buffer[12];
   unsigned char precision = 1, voltagePrecision = 2, bufferPadding;
   itoa(2764, telPacketString, 10);
   telPacketString[4] = ',';
   getMissionTimeString(0, 0, 0);
   memcpy(telPacketString + 5, &missionTimeStringBuffer, 8);
   telPacketString[13] = ',';
   
   itoa(communicationModule.packetCount, buffer, 10);
   bufferPadding =  4 - strlen(buffer);
   uint8_t i = 14;
   while (i< 14 + bufferPadding) telPacketString[i++] = '0';
   
   memcpy(telPacketString + 14 + bufferPadding, buffer, strlen(buffer));
   telPacketString[18] = ',';
   telPacketString[19] = 'S';
   telPacketString[20] = '1';
   telPacketString[21] = ',';
   
   dtostrf(altitude, 6, precision, telPacketString + 22);
   
   telPacketString[28] = ',';
   
   dtostrf(temp, 4, precision, telPacketString + 29);
   
   telPacketString[33] = ',';
   
   itoa(rpm, buffer, 10);
   bufferPadding =  4 - strlen(buffer);
   i = 34;
   while (i< 34 + bufferPadding) telPacketString[i++] = '0';
   
   memcpy(telPacketString + 34 + bufferPadding, buffer, strlen(buffer));
   telPacketString[38] = 0;
   Serial.write(telPacketString, 38);
   Serial.write('\n');
   return telPacketString;
}

void createZeroesString(uint8_t* pointer, uint8_t length) {
  int i = 0;
  while (i< length) pointer[i++] = '0';
  pointer[i] = 0;
}

void getMissionTimeString(uint8_t hh, uint8_t mi, uint8_t ss) { // Format HH:MM:SS
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
  } 
  else {
    missionTimeStringBuffer[6] = '0';
    itoa(ss, missionTimeStringBuffer+7, 10);
  }
}

void setup() {
  Serial.begin(19200);
  Wire.begin();

  containerXBeeSerial.begin(9600);
  containerXBee.setSerial(containerXBeeSerial);

  sensorModule.init();
  
  //Retrieve state variables from EEPROM
  EEPROM.get(SEND_TELEMETRY_EEPROM_ADDR, sendTelemetry);

  communicationModule.setPayloadTelemetryActivated = &setPayloadTelemetryActivated;
  communicationModule.init(containerXBee);
  currentSec = seconds();
  lastMilis = millis();
}

uint16_t seconds(){
  return millis() / 1000;
}

void takeMeasurementsAndSendTelemetry(){
  float altitude = sensorModule.readAltitude();
  float temperature = sensorModule.readTemperature();
  int rpm = sensorModule.readGyroSpeed();
  communicationModule.telemetryPacketQueue.add(createTelemetryPacketStr(altitude, temperature, rpm), 38);
}

void loop() {
  uint8_t rtcSeconds = seconds();
  unsigned long currMillis = millis();
  if (currMillis - lastMilis > 300) {
    communicationModule.loop();
    lastMilis = currMillis;
  }
  
  float altitude = sensorModule.readAltitude();
  if (sendTelemetry == true && rtcSeconds != currentSec) {
    currentSec = rtcSeconds;
    takeMeasurementsAndSendTelemetry();
  }
}
