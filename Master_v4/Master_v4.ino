/*


*/

#include <Wire.h>               // I2C for master/slave communication
#include <LiquidCrystal_I2C.h>  // LCD library
#include <WiFiClient.h>         // Wifi
#include <ESP8266WiFi.h>        // Wifi
#include <ESP8266WiFiMulti.h>   // Include the Wi-Fi-Multi library
#include <ESP8266WebServer.h>   // Include the WebServer library
#include <ESP8266mDNS.h>        // Include the mDNS library
#include <SPI.h>                // SPI communication for RFID
#include <MFRC522.h>            // RFID library

LiquidCrystal_I2C lcd(0x27, 16, 2); // Initialize LCD
ESP8266WiFiMulti wifiMulti;         // Initialize wifi
ESP8266WebServer server(80);        // Create an instance of the server

const char* ssid = "Malthes_Laptop";              // Wifi name
const char* server_password = "34338Malthe";   // Wifi password

// Pin constants
const int LIGHTSENSORPIN = A0;
const int MOTIONSENSORINDOORPIN = D0; 
const int RST_PIN = D4;                           // Reset for RFID
const int SS_PIN = D8;                            // Data for RFID

// The number/address of the slave
const int SLAVE = 11;

// Status constants for simplicity
const int LOCKDOOR = 1;
const int OPENDOOR = 0;
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
const char KEYPADCONTROL = 'k';

// Initial states of control variables
uint8_t keypad_RFID_select = KEYPAD_RFID_ON;
uint8_t alarm_on_off = ALARM_OFF;
int lockPosition = OPENDOOR;
int lightThreshold = 160;                         // Threshold is 240 normal indoor lighting

// Communication arrays for I2C between master/slave
char toSend[3] = { 0, 0 };
char recieved[4] = { 0, 0, 0};

// Placeholders for recieved password from keypad
char inputPassword[4] = { 0, 0, 0, 0 };
//char enteredPassword[4] = { inputPassword[0], inputPassword[1], inputPassword[2] }; // Constrains LCD print to 3 chars

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
  Serial.begin(115200); // Initialize serial monitor for webpage server IP
  delay(10);

  Wire.begin(D2, D1);  // SDA & SCL for the I2C master/slave communication

  // Pin setup
  pinMode(LIGHTSENSORPIN, INPUT_PULLUP);
  pinMode(MOTIONSENSORINDOORPIN, INPUT);

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
  lightsystemIndoor();    // Controls indoor lighting if door is opened
  lightsystemOutdoor();               // Controls outdoor lighting 
  motionAlarm();          // Activates alarm if indoor motion is detected
  getMessage();                       // Requests data from slave to keypad and outdoor motionsensor
  keypad();                           // Controls keypad and LCD
  checkRFID();                        // Activates RFID if selected
  clearDisplay();                     // Clears display 2s after a message is displayed
}


// Compiles and transmits command to slave
void slaveControlWord(char component, int stateValue) {
  toSend[0] = component;
  toSend[1] = stateValue;
  Wire.beginTransmission(SLAVE);         // Opens I2C link to slave with ID 11
  Wire.write(toSend);                 // Sends command to slave
  Wire.endTransmission();
}

// Requests data from slave
void getMessage() {
  memset(recieved, 0, sizeof(recieved));  // Resets recieved message
  Wire.requestFrom(SLAVE, 3);             // Requests 3 bytes of data
  int i = 0;
  while (Wire.available()) {
    recieved[i] = Wire.read();
    i++;
  }
}

// Controls keypad input and LCD prints
void keypad() {
  if ((keypad_RFID_select != RFID_ON) && (recieved[0] == KEYPADCONTROL)) {
    lcd.setCursor(0, 0);
    lcd.print("Enter password: ");
    if ((recieved[1]) && (doClear == 0)) {  // If a button is pressed
      if (recieved[1] == '*') {             // Click * to reset display and password
        cursorPosition = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter password: ");
        memset(inputPassword, 0, sizeof(inputPassword));
      } else if (recieved[1] == '#') {                        // Click # to check if password is correct
        if (cursorPosition == 3) {                            
          if (memcmp(inputPassword, truePassword, 4) == 0) {  // Correct password entered
            handle_door();                                    // Unlocks/Locks door
            lcd.setCursor(0, 1);
            if (lockPosition == OPENDOOR){
              lcd.print("Welcome home ");                     // "Welcome home" when unlocking
            }
            else{
              lcd.print("Goodbye ");                          // "Goodbye" when locking
            }
          } else {                                            // Wrong password entered
            lcd.setCursor(0, 1);
            lcd.print("Wrong password");
          }
        } else if (cursorPosition > 3) {                     // Wrong if more than 3 buttons are pressed
          lcd.setCursor(0, 1);
          lcd.print("Wrong password");
        } else {                                              // Too short input password
          lcd.setCursor(0, 1);
          lcd.print("Complete the Pin!");
        }
        cursorPosition = 0;
        memset(inputPassword, 0, sizeof(inputPassword));      // Reset password
        doClear = 1;                                          // Sets state to clear display
        timestamp = millis();                                 // Sets timer to clear display
      } else if (cursorPosition > 3) {                        // Too long input password
        lcd.setCursor(0, 1);
        lcd.print("Wrong password");

        cursorPosition = 0;
        memset(inputPassword, 0, sizeof(inputPassword));      // Reset password
        doClear = 1;                                          // Sets state to clear display
        timestamp = millis();                                 // Sets timer to clear display

      } else {                                                // Add keypress to array and print on LCD
        inputPassword[cursorPosition] = recieved[1];          // Saves keypress from slave
        cursorPosition++;
        lcd.setCursor(0, 1);
        char enteredPassword[4] = { inputPassword[0], inputPassword[1], inputPassword[2] }; // Constrains LCD print to 3 chars
        lcd.print(enteredPassword);                           
      }
    }
  }
}

// Checks if RFID is activated
void checkRFID() {
  if (keypad_RFID_select != KEYPAD_ON) {
    readKey(&access, &name[0]);
  }
}

// Clears the LCD after 2 seconds
void clearDisplay() {
  if ((doClear == 1) && (timestamp + 2000 < millis())) {  // Check if clear is needed and 2 seconds has passed
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter password: ");
    doClear = 0;
  }
}

// Controls indoor lighting
void lightsystemIndoor() {
  int lightLevel = analogRead(LIGHTSENSORPIN);
  if ((lightLevel > lightThreshold) || (!digitalRead(MOTIONSENSORINDOORPIN))) {     // When the light level is higher than the threshold, turn off indoor LED
    slaveControlWord(INDOOR_LED, LIGHT_OFF);
  } else if ((lockPosition == OPENDOOR) && (digitalRead(MOTIONSENSORINDOORPIN))) {  // If door is open and it is dark turn on indoor LED with depending on light level
    int light = 255 - lightLevel * 254 / lightThreshold;                            // Computes gradient of light from low at threshold to high at max light
    slaveControlWord(INDOOR_LED, light);
  }
}

// Controls outdoor lighting
void lightsystemOutdoor() {
  int lightLevel = analogRead(LIGHTSENSORPIN);
  if ((lightLevel > lightThreshold) || (!(int)recieved[2])) {                       // When the light level is higher than the threshold, turn off outdoor LED   
    slaveControlWord(OUTDOOR_LED, LIGHT_OFF);
  } else if((int)recieved[2]) {                                                     // If it is dark turn on outdoor LED
    slaveControlWord(OUTDOOR_LED, LIGHT_ON);
  }
}

// Check alarm should change
void motionAlarm() {
  if ((lockPosition == LOCKDOOR) && (digitalRead(MOTIONSENSORINDOORPIN))) {  // If the door is locked and there is motion indoor, the alarm starts
    handle_alarm();
  }
}

// Webpage layout
void handleRoot() {
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

// Updates webpage (called within all handle functions to refresh webpage)
void server_update_header() {
  server.sendHeader("Location", "/");   // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                     // Send it back to the browser with an HTTP status 303
}

// Unlocks/Locks door
void handle_door() {
  if (lockPosition) {                   // If the door is locked: open
    slaveControlWord(SERVOCONTROL, OPENDOOR);
    door_text = " Door is open";
  } else {                              // If the door is open: lock
    slaveControlWord(SERVOCONTROL, LOCKDOOR);
    door_text = " Door is locked";
  }
  lockPosition = !lockPosition;
  server_update_header();
}

// Keypad and/or RFID select
void handle_RFID_keypad() {
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

// Updates which users have RFID access
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
  // If all users have access, all access is removed, otherwise grant all access
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

// Change alarm state
void handle_alarm() {
  if ((alarm_on_off == ALARM_OFF) || (digitalRead(MOTIONSENSORINDOORPIN))) {
    alarm_text = " Alarm is on";
    slaveControlWord(ALARMCONTROL, ALARM_ON);
    alarm_on_off = ALARM_ON;
  } else {
    alarm_text = " Alarm is off";
    alarm_on_off = ALARM_OFF;
    slaveControlWord(ALARMCONTROL, ALARM_OFF);
  }
  server_update_header();
}

// Changes keypad password from webpage
void handle_password() { 
  if (!server.hasArg("password") || server.arg("password") == NULL) {     // If the POST request doesn't have a password data
    password_error_text = "Invalid Request. ";
    server_update_header();
    return;
  }
  // Checks password length
  if ((server.arg("password").length()) != 3) {                           // Password invalid length
    password_error_text = "Invalid Password! It has to be 3 characters. ";
    server_update_header();
    return;
  }
  else if (server.arg("password")) {                                      // If the password is correct
    from_String_to_CharArray(server.arg("password"), truePassword);       // Changes correct password
    password_error_text = "Password change successful";
    server_update_header();
  }
  else {                                                                  //#############################################################s
    password_error_text = "Unauthorized ";
    server_update_header();
  }
}

// Convert String to char array
void from_String_to_CharArray(const String& S_word, char* Array) {
  for (int i = 0; i < 4; i++) {
    Array[i] = S_word[i];
  }
}

// Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}

// Init server and print IP
void init_sever_connection() {
  // Connect to WiFi network
  Serial.println();
  wifiMulti.addAP(ssid, server_password);

  Serial.println();
  Serial.print("Connecting ...");

  while (wifiMulti.run() != WL_CONNECTED) { // Print dots while connection in progress
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected to ");
  Serial.println(WiFi.SSID());              // Prints the wifi name
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());           // Prints the server IP

  if (MDNS.begin("iot")) {                  // Start the mDNS responder for esp8266.local
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

// Reads and checks key
void readKey(bool* a, char* n) {
  byte UID[4];

  // Reset the loop if no new card present on the sensor/reader
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  // Read all values of the UID from a card and store them in UID array
  for (int i = 0; i < 4; i++) {
    UID[i] = mfrc522.uid.uidByte[i];
  }
  *a = checkAccess(&UID[0]);
  // If key has access read data, print data and change door state
  if (access == true) { 
    readDataFromKey(n);
    handle_door();
    lcd.setCursor(0, 1);
    if (lockPosition == OPENDOOR){
      lcd.print("Welcome home ");
      lcd.print(name);
    }
    else{
      lcd.print("Goodbye ");
      lcd.print(name);
    }
    doClear = 1;
    timestamp = millis();
  }

  mfrc522.PICC_HaltA();       // Prevents redetection of a card

  mfrc522.PCD_StopCrypto1();  //################################################################################
}

// Checks all stored card numbers with UID, if there is a match return acces granted as true
bool checkAccess(byte* UID) {
  bool accessGranted = false;

  for (int i = 0; i < 4; i++) {     // Runs through all stored cards
    int match = 0;
    for (int k = 0; k < 4; k++) {   // Runs through all numbers in a saved card
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

// Writes data on key
void writeDataToKey(char initials[2]) {

  // Reset the loop if no new card present on the sensor/reader
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  /*
  WARNING FOLLOWING NUMBERS IS OFF LIMITS AND WILL RUIN A SECTOR ON A CARD IF USED!
  3, 7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63
  */
  int blockNumber = 16;  //Number of the block that will be written to
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNumber, &key, &(mfrc522.uid)); //Authentication check for writing
  //Check for success of authentication
  if (status != MFRC522::STATUS_OK) {
    return;
  }
  status = mfrc522.MIFARE_Write(blockNumber, (byte*)initials, 16);  //Try to write data
  //Check for succes of write
  if (status != MFRC522::STATUS_OK) {
    return;
  }

  mfrc522.PICC_HaltA();  //Prevents reditection of a card

  mfrc522.PCD_StopCrypto1();
}

// Reads data from key
void readDataFromKey(char* n) {
  /*
  WARNING FOLLOWING NUMBERS IS OFF LIMITS AND WILL RUIN A SECTOR ON A CARD IF USED!
  3, 7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63
  */
  int blockNumber = 16;  // Number of the block that will be written to
  byte tempData[18];     // Temporary array to store data read from key
  byte tempDataLength = 18;  
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNumber, &key, &(mfrc522.uid));  //Authentication check for writing
  //Check for success of authentication
  if (status != MFRC522::STATUS_OK) {
    return;
  }
  status = mfrc522.MIFARE_Read(blockNumber, tempData, &tempDataLength);  //Try to read from the wanted block and store it in tempData
  tempDataLength = 18;
  //Check for succes of read
  if (status != MFRC522::STATUS_OK) {
    return;
  }
  //Copy data from temp variable to wanted variable
  for (int i = 0; i < 18; i++) {
    *n = tempData[i];
    n++;
  }
}
