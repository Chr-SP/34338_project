//TODO: 
//Sæt så i main code kører man read eller write mode og kører den ønskede function i forlængelse af dette
//Lav keyboard input til først at definere mode
//Hvis write brug keyboard til at læse hvad der skal skrives
//Find ud af hvorfor den kun kører en gang

#include <SPI.h>
#include <MFRC522.h> 

#define RST_PIN   9    // set Reset to digital pin 9
#define SS_PIN   10    // set SDA to digital pin 10

MFRC522 mfrc522(SS_PIN, RST_PIN); //Define a new RFC reader

MFRC522::MIFARE_Key key; //Defines a new instanse of the MIFARE key

MFRC522::StatusCode status; //Defines an instanse of the status code

//Matrix contaning ID's defined as valid
byte validAccess[4][4] = {{0x63, 0xC8, 0xA0, 0x34},
                          {0xF9, 0xAD, 0xD8, 0x15},
                          {0xB9, 0xE1, 0x6C, 0x14},
                          {0x53, 0xB2, 0x05, 0x34}};


bool access;
char name[18];
char bufferLength = 18;


void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init(); //Setup and initialize RFID reader
  //Prepare the keys for authentication
  for(int i = 0 ; i < 6 ; i++){
    key.keyByte[i] = 0xFF;
  }
}

void loop() {
  
  //writeDataToKey(0xFF);
  readID(&access, &name[0]);

}

void readID(bool *a, char* n){
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
    readDataFromKey(n);
    Serial.print("Access granted. Welcome home");
    Serial.println(name);
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

void writeDataToKey(byte initials[2]){
  // Reset the loop if no new card present on the sensor/reader
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	}
  //3, 7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63 
  //IS OFF LIMITS AND WILL RUIN A SECTOR ON A CARD IF USED!
  int blockNumber = 16;
  //Authentication check for writing
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNumber, &key, &(mfrc522.uid));


  if(status != MFRC522::STATUS_OK){
    Serial.println("Error in authentication");
    return;
  }
  else{
    Serial.println("Authentication succesful");
  }

  status = mfrc522.MIFARE_Write(blockNumber, initials, 16);
  if(status != MFRC522::STATUS_OK){
    Serial.println("Failed writing data");
    return;
  }
  else{
    Serial.println("Data was written succesfully");
  }

  mfrc522.PICC_HaltA(); //Prevents reditection of a card

}

void readDataFromKey(char *n){
  //3, 7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63 
  //IS OFF LIMITS AND WILL RUIN A SECTOR ON A CARD IF USED!
  int blockNumber = 16;
  char tempName[18];

  //Authentication check for writing
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNumber, &key, &(mfrc522.uid));


  if(status != MFRC522::STATUS_OK){
    Serial.println("Error in authentication");
    return;
  }
  else{
    Serial.println("Authentication succesful");
  }

  status = mfrc522.MIFARE_Read(blockNumber, tempName, &bufferLength);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else
  {
    Serial.println("Block was read successfully");  
  }
  for(int i = 0; i < 18; i++){
    *n = tempName[i];
    n++;
  }

  //strcpy(*n,tempName);
  

  return;

}
