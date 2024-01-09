#include <SPI.h>
#include <MFRC522.h> 

#define RST_PIN   9                                               // set Reset to digital pin 9
#define SS_PIN   10                                              // set SDA to digital pin 10

MFRC522 mfrc522(SS_PIN, RST_PIN);
byte UID[4];





void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
}

void loop() {
  // put your main code here, to run repeatedly:

  readID(&UID[0]);

  

  
}

void readID(byte *ID){
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	}

  for(int i = 0 ; i < 4; i++){
    ID[i] = mfrc522.uid.uidByte[i];
  }

  for(int i = 0 ; i < 4; i++){
    Serial.print(UID[i], HEX);
  }

  Serial.println(" Card has been read");

  mfrc522.PICC_HaltA();

}
