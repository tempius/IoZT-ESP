//needed for library
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

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

#if defined(BUTTON)
  server_manager->on("/scan", handleScanButton);
#elif defined(SWITCH)
  server_manager->on("/scan", handleScanSwitch);
#elif defined(DOUBLE_SWITCH)
  server_manager->on("/scan", handleScanDoubleSwitch);
#endif

  server_manager->on("/reset", handleReset);
  server_manager->onNotFound(handleNotFound);
  server_manager->begin();
}

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
  connected to this access point to see it.
*/
void handleRoot()
{
  Serial.println("ESP ROOT");

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &jsonObj = jsonBuffer.createObject();
  char JSONmessageBuffer[200];

  jsonObj["wifi"] = wifiNameString;
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
  handleWifiManagerReset();
  ESP.restart(); //ESP.reset();
  delay(2000);
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

#if defined(BUTTON)
void handleScanButton()
{
  Serial.println("ESP SCAN");
  StaticJsonBuffer<250> jsonBuffer;
  JsonObject &jsonObj = jsonBuffer.createObject();
  char JSONmessageBuffer[250];

  jsonObj["componentType"] = "button";
  jsonObj["componentName"] = wifiNameString;
  jsonObj["protocol"] = "ws";
  jsonObj["address"] = "";
  jsonObj["port"] = 81;
  jsonObj["actionI"] = "on";

  jsonObj.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  handleServerHeaders();
  server_manager->send(200, "application/json", JSONmessageBuffer);
}
#elif defined(SWITCH)
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
#elif defined(DOUBLE_SWITCH)
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
#endif

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

void handleClient()
{
  // constantly check for http requests
  server_manager->handleClient();
}