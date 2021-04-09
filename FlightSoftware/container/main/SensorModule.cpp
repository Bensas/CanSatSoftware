#include "SensorModule.h"

void SensorModule::init() {
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

void SensorModule::loop() {
  while (gpsSerial.available() > 0)
    gps.encode(gpsSerial.read());
}

/*

  BMP280 FUNCTIONS

*/
float SensorModule::readAltitude() {
  return bmp.readAltitude(bmpBasePressure);
}

float SensorModule::readTemperature() {
  return bmp.readTemperature();
}

float SensorModule::getAltitudeFromPressure(float currentPa) {
  float altitude;

  float pressure = currentPa; // in Si units for Pascal
  pressure /= 100;

  altitude = 44330 * (1.0 - pow(pressure / bmpBasePressure, 0.1903));

  return altitude;
}