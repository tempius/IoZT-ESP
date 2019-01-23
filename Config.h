/* ESP */
#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>

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
int httm2Out = 14; // GPIO 14 = receives signal from httm 2
int httm2Vcc = 16; // GPIO 16 - used to power up the httm 2
