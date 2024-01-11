/*
  Smart home security sistem WiFi

  The purpes of this program is to create a server and connect a ESP8266 to it
  so you are able to control a security system at home from a web page.
  It controles the door lock, the alarm, if RFID card or keypad is used and witch user is aloud in.
  It is also possible to change the 3 digit password for the keypad.

  Created 8-1-2024
  by Jesper B.K.M. Hansen

*/
#include <ESP8266WiFi.h>        
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>  // Include the Wi-Fi-Multi library
#include <ESP8266WebServer.h>  // Include the WebServer library
#include <ESP8266mDNS.h>       // Include the mDNS library

ESP8266WiFiMulti wifiMulti;
// Create an instance of the server
ESP8266WebServer server(80);

const uint8_t led = D2;
uint8_t door_on_off = 1;  // control variable for door lock
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
/* Server */
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
  // Check if a client has connected
  server.handleClient();
}

void handleRoot() {  // When URI / is requested, send a web page with a button to toggle the LED
  server.send(200, "text/html", "<html><title>Internet of Things - Demonstration</title><meta charset=\"utf8\" \/> \ 
      </head><body><h1>Smart Home Security System</h1> \
      <p>Lock or unlock door</p> \
      <form action=\"/LOCK_DOOR\" method=\"POST\" ><input type=\"submit\" value=\"Lock door\"style=\"width:60px; height:20px; font-size:10px\";<p>"
                                  + door_text + " <p></form> \ 
      <p>Change between RFID or Keypad<p> \
      <form action=\"/RFID\" method=\"POST\" ><input type=\"submit\" value=\"RFID keypad\"style=\"width:60px; height:20px; font-size:10px\";<p>"
                                  + RFID_text + " <p></form> \
      <p>Turn alarm on or off<p>\
      <form action=\"/ALARMON\" method=\"POST\" ><input type=\"submit\" value=\"Alarm on\" style=\"width:60px; height:20px; font-size:10px\";<p>"
                                  + alarm_text + " <p></form> \
      <p>Person RFID access<p>\
      <form action=\"/User1\" method=\"POST\" ><input type=\"submit\" value=\"User1\" style=\"width:60px; height:20px; font-size:10px\"></form> \
      <form action=\"/User2\" method=\"POST\" ><input type=\"submit\" value=\"User2\" style=\"width:60px; height:20px; font-size:10px\"></form> \
      <form action=\"/User3\" method=\"POST\" ><input type=\"submit\" value=\"User3\" style=\"width:60px; height:20px; font-size:10px\"></form> \
      <form action=\"/User4\" method=\"POST\" ><input type=\"submit\" value=\"User4\" style=\"width:60px; height:20px; font-size:10px\"></form> \
      <p>" + user_text + "<p>\
      <p>Change keypad code<p>\
      <form action =\"/PASSWORD\" method=\"POST\"><input type=\"password\" name=\"password\" placeholder=\"Password\"></br><input type=\"submit\" value=\"Change password\"></form> \
      </body></html>");
}
// <form action=\"/PASSWORD\" method=\"POST\"><input type=\"text\" name=\"password\" placeholder=\"New password\"></form>

void handle_door() {  // If a POST request is made to URI /LED
  door_on_off = !door_on_off;
  if (door_on_off) {
    door_text = " Door is locked";
  } else {
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
  wifiMulti.addAP("OnePlus", "OnePlus123");  // add Wi-Fi networks you want to connect to

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
