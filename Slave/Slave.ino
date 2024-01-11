#include <Wire.h>
#include <Servo.h>
#include <Key.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

Servo myservo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int IndoorLEDPin = 5; // D5 - PVM
const int OutdoorLEDPin = 4; // D4
const int ServoMoterPin = 3; // D3

// Keypad is inspired by https://projecthub.arduino.cc/mckean0/keypad-entry-lock-2d9999#
// defining 7 pin numberes for the keypad
#define R1 12
#define R2 11
#define R3 10
#define R4 9
#define C1 8
#define C2 7
#define C3 6

// defining the keypad dimensions and characters
const byte KEYPADROWS = 4;
const byte KEYPADCOLS = 3;
char keys[KEYPADROWS][KEYPADCOLS] = {
{'1','2','3'},
{'4','5','6'},
{'7','8','9'},
{'*','0','#'} //left is clear all, right is enter
};
// Defining the keypad, we need 7 pins in total
byte rowPins[KEYPADROWS] = {R1, R2, R3, R4};
byte colPins[KEYPADCOLS] = {C1, C2, C3};
Keypad keypad_door = Keypad(makeKeymap(keys), rowPins, colPins, KEYPADROWS, KEYPADCOLS);

char inputPassword[4] = { 0, 0, 0 };  // placeholder for password.
int cursorPosition = 0;
char truePassword[4] = { '9', '1', '1' };
char* names[6] = { "JJ", "AW", "JK", "CT", "CP" };

int positionCheck = 0; // initial servo position
int openLock = 0; // skal synces med positionChechk

void setup() {
  Serial.begin(115200);
  Wire.begin(11);
  Wire.onReceive(reader);
  //Wire.onRequest(request);

  pinMode(IndoorLEDPin, OUTPUT);
  pinMode(OutdoorLEDPin, OUTPUT);
  // keypad pins
  pinMode(R1, INPUT);
  pinMode(R2, INPUT);
  pinMode(R3, INPUT);
  pinMode(R4, INPUT);
  pinMode(C1, INPUT);
  pinMode(C2, INPUT);
  pinMode(C3, INPUT);

  myservo.attach(ServoMoterPin);
  myservo.write(-90); // turns servo to the right position

  lcd.init();
  lcd.backlight();
  lcd.clear();
}
int test = 0;
void loop() {
  //checkPassword(truePassword);
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
        Serial.println(strMessage[0]);
        Serial.println((int)strMessage[1]);
        break;

      case 'd':
        Serial.println("test");
        Serial.println("OwO");

        delay(100);
        checkPassword(truePassword);

        break;

      case 'e':
        
        break;
    }
        Serial.println("OwO");

  // INSERT SWITCHCASE FOR EACH FUNCTION
}




int lockChange(int lock){
  //lock == 0 open
  //lock == 1 locked

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



void checkPassword(char* truePassword) {
        Serial.println("OwO");

  //lcd.setCursor(0, 0);
  //lcd.print("Enter password: ");
  int key = keypad_door.getKey();  // the pressed button is saved in 'key'
  
  if (key) {           // when a button is pressed
    if (key == '*') {  // delete all button
      cursorPosition = 0;
      //lcd.clear();
      memset(inputPassword, 0, sizeof(inputPassword));           // reset password
    } else if (key == '#') {                           // when # button is pressed
      if (cursorPosition == 3) {                       // when # button is pressed as botton number 4 check if password is true
        if (memcmp(inputPassword, truePassword, 4) == 0) {  // password is true
          //lcd.setCursor(0, 1);
          //lcd.print("Welcome home ");
          //lcd.print(names[1]);
          //return 1;
        } else {  // entered password was wrong
          //lcd.setCursor(0, 1);
          //lcd.print("Wrong password");
          //return 0;
        }
        cursorPosition = 0;                     // Reset position
        memset(inputPassword, 0, sizeof(inputPassword));  // Reset password
      } else if (cursorPosition > 3) {          //if more than 3 buttons are pressed the password is wrong
        //lcd.setCursor(0, 1);
        //lcd.print("Wrong password");
        //return 0;
      } else {  // when there is not enough numbers entered in the code
        //lcd.setCursor(0, 1);
        //lcd.print("Complete the Pin!");
        cursorPosition = 0;                     // Reset position
        memset(inputPassword, 0, sizeof(inputPassword));  // Reset password
      }
    } else {  // add number to array and print on lcd at the same time
      inputPassword[cursorPosition] = key;
      cursorPosition++;
      //lcd.setCursor(0, 1);
      char enteredPassword[3] = { inputPassword[0], inputPassword[1], inputPassword[2] };
      //lcd.print(enteredPassword);
    }
  }
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


