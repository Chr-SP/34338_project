//Todo: make accessGranted return something that makes sence

#include <SPI.h>
#include <MFRC522.h> 

#define RST_PIN   9    // set Reset to digital pin 9
#define SS_PIN   10    // set SDA to digital pin 10

MFRC522 mfrc522(SS_PIN, RST_PIN); //Define a new RFC reader
byte UID[4];
byte validAccess[4][4] = {{99,200,160,52},
                          {0xF9, 0xAD, 0xD8, 0x15},
                          {0xB9, 0xE1, 0x6C, 0x14},
                          {0x53, 0xB2, 0x05, 0x34}};
//0x63, 0xC8, 0xA0, 0x34
void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init(); //Setup and initialize
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

	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	}

  //Read all values of the UID and store them in UID array
  for(int i = 0 ; i < 4; i++){
    ID[i] = mfrc522.uid.uidByte[i];
  }

  checkAccess();

  //Print function for debugging.
  
  for(int i = 0 ; i < 4; i++){
    Serial.print(UID[i]);
    Serial.print(" ");
  }
  Serial.println(" Card has been read");
  

  mfrc522.PICC_HaltA();

}

bool checkAccess(){
  bool accessGranted = false;
  
  for(int i = 0 ; i < 4 ; i++){
    int count = 0;
    for(int k = 0 ; k < 4 ; k++){
      if (validAccess[i][k] == UID[k]){
        count++;
      }
    }
    if(count == 4){
      accessGranted = true;
    }
  }
  return accessGranted;
}
