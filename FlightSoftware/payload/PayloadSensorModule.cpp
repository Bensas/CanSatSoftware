#include "PayloadSensorModule.h"


void PayloadSensorModule::init() {
  if (!bmp280.begin(BMP_280_S2C_ADDRESS)) {
    Serial.write("Could not find a valid BMP280 sensor, check wiring!");
  }
  bmp280.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
            Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
            Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
            Adafruit_BMP280::FILTER_X16,      /* Filtering. */
            Adafruit_BMP280::STANDBY_MS_500);
  bmpBasePressure = bmp280.readPressure() / 100;

  mpu9250.beginGyro();
}

/*

  BMP280 FUNCTIONS

*/
float PayloadSensorModule::readAltitude() {
  if (bmpBasePressure == -1) bmpBasePressure = bmp280.readPressure() / 100;
  return bmp280.readAltitude(bmpBasePressure);
}

float PayloadSensorModule::readTemperature() {
  return bmp280.readTemperature();
}

int PayloadSensorModule::readGyroSpeed() {
  if (mpu9250.gyroUpdate() == 0) {
    float gZ = mpu9250.gyroZ(); //In degrees per second
    float rpm = gZ / 360;
    return abs(rpm);
  }
  return 0;
}
