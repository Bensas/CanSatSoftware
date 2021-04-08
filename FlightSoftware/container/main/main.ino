#include "ContainerCommunicationModule.h"
#include "ElectromechanicalModule.h"
#include "SensorModule.h"
#include <stdint.h>
#include <stdio.h>
#include <Wire.h>
#include <EEPROM.h>
#include <DS3231.h>

#include <SoftwareSerial.h> 
#include "TinyGPS++.h"

#include <SPI.h>

//External Component pins

#define BEACON_PIN_NUMBER  9//buzzer to arduino pin 9

#define GPS_RX_PIN 4
#define GPS_TX_PIN 5

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

// Types

struct mission_time_t {
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
};
struct ContainerTelemetryPackage {
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

struct PayloadTelemetryPackage {
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
TinyGPSPlus gps;
XBee groundXBee = XBee();
XBee payloadsXBee = XBee();

// The serial connection to the GPS device
SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN);
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

ContainerTelemetryPackage createTelemetryPackage(float altitude, float temp, float voltage, mission_time_t gpsTime, double gpsLat, double gpsLong, float gpsAltitude, uint8_t gpsSats) {
  ContainerTelemetryPackage ret = {TEAM_ID, getMissionTime(rtc.getHour(H12, PM), rtc.getMinute(),rtc.getSecond()),
                                          packageCount, "C\0", 
                                          simulationMode != SIMULATION_ACTIVATED, 
                                          currentState >= STATE_PAYLOAD_1_DEPLOY, currentState >= STATE_PAYLOAD_2_DEPLOY,
                                          altitude, temp, voltage, gpsTime, gpsLat, gpsLong, gpsAltitude, gpsSats, 
                                          currentState, sp1PackageCount, sp2PackageCount, communicationModule.lastCommandEcho};
  return ret;
}

PayloadTelemetryPackage createPayloadTelemetryPackage(uint16_t packetCount, uint8_t packetType[2], float spAltitude, float spTemp, float spRotationRate) {
  PayloadTelemetryPackage ret = {TEAM_ID, getMissionTime(rtc.getHour(H12, PM), rtc.getMinute(), rtc.getSecond()), packetCount, packetType, spAltitude, spTemp, spRotationRate};
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
        float temperatureInCelsius = sensorModule.readTemperature();
        float altitude = sensorModule.readAltitude();
        
        int sensorValue = analogRead(A0);
        float voltage = sensorValue * (5.0 / 1023.0); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
        
        // me faltaria rotacion, con gps?
        //if (gps.time.isUpdated()) { que pasa si "no esta updated"?
          mission_time_t gpsTime = getMissionTime(gps.time.hour(), gps.time.minute(), gps.time.second());
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
