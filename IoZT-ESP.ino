// #define BUTTON
// #define SWITCH
#define DOUBLE_SWITCH

#include "Config.h"

void setup()
{
  delay(1000);
  Serial.begin(115200);
  delay(10);

#if defined(BUTTON)
  buttonSetup();
#elif defined(SWITCH)
  switchSetup();
#elif defined(DOUBLE_SWITCH)
  doubleSwitchSetup();
#endif

  setupWifiManager();

  setupWebSocketsServer();

  setupWebserver();
}

void loop()
{
  // check connection
  checkConnection();
  delay(1);

  // constantly check for websocket events
  handleWebSockets();
  delay(1);

  // constantly check for http requests
  handleClient();
  delay(1);

#if defined(BUTTON)
  buttonLoop();
#elif defined(SWITCH)
  switchLoop();
#elif defined(DOUBLE_SWITCH)
  doubleSwitchLoop();
#endif
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
