
#include <Servo.h>

Servo myservo;

const int ServoMoterPin = 9;

void setup() {
  Serial.begin(115200);
  myservo.attach(ServoMoterPin);
  myservo.write(-90); // turns it to the right position
}

int PositionCheck = 0;


int Servomotoer(int Position){
  //Position == 0 open
  //Position == 1 locked

  if(PositionCheck!=Position){
    
    if (Position == 0){
    myservo.write(-180);
    PositionCheck = 0;
    delay(10);
    }
    if(Position == 1){
      myservo.write(180);
      PositionCheck = 1;
      delay(10);
    }
  }
  return Position;
}

int ServoPosistion; //used to test code

void loop() {

  //This code uses the Serial Monitor to open or lock the door.
  //0 is open and 1 is locked.
  
  while (Serial.available() >0){
    ServoPosistion = Serial.parseInt();
    Servomotoer(ServoPosistion);
    Serial.println(ServoPosistion);
    delay(10);
  }
  
}
