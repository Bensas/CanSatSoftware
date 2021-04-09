#include <Adafruit_BMP280.h>
#include "TinyGPS++.h"

// BMP280 pins
#define BMP_SCK 52
#define BMP_SDO 50
#define BMP_SDI 51 
#define BMP_CS 53

//GPS pins
#define GPS_RX_PIN 50
#define GPS_TX_PIN 51
#define GPS_BAUD_RATE 9600

class SensorModule {
  Adafruit_BMP280 bmp(BMP_CS, BMP_SDI, BMP_SDO,  BMP_SCK);
  float bmpBasePressure;

  TinyGPSPlus gps;
  SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN);

  void init();
  void loop();

  /*
  BMP280 FUNCTIONS
  */
  float readAltitude();
  float readTemperature();
  float getAltitudeFromPressure(float currentPa);

}