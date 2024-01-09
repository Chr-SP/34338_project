
#include <Servo.h>

Servo myservo;

const int ServoMoterPin = 9;

void setup() {
  Serial.begin(115200);
  myservo.attach(ServoMoterPin);
  myservo.write(-90); // turns it 
}

int PositionCheck = 0;

int Servomotoer(int Position){
  //position == 0 open
  //position == 1 locked

  if(PositionCheck!=Position){ // Checkes if the door is already open or locked and does not turn if the input is the same as position
    if (Position == 0){ //Opens the door.
    myservo.write(-180);
    PositionCheck = 0;
    delay(10);
    }
    if(Position == 1){ //Lockes the door.
      myservo.write(180);
      PositionCheck = 1;
      delay(10);
    }
  }
  else if(PositionCheck==Position){
  }
  return Position;
}

int ServoPosistion;

void loop() {

/*
  This code uses the Serial Monitor to open or lock the door.
  0 is open and 1 is locked.
  while (Serial.available() >0){
    ServoPosistion = Serial.parseInt();
    Servomotoer(ServoPosistion);
    delay(10);
  }
*/
}
