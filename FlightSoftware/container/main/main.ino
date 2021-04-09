#include "ContainerCommunicationModule.h"
#include "ElectromechanicalModule.h"
#include "SensorModule.h"
#include <stdint.h>
#include <stdio.h>
#include <Wire.h>
#include <EEPROM.h>
#include <DS3231.h>

#include <SoftwareSerial.h> 

#include <SPI.h>

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


String STATE_STRING_ARRAY[] = {"STARTUP", "PRE_DEPLOY", "PAYLOAD1DEPLOY", "PAYLOAD2DEPLOY", "LANDED"};
// Types

struct mission_time_t {
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
};
struct ContainerTelemetryPacket {
    uint16_t teamID; // 2 bytes 
    mission_time_t missionTime; // 1 sec reslution
    uint16_t packetCount;
    uint8_t packetType[2]; // 1 byte
    bool mode; // 1 byte TRUE = F, FALSE = S
    bool sp1Released;
    bool sp2Released;
    float altitude;
    float temp;
    float voltage;
    mission_time_t gpsTime; // 1 sec reslution
    double gpsLat;
    double gpsLong; //8 bytes
    float gpsAltitude; //4 bytes
    uint8_t gpsSats;
    uint8_t softwareState;
    uint16_t sp1PackageCount;
    uint16_t sp2PackageCount;
    uint8_t cmdEcho[8];
};


struct PayloadTelemetryPacket {
    uint16_t teamID; // 2 bytes
    mission_time_t missionTime; // 1 sec reslution
    uint16_t packetCount;
    uint8_t packetType[2]; 
    float spAltitude;
    float spTemp;
    float spRotationRate;
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

float latestSimulationPressureValue = 0;

//RTC DS3231 info variables
bool H12, PM, CENTURY;
uint8_t currentSec = 0;

ContainerCommunicationModule communicationModule = ContainerCommunicationModule();
ElectromechanicalModule electromechanicalModule = ElectromechanicalModule();
SensorModule sensorModule = SensorModule();

void setContainerTelemetryActivated(bool telemetryActivated) {
  sendTelemetry = telemetryActivated;
}

void setLatestSimulationPressureValue(int pressureVal) {
  latestSimulationPressureValue = pressureVal;
}

void setContainerSimulationMode(uint8_t newSimulationMode) {
  simulationMode = newSimulationMode;
}

void setRTCTime(mission_time_t time) {
  
}

ContainerTelemetryPacket createTelemetryPacket(float altitude, float temp, float voltage, mission_time_t gpsTime, double gpsLat, double gpsLong, float gpsAltitude, uint8_t gpsSats) {
  ContainerTelemetryPacket ret = {TEAM_ID, getMissionTime(rtc.getHour(H12, PM), rtc.getMinute(),rtc.getSecond()),
                                          packageCount, "C\0", 
                                          simulationMode != SIMULATION_ACTIVATED, 
                                          currentState >= STATE_PAYLOAD_1_DEPLOY, currentState >= STATE_PAYLOAD_2_DEPLOY,
                                          altitude, temp, voltage, gpsTime, gpsLat, gpsLong, gpsAltitude, gpsSats, 
                                          currentState, sp1PackageCount, sp2PackageCount, communicationModule.lastCommandEcho};
  return ret;
}

//2764,00:01:32,10,C,F,N,N,700.2,18.2,8.98,20:54:33,42.30402,34.30402,699.3,3,STARTUP,0,0,CXON
String createTelemetryPacketString(float altitude, float temp, float voltage, double gpsLat, double gpsLng, float gpsAltitude, uint8_t gpsSats) {
  String result = "";
  String separator = ",";

  char buffer[12];
  unsigned char precision = 1, voltagePrecision = 2, latLngPrecision = 5;

  result += itoa(TEAM_ID, buffer, 10);
  result += separator;
  result += getMissionTimeString(rtc.getHour(H12, PM), rtc.getMinute(),rtc.getSecond());
  result += separator;
  result += itoa(packageCount, buffer, 10);
  result += separator + "C" + separator;
  result += simulationMode == SIMULATION_ACTIVATED ? "S" : "F";
  result += separator;
  result += currentState >= STATE_PAYLOAD_1_DEPLOY ? "R" : "N";
  result += separator;
  result += currentState >= STATE_PAYLOAD_2_DEPLOY ? "R" : "N";
  result += dtostrf(altitude, 6, precision, buffer);
  result += separator;
  result += dtostrf(temp, 4, precision, buffer);
  result += separator;
  result += dtostrf(voltage, 5, voltagePrecision, buffer);
  result += separator;
  result += getMissionTimeString(sensorModule.gps.time.hour(), sensorModule.gps.time.minute(),sensorModule.gps.time.second());
  result += separator;
  result += dtostrf(gpsLat, 8, latLngPrecision, buffer);
  result += separator;
  result += dtostrf(gpsLng, 8, latLngPrecision, buffer);
  result += separator;
  result += dtostrf(gpsAltitude, 6, precision, buffer);
  result += separator;
  result += itoa(gpsSats, buffer, 10);
  result += separator;
  result += STATE_STRING_ARRAY[currentState];
  result += separator;
  result += itoa(sp1PackageCount, buffer, 10);
  result += separator;
  result += itoa(sp2PackageCount, buffer, 10);
  result += separator;
  result += communicationModule.lastCommandEcho;
  return result;
}

PayloadTelemetryPacket createPayloadTelemetryPacket(uint16_t packetCount, uint8_t packetType[2], float spAltitude, float spTemp, float spRotationRate) {
  PayloadTelemetryPacket ret = {TEAM_ID, getMissionTime(rtc.getHour(H12, PM), rtc.getMinute(), rtc.getSecond()), packetCount, packetType, spAltitude, spTemp, spRotationRate};
  return ret;
}

mission_time_t getMissionTime(uint8_t hh, uint8_t mi, uint8_t ss){
  struct mission_time_t missionTime = {
    hh,
    mi,
    ss
  };
  return missionTime;
}

uint8_t* getMissionTimeString(uint8_t hh, uint8_t mi, uint8_t ss) { // Format HH:MM:SS
  uint8_t result[8];
  itoa(hh, result, 10);
  result[2] = ':';
  itoa(mi, result + 3, 10);
  result[5] = ':';
  itoa(ss, result + 6, 10);
  return result;
}

void setup() {
  Serial.begin(9600);
  Wire.begin();

  gpsSerial.begin(9600);
  groundXBeeSerial.begin(9600);
  payloadsXBeeSerial.begin(9600);
  groundXBee.setSerial(groundXBeeSerial);
  payloadsXBee.setSerial(payloadsXBeeSerial);

  sensorModule.init();

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

  currentSec = rtc.getSecond();

  //If there was no state saved in EEPROM, currentState will equal STATE_STARTUP
  if(currentState == STATE_STARTUP) {
    switchToState(STATE_PRE_DEPLOY);
  } 
  else {
    switchToState(currentState);
  }
}

void loop() {
  uint8_t rtcSeconds = rtc.getSecond();
  communicationModule.loop();
  sensorModule.loop();
  switch(currentState) {
    case STATE_STARTUP:
      break;
    case STATE_PRE_DEPLOY:
      //si esto se repite en STATE_PAYLOAD_1_DEPLOY podriamos meterlo en una funcion y llamar eso directamente?
      float altitude = sensorModule.readAltitude();
      if (sendTelemetry == true && simulationMode != SIMULATION_ACTIVATED && rtcSeconds != currentSec) {
        currentSec = rtcSeconds;
        float altitude = sensorModule.readAltitude();
        float voltage = analogRead(A0) * (5.0 / 1023.0);

      }
      if (altitude < 500) {
        switchToState(STATE_PAYLOAD_1_DEPLOY);
      }
      break;
    case STATE_PAYLOAD_1_DEPLOY:
      //take all sensor measurements
      if (sendTelemetry == true && simulationMode == SIMULATION_ACTIVATED && rtcSeconds != currentSec) {
        currentSec = rtcSeconds;
        float temperature = sensorModule.readTemperature();
        float altitude = sensorModule.readAltitude();
        
        int voltageSensorValue = analogRead(A0);
        float voltage = voltageSensorValue * (5.0 / 1023.0); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
        
        double gpsLat = sensorModule.gps.location.lat();
        double gpsLng = sensorModule.gps.location.lng();
        double gpsAltitude = sensorModule.gps.altitude.meters();
        uint32_t gpsSatellites = sensorModule.gps.satellites.value();

        communicationModule.telemetryPacketQueue.add(createTelemetryPacketString())

        communicationModule.stringifyAndEnqueueTelemetryPacketToGround(createTelemetryPacketString(altitude, temperature, voltage, gpsLat, gpsLng, gpsAltitude, gpsSatellites));

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
