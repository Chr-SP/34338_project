#include <SPI.h>
#include <MFRC522.h> 

#define RST_PIN         9                                               // set Reset to digital pin 9
#define SS_PIN          10                                              // set SDA to digital pin 10

MFRC522 rfcreader(SS_PIN, RST_PIN);




void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
