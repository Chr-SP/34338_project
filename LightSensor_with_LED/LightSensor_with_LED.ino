
const int IndoorLEDPin = 9; //
const int OutdoorLEDPin = 8;

int LightThreshold = 20; //A threshold that controls when light level is low

void setup() {
  Serial.begin(115200);
  pinMode(A0, INPUT_PULLUP);
  pinMode(IndoorLEDPin, OUTPUT);
  pinMode(OutdoorLEDPin, OUTPUT);
}

void loop() {

}

//Function that controls the lights inside.
//It checks if there the door is open and if there is movement to start.
// It also checks the light level and turns on lights gradually dependent of how dark it is.

void LightsystemIn(int Position, bool Movement){
  int lightLevel = analogRead(A0); //reads light level.
  float voltage = (float)lightLevel * (5.0 / 1023.0); // truns the signal into digital
  int val = (int)voltage * 100; //just a factor.

  if(val > LightThreshold){ //when there is light (light level is higher than threshold), then turn off LED
    digitalWrite(IndoorLEDPin,LOW);
  }
  else if(val < LightThreshold){ // If it is dark (light level is less than threshold) turn on LED an amout dependet on how dark it is.
   if(Position == 0 && Movement ==true){
      int Light = 100-val;
      analogWrite(IndoorLEDPin,(Light));
      //Serial.println(Light);
    }
  }
}

//This function turns on light outside if there is movement and it is dark.
void LightsystemOut(bool Movement){
  int lightLevel = analogRead(A0); 
  float voltage = (float)lightLevel * (5.0 / 1023.0);
  int val = (int)voltage * 100;

  if(val < LightThreshold && Movement == true){ // If the light level is lower then threshold and movement, turn lights on.
    digitalWrite(OutdoorLEDPin,HIGH);
  }
  else{ //else, turn off lights 
    digitalWrite(OutdoorLEDPin,LOW);
  }
}



