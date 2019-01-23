#if defined(SWITCH)

/* ESP */
#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino

void switchSetup()
{
    /* httm power */
    pinMode(httmVcc, OUTPUT);
    digitalWrite(httmVcc, HIGH);

    /* signal from httm */
    pinMode(httmOut, INPUT);

    /* relay */
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, relayState);

    delay(10);

    /* set init states */
    httmPinLastState = digitalRead(httmOut);
    relayState = digitalRead(httmOut);
    relayLastState = relayState;
}

void switchLoop()
{
    if (httmPinLastState != digitalRead(httmOut))
    {
        Serial.println("httm state changed...");
        httmPinLastState = digitalRead(httmOut);
        relayState = digitalRead(httmOut);
    }
    delay(1);

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
        // Reset httm
        digitalWrite(httmVcc, LOW);
        delay(1);
        httmPinLastState = digitalRead(httmOut);
        digitalWrite(httmVcc, HIGH);
        // send state
        handleWebSocketsBroadcast("off");
    }
    delay(1);
}

#endif