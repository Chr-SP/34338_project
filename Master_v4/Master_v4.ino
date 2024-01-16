/*


*/

#include <Wire.h>               // I2C for master / slave communication
#include <LiquidCrystal_I2C.h>  // LCD library
#include <WiFiClient.h>         // Wifi
#include <ESP8266WiFi.h>        // Wifi
#include <ESP8266WiFiMulti.h>   // Include the Wi-Fi-Multi library
#include <ESP8266WebServer.h>   // Include the WebServer library
#include <ESP8266mDNS.h>        // Include the mDNS library
#include <SPI.h>                // SPI communication for RFID
#include <MFRC522.h>            // RFID library

LiquidCrystal_I2C lcd(0x27, 16, 2); // Init LCD
ESP8266WiFiMulti wifiMulti;         // Init wifi
ESP8266WebServer server(80);        // Create an instance of the server

const char* ssid = "Youcanforgetaboutit"; // Wifi name
const char* server_password = "Anna1234"; // Wifi password

// Pin constants
const int lightSensorPin = A0;
const int motionSensorIndoorPin = D0;
const int RST_PIN = D4;  // Reset for RFID
const int SS_PIN = D8;   // Data for RFID

// Status constants for simplicity
const int LOCKDOOR = 0;
const int OPENDOOR = 1;
const int KEYPAD_RFID_ON = 0;
const int RFID_ON = 1;
const int KEYPAD_ON = 2;
const int ALARM_ON = 1;
const int ALARM_OFF = 0;
const int LIGHT_ON = 1;
const int LIGHT_OFF = 0;
const char INDOOR_LED = 'a';
const char OUTDOOR_LED = 'b';
const char SERVOCONTROL = 'c';
const char ALARMCONTROL = 'd';

// Initial states of control variables
uint8_t keypad_RFID_select = KEYPAD_RFID_ON;
uint8_t alarm_on_off = ALARM_OFF;
int lockPosition = OPENDOOR;
int lightThreshold = 160;         // Threshold is 240 normal indoor lighting
int motionDetectedOutdoor = false; 

// Communication arrays for I2C between master / slave
char toSend[3] = { 0, 0 };
char recieved[4] = { 0, 0 , 0};

// Placeholders for recieved password from keypad
char inputPassword[4] = { 0, 0, 0, 0 };
char enteredPassword[4] = { inputPassword[0], inputPassword[1], inputPassword[2] }; // Constrains LCD print to 3 chars

// Variables for keypad - LCD
int cursorPosition = 0; // Startpoistion for display printing
char truePassword[4] = { '9', '1', '1', 0 }; // Correct password

// Variables for tracking when to clear display
int doClear = 0; // 
unsigned long timestamp;

// Variable text strings used in HTML code
String door_text = " Door is open";
String RFID_text = " Keypad and RFID is active";
String alarm_text = " Alarm is off";
String user1_text = " has accsess";
String user2_text = " has accsess";
String user3_text = " has accsess";
String password_error_text = "";

// Handle functions for the server
void handleRoot();
void handleNotFound();
void handle_door();
void handle_RFID_keypad();
void handle_user1();
void handle_user2();
void handle_user3();
void handle_all_users();
void handle_alarm();
void handle_password();


MFRC522 mfrc522(SS_PIN, RST_PIN);  // Defines the RFID reader
MFRC522::MIFARE_Key key;           // Defines instance of the MIFARE key
MFRC522::StatusCode status;        // Defines an instance of the status code

// Matrix containing RFID's defined as valid users
byte validAccess[4][4] = {{0x63, 0xC8, 0xA0, 0x34},
                          {0xB9, 0xE1, 0x6C, 0x14},
                          {0x53, 0xB2, 0x05, 0x34}};
byte userAccess[4] = {1, 1, 1};  // Users who have access
// Variables for RFID access check and name printing
bool access;
char name[18];


void setup() {
  Serial.begin(115200); //############################################################################################################
  delay(10);

  Wire.begin(D2, D1);  // SDA & SCL for the I2C master / slave communication

  // Pin setup
  pinMode(lightSensorPin, INPUT_PULLUP);
  pinMode(motionSensorIndoorPin, INPUT);

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);            
  lcd.print("Enter password: ");

  delay(10); //############################################################################################################
  // Server post-requests 
  server.on("/LOCK_DOOR", HTTP_POST, handle_door);
  server.on("/RFID", HTTP_POST, handle_RFID_keypad);
  server.on("/User1", HTTP_POST, handle_user1);
  server.on("/User2", HTTP_POST, handle_user2);
  server.on("/User3", HTTP_POST, handle_user3);
  server.on("/AllUsers", HTTP_POST, handle_all_users);
  server.on("/ALARMON", HTTP_POST, handle_alarm);
  server.on("/PASSWORD", HTTP_POST, handle_password);
  init_sever_connection();

  // Setup and initialize RFID reader
  SPI.begin();
  mfrc522.PCD_Init();

  // Setup the keys for memory authentication
  for (int i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}


void loop() {
  char toSend[3] = { 0, 0 }; // Resets message to slave ###################################################################
  server.handleClient();              // Updates server
  lightsystemIndoor(lockPosition);    // Controls indoor lighting if door is opened
  lightsystemOutdoor();               // Controls outdoor lighting 
  motionAlarm(lockPosition);          // Activates alarm if indoor motion is detected
  getMessage();                       // Requests info from slave to control keypad and outdoor motionsensor
  checkRFID();                        // Activates RFID if selected
  clearDisplay();                     // Clears display 2s after a message is displayed
}

/*
// Transmit command to slave
void sendMessage(char toSend[]) { 
  Wire.beginTransmission(11);         // Opens I2C link to slave with ID 11
  Wire.write(toSend);                 // Sends command to slave
  Wire.endTransmission();
}

// Compiles command to slave
void slaveControlWord(char component, int stateValue) {
  toSend[0] = component;
  toSend[1] = stateValue;
  sendMessage(toSend);
}
*/

// Compiles and transmits command to slave
void slaveControlWord(char component, int stateValue) {
  toSend[0] = component;
  toSend[1] = stateValue;
  Wire.beginTransmission(11);         // Opens I2C link to slave with ID 11
  Wire.write(toSend);                 // Sends command to slave
  Wire.endTransmission();
}

void getMessage() {
  char recieved[4] = { 0, 0, 0};
  Wire.requestFrom(11, 3);
  int i = 0;
  while (Wire.available()) {
    recieved[i] = Wire.read();
    i++;
  }

  motionDetectedOutdoor = (int)recieved[2];

  if ((keypad_RFID_select != 1) && (recieved[0] == 'k')) {  ///////////////////dont ask why it's not 'k'

    // Serial.println(recieved[1]);
    lcd.setCursor(0, 0);
    lcd.print("Enter password: ");

    if ((recieved[1]) && (doClear == 0)) {  // when a button is pressed

      if (recieved[1] == '*') {  // delete all button
        cursorPosition = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter password: ");
        memset(inputPassword, 0, sizeof(inputPassword));  // reset password


      } else if (recieved[1] == '#') {                        // when # button is pressed
        if (cursorPosition == 3) {                            // when # button is pressed as botton number 4 check if password is true
          if (memcmp(inputPassword, truePassword, 4) == 0) {  // password is true
            lcd.setCursor(0, 1);
            lcd.print("Welcome home ");
            handle_door();
          } else {  // entered password was wrong
            lcd.setCursor(0, 1);
            lcd.print("Wrong password");
          }
        } else if (cursorPosition >= 3) {  //if more than 3 buttons are pressed the password is wrong
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

void checkRFID() {
  if (keypad_RFID_select < 2) {       // RFID is
    readID(&access, &name[0]);
  }
}

void clearDisplay() {
  if ((doClear == 1) && (timestamp + 2000 < millis())) {  // check if clear is needed
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter password: ");
    doClear = 0;
  }
}

void lightsystemIndoor(int lockPosition) {
  int lightLevel = analogRead(lightSensorPin);

  if ((lightLevel > lightThreshold) || (!digitalRead(motionSensorIndoorPin))) {  //when there is light (light level is higher than threshold), then turn off LED
    slaveControlWord(INDOOR_LED, LIGHT_OFF);
  } else if ((lockPosition == 0) && (digitalRead(motionSensorIndoorPin))) {  // If it is dark (light level is less than threshold) turn on LED an amout dependet on how dark it is.
    int light = 255 - lightLevel * 254 / lightThreshold;                     // SKAL SKALERES YDERLIGERE
    slaveControlWord(INDOOR_LED, light);
  }
}

void lightsystemOutdoor() {
  int lightLevel = analogRead(lightSensorPin);

  if ((lightLevel > lightThreshold) || (!motionDetectedOutdoor)) {
    slaveControlWord(OUTDOOR_LED, LIGHT_OFF);
  } else if(motionDetectedOutdoor) {
    slaveControlWord(OUTDOOR_LED, LIGHT_ON);
  }
}

void motionAlarm(int lockPosition) {
  if ((!lockPosition == LOCKDOOR) && (digitalRead(motionSensorIndoorPin))) {  //If the door is locked and there is motion, the alarm starts
    handle_alarm();
  }
}

void handleRoot() {  // When URI / is requested, send a web page with a button to toggle the LED
  server.send(200, "text/html", "<html><title>Internet of Things - Demonstration</title><meta charset=\"utf8\">\
      </head><body><h1>Smart Home Security System</h1> \
      <p>Lock or unlock door</p> \
      <form action=\"/LOCK_DOOR\" method=\"POST\" ><input type=\"submit\" value=\"Lock door\"style=\"width:100px; height:20px; font-size:10px; background-color: #ff88cc; border-color: ##ff0080\";<p>"
                                  + door_text + " <p></form>\
      <p>Change between RFID or Keypad<p> \
      <form action=\"/RFID\" method=\"POST\" ><input type=\"submit\" value=\"RFID keypad\"style=\"width:100px; height:20px; font-size:10px; background-color: #ff88cc; border-color: ##ff0080\";<p>"
                                  + RFID_text + " <p></form> \
      <p>Turn alarm on or off<p>\
      <form action=\"/ALARMON\" method=\"POST\" ><input type=\"submit\" value=\"Alarm on\" style=\"width:100px; height:20px; font-size:10px; background-color: #ff88cc; border-color: ##ff0080\";<p>"
                                  + alarm_text + " <p></form> \
      <p>Person RFID access<p>\
      <form action=\"/User1\" method=\"POST\" ><input type=\"submit\" value=\"User1\" style=\"width:100px; height:20px; font-size:10px; background-color: #ff88cc; border-color: ##ff0080\"; <p>" 
                                  + user1_text +"<p></form> \
      <form action=\"/User2\" method=\"POST\" ><input type=\"submit\" value=\"User2\" style=\"width:100px; height:20px; font-size:10px; background-color: #ff88cc; border-color: ##ff0080\"; <p>" 
                                  + user2_text +"<p></form> \
      <form action=\"/User3\" method=\"POST\" ><input type=\"submit\" value=\"User3\" style=\"width:100px; height:20px; font-size:10px; background-color: #ff88cc; border-color: ##ff0080\"; <p>" 
                                  + user3_text +"<p></form> \
      <form action=\"/AllUsers\" method=\"POST\" ><input type=\"submit\" value=\"All users\" style=\"width:100px; height:20px; font-size:10px; background-color: #ff88cc; border-color: ##ff0080\"></form> \
      <p>Change keypad code<p>\
      <form action =\"/PASSWORD\" method=\"POST\"><input type=\"password\" name=\"password\" placeholder=\"Password\"></br><input type=\"submit\" value=\"Change password\"></form> \
      <p>"+ password_error_text + "<p>\
      <p>""Your current password is: " + truePassword +  "<p>\
      </body></html>");
}
void server_update_header() {
  server.sendHeader("Location", "/");  // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                    // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

void handle_door() {   // If a POST request is made to URI /LED
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
  if (keypad_RFID_select == KEYPAD_RFID_ON) {
    RFID_text = " RFID is active";
    keypad_RFID_select = RFID_ON;
  } else if (keypad_RFID_select == RFID_ON) {
    RFID_text = " Keypad is active";
    keypad_RFID_select = KEYPAD_ON;
  } else {
    RFID_text = " Keypad and RFID is active";
    keypad_RFID_select = KEYPAD_RFID_ON;
  }
  server_update_header();
}
void handle_user1() {
  userAccess[0] = !userAccess[0];
  if (userAccess[0]){
    user1_text = " has access";
  } else {
    user1_text = "";
  }    
  server_update_header();
}
void handle_user2() {
  userAccess[1] = !userAccess[1];
  if (userAccess[1]){
    user2_text = " has access";
  } else {
    user2_text = "";
  }    
  server_update_header();
}
void handle_user3() {
  userAccess[2] = !userAccess[2];
  if (userAccess[2]){
    user3_text = " has access";
  } else {
    user3_text = "";
  }    
  server_update_header();
}
void handle_all_users() {
  if (userAccess[0] && userAccess[1] && userAccess[2]){
    user1_text = "";
    user2_text = "";
    user3_text = "";
    for (int i; i<3; i++){
      userAccess[i] = 0;
    }
  } else {
    user1_text = " has access";
    user2_text = " has access";
    user3_text = " has access";
    for (int i; i<3; i++){
      userAccess[i] = 1;
    }
  }    
  server_update_header();
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
  password_error_text = "Invalid Request. ";
    // server.send(400, "text/plain", "400: Invalid Request");            // The request is invalid, so send HTTP status 400
    server_update_header();

    return;
  }
  // Check for password is 3 ing length
  if ((server.arg("password").length()) != 3) {   
    password_error_text = "Invalid Password! It has to be 3 characters. ";                                         // If the POST request doesn't have a password data
    // server.send(400, "text/plain", "400: Invalid! Password has to have 3 characters long");  // The request is invalid, so send HTTP status 400
    server_update_header();

    return;
  } else if (server.arg("password")) {  // If both the username and the password are correct
    // server.send(200, "text/html", "<h1>Your new password is, " + server.arg("password") + "!</h1><p>Password change successful</p>");
    from_String_to_CharArray(server.arg("password"), truePassword);
    password_error_text = "Password change successful";
    server_update_header();
  } else {  // Username and password don't match
    // server.send(401, "text/plain", "401: Unauthorized");
    password_error_text = "Unauthorized ";
    server_update_header();
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

void readID(bool* a, char* n) {
  byte UID[4];

  // Reset the loop if no new card present on the sensor/reader
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  //Read all values of the UID from a card and store them in UID array
  for (int i = 0; i < 4; i++) {
    UID[i] = mfrc522.uid.uidByte[i];
  }


  *a = checkAccess(&UID[0]);



  if (access == true) {
    readDataFromKey(n);
    lcd.setCursor(0, 1);
    lcd.print("Welcome home ");
    lcd.print(name);
    handle_door();
    doClear = 1;
    timestamp = millis();
  }

  //Print function for debugging.

  for (int i = 0; i < 4; i++) {
    Serial.print(UID[i], HEX);
    Serial.print(" ");
  }
  Serial.println(" Card has been read");

  Serial.println(name);


  mfrc522.PICC_HaltA();  //Prevents redetection of a card

  mfrc522.PCD_StopCrypto1();
}

//Checks all stored card numbers with UID, if there is a match return acces granted as true
bool checkAccess(byte* UID) {
  bool accessGranted = false;

  for (int i = 0; i < 4; i++) {  //Runs through all stored cards
    int match = 0;
    for (int k = 0; k < 4; k++) {  //Runs through all numbers in a saved card
      if (validAccess[i][k] == UID[k]) {
        match++;
      }
    }
    if ((match == 4) && (userAccess[i])) {
      accessGranted = true;
    }
  }
  return accessGranted;
}

void writeDataToKey(char initials[2]) {

  // Reset the loop if no new card present on the sensor/reader
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  //3, 7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63
  //IS OFF LIMITS AND WILL RUIN A SECTOR ON A CARD IF USED!
  int blockNumber = 16;  //Number of the block that will be written to

  //Authentication check for writing
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNumber, &key, &(mfrc522.uid));

  //Check for success of authentication
  if (status != MFRC522::STATUS_OK) {
    //Serial.println("Error in authentication");
    return;
  } else {
    Serial.println("Authentication succesful");
  }



  status = mfrc522.MIFARE_Write(blockNumber, (byte*)initials, 16);  //Try to write data

  //Check for succes of write
  if (status != MFRC522::STATUS_OK) {
    Serial.println("Failed writing data");
    return;
  } else {
    Serial.println("Data was written succesfully");
  }


  mfrc522.PICC_HaltA();  //Prevents reditection of a card

  mfrc522.PCD_StopCrypto1();
}

void readDataFromKey(char* n) {
  //3, 7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63
  //IS OFF LIMITS AND WILL RUIN A SECTOR ON A CARD IF USED!
  int blockNumber = 16;  //Number of the block that will be written to
  byte tempData[18];
  byte bufferLength = 18;

  //Authentication check for writing
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNumber, &key, &(mfrc522.uid));

  //Check for success of authentication
  if (status != MFRC522::STATUS_OK) {
    //Serial.println("Error in authentication");
    return;
  }
  /*
  else{
    Serial.println("Authentication succesful");
  }
  */


  status = mfrc522.MIFARE_Read(blockNumber, tempData, &bufferLength);  //Try to read from the wanted block and store it in tempData
  bufferLength = 18;
  //Check for succes of read
  if (status != MFRC522::STATUS_OK) {
    //Serial.print("Failed to read data");
    return;
  }
  /*
  else{
    Serial.println("Data was read succesfully");  
  }
  */

  //Copy data from temp variable to wanted variable
  for (int i = 0; i < 18; i++) {
    *n = tempData[i];
    n++;
  }
}
