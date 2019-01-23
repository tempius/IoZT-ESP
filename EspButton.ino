#if defined(BUTTON)

/* ESP */
#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino

void buttonSetup()
{
    /* relay */
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, relayState);

    delay(10);

    /* set init states */
    relayState = digitalRead(httmOut);
    relayLastState = relayState;
}

void buttonLoop()
{
    if (relayState == HIGH && relayState != relayLastState)
    {
        relayLastState = relayState;
        // turns the relay ON
        Serial.write(relON, sizeof(relON));
        digitalWrite(relayPin, HIGH);
        handleWebSocketsBroadcast("on");
    }
    else if (relayState == LOW && relayState != relayLastState)
    {
        relayLastState = relayState;
        // turns the relay OFF
        Serial.write(relOFF, sizeof(relOFF));
        digitalWrite(relayPin, LOW);
        // send state
        handleWebSocketsBroadcast("off");
    }
    delay(1);
}

#endif