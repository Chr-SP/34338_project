#include <Wire.h>

const int redLed = 9;
const int greenLed = 8;
volatile char input;

void setup() {
  Serial.begin(115200);
  Wire.begin(11);
  Wire.onReceive(reader);
  Wire.onRequest(request);

  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
}

void loop() {
  delay(10);

}

void reader(){
  input = Wire.read();
  if (input=='g'){
    digitalWrite(greenLed, HIGH);
    digitalWrite(redLed, LOW);
  } else if (input=='r'){
    digitalWrite(redLed, HIGH);
    digitalWrite(greenLed, LOW);
  } else if(input=='s'){
      
  } else {
    digitalWrite(greenLed, LOW);
    digitalWrite(redLed, LOW);
  }
}

void request(){
  if (digitalRead(greenLed)){
    Wire.write("g");
  } else if (digitalRead(redLed)){
    Wire.write("r");
  } else {
    Wire.write("0");
  }
}








