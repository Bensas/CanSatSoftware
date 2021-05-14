#include <SPI.h>
//#include <Adafruit_BMP280.h>
//#include "TinyGPS++.h"
#include <SoftwareSerial.h>

// BMP280 pins
#define BMP_SCK 52
#define BMP_SDO 50
#define BMP_SDI 51 
#define BMP_CS 53

//GPS pins
#define GPS_RX 50
#define GPS_TX 51
#define GPS_BAUD_RATE 9600

class SensorModule {
  public:
  float bmpBasePressure = -1;
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
