#include <SPI.h>
#include <Adafruit_BMP280.h>
//#include "TinyGPS++.h"
#include <SoftwareSerial.h>
#include <HardwareSerial.h>

//GPS pins
#define GPS_RX 4
#define GPS_TX 3
#define GPS_BAUD_RATE 9600

// BMP280 pins

class SensorModule {
  public:
  float bmpBasePressureHPa = -1;
//  TinyGPSPlus gps;
  
  void init();
  void loop();

  /*
  BMP280 FUNCTIONS
  */
  float readAltitude();
  float readTemperature();
  float getAltitudeFromPressure(float currentPa);

};
