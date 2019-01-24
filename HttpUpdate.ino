//needed for library
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

void updateFirmware(String url)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.print("get bin from: ");
        Serial.println(url);

        // t_httpUpdate_return ret = ESPhttpUpdate.update(url);
        // //t_httpUpdate_return  ret = ESPhttpUpdate.update("https://server/file.bin", "", "fingerprint");

        // switch (ret)
        // {
        // case HTTP_UPDATE_FAILED:
        //     Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        //     break;

        // case HTTP_UPDATE_NO_UPDATES:
        //     Serial.println("HTTP_UPDATE_NO_UPDATES");
        //     break;

        // case HTTP_UPDATE_OK:
        //     Serial.println("HTTP_UPDATE_OK");
        //     break;
        // }
    }
}