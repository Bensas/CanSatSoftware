#include <stdint.h>
#include <Servo.h> 

#define SERVO_PIN 7

class ElectromechanicalModule {
  Servo servo;
  public:

    void init(){
      servo.attach(SERVO_PIN); 
      Serial.println("Tuvi");
//      resetServo();
    }

    void resetServo() {
//      servo.write(90);
    }
    void releasePayload1() {
//      servo.write(137);
    }
    void releasePayload2() {
//      servo.write(43);
    }
};
