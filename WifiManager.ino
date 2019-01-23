//needed for library
#include <DNSServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager

void setupWifiManager()
{
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
}

void configModeCallback(WiFiManager *myWiFiManager)
{
    Serial.println("Entered config mode");
    Serial.println(String(WiFi.softAPIP()));
    //if you used auto generated SSID, print it
    Serial.println(String(myWiFiManager->getConfigPortalSSID()));
}

void handleWifiManagerReset()
{
    WiFiManager wifiManager;
    //reset WIFI settings
    wifiManager.resetSettings();
}