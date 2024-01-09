const int motionSensorPin = 2;
const int motionLedPin = LED_BUILTIN;

void setup() {
  Serial.begin(115200);
  pinMode(motionSensorPin, INPUT);
  pinMode(motionLedPin, OUTPUT);
}

void loop() {
  
  if (motionSensed() == true){
    digitalWrite(motionLedPin, HIGH);
  }
  else if (motionSensed() == false){
    digitalWrite(motionLedPin, LOW);
  }
  
  //Serial.println(motionSensed());
  //delay(200);
}

bool motionSensed(){
  bool motionSens = digitalRead(motionSensorPin);
  return motionSens;
}
