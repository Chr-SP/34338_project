#include <Wire.h>

const int lightSensorPin = A0;

int lightThreshold = 20; //A threshold that controls when light level is low


char toSend[20] = "0"; // the char defining a command to send to slave

void setup() {
  Serial.begin(115200);
  Wire.begin(D2,D1); // SDA, SCL

  pinMode(lightSensorPin, INPUT_PULLUP); // light sensor
}

void loop() {
  int position = 0;
  movementSensed = true;
  lightsystemIn(int position, bool movementSensed)



  sendMessage(toSend);
  delay(2000);


  /*
  Wire.beginTransmission(11);
  if (Serial.available() > 0) {  // checks for input
    char input = (char)tolower(Serial.read());  // typecasting the input to a char
    if (input=='a'){
      // toSend[100] = "a was send";
      Wire.write("a was sent");
    }
    // Wire.write(toSend);
  Wire.endTransmission();
  }
  */
  

  /*
  if (input == 's'){
    Wire.requestFrom(11,1);
    while(Wire.available()){
      char a = Wire.read();
      Serial.print("LED turned on: ");
      Serial.println(a);
    }
  }
  */
}


void sendMessage(char toSend[]){
  Wire.beginTransmission(11);
  Wire.write(toSend);
  Wire.endTransmission();
}


int lightLevelSensed(){ // computes reading of light sensor
  int lightLevel = analogRead(A0); //reads light level.
  float voltage = (float)lightLevel * (5.0 / 1023.0); // truns the signal into digital
  int val = (int)voltage * 100; //just a factor.
  return val;
}


void lightsystemIn(int position, bool movementSensed){
  float lightLevel = (float)lightLevelSensed();

  if(lightLevel > lightThreshold){ //when there is light (light level is higher than threshold), then turn off LED
    char toSend[20] = "insideLEDOff";
    sendMessage(char toSend[]);
  }
  else if((position == 0) && (movementSensed==true)){ // If it is dark (light level is less than threshold) turn on LED an amout dependet on how dark it is.
    char toSend[20] = "insideLEDOn";
    char light[2] = {(char)(lightLevel/500)}; // SKAL SKALERES TIL AT VÆRE MELLEM 0-1
    strcat(toSend, light);
    sendMessage(char toSend[]);
  }
}
