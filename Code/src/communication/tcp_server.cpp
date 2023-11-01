#include "tcp_server.h"

#include <WiFi.h>

#include "mic/debug.h"

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

      DEBUG("[TCP Server] Client disconnected\r\n");
    }
  }
  else
  {
    WiFiClient client = wifiServer.available();
    if (client && client.connected())
    {
      currentClient = client;
      clientConnected = true;

      DEBUG("[TCP Server] Client connected\r\n");
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

    DEBUG("[TCP] Received message: %s\r\n", buffer.c_str());
  }

  return buffer;
}

void tcp_message_send(String data)
{
  if (currentClient.connected())
  {
    currentClient.print(data.c_str());

#ifdef ENABLE_DEBUG
    data.trim();
    DEBUG("[TCP] Send message: %s\r\n", data.c_str());
#endif
  }
}
