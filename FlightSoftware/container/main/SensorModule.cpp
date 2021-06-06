#include "SensorModule.h"

Adafruit_BMP280 bmp280;

void SensorModule::init() {
  if (!bmp280.begin(0x76)){
//    Serial.write("!BMP");
  }
  bmp280.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
            Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
            Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
            Adafruit_BMP280::FILTER_X16,      /* Filtering. */
            Adafruit_BMP280::STANDBY_MS_500);
  bmpBasePressureHPa = bmp280.readPressure() / 100;

}

void SensorModule::loop() {
// while (gpsSerial.available() > 0)
//   gps.encode(gpsSerial.read());
}

/*

  BMP280 FUNCTIONS

*/
float SensorModule::readAltitude() {
//  if (bmpBasePressureHPa == -1) bmpBasePressureHPa = bmp280.readPressure() / 100;
//  Serial.println(bmpBasePressureHPa);
  return bmp280.readAltitude(bmpBasePressureHPa);
}

float SensorModule::readTemperature() {
  return bmp280.readTemperature();
}

float SensorModule::getAltitudeFromPressure(float currentPa) {
  if (currentPa == -1) return 0;
  if (bmpBasePressureHPa == -1)
    bmpBasePressureHPa = currentPa / 100;
    
  float pressure = currentPa; // in Si units for Pascal
  pressure /= 100;
  return 44330 * (1 - pow(pressure / bmpBasePressureHPa, 0.1903));
}
