#include <Adafruit_BMP280.h>
#include <MPU9250_asukiaaa.h>

// BMP280 pins
#define BMP_280_S2C_ADDRESS 0x76


class PayloadSensorModule {
  public:
  float bmpBasePressure = -1;
  MPU9250_asukiaaa mpu9250;
  Adafruit_BMP280 bmp280;
  
  void init();
  void loop();

  /*
  BMP280 FUNCTIONS
  */
  float readAltitude();
  int readGyroSpeed();
  float readTemperature();
  float getAltitudeFromPressure(float currentPa);

};
