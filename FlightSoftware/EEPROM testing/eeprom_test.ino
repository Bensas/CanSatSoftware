#include <EEPROM.h>

#define CURRENT_STATE_EEPROM_ADDR 0
#define SEND_TELEMETRY_EEPROM_ADDR 1
#define SIMULATION_MODE_EEPROM_ADDR 2
#define PACKAGE_COUNT_EEPROM_ADDR 4
#define SP1_PCOUNT_EEPROM_ADDR 6
#define SP2_PCOUNT_EEPROM_ADDR 8
#define SP1_DEPLOY_TIME_EEPROM_ADDR 10
#define SP2_DEPLOY_TIME_EEPROM_ADDR 18

uint8_t currentState = STATE_STARTUP;
bool sendTelemetry = false;
uint8_t simulationMode = SIMULATION_DISABLED;
uint16_t packageCount = 0;
uint16_t sp1PackageCount = 0;
uint16_t sp2PackageCount = 0;
time_t sp1DeployTime = 0;
time_t sp2DeployTime = 0;

unsigned long startMilis = 0;
unsigned long elapsedMillis = 0;

void setup() {
  lastMillis = millis();
  pinMode(LED_PIN, OUTPUT);
  XbeeSerial.begin(9600);
  xbee.setSerial(XbeeSerial);
  switchToNetId(GROUND_NET_ID);
}

void loop() {
  delay(1000);
  Serial.println("Fetching variables from EEPROM:");
  startMilis = millis();
  EEPROM.get(SEND_TELEMETRY_EEPROM_ADDR, sendTelemetry);
  elapsedMillis = millis() - startMilis;
  Serial.println(sendTelemetry);
  Serial.println(elapsedMillis);
  startMilis = millis();
  EEPROM.get(SIMULATION_MODE_EEPROM_ADDR, simulationMode);
  elapsedMillis = millis() - startMilis;
  Serial.println(simulationMode);
  Serial.println(elapsedMillis);
  startMilis = millis();
  EEPROM.get(PACKAGE_COUNT_EEPROM_ADDR, packageCount);
  elapsedMillis = millis() - startMilis;
  Serial.println(packageCount);
  Serial.println(elapsedMillis);
  startMilis = millis();
  EEPROM.get(SP1_PCOUNT_EEPROM_ADDR, sp1PackageCount);
  elapsedMillis = millis() - startMilis;
  Serial.println(sp1PackageCount);
  Serial.println(elapsedMillis);
  startMilis = millis();
  EEPROM.get(SP2_PCOUNT_EEPROM_ADDR, sp2PackageCount);
  elapsedMillis = millis() - startMilis;
  Serial.println(sp2PackageCount);
  Serial.println(elapsedMillis);
  startMilis = millis();
  EEPROM.get(CURRENT_STATE_EEPROM_ADDR, currentState);
  elapsedMillis = millis() - startMilis;
  Serial.println(currentState);
  Serial.println(elapsedMillis);
  startMilis = millis();
  EEPROM.get(SP1_DEPLOY_TIME_EEPROM_ADDR, sp1DeployTime);
  elapsedMillis = millis() - startMilis;
  Serial.println(sp1DeployTime);
  Serial.println(elapsedMillis);
  startMilis = millis();
  EEPROM.get(SP2_DEPLOY_TIME_EEPROM_ADDR, sp2DeployTime);
  elapsedMillis = millis() - startMilis;
  Serial.println(sp2DeployTime);
  Serial.println(elapsedMillis);
  // Serial.println("Writing variables to EEPROM");
}
