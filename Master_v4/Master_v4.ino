#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>  // Include the Wi-Fi-Multi library
#include <ESP8266WebServer.h>  // Include the WebServer library
#include <ESP8266mDNS.h>       // Include the mDNS library
#include <SPI.h>
#include <MFRC522.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WiFiMulti wifiMulti;
// Create an instance of the server
ESP8266WebServer server(80);

const char* ssid = "Youcanforgetaboutit";
const char* server_password = "Anna1234";

const int lightSensorPin = A0;
const int motionSensorIndoorPin = D0;
const int motionSensorOutdoorPin = D3;
const int RST_PIN = D4;    // set Reset to digital pin 9
const int SS_PIN = D8;    // set SDA to digital pin 10

const int LOCKDOOR = 0;
const int OPENDOOR = 1;
const int KEYPAD_ON = 0;
const int RFID_ON = 1;
const int ALARM_ON = 1;
const int ALARM_OFF = 0;
const int LIGHT_ON = 1;
const int LIGHT_OFF = 0;

uint8_t keypad_RFID_select = RFID_ON;   // control variable to switch between RFID and keypad (1 fot RFID, 0 for keypad)
uint8_t alarm_on_off = 0;  // control variable for the alarm (0 for off)

const char INDOOR_LED = 'a';
const char OUTDOOR_LED = 'b';
const char SERVOCONTROL = 'c';  // Servo destination?
const char ALARMCONTROL = 'd';


int lightThreshold = 160;  //A threshold that controls when light level is low
int lockPosition = OPENDOOR;      // position of the servo / lock


char toSend[3] = { 0, 0 };  // the char defining a command to send to slave
char recieved[3] = { 0, 0 };


// variables for keypad - lcd
char inputPassword[4] = { 0, 0, 0, 0 };  // placeholder for password.
char enteredPassword[4] = { inputPassword[0], inputPassword[1], inputPassword[2] };
int cursorPosition = 0;
char truePassword[4] = { '9', '1', '1', 0 };
int doClear = 0;
unsigned long timestamp;
//char* names[6] = { "JJ", "AW", "JK", "CT", "CP" };
//int lock = 0; /////////////////////// Still needed?????


/* Variable text strings used in HTML code*/
String door_text = " Door is open";
String RFID_text = " RFID is active";
String user_text = " User 1 is active";
String alarm_text = " Alarm is off";

char password[4] = { 0, 0, 0 };  // Password for keypad
/* Handle functions for the server*/
void handleRoot();
void handleNotFound();
void handle_door();
void handle_RFID_keypad();
void handle_user1();
void handle_user2();
void handle_user3();
void handle_user4();
void handle_alarm();
void handle_password();


MFRC522 mfrc522(SS_PIN, RST_PIN); //Define a new RFC reader
MFRC522::MIFARE_Key key; //Defines a new instanse of the MIFARE key
MFRC522::StatusCode status; //Defines an instanse of the status code
//Matrix contaning ID's defined as valid
byte validAccess[4][4] = {{0x63, 0xC8, 0xA0, 0x34},
                          {0xF9, 0xAD, 0xD8, 0x15},
                          {0xB9, 0xE1, 0x6C, 0x14},
                          {0x53, 0xB2, 0x05, 0x34}};
byte userAccess[4] = {1, 1, 1, 1};  // users who have access              
bool access;
char name[18];

void setup() {
  Serial.begin(115200);
  delay(10);

  Wire.begin(D2, D1);  // SDA, SCL

  pinMode(lightSensorPin, INPUT_PULLUP);  // light sensor
  pinMode(motionSensorIndoorPin, INPUT);
  pinMode(motionSensorOutdoorPin, INPUT);

  // init LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);            //////////////////////////////////////////////////////////////
  lcd.print("Enter password: ");  ////////////////////////////////////////////////////

  delay(10);
  server.on("/LOCK_DOOR", HTTP_POST, handle_door);
  server.on("/RFID", HTTP_POST, handle_RFID_keypad);
  server.on("/User1", HTTP_POST, handle_user1);
  server.on("/User2", HTTP_POST, handle_user2);
  server.on("/User3", HTTP_POST, handle_user3);
  server.on("/User4", HTTP_POST, handle_user4);
  server.on("/ALARMON", HTTP_POST, handle_alarm);
  server.on("/PASSWORD", HTTP_POST, handle_password);

  init_sever_connection();

  SPI.begin();
  mfrc522.PCD_Init(); //Setup and initialize RFID reader
  //Prepare the keys for authentication
  for(int i = 0 ; i < 6 ; i++){
    key.keyByte[i] = 0xFF;
  }
}

void loop() {
  char toSend[3] = {0,0};

  server.handleClient();

  
  // Check lighting
  Serial.println(digitalRead(motionSensorOutdoorPin));

  lightsystemIndoor(lockPosition);
  lightsystemOutdoor();
  
  motionAlarm(lockPosition);

  readID(&access, &name[0]);
  getMessage();
  /*
  if (keypad_RFID_select) { // RFID is selecet
    readID(&access, &name[0]);
  } else{
    getMessage();
  }
  */
  //delay(10);

  if ((doClear == 1) && (timestamp + 2000 < millis())) {  // check if clear is needed
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter password: ");
    doClear = 0;
  }
}



void sendMessage(char toSend[]) {  // transmit command to slave
  Wire.beginTransmission(11);
  Wire.write(toSend);
  Wire.endTransmission();
}

void slaveControlWord(char component, int stateValue) {
  toSend[0] = component;
  toSend[1] = stateValue;  // Lock the servo
  sendMessage(toSend);
}

void getMessage() {
  char recieved[3] = { 0, 0 };
  Wire.requestFrom(11, 2);
  int i = 0;
  while (Wire.available()) {
    recieved[i] = Wire.read();
    i++;
  }

  /*
  if ((doClear == 1) && (timestamp + 2000 < millis())) {  // check if clear is needed
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter password: ");
    doClear = 0;
  }
  */

  if (recieved[0] == 'k') {  ///////////////////dont ask why it's not 'k'

    // Serial.println(recieved[1]);
    lcd.setCursor(0, 0);
    lcd.print("Enter password: ");

    if ((recieved[1]) && (doClear == 0)) {  // when a button is pressed

      if (recieved[1] == '*') {             // delete all button
        cursorPosition = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter password: ");
        memset(inputPassword, 0, sizeof(inputPassword));      // reset password

        
      } else if (recieved[1] == '#') {                                // when # button is pressed
        if (cursorPosition == 3) {                            // when # button is pressed as botton number 4 check if password is true
          if (memcmp(inputPassword, truePassword, 4) == 0) {  // password is true
            lcd.setCursor(0, 1);
            lcd.print("Welcome home ");
            handle_door();
          } else {  // entered password was wrong
            lcd.setCursor(0, 1);
            lcd.print("Wrong password");
          }
        } else if (cursorPosition >= 3) {                   //if more than 3 buttons are pressed the password is wrong
          lcd.setCursor(0, 1);
          lcd.print("Wrong password");
        } else {  // when there is not enough numbers entered in the code
          lcd.setCursor(0, 1);
          lcd.print("Complete the Pin!");
        }
        cursorPosition = 0;                               // Reset position
        memset(inputPassword, 0, sizeof(inputPassword));  // Reset password
        doClear = 1;
        timestamp = millis();
      } else if (cursorPosition >= 3) {
        lcd.setCursor(0, 1);
        lcd.print("Wrong password");

        cursorPosition = 0;
        memset(inputPassword, 0, sizeof(inputPassword));  // Reset password
        doClear = 1;
        timestamp = millis();
        
      } else {  // add number to array and print on lcd at the same time
        inputPassword[cursorPosition] = recieved[1];
        cursorPosition++;
        lcd.setCursor(0, 1);
        char enteredPassword[4] = { inputPassword[0], inputPassword[1], inputPassword[2] };
        lcd.print(enteredPassword);
      }
    }
  }
}

void lightsystemIndoor(int lockPosition) {
  int lightLevel = analogRead(lightSensorPin);

  if ((lightLevel > lightThreshold) || (!digitalRead(motionSensorIndoorPin))) {  //when there is light (light level is higher than threshold), then turn off LED
    slaveControlWord(INDOOR_LED, LIGHT_OFF);
  } else if ((lockPosition == 0) && (digitalRead(motionSensorIndoorPin))) {  // If it is dark (light level is less than threshold) turn on LED an amout dependet on how dark it is.
    int light = 255 - lightLevel * 254 / lightThreshold;   // SKAL SKALERES YDERLIGERE
    slaveControlWord(INDOOR_LED, light);
  }
}

void lightsystemOutdoor() {
  int lightLevel = analogRead(lightSensorPin);

  if ((lightLevel > lightThreshold) || (!digitalRead(motionSensorOutdoorPin))) {
    slaveControlWord(OUTDOOR_LED, LIGHT_OFF);
  } else {
    slaveControlWord(OUTDOOR_LED, LIGHT_ON);
  }
}

void motionAlarm(int lockPosition){ 
  if((!lockPosition == LOCKDOOR) && (digitalRead(motionSensorIndoorPin))){ //If the door is locked and there is motion, the alarm starts
    handle_alarm();
  }
}


/*
bool motionSensed(int whichMotionSensor) {
  bool motionSensed = digitalRead(whichMotionSensor);
  return motionSensed;
}
*/

/*
void servoLock(int control_door, char slave_servo_lock) {
  toSend[0] = slave_servo_lock;
  toSend[1] = control_door;  // Lock the servo
  sendMessage(toSend);
}
*/

void handleRoot() {  // When URI / is requested, send a web page with a button to toggle the LED
  server.send(200, "text/html", "<html><title>Internet of Things - Demonstration</title><meta charset=\"utf8\" \/> \ 
      </head><body><h1>Smart Home Security System</h1> \
      <p>Lock or unlock door</p> \
      <form action=\"/LOCK_DOOR\" method=\"POST\" ><input type=\"submit\" value=\"Lock door\"style=\"width:60px; height:20px; font-size:10px; background-color: #ff88cc; border-color: ##ff0080\";<p>"
                                  + door_text + " <p></form> \ 
      <p>Change between RFID or Keypad<p> \
      <form action=\"/RFID\" method=\"POST\" ><input type=\"submit\" value=\"RFID keypad\"style=\"width:60px; height:20px; font-size:10px; background-color: #ff88cc; border-color: ##ff0080\";<p>"
                                  + RFID_text + " <p></form> \
      <p>Turn alarm on or off<p>\
      <form action=\"/ALARMON\" method=\"POST\" ><input type=\"submit\" value=\"Alarm on\" style=\"width:60px; height:20px; font-size:10px; background-color: #ff88cc; border-color: ##ff0080\";<p>"
                                  + alarm_text + " <p></form> \
      <p>Person RFID access<p>\
      <form action=\"/User1\" method=\"POST\" ><input type=\"submit\" value=\"User1\" style=\"width:60px; height:20px; font-size:10px; background-color: #ff88cc; border-color: ##ff0080\"></form> \
      <form action=\"/User2\" method=\"POST\" ><input type=\"submit\" value=\"User2\" style=\"width:60px; height:20px; font-size:10px; background-color: #ff88cc; border-color: ##ff0080\"></form> \
      <form action=\"/User3\" method=\"POST\" ><input type=\"submit\" value=\"User3\" style=\"width:60px; height:20px; font-size:10px; background-color: #ff88cc; border-color: ##ff0080\"></form> \
      <form action=\"/User4\" method=\"POST\" ><input type=\"submit\" value=\"User4\" style=\"width:60px; height:20px; font-size:10px; background-color: #ff88cc; border-color: ##ff0080\"></form> \
      <p>" + user_text + "<p>\
      <p>Change keypad code<p>\
      <form action =\"/PASSWORD\" method=\"POST\"><input type=\"password\" name=\"password\" placeholder=\"Password\"></br><input type=\"submit\" value=\"Change password\"></form> \
      </body></html>");
}
// <form action=\"/PASSWORD\" method=\"POST\"><input type=\"text\" name=\"password\" placeholder=\"New password\"></form>
void server_update_header() {
  server.sendHeader("Location", "/");  // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                    // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

void handle_door() {  // If a POST request is made to URI /LED
  if (lockPosition) {  // Lock door request
    slaveControlWord(SERVOCONTROL, LOCKDOOR);
    door_text = " Door is locked";
  } else {
    slaveControlWord(SERVOCONTROL, OPENDOOR);
    door_text = " Door is open";
  }
  lockPosition = !lockPosition;
  server_update_header();
}

void handle_RFID_keypad() {  // If a POST request is made to URI /LED
  keypad_RFID_select = !keypad_RFID_select;
  if (keypad_RFID_select) {
    RFID_text = " RFID is active";
  } else {
    RFID_text = " Keypad is active";
  }
  server_update_header();
}
void handle_user1() {
  user_text = " User1 has access";
  userAccess[4] = {1, 0, 0, 0};  // users who have access              

  server_update_header();
}
void handle_user2() {
  user_text = " User2 has access";
  server_update_header();
}
void handle_user3() {
  user_text = " User3 has access";
  server_update_header();
}
void handle_user4() {
  user_text = " User4 has access";
  server_update_header();
  userAccess[4] = {1, 1, 1, 1};
}

void handle_alarm() {  // If a POST request is made to URI /LED
  if ((!alarm_on_off) || (digitalRead(motionSensorIndoorPin))) {
    alarm_text = " Alarm is on";
    slaveControlWord(ALARMCONTROL, ALARM_ON);
    alarm_on_off = 1;
  } else {
    alarm_text = " Alarm is off";
    alarm_on_off = 0;
    slaveControlWord(ALARMCONTROL, ALARM_OFF);
  }
  //alarm_on_off = !alarm_on_off;
  server_update_header();
}


void handle_password() {                                               // If a POST request is made to URI /login
  if (!server.hasArg("password") || server.arg("password") == NULL) {  // If the POST request doesn't have a password data
    server.send(400, "text/plain", "400: Invalid Request");            // The request is invalid, so send HTTP status 400
    return;
  }
  // Check for password is 3 ing length
  if ((server.arg("password").length()) != 3) {                                                        // If the POST request doesn't have a password data
    server.send(400, "text/plain", "400: Invalid! Password has to have 3 characters long");            // The request is invalid, so send HTTP status 400
    return;
  }else if (server.arg("password")) {                                                                  // If both the username and the password are correct
    server.send(200, "text/html", "<h1>Your new password is, " + server.arg("password") + "!</h1><p>Password change successful</p>");
    from_String_to_CharArray(server.arg("password"), truePassword);
  } else {  // Username and password don't match
    server.send(401, "text/plain", "401: Unauthorized");
  }
}

void from_String_to_CharArray(const String& S_word, char* Array) {
  for (int i = 0; i < 4; i++) {
    Array[i] = S_word[i];
  }
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");  // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
void init_sever_connection() {
  // Connect to WiFi network
  Serial.println();
  wifiMulti.addAP(ssid, server_password);  // add Wi-Fi networks you want to connect to ##########################################################################

  Serial.println();
  Serial.print("Connecting ...");
  // WiFi.begin(ssid, password);

  while (wifiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected to ");
  Serial.println(WiFi.SSID());
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("iot")) {  // Start the mDNS responder for esp8266.local
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }
  server.on("/", HTTP_GET, handleRoot);
  server.onNotFound(handleNotFound);
  // Start the server
  server.begin();
  Serial.println("Server started");
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

  
  
  if(access == true){
    readDataFromKey(n);
    lcd.setCursor(0, 1);
    lcd.print("Welcome home ");
    lcd.print(name);
    handle_door();
    doClear = 1;
    timestamp = millis();
  }

  //Print function for debugging.
  
  for(int i = 0 ; i < 4; i++){
    Serial.print(UID[i],HEX);
    Serial.print(" ");
  }
  Serial.println(" Card has been read");

  Serial.println(name);
  

  mfrc522.PICC_HaltA(); //Prevents redetection of a card

  mfrc522.PCD_StopCrypto1();

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
    if((match == 4) && (userAccess[i])){
      accessGranted = true;
    }
  }
  return accessGranted;
}

void writeDataToKey(char initials[2]){

  // Reset the loop if no new card present on the sensor/reader
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}
	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	}
  
  //3, 7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63 
  //IS OFF LIMITS AND WILL RUIN A SECTOR ON A CARD IF USED!
  int blockNumber = 16; //Number of the block that will be written to

  //Authentication check for writing
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNumber, &key, &(mfrc522.uid));

  //Check for success of authentication
  if(status != MFRC522::STATUS_OK){
    //Serial.println("Error in authentication");
    return;
  }
  else{
    Serial.println("Authentication succesful");
  }
  

  
  status = mfrc522.MIFARE_Write(blockNumber, (byte*)initials, 16); //Try to write data

  //Check for succes of write
  if(status != MFRC522::STATUS_OK){
    Serial.println("Failed writing data");
    return;
  } 
  else{
    Serial.println("Data was written succesfully");
  }
  

  mfrc522.PICC_HaltA(); //Prevents reditection of a card

  mfrc522.PCD_StopCrypto1();

}

void readDataFromKey(char *n){
  //3, 7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63 
  //IS OFF LIMITS AND WILL RUIN A SECTOR ON A CARD IF USED!
  int blockNumber = 16; //Number of the block that will be written to
  byte tempData[18];
  byte bufferLength = 18; 

  //Authentication check for writing
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNumber, &key, &(mfrc522.uid));

  //Check for success of authentication
  if(status != MFRC522::STATUS_OK){
    //Serial.println("Error in authentication");
    return;
  }
  /*
  else{
    Serial.println("Authentication succesful");
  }
  */


  status = mfrc522.MIFARE_Read(blockNumber, tempData, &bufferLength); //Try to read from the wanted block and store it in tempData
  bufferLength = 18;
  //Check for succes of read
  if (status != MFRC522::STATUS_OK)
  {
    //Serial.print("Failed to read data");
    return;
  }
  /*
  else{
    Serial.println("Data was read succesfully");  
  }
  */

  //Copy data from temp variable to wanted variable
  for(int i = 0; i < 18; i++){
    *n = tempData[i];
    n++;
  }
}




