#include <Wire.h>
#include <DS3231.h>

#include <SoftwareSerial.h> 
#include <TinyGPS++.h>

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

//ver direcciones en memoria de EEPROM
#define SEND_TELEMETRY_ADDR ??
#define SIMULATION_MODE_ADDR ??
#define PACKAGE_COUNT_ADDR ??
#define SP1_PCOUNT_ADDR ??
#define SP2_PCOUNT_ADDR ??
#define CURRENT_STATE_ADDR ??

#define SERVO_PIN ??

#define TEAM_ID 1111

DS3231 rtc;

SoftwareSerial gpsSerial(3,4);
TinyGPS gps;

Adafruit_BMP280 bmp;

Servo servo;

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
    uint16_t cmdEcho;
};

struct PayloadTelemetryPackage {
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
bool simulationMode = false;
bool sp1Released = false;
bool sp2Released = false;
bool preDeploy = false;
int16_t packageCount = 0;
int16_t sp1PackageCount = 0;
int16_t sp2PackageCount = 0;

void setup() {
  Wire.begin();
  h12 = false;
  PM = false; // cosas del ds3231
  Century = false;

  gpsSerial.begin(9600);

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                Adafruit_BMP280::STANDBY_MS_500);

  servo.attach(SERVO_PIN); 

  pinMode(BEACON_PIN_NUMBER, OUTPUT); // Set buzzer - pin 9 as an output
  
  //retrieve variables
  EEPROM.get(SEND_TELEMETRY_ADDR, sendTelemetry);
  EEPROM.get(SIMULATION_MODE_ADDR, simulationMode);
  EEPROM.get(PACKAGE_COUNT_ADDR, packageCount);
  EEPROM.get(SP1_PCOUNT_ADDR, sp1PackageCount);
  EEPROM.get(SP2_PCOUNT_ADDR, sp2PackageCount);
  EEPROM.get(CURRENT_STATE_ADDR, currentState);
  
  //ESTO ES NECESARIO?
  if(currentState == 0 || currentState == STATE_PRE_DEPLOY) {//en el state diagram dice si currentState = null (pero currentState es int ?) 
    //write currentState == PRE_DEPLOY en EEPROM
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
      checkCommands();
      //si esto se repite en STATE_PAYLOAD_1_DEPLOY podriamos meterlo en una funcion y llamar eso directamente?
      float altitude = bmp.readAltitude(1013.25);
      actualTime = millis();
      if (send_telemetry == true && simulationActivated == false && actualTime - lastTime > 1000) {
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
      
      checkCommands(); 
      
      if (payloadMessage == true) { // esto lo tomaria del xbee?
        payloadMessage = false;
        sp1PackageCount++; // habria que ver si los recibe?
        struct PayloadTelemetryPackage package = createPayloadTelemetryPackage(sp1PackageCoun, "S1", ufloat spAltitude, ufloat spTemp, ufloat spRotationRate)
        // Relay data
      }
      
      //take all sensor measurements
      actualTime = millis();
      if (send_telemetry == true && simulationActivated == false && actualTime - lastTime > 1000) {
        lastTime = actualTime;
        float temperatureInCelsius = bmp.readTemperature();
        float pressure = bmp.readPressure();
        float altitude = bmp.readAltitude(1013.25);
        
        int sensorValue = analogRead(A0);
        float voltage = sensorValue * (5.0 / 1023.0); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
        
        // me faltaria rotacion, con gps?
        packageCount++; // habria que ver si los recibe?
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
          u_int32 satelites = gps.satellites.value(); // Number of satellites in use (u32)


        struct ContainerTelemetryPackage package = createTelemetryPackage(altitude, temperatureInCelsius, voltage, gpsTime, lat, lng, altitudeMeters, satelites);
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
      sp1Released = true;
      servo.write(x); // Cuantos grados?
      //send command??
      break;
    case STATE_PAYLOAD_2_DEPLOY:
      sp2Released = true;
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

struct ContainerTelemetryPackage createTelemetryPackage(float altitude, float temp, float voltage, time_t gpsTime, double gpsLat, double gpsLong, float gpsAltitude, uint8_t gpsSats) {
  struct ContainerTelemetryPackage ret = {TEAM_ID, getActualUnix(Clock.getYear(), Clock.getMonth(Century), 
                                          Clock.getDate(), Clock.getHour(h12, PM), Clock.getMinute(), 
                                          Clock.getSecond()), packageCount, "C\0", 
                                          simulationEnabled == false ? true : false, 
                                          sp1Released, sp2Released, altitude, temp, voltage, 
                                          gpsTime, gpsLat, gpsLong, gpsAltitude, gpsSats, 
                                          currentState, sp1PackageCount, sp2PackageCount, ECHO};
  return ret;
}

struct PayloadTelemetryPackage createPayloadTelemetryPackage(uint16_t packetCount, uint8_t packetType[2], ufloat spAltitude, ufloat spTemp, ufloat spRotationRate) {
  struct PayloadTelemetryPackage ret = {TEAM_ID, getActualUnix(Clock.getYear(), Clock.getMonth(Century), Clock.getDate(), Clock.getHour(h12, PM), 
                                        Clock.getMinute(), Clock.getSecond()), packetCount, packetType, spAltitude, spTemp, spRotationRate};
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
