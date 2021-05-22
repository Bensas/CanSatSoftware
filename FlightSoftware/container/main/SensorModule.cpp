#include "SensorModule.h"

//Adafruit_BMP280 bmp280(BMP_CS, BMP_SDI, BMP_SDO,  BMP_SCK);
SoftwareSerial gpsSerial(GPS_RX, GPS_TX);

void SensorModule::init() {
  if (!bmp280.begin(BMP_280_S2C_ADDRESS)) {
    Serial.write("Could not find a valid BMP280 sensor, check wiring!");
  }
  bmp280.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
            Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
            Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
            Adafruit_BMP280::FILTER_X16,      /* Filtering. */
            Adafruit_BMP280::STANDBY_MS_500);
  bmpBasePressureHPa = bmp280.readPressure() / 100;

  // GPS SETUP
  gpsSerial.begin(GPS_BAUD_RATE);
}

void SensorModule::loop() {
// while (gpsSerial.available() > 0)
//   gps.encode(gpsSerial.read());
}

/*

  BMP280 FUNCTIONS

*/
float SensorModule::readAltitude() {
  return 843.2;
//  if (bmpBasePressureHPa == -1) bmpBasePressureHPa = bmp280.readPressure() / 100;
//  return bmp280.readAltitude(bmpBasePressureHPa);
}

float SensorModule::readTemperature() {
  return 42;
//  return bmp280.readTemperature();
}

float SensorModule::getAltitudeFromPressure(float currentPa) {
  if (bmpBasePressureHPa == -1)
    bmpBasePressureHPa = currentPa / 100;
  float pressure = currentPa; // in Si units for Pascal
  pressure /= 100;

  return 44330 * (1.0 - pow(pressure / bmpBasePressureHPa, 0.1903));
}
