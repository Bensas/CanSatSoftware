#include "SensorModule.h"

//Adafruit_BMP280 bmp280(BMP_CS, BMP_SDI, BMP_SDO,  BMP_SCK);
SoftwareSerial gpsSerial(GPS_RX, GPS_TX);

void SensorModule::init() {
  // BMP280 SETUP
//  if (!bmp280.begin()) {
//    Serial.write("Could not find a valid BMP280 sensor, check wiring!");
////    while (1);
//  }
//  bmp280.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
//            Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
//            Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
//            Adafruit_BMP280::FILTER_X16,      /* Filtering. */
//            Adafruit_BMP280::STANDBY_MS_500);
//  bmpBasePressure = bmp280.readPressure();

  // GPS SETUP
  gpsSerial.begin(GPS_BAUD_RATE);
}

void SensorModule::loop() {
//  while (gpsSerial.available() > 0)
//    gps.encode(gpsSerial.read());
}

/*

  BMP280 FUNCTIONS

*/
float SensorModule::readAltitude() {
  //if (bmpBasePressure == -1) bmpBasePressure = bmp280.readAltitude()?
  return 700;
//  return bmp280.readAltitude(bmpBasePressure);
}

float SensorModule::readTemperature() {
  return 42;
//  return bmp280.readTemperature();
}

float SensorModule::getAltitudeFromPressure(float currentPa) {
  return currentPa;
//  if (bmpBasePressure == -1)
//    bmpBasePressure = currentPa;
//  float pressure = currentPa; // in Si units for Pascal
//  pressure /= 100;
//
//  return 44330 * (1.0 - pow(pressure / bmpBasePressure, 0.1903));
}
