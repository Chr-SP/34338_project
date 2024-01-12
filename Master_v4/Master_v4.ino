#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>        
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>  // Include the Wi-Fi-Multi library
#include <ESP8266WebServer.h>  // Include the WebServer library
#include <ESP8266mDNS.h>       // Include the mDNS library

LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WiFiMulti wifiMulti;
// Create an instance of the server
ESP8266WebServer server(80);

const int lightSensorPin = A0;
const int motionSensorIndoorPin = D0;
const int motionSensorOutdoorPin = D3;

int lightThreshold = 100; //A threshold that controls when light level is low
int position = 0; // position of the servo / lock

char toSend[3] = {0,0}; // the char defining a command to send to slave
char recieved[3] = {0,0};


// variables for keypad - lcd
char inputPassword[4] = { 0, 0, 0 , 0};  // placeholder for password.
char enteredPassword[4] = { inputPassword[0], inputPassword[1], inputPassword[2] };
int cursorPosition = 0;
char truePassword[4] = { '9', '1', '1' , 0};
int doClear = 0;
unsigned long timestamp;
//char* names[6] = { "JJ", "AW", "JK", "CT", "CP" };
//int lock = 0; /////////////////////// Still needed?????


const uint8_t led = D2;
//uint8_t door_on_off = 1;  // control variable for door lock
uint8_t keypad_RFID = 1;  // control variable to switch between RFID and keypad (1 fot RFID, 0 for keypad)
uint8_t alarm_on_off = 0; // control variable for the alarm (0 for off)
/* Variable text strings used in HTML code*/
String door_text = " Door is locked";
String RFID_text = " RFID is active";
String user_text = " User 1 is active";
String alarm_text = " Alarm is off";
String new_password = "000";

char password[4] = { 0, 0, 0 }; // Password for keypad
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


void setup() {
  Serial.begin(115200);
  delay(10);

  Wire.begin(D2,D1); // SDA, SCL

  pinMode(lightSensorPin, INPUT_PULLUP); // light sensor
  pinMode(motionSensorIndoorPin, INPUT);
  pinMode(motionSensorOutdoorPin, INPUT);

  // init LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0); //////////////////////////////////////////////////////////////
  lcd.print("Enter password: "); ////////////////////////////////////////////////////
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
}

void loop() {
  server.handleClient();
  /*
  char toSend[3] = {0,0};

  // Check lighting
  bool motionSensedIndoor = motionSensed(motionSensorIndoorPin);
  bool motionSensedOutdoor = motionSensed(motionSensorOutdoorPin);
  lightsystemIndoor(position, motionSensedIndoor);
  lightsystemOutdoor(motionSensedOutdoor);

  
  */

  getMessage();
  //delay(10);
  
}


void sendMessage(char toSend[]){ // transmit command to slave
  Wire.beginTransmission(11);
  Wire.write(toSend);
  //delay(100);
  Wire.endTransmission();
}


void getMessage(){
  char recieved[3] = {0,0};
  Wire.requestFrom(11,2);
  int i = 0;
  while(Wire.available()){
    recieved[i] = Wire.read();
    i++;
  }


  if ((doClear == 1) && (timestamp + 2000 < millis())){ // check if clear is needed
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter password: ");
    doClear = 0;
  }

  if (recieved[0]=='k'){ ///////////////////dont ask why it's not 'k'

    /*
    if ((toClear[0] == 1) && (toClear[1] + 1000 < millis())){ // clear display when new press
      lcd.clear();
      toClear[0] = 0;
    }
    */

    Serial.println(recieved[1]);
    lcd.setCursor(0, 0);
    lcd.print("Enter password: ");
    int key = recieved[1];  // the pressed button is saved in 'key'
    //Serial.println(recieved[0]);
    //Serial.println(key);
    


    if ((key) && (doClear == 0)) {           // when a button is pressed
      if (key == '*') {  // delete all button
        cursorPosition = 0;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter password: ");
        memset(inputPassword, 0, sizeof(inputPassword));           // reset password
      }
      else if (key == '#') {                           // when # button is pressed
        if (cursorPosition == 3) {                       // when # button is pressed as botton number 4 check if password is true
          if (memcmp(inputPassword, truePassword, 4) == 0) {  // password is true
            lcd.setCursor(0, 1);
            lcd.print("Welcome home ");
            doClear = 1;
            timestamp = millis();
            servoLock(position); // Unlock the door
            //lcd.print(names[1]);
            //return 1;
          }
          else {  // entered password was wrong
            lcd.setCursor(0, 1);
            lcd.print("Wrong password");
            cursorPosition = 0;
            memset(inputPassword, 0, sizeof(inputPassword));  // Reset password
            doClear = 1;
            timestamp = millis();
            //return 0;
          }
          cursorPosition = 0;                     // Reset position
          memset(inputPassword, 0, sizeof(inputPassword));  // Reset password
        }
        else if (cursorPosition >= 3) {          //if more than 3 buttons are pressed the password is wrong
          lcd.setCursor(0, 1);
          lcd.print("Wrong password");
          cursorPosition = 0;
          memset(inputPassword, 0, sizeof(inputPassword));  // Reset password
          doClear = 1;
          timestamp = millis();
          //return 0;
        }
        else {  // when there is not enough numbers entered in the code
          lcd.setCursor(0, 1);
          lcd.print("Complete the Pin!");
          cursorPosition = 0;                     // Reset position
          memset(inputPassword, 0, sizeof(inputPassword));  // Reset password
          doClear = 1;
          timestamp = millis();
        }
      }
      else if(cursorPosition >= 3){
        lcd.setCursor(0, 1);
        lcd.print("Wrong password");
        cursorPosition = 0;
        memset(inputPassword, 0, sizeof(inputPassword));  // Reset password
        doClear = 1;
        timestamp = millis();
      }
      else {  // add number to array and print on lcd at the same time
        inputPassword[cursorPosition] = key;
        cursorPosition++;
        lcd.setCursor(0, 1);
        char enteredPassword[4] = { inputPassword[0], inputPassword[1], inputPassword[2] };
        lcd.print(enteredPassword);
      }
    }
  }
}

void lightsystemIndoor(int position, bool motionSensed){
  int lightLevel = analogRead(lightSensorPin);

  if((lightLevel > lightThreshold) || (motionSensed==false)){ //when there is light (light level is higher than threshold), then turn off LED
    toSend[0] = 'a';
    toSend[1] = 0;
    sendMessage(toSend);
  }
  else if((position == 0) && (motionSensed==true)){ // If it is dark (light level is less than threshold) turn on LED an amout dependet on how dark it is.
    int light = 255 - lightLevel * 254 / lightThreshold; // SKAL SKALERES YDERLIGERE
    toSend[0] = 'a';
    toSend[1] = light; // light
    sendMessage(toSend);
  }
}

void lightsystemOutdoor(bool motionSensed){
  int lightLevel = analogRead(lightSensorPin);

  if ((lightLevel > lightThreshold) || (motionSensed == false)){
    toSend[0] = 'b';
    toSend[1] = 0; // Outside LED off
    sendMessage(toSend);
  }
  else{
    toSend[0] = 'b';
    toSend[1] = 1; // Outside LED on
    sendMessage(toSend);
  }
}

bool motionSensed(int whichMotionSensor){
  bool motionSensed = digitalRead(whichMotionSensor);
  return motionSensed;
}

void servoLock(int lock){
  position = !position;
  if (lock==1){
    toSend[0] = 'c';
    toSend[1] = 1; // Lock the servo
    sendMessage(toSend);
  }
  else{
    toSend[0] = 'c';
    toSend[1] = 0; // Unlock the servo
    sendMessage(toSend);
  }
  Serial.println(toSend[0]);
  Serial.println((int)toSend[1]);
}




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

void handle_door() {  // If a POST request is made to URI /LED
  //door_on_off = !door_on_off;
  if (position == 0) {
    servoLock(position);
    door_text = " Door is locked";
  } else {
    servoLock(position);
    door_text = " Door is open";
  }
  server.sendHeader("Location", "/");  // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                    // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

void handle_RFID_keypad() {  // If a POST request is made to URI /LED
  keypad_RFID = !keypad_RFID;
  if (keypad_RFID) {
    RFID_text = " RFID is active";
  } else {
    RFID_text = " Keypad is active";
  }
  server.sendHeader("Location", "/");  // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                    // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}
void handle_user1() {
  user_text = " User1 has access";
  server.sendHeader("Location", "/");  // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                    // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}
void handle_user2() {
  user_text = " User2 has access";
  server.sendHeader("Location", "/");  // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                    // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}
void handle_user3() {
  user_text = " User3 has access";
  server.sendHeader("Location", "/");  // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                    // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}
void handle_user4() {
  user_text = " User4 has access";
  server.sendHeader("Location", "/");  // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                    // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}
void handle_alarm() {  // If a POST request is made to URI /LED
  alarm_on_off = !alarm_on_off;
  if (alarm_on_off) {
    alarm_text = " Alarm is on";
  } else {
    alarm_text = " Alarm is off";
  }
  server.sendHeader("Location", "/");  // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                    // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}
void handleLED() {                       // If a POST request is made to URI /LED
  digitalWrite(led, !digitalRead(led));  // Change the state of the LED
  server.sendHeader("Location", "/");    // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                      // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}
void handle_password() {                                               // If a POST request is made to URI /login
  if (!server.hasArg("password") || server.arg("password") == NULL) {  // If the POST request doesn't have a password data
    server.send(400, "text/plain", "400: Invalid Request");            // The request is invalid, so send HTTP status 400
    return;
  }

  //----------------------------------
  // Check for password longer than 3
  if (!server.hasArg("password") || server.arg("password") == NULL) {  // If the POST request doesn't have a password data
    server.send(400, "text/plain", "400: Invalid Request");            // The request is invalid, so send HTTP status 400
    return;
  }
  //----------------------------------

  if (server.arg("password")) {  // If both the username and the password are correct
    server.send(200, "text/html", "<h1>Your new password is, " + server.arg("password") + "!</h1><p>Password change successful</p>");
    new_password = server.arg("password");
    from_String_to_CharArray(server.arg("password"), password );
    // password[0] = new_password[0];
    Serial.println(password);
  } else {  // Username and password don't match
    server.send(401, "text/plain", "401: Unauthorized");
  }
}
void from_String_to_CharArray(const String& S_word, char* Array){
  for (int i = 0; i < 4; i++) {
  Array[i]=S_word[i];
  }
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");  // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
void init_sever_connection() {
  // Connect to WiFi network
  Serial.println();
  wifiMulti.addAP("Youcanforgetaboutit", "Anna1234");  // add Wi-Fi networks you want to connect to ##########################################################################

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


