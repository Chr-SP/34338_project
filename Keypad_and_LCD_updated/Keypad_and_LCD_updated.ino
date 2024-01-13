
#include <Key.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

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
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { '*', '0', '#' }  //* is clear all, # is enter
};

// pins for keypad
byte rowPins[KEYPADROWS] = { R1, R2, R3, R4 };
byte colPins[KEYPADCOLS] = { C1, C2, C3 };

// Defining the keypad
Keypad keypad_door = Keypad(makeKeymap(keys), rowPins, colPins, KEYPADROWS, KEYPADCOLS);


char password[4] = { 0, 0, 0 };  // placement for password.
int cursorPosition = 0;
int openLock = 0;
char truePassword[4] = { '9', '1', '1' };
char* names[6] = { "JJ", "AW", "JK", "CT", "CP" };

void setup() {
  pinMode(R1, INPUT);
  pinMode(R2, INPUT);
  pinMode(R3, INPUT);
  pinMode(R4, INPUT);
  pinMode(C1, INPUT);
  pinMode(C2, INPUT);
  pinMode(C3, INPUT);

  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
}

void loop() {
  checkPassword();
}

int checkPassword() {
  lcd.setCursor(0, 0);
  lcd.print("Enter password: ");
  int key = keypad_door.getKey();  // the pressed button is saved in 'key'

  if (key) {           // when a button is pressed
    if (key == '*') {  // delete all button
      cursorPosition = 0;
      lcd.clear();
      memset(password, 0, sizeof(password));           // reset password
    } else if (key == '#') {                           // when # button is pressed
      if (cursorPosition == 3) {                       // when # button is pressed as botton number 4 check if password is true
        if (memcmp(password, truePassword, 4) == 0) {  // password is true
          lcd.setCursor(0, 1);
          lcd.print("Welcome home ");
          lcd.print(names[1]);
          return 1;
        } else {  // entered password was wrong
          lcd.setCursor(0, 1);
          lcd.print("Wrong password");
          return 0;
        }
        cursorPosition = 0;                     // Reset position
        memset(password, 0, sizeof(password));  // Reset password
      } else if (cursorPosition > 3) {          //if more than 3 buttons are pressed the password is wrong
        lcd.setCursor(0, 1);
        lcd.print("Wrong password");
        return 0;
      } else {  // when there is not enough numbers entered in the code
        lcd.setCursor(0, 1);
        lcd.print("Complete the Pin!");
        cursorPosition = 0;                     // Reset position
        memset(password, 0, sizeof(password));  // Reset password
      }
    } else {  // add number to array and print on lcd at the same time
      password[cursorPosition] = key;
      cursorPosition++;
      lcd.setCursor(0, 1);
      char enteredPassword[3] = { password[0], password[1], password[2] };
      lcd.print(enteredPassword);
    }
  }
}
