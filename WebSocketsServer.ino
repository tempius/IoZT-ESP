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

        StaticJsonBuffer<300> jsonBuffer;
        JsonObject &jsonObj = jsonBuffer.createObject();
        char JSONmessageBuffer[300];

        jsonObj["state"] = relayState == HIGH ? "on" : "off";
        jsonObj["state2"] = relay2State == HIGH ? "on" : "off";
        jsonObj["name"] = wifiNameString;
        jsonObj["esp"] = ESP.getChipId();
        jsonObj["firmware"] = FIRMWARE_VERSION;

        // send message to client
        jsonObj.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
        webSocket.sendTXT(num, JSONmessageBuffer);
        break;
    }
    case WStype_TEXT:
    {
        Serial.printf("[%u] action: %s\n", num, payload);

        StaticJsonBuffer<300> jsonBuffer;
        JsonObject &jsonObj = jsonBuffer.parseObject(payload);

        if (jsonObj["type"] == "state")
        {
            relayState = jsonObj["msg"] == "on" ? HIGH : LOW;
        }
        else if (jsonObj["type"] == "state2")
        {
            relay2State = jsonObj["msg"] == "on" ? HIGH : LOW;
        }
        else if (jsonObj["type"] == "update")
        {
            httpFirmwareUpdate(jsonObj["msg"], jsonObj["fingerprint"]);
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

void handleWebSocketsBroadcast(String type, String msg)
{
    StaticJsonBuffer<300> jsonBuffer;
    JsonObject &jsonObj = jsonBuffer.createObject();
    char JSONmessageBuffer[300];

    jsonObj[type] = msg;
    jsonObj["name"] = wifiNameString;
    jsonObj["esp"] = ESP.getChipId();
    jsonObj["firmware"] = FIRMWARE_VERSION;

    // send message to client
    jsonObj.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    webSocket.broadcastTXT(JSONmessageBuffer);
}
