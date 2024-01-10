#include <Wire.h>


const int lightSensorPin = A0;
const int motionSensorIndoorPin = D3;
const int motionSensorOutdoorPin = D4;


int lightThreshold = 100; //A threshold that controls when light level is low
int position = 0;

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

  bool motionSensedIndoor = motionSensed(motionSensorIndoorPin);
  bool motionSensedOutdoor = motionSensed(motionSensorOutdoorPin);


  lightsystemIndoor(position, motionSensedIndoor);
  lightsystemOutdoor(motionSensedOutdoor);


  //sendMessage(toSend);
  delay(20);


  while (Serial.available() >0){
    int servoPosistion = Serial.parseInt();
    servoLock(servoPosistion);
    delay(10);
  }

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


void sendMessage(char toSend[]){ // transmit command to slave
  Wire.beginTransmission(11);
  Wire.write(toSend);
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
}

