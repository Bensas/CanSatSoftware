#include <stdint.h>
#include <Servo.h> 
//#include "ServoTimer2.h"

#define SERVO_PIN A2

class ElectromechanicalModule {
  Servo servo;
//  ServoTimer2 servo;
  public:

    void init(){
      servo.attach(SERVO_PIN); 
//      Serial.println("Tuvi");
//      resetServo();
    }

    void resetServo() {
      servo.write(90);
    }
    void releasePayload1() {
      servo.write(137);
    }
    void releasePayload2() {
      servo.write(43);
    }
};
