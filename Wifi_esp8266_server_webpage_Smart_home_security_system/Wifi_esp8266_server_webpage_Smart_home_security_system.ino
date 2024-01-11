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
const char* ssid = "OnePlus";
const char* wifi_password = "OnePlus123";

ESP8266WiFiMulti wifiMulti;
// Create an instance of the server
ESP8266WebServer server(80);

uint8_t door_on_off = 1;   // control variable for door lock
uint8_t keypad_RFID = 1;   // control variable to switch between RFID and keypad (1 fot RFID, 0 for keypad)
uint8_t alarm_on_off = 0;  // control variable for the alarm (0 for off)
/* Variable text strings used in HTML code*/
String door_text = " Door is locked";
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

void setup() {
  Serial.begin(115200);
  delay(10);
  /* Server post request options */
  init_server_request();

  init_sever_connection();
}

void loop() {
  // Check if a client has connected and handles reqests
  server.handleClient();
}
// Setup server handle requests
void init_server_request(){
  server.on("/LOCK_DOOR", HTTP_POST, handle_door);
  server.on("/RFID", HTTP_POST, handle_RFID_keypad);
  server.on("/User1", HTTP_POST, handle_user1);
  server.on("/User2", HTTP_POST, handle_user2);
  server.on("/User3", HTTP_POST, handle_user3);
  server.on("/User4", HTTP_POST, handle_user4);
  server.on("/ALARMON", HTTP_POST, handle_alarm);
  server.on("/PASSWORD", HTTP_POST, handle_password);
}

// Sends the main page to server
void handleRoot() {
  // The HTML String for the webpage
  server.send(200, "text/html", "<html><title>Internet of Things - Demonstration</title><meta charset=\"utf8\" \/> \ 
      </head><body><h1>Smart Home Security System</h1> \
      <p>Lock or unlock door</p> \
      <form action=\"/LOCK_DOOR\" method=\"POST\" ><input type=\"submit\" value=\"Lock door\"style=\"width:60px; height:20px; font-size:10px; background-color:#ff99cc;\";<p>"
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
// Handles the door lock and text to server
void handle_door() {
  door_on_off = !door_on_off;
  if (door_on_off) {
    door_text = " Door is locked";
  } else {
    door_text = " Door is open";
  }
  opdate_server_page();
}
// Handles if the RFID or keypad is active and sends text to server
void handle_RFID_keypad() {
  keypad_RFID = !keypad_RFID;
  if (keypad_RFID) {
    RFID_text = " RFID is active";
  } else {
    RFID_text = " Keypad is active";
  }
  opdate_server_page();
}
// Handles witch user has access
void handle_user1() {
  user_text = " User1 has access";
  opdate_server_page();
}
void handle_user2() {
  user_text = " User2 has access";
  opdate_server_page();
}
void handle_user3() {
  user_text = " User3 has access";
  opdate_server_page();
}
void handle_user4() {
  user_text = " User4 has access";
  opdate_server_page();
}

void handle_alarm() {  // If a POST request is made to URI /LED
  alarm_on_off = !alarm_on_off;
  if (alarm_on_off) {
    alarm_text = " Alarm is on";
  } else {
    alarm_text = " Alarm is off";
  }
  opdate_server_page();
}
// void handleLED() {                       // If a POST request is made to URI /LED
//   digitalWrite(led, !digitalRead(led));  // Change the state of the LED
//   server.sendHeader("Location", "/");    // Add a header to respond with a new location for the browser to go to the home page again
//   server.send(303);                      // Send it back to the browser with an HTTP status 303 (See Other) to redirect
// }
void handle_password() {      // If a POST request is made to URI /login
  if (!server.hasArg("password") || server.arg("password") == NULL) {  // If the POST request doesn't have a password
    server.send(400, "text/plain", "400: Invalid Request");            // The request is invalid, so send HTTP status 400
    return;
  }

  //----------------------------------
  // Check for password longer than 3
  // if (!server.hasArg("password") || server.arg("password") == NULL) {  // If the POST request doesn't have a password
  //   server.send(400, "text/plain", "400: Invalid Request");            // The request is invalid, so send HTTP status 400
  //   return;
  // }
  //----------------------------------

  if (server.arg("password")) {  // If the password are correct
    server.send(200, "text/html", "<h1>Your new password is, " + server.arg("password") + "!</h1><p>Password change successful</p>");
    from_String_to_CharArray(server.arg("password"), password);
    // Serial.println(password);
  } else {  // Username and password don't match
    server.send(401, "text/plain", "401: Unauthorized");
  }
}
// Function to kopi string to char array
void from_String_to_CharArray(const String& S_word, char* char_array) {
  for (int i = 0; i < 4; i++) {
    char_array[i] = S_word[i];
  }
}
// Add a header to respond with a new location for the browser to go to the home page again
void opdate_server_page() {
  server.sendHeader("Location", "/");
  server.send(303);  // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}
void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");  // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
void init_sever_connection() {
  // Connect to WiFi network
  Serial.println();
  // wifiMulti.addAP("ssid", wifi_password);  // add Wi-Fi networks you want to connect to
  WiFi.begin(ssid, wifi_password);
  Serial.println();
  Serial.print("Connecting ...");

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
