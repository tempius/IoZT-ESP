#include <ESP8266WebServer.h>

// Create an instance of the server
std::unique_ptr<ESP8266WebServer> server_manager;

void setupWebserver()
{
  // specify the port to listen on as an argument
  server_manager.reset(new ESP8266WebServer(WiFi.localIP(), 80));
  IPAddress myIP = WiFi.localIP();
  Serial.println("AP IP address: ");
  Serial.print(myIP);
  Serial.println();
  server_manager->on("/", handleRoot);
  server_manager->on("/on", handleOn);
  server_manager->on("/off", handleOff);
#ifdef defined(ESP_SWITCH)
  server_manager->on("/scan", handleScanSwitch);
#elif defined(ESP_DOUBLE_SWITCH)
  server_manager->on("/on2", handleOn2);
  server_manager->on("/off2", handleOff2);
  server_manager->on("/scan", handleScanDoubleSwitch);
#endif
  server_manager->on("/reset", handleReset);
  server_manager->onNotFound(handleNotFound);
  server_manager->begin();
}

void handleClient()
{
  // constantly check for http requests
  server_manager->handleClient();
}

void handleScanSwitch()
{
  Serial.println("ESP SCAN");
  StaticJsonBuffer<250> jsonBuffer;
  JsonObject &jsonObj = jsonBuffer.createObject();
  char JSONmessageBuffer[250];

  jsonObj["componentType"] = "switch";
  jsonObj["componentName"] = wifiNameString;
  jsonObj["protocol"] = "ws";
  jsonObj["address"] = "";
  jsonObj["port"] = 81;
  jsonObj["actionI"] = "on";
  jsonObj["actionO"] = "off";

  jsonObj.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  handleServerHeaders();
  server_manager->send(200, "application/json", JSONmessageBuffer);
}

void handleScanDoubleSwitch()
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