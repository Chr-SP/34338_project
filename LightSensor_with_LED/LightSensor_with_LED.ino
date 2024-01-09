
const int IndoorLEDPin = 9;
const int OutdoorLEDPin = 9;

int lightLevel;
int Light;
int LightThresholde = 20;

void setup() {
  Serial.begin(115200);
  pinMode(A0, INPUT_PULLUP);
  pinMode(OutdoorLEDPin, OUTPUT);

}


void loop() {

}

void LightsystemIn(int Position, bool Movement){
  lightLevel = analogRead(A0); 
  float voltage = lightLevel * (5.0 / 1023.0);
  int val = voltage * 100;
  Serial.println(val);

  if(val > LightThresholde){ //when there is light, then turn off LED
    digitalWrite(IndoorLEDPin,LOW);
  }
  else if(val < LightThresholde){ // If it is dark turn on LED an amout dependet on how dark it is.
   if(Position == 0 && Movement ==true){
      Light = 100-val;
      analogWrite(IndoorLEDPin,(Light));
      //Serial.println(Light);
    }
  }
}

void LightsystemOut(bool Movement){
  lightLevel = analogRead(A0); 
  float voltage = lightLevel * (5.0 / 1023.0);
  int val = voltage * 100;
  Serial.println(val);

  if(val < LightThresholde ){
    digitalWrite(OutdoorLEDPin,HIGH);
  }
  else if(val > LightThresholde && Movement == true){
    digitalWrite(OutdoorLEDPin,LOW);
  }
}