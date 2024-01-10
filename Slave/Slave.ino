#include <Wire.h>

const int IndoorLEDPin = 11; // PVM
const int OutdoorLEDPin = 12;
volatile char input;

void setup() {
  Serial.begin(115200);
  Wire.begin(11);
  Wire.onReceive(reader);
  //Wire.onRequest(request);

  pinMode(IndoorLEDPin, OUTPUT);
  pinMode(OutdoorLEDPin, OUTPUT);
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
  char strMessage[20] = "";
  for(int i=0; i<howMany; i++){
    char tempChar[2] = {Wire.read()};
    strcat(strMessage,tempChar);
  }

  Serial.println(strMessage);

  switch (strtok(strMessage,"-")) {     // ensuring both upper and lower case works
      case 'insideLEDOff':
        digitalWrite(IndoorLEDPin,LOW);;  // if the input is 'a', LED a turns on
        break;

      case 'insideLEDOn':
        float light = strMessage[12]
        analogWrite(IndoorLEDPin,(light));
        break;

      case 'c':
        
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






