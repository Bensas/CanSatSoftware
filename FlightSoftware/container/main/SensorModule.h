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


  void init(){

    // BMP280 SETUP
    if (!bmp280.begin()) {
      Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
      while (1);
    }
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
              Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
              Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
              Adafruit_BMP280::FILTER_X16,      /* Filtering. */
              Adafruit_BMP280::STANDBY_MS_500);
    bmpBasePressure = bmp.readPressure();

    // GPS SETUP
    gpsSerial.begin(GPS_BAUD_RATE);
  }

  void loop() {
    while (gpsSerial.available() > 0)
      gps.encode(gpsSerial.read());
  }

  /*
  
  BMP280 FUNCTIONS

  */
  float readAltitude() {
    return bmp.readAltitude(bmpBasePressure);
  }

  float readTemperature() {
    return bmp.readTemperature();
  }

  float getAltitudeFromPressure(float currentPa) {
    float altitude;

    float pressure = currentPa; // in Si units for Pascal
    pressure /= 100;

    altitude = 44330 * (1.0 - pow(pressure / bmpBasePressure, 0.1903));

    return altitude;
  }

}