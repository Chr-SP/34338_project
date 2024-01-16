#include <Wire.h>
#include <Servo.h>
#include <Key.h>
#include <Keypad.h>

Servo myservo;

const int IndoorLEDPin = 5; //PVM
const int OutdoorLEDPin = 4; //
const int ServoMoterPin = 3; //
const int motionSensorOutdoorPin = A2; // A2

const int BuzzerPin = 2;
const int ledred = 13;

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

char charToSend[4] = {0, 0, 0}; ///////////////////////////////////////////////////////////////////////////////////////
char keyTemp = NO_KEY;

int positionCheck = 0; // initial servo position

int alarm_on_off = 0; // initial alarm state (off)
int alarm_led_intensity = 0; // initial state of the blinking alarm led
unsigned long timestamp = 0;
byte tone_pitch[] = {240, 440};
int tone_select = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(11);
  Wire.onReceive(reader);
  Wire.onRequest(request);

  pinMode(IndoorLEDPin, OUTPUT);
  pinMode(OutdoorLEDPin, OUTPUT);

  pinMode(motionSensorOutdoorPin, INPUT);

  // keypad pins
  pinMode(R1, INPUT);
  pinMode(R2, INPUT);
  pinMode(R3, INPUT);
  pinMode(R4, INPUT);
  pinMode(C1, INPUT);
  pinMode(C2, INPUT);
  pinMode(C3, INPUT);

  pinMode(BuzzerPin, OUTPUT);
  pinMode(ledred, OUTPUT);
  //pinMode(ledblue, OUTPUT);

  myservo.attach(ServoMoterPin);
  myservo.write(-90); // turns servo to the right position
}
int test = 0;
void loop() {
  keyTemp = keypad_door.getKey();
  if (keyTemp != NO_KEY){
    charToSend[0] = 107; // k in ASCII
    charToSend[1] = keyTemp;
  }

  charToSend[2] = (char)digitalRead(motionSensorOutdoorPin);

  if (alarm_on_off){
    if (timestamp + 200 < millis()){
      alarm_led_intensity = !alarm_led_intensity;
      digitalWrite(ledred, alarm_led_intensity);
      tone_select = !tone_select;
      //tone(BuzzerPin,tone_pitch[tone_select]);
      timestamp = millis();
    }
  }
}

void reader(int howMany){
  char strMessage[3] = {0,0};
  for(int i=0; i<howMany; i++){
    strMessage[i] = Wire.read();
  }

  switch (strMessage[0]) {     // ensuring both upper and lower case works
    case 'a': // Inside LED
      analogWrite(IndoorLEDPin,(int)strMessage[1]);  // if the input is 'a', LED a turns on
      break;

    case 'b': // Outside LED
      digitalWrite(OutdoorLEDPin,(int)strMessage[1]);
      break;

    case 'c': // Servo
      lockChange((int)strMessage[1]);
      break;

    case 'd': // Control alarm
      alarm((int)strMessage[1]);
      break;
  }
}

void request(){
  Wire.write(charToSend[0]);
  Wire.write(charToSend[1]);
  Wire.write(charToSend[2]);
  memset(charToSend, 0, sizeof(charToSend));
}


int lockChange(int lock){
  //lock == 0 open
  //lock == 1 locked
  if(positionCheck!=lock){ // Checks if the door is already open or locked and does not turn if the input is the same as position
    if (lock == 0){ //Opens the door.
      myservo.write(-180);
      positionCheck = 0;
      Serial.println("Door opened");
      alarm(0);
    }
    if(lock == 1){ //Lockes the door.
      myservo.write(180);
      positionCheck = 1;
      Serial.println("Door locked");
    }
  }
  return lock;
}


void alarm(int on_off){ //If the door is locked and there is motion, then alarm starts
  if(on_off){
    Serial.println("alarm on");
    alarm_on_off = 1;
  } else{
    Serial.println("alarm off");
    alarm_on_off = 0;
    noTone(BuzzerPin);
    digitalWrite(ledred, LOW);
  }
}
