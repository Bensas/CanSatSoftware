#include <Wire.h>

#include <cstdint>

#include <Servo.h> 

#include <SPI.h>
#include <Adafruit_BMP280.h>
#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)


#define STATE_STARTUP 0
#define STATE_PRE_DEPLOY 1
#define STATE_PAYLOAD_1_DEPLOY 2
#define STATE_PAYLOAD_2_DEPLOY 3
#define STATE_LANDED 4

#define SERVO_PIN

DS3231 rtc;

Adafruit_BMP280 bmp;

Servo servo;

struct ContainerTelemetryPackage {
    uint16_t teamID; // 2 bytes
    time_t missionTime; // 1 sec reslution
    uint16_t packetCount;
    uint8_t packetType[2]; // 1 byte
    bool mode; // 1 byte
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
    uint16_t cmdEcho;
};

struct PayloadTelemetryPackage {
    uint8_t payloadId;
    uint16_t teamID; // 2 bytes
    time_t missionTime; // 1 sec reslution
    uint16_t packetCount;
    uint8_t packetType[2]; 
    ufloat spAltitude;
    ufloat spTemp;
    ufloat spRotationRate;
};

const int BEACON_PIN_NUMBER = 9; //buzzer to arduino pin 9

int8_t currentState = 0;
bool sendTelemetry = false;
bool simulationEnabled = false;
bool simulationActivated = false;
int16_t packageCount = 0;
int16_t sp1PackageCount = 0;
int16_t sp2PackageCount = 0;

void setup() {
  Wire.begin();
  h12 = false;
  PM = false; // cosas del ds3231
  Century = false;

  servo.attach(SERVO_PIN); 

  pinMode(BEACON_PIN_NUMBER, OUTPUT); // Set buzzer - pin 9 as an output

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                Adafruit_BMP280::STANDBY_MS_500);

  
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
      
      checkCommands(); 
      
      if (payloadMessage == true) { // esto lo tomaria del xbee?
        payloadMessage = false;
        // sp1PackageCount++;
        // Armar package del payload con la info recibida para ground
        // Relay data
      }
      
      //take all sensor measurements
      actualTime = millis();
      if (send_telemetry == true && simulationEnabled== false && actualTime - lastTime > 1000) {
        lastTime = actualTime;
        float temperatureInCelsius = bmp.readTemperature();
        float pressure = bmp.readPressure();
        float altitude = bmp.readAltitude(1013.25);
        //packageCount++; // habria que ver si los recibe?
        // send telemetryPackage to ground
        communicationModule.addTelemetryPackageToGround()
      } else if (simulationActivated == true) {
        // if (variable de SIMP activada, recibi comando) {
        //   float temperatureInCelsius = bmp.readTemperature();
        //   float pressure = valor del SIMP;
        //   float altitude = readAltitude(1013.25, pressure);
        //   packageCount++;
        //   send telemetryPackage to ground
        // }
      }
     
      if (altitude < 400)
        switchToState(STATE_PAYLOAD_2_DEPLOY);

      break;
      
    case STATE_PAYLOAD_2_DEPLOY:
			checkCommands(); 
      if (payloadMessage == true) { // esto lo tomaria del xbee?
        payloadMessage = false;
        // ver de que payload vino
        // if (vino de 1)
        //   sp1PackageCount++;
        // else
        //   sp2PackageCount++;
        // 
        // Armar package del payload con la info recibida para ground
        // Relay data
      }
      
      //take all sensor measurements
      actualTime = millis();
      if (send_telemetry == true && simulationActivated == false && actualTime - lastTime > 1000) {
        lastTime = actualTime;
        float temperatureInCelsius = bmp.readTemperature();
        float pressure = bmp.readPressure();
        float altitude = bmp.readAltitude(1013.25);
        // me faltaria rotacion, con gps?
        packageCount++; // habria que ver si los recibe?
        // send telemetryPackage to ground
      } else if (simulationActivated == true) {
        // if (variable de SIMP activada, recibi comando) {
        //   float temperatureInCelsius = bmp.readTemperature();
        //   float pressure = valor del SIMP;
        //   float altitude = readAltitude(1013.25, pressure);
        //   packageCount++;
        //   send telemetryPackage to ground
        // }
      }
     
      if (altitude es constante) // guardar una altitud previa y ver delta?
        switchToState(STATE_LANDED);
  
      break;
      
    case STATE_LANDED:
			//tone(buzzer, 1000); // Send 1KHz sound signal...
      //delay(1000);        // Habria que hacer esto?
      //noTone(buzzer);     // Los delays son malos, no se como deberia sonar el buzzer, si es constante usamos lo del setup, sino hacemos con millis algo.

      checkCommands(); //solo deberia recibir spx off

      if (payloadMessage == true) { // esto lo tomaria del xbee?
        payloadMessage = false;
        // ver de que payload vino
        // if (vino de 1)
        //   sp1PackageCount++;
        // else
        //   sp2PackageCount++;
        // 
        // Armar package del payload con la info recibida (si es necesario) para ground
        // Relay data
      }
      
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
      servo.write(x); // Cuantos grados?
      //send command??
      break;
    case STATE_PAYLOAD_2_DEPLOY:
      servo.write(x); // Cuantos grados?
      //send command??
      break;
    case STATE_LANDED:
			sendTelemetry = false;
      tone(buzzer, 1000); // Send 1KHz sound signal...
      break;
	}
}

void checkCommands(){
  //cosas del XBEE 
  switch(command){
    case CX:
      // if (ON)
      //   sendTelemetry = true;
      // else
      //   sendTelemetry = false;
      break; 
    case ST:
      // set mission time
      break;
    case SIM: 
      //switch(mode){
      //  case DISABLE: 
      //    simulationEnabled = false;
      //    simulationActivated = false;
      //    break;
      //  case ENABLED: 
      //    simulationEnabled = true;
      //    break;
      //  case ENABLED: 
      //    simulationActivated = true;
      //    break;
      //}
      break;
    case SIMP:
      // cuando recibo comando, activar variable?
    case SPX:
      // avisar al payload correspondiente que empiece a mandar telemetria 
      break;
  }
}

float readAltitude(float seaLevelhPa, float currentPa) {
  float altitude;

  float pressure = currentPa; // in Si units for Pascal
  pressure /= 100;

  altitude = 44330 * (1.0 - pow(pressure / seaLevelhPa, 0.1903));

  return altitude;
}
