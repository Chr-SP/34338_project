int BuzzerPin = 9;
const byte ledblue = 3;
const byte ledred = 5;
bool Movement = true;
int Lock ;

void setup() {
  Serial.begin(115200);
  pinMode(BuzzerPin,OUTPUT);
  pinMode(ledblue, OUTPUT);
  pinMode(ledred, OUTPUT);
}

void loop() {
  while (Serial.available() > 0){
    Lock = Serial.parseInt();
    ALARM(Lock,Movement);
    Serial.println(Lock);
  }
}

void ALARM(int Position1, bool Movement){ //If the door is locked and there is motion, then alarm starts
  if(Position1 == 1 && Movement == true){
    tone(BuzzerPin,240);
    digitalWrite(ledblue, HIGH);
    delay(200);
    digitalWrite(ledblue, LOW);
    
    delay(200);
    tone(BuzzerPin,440);
    delay(200);
    digitalWrite(ledred, HIGH);
    delay(500);
    digitalWrite(ledred, LOW);
    noTone(BuzzerPin);
  }
}