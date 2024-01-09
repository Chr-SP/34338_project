const int motionSensorPin = 3;
const int motionSensorLedPin = 12;

void setup() {
  Serial.begin(115200);
  pinMode(motionSensorPin, INPUT);
  pinMode(motionSensorLedPin, OUTPUT);

}

void loop() {
  int value = digitalRead(motionSensorPin);
  digitalWrite(motionSensorLedPin, value);

  Serial.println(value);
  delay(100);
}
