//needed for library
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

void httpFirmwareUpdate(String url, String fingerprint)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println();
        Serial.print("get bin from: ");
        Serial.println(url);

        t_httpUpdate_return ret;
        if (fingerprint)
        {
            ret = ESPhttpUpdate.update(url, "", string2char(fingerprint));
        }
        else
        {
            ret = ESPhttpUpdate.update(url);
        }

        switch (ret)
        {
        case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            break;

        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("HTTP_UPDATE_NO_UPDATES");
            break;

        case HTTP_UPDATE_OK:
            Serial.println("HTTP_UPDATE_OK");
            break;
        }
    }
}

char *string2char(String command)
{
    if (command.length() != 0)
    {
        char *p = const_cast<char *>(command.c_str());
        return p;
    }
}
