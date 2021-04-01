#include <stdint.h>
#include <Servo.h> 

#define SERVO_1_PIN 4
#define SERVO_2_PIN 5

class ElectromechanicalModule {
  Servo servo1;
  Servo servo2;
  public:

    void init(){
      servo1.attach(SERVO_1_PIN); 
      servo2.attach(SERVO_2_PIN); 
    }
    void movePayload1Servo(uint16_t degrees) {
      servo1.write(degrees);
    }
    void movePayload2Servo(uint16_t degrees) {
      servo2.write(degrees);
    }
}