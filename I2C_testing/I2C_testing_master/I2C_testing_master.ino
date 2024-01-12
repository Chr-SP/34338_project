#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

char toSend[3] = {0,0}; // the char defining a command to send to slave
char recieved[3] = {0,0};


// variables for keypad - lcd
char inputPassword[4] = { 0, 0, 0 , 0};  // placeholder for password.
int cursorPosition = 0;
char truePassword[4] = { '9', '1', '1' , 0};
int doClear = 0;
unsigned long timestamp;
//char* names[6] = { "JJ", "AW", "JK", "CT", "CP" };
int openLock = 0; // skal synces med positionChechk


void setup() {
  Serial.begin(115200);
  Wire.begin(D2,D1); // SDA, SCL

  // init LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0); //////////////////////////////////////////////////////////////
  lcd.print("Enter password: "); ////////////////////////////////////////////////////
}

void loop() {
  
  /*
  char toSend[3] = {0,0};


  while (Serial.available() >0){
    int test = Serial.parseInt();
    if (test == 0){
      toSend[0] = 'a';
      toSend[1] = 0;
    } else {
      toSend[0] = 'd';
      toSend[1] = 0;
    }    
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
  delay(100);
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

  if (recieved[0]=='L'){ ///////////////////dont ask why it's 'L' and not 'k'

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
    


    if (key) {           // when a button is pressed
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


/*
void checkPassword(char* truePassword) {
  lcd.setCursor(0, 0);
  lcd.print("Enter password: ");
  int key = keypad_door.getKey();  // the pressed button is saved in 'key'
  
  if (key) {           // when a button is pressed
    if (key == '*') {  // delete all button
      cursorPosition = 0;
      lcd.clear();
      memset(inputPassword, 0, sizeof(inputPassword));           // reset password
    } else if (key == '#') {                           // when # button is pressed
      if (cursorPosition == 3) {                       // when # button is pressed as botton number 4 check if password is true
        if (memcmp(inputPassword, truePassword, 4) == 0) {  // password is true
          lcd.setCursor(0, 1);
          lcd.print("Welcome home ");
          lcd.print(names[1]);
          //return 1;
        } else {  // entered password was wrong
          lcd.setCursor(0, 1);
          lcd.print("Wrong password");
          //return 0;
        }
        cursorPosition = 0;                     // Reset position
        memset(inputPassword, 0, sizeof(inputPassword));  // Reset password
      } else if (cursorPosition > 3) {          //if more than 3 buttons are pressed the password is wrong
        lcd.setCursor(0, 1);
        lcd.print("Wrong password");
        //return 0;
      } else {  // when there is not enough numbers entered in the code
        lcd.setCursor(0, 1);
        lcd.print("Complete the Pin!");
        cursorPosition = 0;                     // Reset position
        memset(inputPassword, 0, sizeof(inputPassword));  // Reset password
      }
    } else {  // add number to array and print on lcd at the same time
      inputPassword[cursorPosition] = key;
      cursorPosition++;
      lcd.setCursor(0, 1);
      char enteredPassword[3] = { inputPassword[0], inputPassword[1], inputPassword[2] };
      lcd.print(enteredPassword);
    }
  }
}

*/
