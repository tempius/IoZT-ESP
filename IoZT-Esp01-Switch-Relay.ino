/* ESP */
#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino
//needed for library
#include <WiFiClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>
#include <WebSocketsServer.h>

byte relON[] = {0xA0, 0x01, 0x01, 0xA2};  //Hex command to send to serial for open relay
byte relOFF[] = {0xA0, 0x01, 0x00, 0xA1}; //Hex command to send to serial for close relay

/* control FLAGS */
int relayState = LOW;
int relayLastState = LOW;
int relay2State = LOW;
int relay2LastState = LOW;
int httmPinLastState = LOW;
int httm2PinLastState = LOW;
int connectionTimeout = 180;

/* vars */
String wifiNameString = "IOZT-ESP#" + String(ESP.getChipId());
const char *wifiName = wifiNameString.c_str();
const char *wifiPass = "esp-iozt";
int relayPin = 4;  // GPIO 4 - used to power up the relay
int relay2Pin = 5; // GPIO 5 - used to power up the relay
int httmOut = 13;  // GPIO 13 = receives signal from httm
int httmVcc = 12;  // GPIO 12 - used to power up the httm
int httm2Out = 14;  // GPIO 14 = receives signal from httm 2
int httm2Vcc = 16;  // GPIO 16 - used to power up the httm 2

// Create an instance of the server
std::unique_ptr<ESP8266WebServer> server_manager;
WebSocketsServer webSocket = WebSocketsServer(81);

void handleServerHeaders()
{
  // if (server_manager->method() == HTTP_OPTIONS)
  // {
  server_manager->sendHeader("Access-Control-Allow-Origin", "*");
  server_manager->sendHeader("Access-Control-Max-Age", "10000");
  server_manager->sendHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
  server_manager->sendHeader("Access-Control-Allow-Headers", "*");
  // }
}

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
  connected to this access point to see it.
*/
void handleRoot()
{
  Serial.println("ESP ROOT");
  handleServerHeaders();
  server_manager->send(200, "text/plain", wifiNameString);
}

void handleScan()
{
  Serial.println("ESP SCAN");
  StaticJsonBuffer<250> jsonBuffer;
  JsonObject &jsonObj = jsonBuffer.createObject();
  char JSONmessageBuffer[250];

  jsonObj["componentType"] = "double switch";
  jsonObj["componentName"] = wifiNameString;
  jsonObj["protocol"] = "ws";
  jsonObj["address"] = "";
  jsonObj["port"] = 81;
  jsonObj["actionI"] = "on";
  jsonObj["actionO"] = "off";
  jsonObj["actionI2"] = "on2";
  jsonObj["actionO2"] = "off2";

  jsonObj.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  handleServerHeaders();
  server_manager->send(200, "application/json", JSONmessageBuffer);
}

void handleReset()
{
  Serial.println("ESP RESET");
  handleServerHeaders();
  server_manager->send(200, "text/plain", "ESP RESET");
  delay(2000);
  WiFiManager wifiManager;
  //reset WIFI settings
  wifiManager.resetSettings();
  ESP.restart(); //ESP.reset();
  delay(2000);
}

void handleOn()
{
  Serial.println("ESP RELAY ON");
  relayState = HIGH;

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &jsonObj = jsonBuffer.createObject();
  char JSONmessageBuffer[200];

  jsonObj["state"] = true;
  jsonObj.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  handleServerHeaders();
  server_manager->send(200, "application/json", JSONmessageBuffer);
}

void handleOff()
{
  Serial.println("ESP RELAY OFF");
  relayState = LOW;

  // Reset httm
  digitalWrite(httmVcc, LOW);
  delay(50);
  httmPinLastState = digitalRead(httmOut);
  digitalWrite(httmVcc, HIGH);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &jsonObj = jsonBuffer.createObject();
  char JSONmessageBuffer[200];

  jsonObj["state"] = false;
  jsonObj.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  handleServerHeaders();
  server_manager->send(200, "application/json", JSONmessageBuffer);
}

void handleOn2()
{
  Serial.println("ESP RELAY2 ON");
  relay2State = HIGH;

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &jsonObj = jsonBuffer.createObject();
  char JSONmessageBuffer[200];

  jsonObj["state"] = true;
  jsonObj.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  handleServerHeaders();
  server_manager->send(200, "application/json", JSONmessageBuffer);
}

void handleOff2()
{
  Serial.println("ESP RELAY2 OFF");
  relay2State = LOW;

  // Reset httm 2
  digitalWrite(httm2Vcc, LOW);
  delay(50);
  httm2PinLastState = digitalRead(httm2Out);
  digitalWrite(httm2Vcc, HIGH);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &jsonObj = jsonBuffer.createObject();
  char JSONmessageBuffer[200];

  jsonObj["state"] = false;
  jsonObj.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  handleServerHeaders();
  server_manager->send(200, "application/json", JSONmessageBuffer);
}

void handleNotFound()
{
  Serial.println("NOT FOUND");
  String message = "Not Found\n\n";
  message += "URI: ";
  message += server_manager->uri();
  message += "\nMethod: ";
  message += (server_manager->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server_manager->args();
  message += "\n";
  for (uint8_t i = 0; i < server_manager->args(); i++)
  {
    message += " " + server_manager->argName(i) + ": " + server_manager->arg(i) + "\n";
  }
  handleServerHeaders();
  server_manager->send(404, "text/plain", message);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
  {
    Serial.printf("[%u] Disconnected!\n", num);
    break;
  }
  case WStype_CONNECTED:
  {
    IPAddress ip = webSocket.remoteIP(num);
    Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

    // send message to client
    webSocket.sendTXT(num, relayState == HIGH ? "on" : "off");
    webSocket.sendTXT(num, relay2State == HIGH ? "on2" : "off2");
    break;
  }
  case WStype_TEXT:
  {
    Serial.printf("[%u] action: %s\n", num, payload);

    if (String((char *)payload) == "on" || String((char *)payload) == "off")
    {
      relayState = String((char *)payload) == "on" ? HIGH : LOW;
    }
    else if (String((char *)payload) == "on2" || String((char *)payload) == "off2")
    {
      relay2State = String((char *)payload) == "on2" ? HIGH : LOW;
    }

    break;
  }
  case WStype_BIN:
  {
    Serial.printf("[%u] get binary length: %u\n", num, length);
    hexdump(payload, length);

    // send message to client
    // webSocket.sendBIN(num, payload, length);
    break;
  }
  }
}

void checkConnection()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("no connection...");
    ESP.restart(); //ESP.reset();
    delay(2000);
  }
}

void configModeCallback(WiFiManager *myWiFiManager)
{
  Serial.println("Entered config mode");
  Serial.println(String(WiFi.softAPIP()));
  //if you used auto generated SSID, print it
  Serial.println(String(myWiFiManager->getConfigPortalSSID()));
}

void setup()
{
  delay(1000);
  Serial.begin(115200);
  delay(10);

  /* httm power */
  pinMode(httmVcc, OUTPUT);
  digitalWrite(httmVcc, HIGH);
  pinMode(httm2Vcc, OUTPUT);
  digitalWrite(httm2Vcc, HIGH);
  pinMode(15, OUTPUT);      //GND for httm 2
  digitalWrite(15, LOW);    //GND for httm 2

  /* signal from httm */
  pinMode(httmOut, INPUT);
  pinMode(httm2Out, INPUT);

  /* relay */
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, relayState);
  pinMode(relay2Pin, OUTPUT);
  digitalWrite(relay2Pin, relay2State);
  delay(10);

  /* set init states */
  httmPinLastState = digitalRead(httmOut);
  httm2PinLastState = digitalRead(httm2Out);
  relayState = digitalRead(httmOut);
  relayLastState = relayState;
  relay2State = digitalRead(httm2Out);
  relay2LastState = relay2State;

  WiFiManager wifiManager;

  //reset settings - for testing
  // wifiManager.resetSettings();

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  //sets timeout for which to attempt connecting, useful if you get a lot of failed connects
  wifiManager.setConnectTimeout(connectionTimeout); //segundos

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect(wifiName, wifiPass))
  {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...");

  webSocket.begin();                 // start the websocket server
  webSocket.onEvent(webSocketEvent); // if there's an incomming websocket message, go to function 'webSocketEvent'
  Serial.println("WebSocket server started.");

  // specify the port to listen on as an argument
  server_manager.reset(new ESP8266WebServer(WiFi.localIP(), 80));
  IPAddress myIP = WiFi.localIP();
  Serial.println("AP IP address: ");
  Serial.print(myIP);
  Serial.println();
  server_manager->on("/", handleRoot);
  server_manager->on("/on", handleOn);
  server_manager->on("/off", handleOff);
  server_manager->on("/on2", handleOn2);
  server_manager->on("/off2", handleOff2);
  server_manager->on("/scan", handleScan);
  server_manager->on("/reset", handleReset);
  server_manager->onNotFound(handleNotFound);
  server_manager->begin();
}

void loop()
{
  // check connection
  checkConnection();

  delay(1);

  // constantly check for websocket events
  webSocket.loop();

  delay(1);

  // constantly check for http requests
  server_manager->handleClient();

  delay(1);

  if (httmPinLastState != digitalRead(httmOut))
  {
    Serial.println("httm state changed...");
    httmPinLastState = digitalRead(httmOut);
    relayState = digitalRead(httmOut);
  }

  delay(1);

  if (httm2PinLastState != digitalRead(httm2Out))
  {
    Serial.println("httm 2 state changed...");
    httm2PinLastState = digitalRead(httm2Out);
    relay2State = digitalRead(httm2Out);
  }

  delay(1);

  if (relayState == HIGH && relayState != relayLastState)
  {
    relayLastState = relayState;
    // turns the relay ON
    Serial.write(relON, sizeof(relON));
    digitalWrite(relayPin, HIGH);
    webSocket.broadcastTXT("on");
  }
  else if (relayState == LOW && relayState != relayLastState)
  {
    relayLastState = relayState;
    // turns the relay OFF
    Serial.write(relOFF, sizeof(relOFF));
    digitalWrite(relayPin, LOW);
    webSocket.broadcastTXT("off");
  }

  delay(1);

  if (relay2State == HIGH && relay2State != relay2LastState)
  {
    relay2LastState = relay2State;
    // turns the relay2 ON
    digitalWrite(relay2Pin, HIGH);
    webSocket.broadcastTXT("on2");
  }
  else if (relay2State == LOW && relay2State != relay2LastState)
  {
    relay2LastState = relay2State;
    // turns the relay2 OFF
    digitalWrite(relay2Pin, LOW);
    webSocket.broadcastTXT("off2");
  }

  delay(1);
}
