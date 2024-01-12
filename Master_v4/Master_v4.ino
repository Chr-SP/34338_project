#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int lightSensorPin = A0;
const int motionSensorIndoorPin = D0;
const int motionSensorOutdoorPin = D3;

int lightThreshold = 100; //A threshold that controls when light level is low
int position = 0; // position of the servo / lock

char toSend[3] = {0,0}; // the char defining a command to send to slave
char recieved[3] = {0,0};


// variables for keypad - lcd
char inputPassword[4] = { 0, 0, 0 , 0};  // placeholder for password.
char enteredPassword[4] = { inputPassword[0], inputPassword[1], inputPassword[2] };
int cursorPosition = 0;
char truePassword[4] = { '9', '1', '1' , 0};
int doClear = 0;
unsigned long timestamp;
//char* names[6] = { "JJ", "AW", "JK", "CT", "CP" };
int openLock = 0; /////////////////////// Still needed?????


void setup() {
  Serial.begin(115200);
  Wire.begin(D2,D1); // SDA, SCL

  pinMode(lightSensorPin, INPUT_PULLUP); // light sensor
  pinMode(motionSensorIndoorPin, INPUT);
  pinMode(motionSensorOutdoorPin, INPUT);

  // init LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0); //////////////////////////////////////////////////////////////
  lcd.print("Enter password: "); ////////////////////////////////////////////////////
}

void loop() {
  
  char toSend[3] = {0,0};

  // Check lighting
  bool motionSensedIndoor = motionSensed(motionSensorIndoorPin);
  bool motionSensedOutdoor = motionSensed(motionSensorOutdoorPin);
  lightsystemIndoor(position, motionSensedIndoor);
  lightsystemOutdoor(motionSensedOutdoor);

  /*
  while (Serial.available() >0){
    int test = Serial.parseInt();
    toSend[0] = 'c';
    toSend[1] = test;

    sendMessage(toSend);
    Serial.println(toSend[0]);
    Serial.println((int)toSend[1]);
  }
  */
  
  

  getMessage();
  //delay(10);
}


void sendMessage(char toSend[]){ // transmit command to slave
  Wire.beginTransmission(11);
  Wire.write(toSend);
  //delay(100);
  Wire.endTransmission();
}


void getMessage(){
  char recieved[3] = {0,0};
  Wire.requestFrom(11,2);
  int i = 0;
  while(Wire.available()){
    recieved[i] = Wire.read();
    i++;
  }


  if ((doClear == 1) && (timestamp + 2000 < millis())){ // check if clear is needed
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter password: ");
    doClear = 0;
  }

  if (recieved[0]=='k'){ ///////////////////dont ask why it's not 'k'

    /*
    if ((toClear[0] == 1) && (toClear[1] + 1000 < millis())){ // clear display when new press
      lcd.clear();
      toClear[0] = 0;
    }
    */

    Serial.println(recieved[1]);
    lcd.setCursor(0, 0);
    lcd.print("Enter password: ");
    int key = recieved[1];  // the pressed button is saved in 'key'
    //Serial.println(recieved[0]);
    //Serial.println(key);
    


    if ((key) && (doClear == 0)) {           // when a button is pressed
      if (key == '*') {  // delete all button
        cursorPosition = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter password: ");
        memset(inputPassword, 0, sizeof(inputPassword));           // reset password
      }
      else if (key == '#') {                           // when # button is pressed
        if (cursorPosition == 3) {                       // when # button is pressed as botton number 4 check if password is true
          if (memcmp(inputPassword, truePassword, 4) == 0) {  // password is true ........................................4
            lcd.setCursor(0, 1);
            lcd.print("Welcome home ");
            doClear = 1;
            timestamp = millis();
            servoLock(position); // Unlock the door
            //lcd.print(names[1]);
            //return 1;
          }
          else {  // entered password was wrong
            lcd.setCursor(0, 1);
            lcd.print("Wrong password");
            cursorPosition = 0;
            memset(inputPassword, 0, sizeof(inputPassword));  // Reset password
            doClear = 1;
            timestamp = millis();
            //return 0;
          }
          cursorPosition = 0;                     // Reset position
          memset(inputPassword, 0, sizeof(inputPassword));  // Reset password
        }
        else if (cursorPosition >= 3) {          //if more than 3 buttons are pressed the password is wrong
          lcd.setCursor(0, 1);
          lcd.print("Wrong password");
          cursorPosition = 0;
          memset(inputPassword, 0, sizeof(inputPassword));  // Reset password
          doClear = 1;
          timestamp = millis();
          //return 0;
        }
        else {  // when there is not enough numbers entered in the code
          lcd.setCursor(0, 1);
          lcd.print("Complete the Pin!");
          cursorPosition = 0;                     // Reset position
          memset(inputPassword, 0, sizeof(inputPassword));  // Reset password
          doClear = 1;
          timestamp = millis();
        }
      }
      else if(cursorPosition >= 3){
        lcd.setCursor(0, 1);
        lcd.print("Wrong password");
        cursorPosition = 0;
        memset(inputPassword, 0, sizeof(inputPassword));  // Reset password
        doClear = 1;
        timestamp = millis();
      }
      else {  // add number to array and print on lcd at the same time
        inputPassword[cursorPosition] = key;
        cursorPosition++;
        lcd.setCursor(0, 1);
        char enteredPassword[4] = { inputPassword[0], inputPassword[1], inputPassword[2] };
        lcd.print(enteredPassword);
      }
    }
  }
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
  position = !position;
  Serial.println(toSend[0]);
  Serial.println((int)toSend[1]);
}


