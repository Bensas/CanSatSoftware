#include <cstdint>

#define STATE_STARTUP 0
#define STATE_PRE_DEPLOY 1
#define STATE_PAYLOAD_1_DEPLOY 2
#define STATE_PAYLOAD_2_DEPLOY 3
#define STATE_LANDED 4

int8_t currentState = 0;
bool sendTelemetry = false;
bool simulationMode = false;
int16_t packageCount = 0;
int16_t sp1PackageCount = 0;
int16_t sp2PackageCount = 0;

void setup() {
  //startupStateLogic
  //retrieve sendTelemetry from EEPROM
  //retrieve simulationMode from EEPROM
  //retrieve packageCount from EEPROM
  //retrieve sp1PackageCount from EEPROM
  //retrieve sp2PackageCount from EEPROM

  //retrieve currentState from EEPROM and switch to that state
	// switchToState(currentState);

}

void loop() {
  switch(currentState) {
    case STATE_STARTUP:
			//ejecutar stateLoop
      break;
    case STATE_PRE_DEPLOY:
			//ejecutar stateLoop
      break;
    case STATE_PAYLOAD_1_DEPLOY:
			//ejecutar stateLoop
      break;
    case STATE_PAYLOAD_2_DEPLOY:
			//ejecutar stateLoop
      break;
    case STATE_LANDED:
			//ejecutar stateLoop
      break;
	}
}

void switchToState(int8_t newState) {
  currentState = newState;
	switch(newState) {
    case STATE_PRE_DEPLOY:
			//ejecutar stateInit
      break;
    case STATE_PAYLOAD_1_DEPLOY:
			//ejecutar stateInit
      break;
    case STATE_PAYLOAD_2_DEPLOY:
			//ejecutar stateInit
      break;
    case STATE_LANDED:
			//ejecutar stateInit
      break;
	}
}