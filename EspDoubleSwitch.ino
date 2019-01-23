#if defined(DOUBLE_SWITCH)

/* ESP */
#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino

void doubleSwitchSetup()
{
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
}

void doubleSwitchLoop()
{
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

    if (relay2State == HIGH && relay2State != relay2LastState)
    {
        relay2LastState = relay2State;
        // turns the relay2 ON
        digitalWrite(relay2Pin, HIGH);
        handleWebSocketsBroadcast("on2");
    }
    else if (relay2State == LOW && relay2State != relay2LastState)
    {
        relay2LastState = relay2State;
        // turns the relay2 OFF
        digitalWrite(relay2Pin, LOW);
        // Reset httm
        digitalWrite(httm2Vcc, LOW);
        delay(1);
        httm2PinLastState = digitalRead(httm2Out);
        digitalWrite(httm2Vcc, HIGH);
        // send state
        handleWebSocketsBroadcast("off2");
    }
    delay(1);
}

#endif