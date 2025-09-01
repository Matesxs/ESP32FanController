//
// Created by Martin on 7. 7. 2025.
//

#include <WiFi.h>

#include <custom_debug.h>

bool disabled = true;

void wifiReconnect()
{
  if (WiFi.isConnected())
    WiFi.disconnect();

  if (!disabled)
    WiFi.begin();
}

void wiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  LOG_INFO("WiFi station connected");
}

void wiFiStationGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
  LOG_INFO("WiFi station got IP: %s", WiFi.localIP().toString().c_str());
}

void wiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  LOG_INFO("WiFi station disconnected");
}

void wifiInit()
{
  WiFi.onEvent(wiFiStationDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.onEvent(wiFiStationConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(wiFiStationGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
}

void wifiEnable()
{
  disabled = false;
  wifiReconnect();
}

void wifiDisable()
{
  disabled = true;
  WiFi.disconnect();
}

void wifiSetCredentials(const char* ssid, const char* password)
{
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  disabled = false;
}