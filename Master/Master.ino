#include <Wire.h>

const int lightSensorPin = A0;
const int motionSensorIndoorPin = D0;
const int motionSensorOutdoorPin = D3;



int lightThreshold = 100; //A threshold that controls when light level is low
int position = 0; // position of the servo / lock

char toSend[3] = {0,0}; // the char defining a command to send to slave

void setup() {
  Serial.begin(115200);
  Wire.begin(D2,D1); // SDA, SCL

  pinMode(lightSensorPin, INPUT_PULLUP); // light sensor
  pinMode(motionSensorIndoorPin, INPUT);
  pinMode(motionSensorOutdoorPin, INPUT);
}

void loop() {
  char toSend[3] = {0,0};

  // Check lighting
  bool motionSensedIndoor = motionSensed(motionSensorIndoorPin);
  bool motionSensedOutdoor = motionSensed(motionSensorOutdoorPin);
  //lightsystemIndoor(position, motionSensedIndoor);
  //lightsystemOutdoor(motionSensedOutdoor);



  while (Serial.available() >0){
    int servoPosistion = Serial.parseInt();
    toSend[0] = 'd';
    toSend[1] = 0;
    sendMessage(toSend);
    Serial.println(toSend[0]);
    Serial.println((int)toSend[1]);

  }
  /*
  while (Serial.available() >0){
    int servoPosistion = Serial.parseInt();
      servoLock(servoPosistion);
  }
  */


}


void sendMessage(char toSend[]){ // transmit command to slave
  Wire.beginTransmission(11);
  Wire.write(toSend);
  delay(100);
  Wire.endTransmission();
}

void lightsystemIndoor(int position, bool motionSensed){
  int lightLevel = analogRead(lightSensorPin);

  if((lightLevel > lightThreshold) || (motionSensed==false)){ //when there is light (light level is higher than threshold), then turn off LED
    toSend[0] = 'a';
    toSend[1] = 0;
    sendMessage(toSend);
  }
  else if((position == 0) && (motionSensed==true)){ // If it is dark (light level is less than threshold) turn on LED an amout dependet on how dark it is.
    int light = 255 - lightLevel * 254 / lightThreshold; // SKAL SKALERES YDERLIGERE
    toSend[0] = 'a';
    toSend[1] = light; // light
    sendMessage(toSend);
  }
}

void lightsystemOutdoor(bool motionSensed){
  int lightLevel = analogRead(lightSensorPin);

  if ((lightLevel > lightThreshold) || (motionSensed == false)){
    toSend[0] = 'b';
    toSend[1] = 0; // Outside LED off
    sendMessage(toSend);
  }
  else{
    toSend[0] = 'b';
    toSend[1] = 1; // Outside LED on
    sendMessage(toSend);
  }
}

bool motionSensed(int whichMotionSensor){
  bool motionSensed = digitalRead(whichMotionSensor);
  return motionSensed;
}

void servoLock(int lock){
  if (lock==1){
    toSend[0] = 'c';
    toSend[1] = 1; // Lock the servo
    sendMessage(toSend);
  }
  else{
    toSend[0] = 'c';
    toSend[1] = 0; // Unlock the servo
    sendMessage(toSend);
  }
  Serial.println(toSend[0]);
  Serial.println((int)toSend[1]);
}

