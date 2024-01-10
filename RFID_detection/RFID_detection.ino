#include <SPI.h>
#include <MFRC522.h> 

#define RST_PIN   9    // set Reset to digital pin 9
#define SS_PIN   10    // set SDA to digital pin 10

MFRC522 mfrc522(SS_PIN, RST_PIN); //Define a new RFC reader

//Matrix contaning ID's defined as valid
byte validAccess[4][4] = {{0x63, 0xC8, 0xA0, 0x34},
                          {0xF9, 0xAD, 0xD8, 0x15},
                          {0xB9, 0xE1, 0x6C, 0x14},
                          {0x53, 0xB2, 0x05, 0x34}};


bool access;
char name[20];

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init(); //Setup and initialize RFID reader
}

void loop() {

  readID(&access, &name[0]);

}

void readID(bool *a, char *name){
  byte UID[4];

  // Reset the loop if no new card present on the sensor/reader
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	}

  //Read all values of the UID from a card and store them in UID array
  for(int i = 0 ; i < 4; i++){
    UID[i] = mfrc522.uid.uidByte[i];
  }


  *a = checkAccess(&UID[0]);

  //Print function for debugging. (COMMENT OUT BEFORE COMMIT!)
  
  for(int i = 0 ; i < 4; i++){
    Serial.print(UID[i],HEX);
    Serial.print(" ");
  }
  Serial.println(" Card has been read");

  
  if(access == true){
    Serial.println("Access granted. Welcome home");
  }
  else{
    Serial.println("Access Denied");
  }
  

  mfrc522.PICC_HaltA(); //Prevents reditection of a card

}

//Checks all stored card numbers with UID, if there is a match return acces granted as true
bool checkAccess(byte *UID){
  bool accessGranted = false;
  
  for(int i = 0 ; i < 4 ; i++){ //Runs through all stored cards
    int match = 0;
    for(int k = 0 ; k < 4 ; k++){ //Runs through all numbers in a saved card
      if (validAccess[i][k] == UID[k]){ 
        match++;
      }
    }
    if(match == 4){
      accessGranted = true;
    }
  }
  return accessGranted;
}
