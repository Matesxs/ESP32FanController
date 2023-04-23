#include "tcp_server.h"

#include <WiFi.h>

#include "settings.h"

WiFiServer wifiServer(80);
bool clientConnected = false;
WiFiClient currentClient;

void begin_tcp_server()
{
  wifiServer.begin();
}

void tcp_server_update()
{
  if (clientConnected)
  {
    if (!currentClient.connected())
    {
      currentClient.stop();
      clientConnected = false;

#ifdef DEBUG
      Serial.printf("[TCP Server] Client disconnected");
#endif
    }
  }
  else
  {
    WiFiClient client = wifiServer.available();
    if (client && client.connected())
    {
      currentClient = client;
      clientConnected = true;

#ifdef DEBUG
      Serial.printf("[TCP Server] Client connected");
#endif
    }
  }
}

bool tcp_message_available()
{
  return currentClient.available() > 0;
}

String tcp_message_read()
{
  String buffer;
  if (currentClient.connected() && currentClient.available() > 0)
  {
    buffer = currentClient.readStringUntil('\n');
    buffer.trim();

#ifdef DEBUG
    Serial.printf("[TCP] Received message: %s\r\n", buffer.c_str());
#endif
  }

  return buffer;
}

void tcp_message_send(String data)
{
  if (currentClient.connected())
  {
    currentClient.print(data.c_str());

#ifdef DEBUG
    data.trim();
    Serial.printf("[TCP] Send message: %s\r\n", data.c_str());
#endif
  }
}
