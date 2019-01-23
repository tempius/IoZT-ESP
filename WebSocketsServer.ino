//needed for library
#include <WebSocketsServer.h>

// Create an instance of the server
WebSocketsServer webSocket = WebSocketsServer(81);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
    {
        Serial.printf("[%u] Disconnected!\n", num);
        break;
    }
    case WStype_CONNECTED:
    {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, relayState == HIGH ? "on" : "off");
        webSocket.sendTXT(num, relay2State == HIGH ? "on2" : "off2");
        break;
    }
    case WStype_TEXT:
    {
        Serial.printf("[%u] action: %s\n", num, payload);

        if (String((char *)payload) == "on" || String((char *)payload) == "off")
        {
            relayState = String((char *)payload) == "on" ? HIGH : LOW;
        }
        else if (String((char *)payload) == "on2" || String((char *)payload) == "off2")
        {
            relay2State = String((char *)payload) == "on2" ? HIGH : LOW;
        }

        break;
    }
    case WStype_BIN:
    {
        Serial.printf("[%u] get binary length: %u\n", num, length);
        hexdump(payload, length);

        // send message to client
        // webSocket.sendBIN(num, payload, length);
        break;
    }
    }
}

void setupWebSocketsServer()
{
    webSocket.begin();                 // start the websocket server
    webSocket.onEvent(webSocketEvent); // if there's an incomming websocket message, go to function 'webSocketEvent'
    Serial.println("WebSocket server started.");
}

void handleWebSockets()
{
    webSocket.loop();
}

void handleWebSocketsBroadcast(String msg)
{
    webSocket.broadcastTXT(msg);
}
