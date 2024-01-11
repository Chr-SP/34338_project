#include <Wire.h>
#include <Servo.h>
Servo myservo;


const int IndoorLEDPin = 11; // D11 - PVM
const int OutdoorLEDPin = 12; // D12
const int ServoMoterPin = 3; // D3

int positionCheck = 0; // initial servo position

void setup() {
  Serial.begin(115200);
  Wire.begin(11);
  Wire.onReceive(reader);
  //Wire.onRequest(request);

  pinMode(IndoorLEDPin, OUTPUT);
  pinMode(OutdoorLEDPin, OUTPUT);
  myservo.attach(ServoMoterPin);
  myservo.write(-90); // turns servo to the right position
}

void loop() {
  delay(10);

}
/*
char* buildMessage(int howMany){
  char strMessage[20];
  for(int i=0; i<howMany; i++){
    char tempChar = Wire.read();
    strcat(strMessage,tempChar);
  }
  return strMessage;
}
*/

void reader(int howMany){
  char strMessage[3] = {0,0};
  for(int i=0; i<howMany; i++){
    strMessage[i] = Wire.read();
  }

  // Serial.print(strMessage[0]);
  // Serial.println((int)strMessage[1]);

  switch (strMessage[0]) {     // ensuring both upper and lower case works
      case 'a': // Inside LED
        analogWrite(IndoorLEDPin,(int)strMessage[1]);  // if the input is 'a', LED a turns on
        break;

      case 'b': // Outside LED
        digitalWrite(OutdoorLEDPin,(int)strMessage[1]);
        break;

      case 'c': // Servo
        lockChange((int)strMessage[1]);
        Serial.println((int)strMessage[1]);
        break;

      case 'd':
        
        break;

      case 'e':
        
        break;
    }

  // INSERT SWITCHCASE FOR EACH FUNCTION
}

/*
void request(){
  if (digitalRead(greenLed)){
    Wire.write("g");
  } else if (digitalRead(redLed)){
    Wire.write("r");
  } else {
    Wire.write("0");
  }
}

*/

int lockChange(int lock){
  //position == 0 open
  //position == 1 locked

  if(positionCheck!=lock){ // Checks if the door is already open or locked and does not turn if the input is the same as position
    if (lock == 0){ //Opens the door.
    myservo.write(-180);
    positionCheck = 0;
    }
    if(lock == 1){ //Lockes the door.
      myservo.write(180);
      positionCheck = 1;
    }
  }
  return lock;
}





