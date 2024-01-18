/*
THIS IS THE SKETCH FOR THE ARDUINO UNO SLAVE IN "SMART HOME SECURITY SYSTEM" PROJECT BY GROUP 3 - COURSE NO: 34338
*/

#include <Wire.h>     // I2C for master/slave communication
#include <Servo.h>    // Include the library for servomotor control
#include <Keypad.h>   // Include a library for the keypad
#include <Key.h>      // Include a library for the keypad

Servo myservo;        // Initialize the servomotor

// The number/address of the slave
const int SLAVE = 11;

// Pin constants
const int INDOORLEDPIN = 5;
const int OUTDOORLEDPIN = 4;
const int SERVOMOTORPIN = 3;
const int MOTIONSENSOROUTDOORPIN = A2;
const int BUZZERPIN = 2;
const int ALARMLED = 13;
// Defining 7 pin numberes for the keypad - keypad is inspired by https://projecthub.arduino.cc/mckean0/keypad-entry-lock-2d9999#
const int R1 = 12;  //#define R1 12
const int R2 = 11;  //#define R2 11
const int R3 = 10;  //#define R3 10
const int R4 = 9;   //#define R4 9
const int C1 = 8;   //#define C1 8
const int C2 = 7;   //#define C2 7
const int C3 = 6;   //#define C3 6

// Defining the keypad dimensions and buttons
const byte KEYPADROWS = 4;
const byte KEYPADCOLS = 3;
char keys[KEYPADROWS][KEYPADCOLS] = {
{'1','2','3'},
{'4','5','6'},
{'7','8','9'},
{'*','0','#'}
};
// Defining the keypad buttons to the correct pins
byte rowPins[KEYPADROWS] = {R1, R2, R3, R4};
byte colPins[KEYPADCOLS] = {C1, C2, C3};
Keypad keypad_door = Keypad(makeKeymap(keys), rowPins, colPins, KEYPADROWS, KEYPADCOLS);

// Communication array for I2C between master/slave
char charToSend[4] = {0, 0, 0};

// Variable for keypress - initially no key
char keyTemp = NO_KEY;

// Defining buzzer tones for alarm
byte tone_pitch[] = {240, 440};

// Variable timestamp to control alarm function
unsigned long timestamp = 0;

// Status constants for simplicity
const int LOCKDOOR = 1;
const int OPENDOOR = 0;
const int ALARM_ON = 1;
const int ALARM_OFF = 0;
const int LIGHT_ON = 1;
const int LIGHT_OFF = 0;
const int SERVOOPEN = -180;
const int SERVOLOCK = 180;
const char INDOOR_LED = 'a';
const char OUTDOOR_LED = 'b';
const char SERVOCONTROL = 'c';
const char ALARMCONTROL = 'd';
const char KEYPADCONTROL = 107; // 'k' in ASCII

// Initial states of control variables
int lockPosition = OPENDOOR;
int alarm_on_off = ALARM_OFF;
int alarm_led_intensity = LIGHT_OFF;
int tone_select = 0;

void setup() {
  Serial.begin(115200);     // Used for debugging with serial monitor #####################################################################
  Wire.begin(SLAVE);        // Defining its own slave-address
  Wire.onReceive(reader);   // Run function 'reader' when master sends command
  Wire.onRequest(request);  // Run function 'request' when master requests data

  // Pin setup
  pinMode(INDOORLEDPIN, OUTPUT);
  pinMode(OUTDOORLEDPIN, OUTPUT);
  pinMode(MOTIONSENSOROUTDOORPIN, INPUT);
  pinMode(R1, INPUT);
  pinMode(R2, INPUT);
  pinMode(R3, INPUT);
  pinMode(R4, INPUT);
  pinMode(C1, INPUT);
  pinMode(C2, INPUT);
  pinMode(C3, INPUT);
  pinMode(BUZZERPIN, OUTPUT);
  pinMode(ALARMLED, OUTPUT);

  // Initialize servo and turn to start position (OPENDOOR)
  myservo.attach(SERVOMOTORPIN);
  myservo.write(-90); 
}


void loop() {
  keyTemp = keypad_door.getKey();                             // Save keypress
  if (keyTemp != NO_KEY){                                     // Update data to send when a key is pressed
    charToSend[0] = KEYPADCONTROL;                            // Saves a 'k' to send to let master know a new keypress is present
    charToSend[1] = keyTemp;                                  // Sends the keypress to send
  }
  charToSend[2] = (char)digitalRead(MOTIONSENSOROUTDOORPIN);  // Save outdoor motionsensor data to send
  alarm();                                                    // Controls alarm when it is active
}


// Handles commands from master (Indoor LED, Outdoor LED, Servolock, Alarm)
void reader(int howMany){
  char strMessage[3] = {0,0};       // Declares incoming command

  // Saves all incomingn data in array
  for(int i=0; i<howMany; i++){
    strMessage[i] = Wire.read();
  }

  // Switch-Case to interpret masters command as different functions to run
  switch (strMessage[0]) {
    case INDOOR_LED: // Inside LED state is updated
      analogWrite(INDOORLEDPIN,(int)strMessage[1]);
      break;

    case OUTDOOR_LED: // Outside LED state is updated
      digitalWrite(OUTDOORLEDPIN,(int)strMessage[1]);
      break;

    case SERVOCONTROL: // Servolock state is changed
      lockChange((int)strMessage[1]);
      break;

    case ALARMCONTROL: // Alarm state is changed
      alarmChange((int)strMessage[1]);
      break;
  }
}

// Sends back data to master when requested
void request(){
  Wire.write(charToSend[0]);
  Wire.write(charToSend[1]);
  Wire.write(charToSend[2]);
  memset(charToSend, 0, sizeof(charToSend));  // Resets data to send
}

// Changes the position of the servolock
void lockChange(int lock){
  if(lockPosition!=lock){           // Checks if the door is already open or locked and does not turn if the input is the same as position
    if (lock == OPENDOOR){          // Opens the door and turns off alarm
      myservo.write(SERVOOPEN);
      lockPosition = OPENDOOR;
      alarmChange(ALARM_OFF);
    }
    else if(lock == LOCKDOOR){      // Lockes the door
      myservo.write(SERVOLOCK);
      lockPosition = LOCKDOOR;
    }
  }
}

// Changes the alarm state 
void alarmChange(int on_off){
  if(on_off){
    alarm_on_off = ALARM_ON;
  } else{
    alarm_on_off = ALARM_OFF;
    noTone(BUZZERPIN);
    digitalWrite(ALARMLED, LOW);
  }
}

// Controls the buzzer and LED of the alarm
void alarm(){
  if (alarm_on_off == ALARM_ON){
    if (timestamp + 200 < millis()){              // Change state of LED and tone of buzzer every 200ms
      alarm_led_intensity = !alarm_led_intensity;
      digitalWrite(ALARMLED, alarm_led_intensity);
      tone_select = !tone_select;
      tone(BUZZERPIN,tone_pitch[tone_select]);    // ########## OUTCOMMENT THIS LINE TO KEEP SANE WHEN TESTING ########## //
      timestamp = millis();
    }
  }
}
