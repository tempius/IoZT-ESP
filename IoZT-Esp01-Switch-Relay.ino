/* ESP */
#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>
#include <WebSocketsServer.h>

#define DEBUG
#define BUILTIN_LED 1

byte relON[] = {0xA0, 0x01, 0x01, 0xA2};  //Hex command to send to serial for open relay
byte relOFF[] = {0xA0, 0x01, 0x00, 0xA1}; //Hex command to send to serial for close relay
/* FLAGS de Controlo */
int relayState = LOW;
int relayLastState = LOW;
int httmPinLastState = LOW;
int connectionTimeout = 180;

/* constantes */
const char *wifiName = "ESP-01#01";
const char *wifiPass = "esp8266-01";
int txPin = 1;   //Tx = GPIO 1
int httmOut = 3; // GPIO 3 = Rx - receives signal from httm
int httmVcc = 2; //GPIO 2 - used to power up the httm

// Create an instance of the server
std::unique_ptr<ESP8266WebServer> server_manager;
WebSocketsServer webSocket = WebSocketsServer(81);

void handleRoot()
{
  debugln("ESP ROOT");
  server_manager->send(200, "text/plain", String(wifiName));
}

void handleScan()
{
  debugln("ESP SCAN");

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &jsonObj = jsonBuffer.createObject();
  char JSONmessageBuffer[200];

  jsonObj["componentType"] = "switch";
  jsonObj["componentName"] = String(wifiName);
  jsonObj["protocol"] = "ws";
  jsonObj["address"] = "";
  jsonObj["port"] = 81;
  jsonObj["actionI"] = "on";
  jsonObj["actionO"] = "off";

  jsonObj.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  server_manager->send(200, "application/json", JSONmessageBuffer);
}

void handleOn()
{
  debugln("ESP RELAY ON");
  relayState = HIGH;

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &jsonObj = jsonBuffer.createObject();
  char JSONmessageBuffer[200];

  jsonObj["state"] = true;
  jsonObj.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  server_manager->send(200, "application/json", JSONmessageBuffer);
}

void handleOff()
{
  debugln("ESP RELAY OFF");
  relayState = LOW;
  // Reset httm
  digitalWrite(httmVcc, LOW);
  delay(1);
  httmPinLastState = digitalRead(httmOut);
  digitalWrite(httmVcc, HIGH);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &jsonObj = jsonBuffer.createObject();
  char JSONmessageBuffer[200];

  jsonObj["state"] = false;
  jsonObj.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  server_manager->send(200, "application/json", JSONmessageBuffer);
}

void handleNotFound()
{
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
  server_manager->send(404, "text/plain", message);
}

void configModeCallback(WiFiManager *myWiFiManager)
{
  debugln("Entered config mode");
  debugln(String(WiFi.softAPIP()));
  //if you used auto generated SSID, print it
  debugln(String(myWiFiManager->getConfigPortalSSID()));
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
#ifdef DEBUG
    Serial.printf("[%u] Disconnected!\n", num);
#endif
    break;
  case WStype_CONNECTED:
  {
    IPAddress ip = webSocket.remoteIP(num);
#ifdef DEBUG
    Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
#endif

    // send message to client
    webSocket.sendTXT(num, relayState == HIGH ? "on" : "off");
  }
  break;
  case WStype_TEXT:
#ifdef DEBUG
    Serial.printf("[%u] get Text: %s\n", num, payload);
#endif

    relayState = String((char *)payload) == "on" ? HIGH : LOW;
    break;
  case WStype_BIN:
#ifdef DEBUG
    Serial.printf("[%u] get binary length: %u\n", num, length);
#endif
    hexdump(payload, length);

    // send message to client
    // webSocket.sendBIN(num, payload, length);
    break;
  }
}

void setup()
{
  Serial.begin(9600);
  delay(10);

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
    debugln("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  //if you get here you have connected to the WiFi
  debugln("connected...");

  webSocket.begin();                 // start the websocket server
  webSocket.onEvent(webSocketEvent); // if there's an incomming websocket message, go to function 'webSocketEvent'
  debugln("WebSocket server started.");

  // specify the port to listen on as an argument
  server_manager.reset(new ESP8266WebServer(WiFi.localIP(), 80));

  server_manager->on("/", handleRoot);
  server_manager->on("/on", handleOn);
  server_manager->on("/off", handleOff);
  server_manager->on("/scan", handleScan);
  server_manager->onNotFound(handleNotFound);
  server_manager->begin();

  /* INPUTS */

  /* httm power */
  pinMode(httmVcc, OUTPUT);
  digitalWrite(httmVcc, HIGH);

  /* signal from httm */
  pinMode(httmOut, INPUT);

  /* led do esp-01 */
  //pinMode(BUILTIN_LED, OUTPUT);
  //digitalWrite(BUILTIN_LED, HIGH);
}

void checkConnection()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    debugln("no connection...");
    ESP.restart(); //ESP.reset();
    delay(2000);
  }
}

void loop()
{
  // check connection
  checkConnection();

  // constantly check for websocket events
  webSocket.loop();

  // constantly check for http requests
  server_manager->handleClient();

checkHttmSignal:
  if (httmPinLastState != digitalRead(httmOut))
  {
    httmPinLastState = digitalRead(httmOut);
    relayState = digitalRead(httmOut);
  }
  if (relayState == HIGH && relayState != relayLastState)
  {
    relayLastState = relayState;
    Serial.write(relON, sizeof(relON)); // turns the relay ON
    //digitalWrite(BUILTIN_LED, LOW);
    webSocket.broadcastTXT("on");
  }
  else if (relayState == LOW && relayState != relayLastState)
  {
    relayLastState = relayState;
    Serial.write(relOFF, sizeof(relOFF)); // turns the relay OFF
    //digitalWrite(BUILTIN_LED, HIGH);
    webSocket.broadcastTXT("off");
  }
}

void debug(String msg)
{
#ifdef DEBUG
  Serial.print(msg);
#endif
}

void debugln(String msg)
{
#ifdef DEBUG
  Serial.println(msg);
#endif
}
