#include <Wire.h>
char input;

void setup() {
  Serial.begin(115200);
  Wire.begin(D2,D1); // SDA, SCL
}

void loop() {
  Wire.beginTransmission(11);
  if (Serial.available() > 0) {  // checks for input
    input = (char)tolower(Serial.read());  // typecasting the input to a char
    Wire.write(input);
    Wire.endTransmission();

    if (input == 's'){
      Wire.requestFrom(11,1);
      while(Wire.available()){
        char a = Wire.read();
        Serial.print("LED turned on: ");
        Serial.println(a);
      }
    }
  }
}
