// #define ESP_SWITCH
#define ESP_DOUBLE_SWITCH

#include "Config.h"

// Create an instance of the server
WebSocketsServer webSocket = WebSocketsServer(81);

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
  pinMode(15, OUTPUT);   //GND for httm 2
  digitalWrite(15, LOW); //GND for httm 2

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

  setupWebserver();
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
  handleClient();

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
