#include "wifiHandling.h"

#include <Arduino.h>
#include <WiFi.h>

#include "settings.h"

bool disabled = true;
bool connected = false;
String wifi_ssid;
String wifi_password;

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
#ifdef DEBUG
  Serial.printf("[WIFI] Station connected, received ip address %s\r\n", WiFi.localIP().toString().c_str());
#endif

  connected = true;
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
#ifdef DEBUG
  Serial.printf("[WIFI] Station disconnected\r\n");
#endif

  connected = false;

  if (!disabled)
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
}

void wifi_set_credentials(const char* ssid, const char* password)
{
  wifi_ssid = String(ssid);
  wifi_password = String(password);

  if (WiFi.isConnected())
    WiFi.disconnect();

  if (wifi_ssid != "")
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
}

void wifi_setup(const char* ssid, const char* password)
{
  WiFi.onEvent(WiFiGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiStationDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  WiFi.mode(WIFI_STA);
  disabled = false;
  wifi_set_credentials(ssid, password);
}

void wifi_end()
{
  disabled = true;
  WiFi.disconnect();
}
