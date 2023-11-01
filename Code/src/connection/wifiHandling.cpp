#include "wifiHandling.h"

#include <Arduino.h>
#include <WiFi.h>

#include "settings.h"

bool disabled = true;
String wifi_ssid;
String wifi_password;

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  wifi_reconnect();
}

void wifi_setup()
{
  WiFi.onEvent(WiFiStationDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.mode(WIFI_STA);
}

void wifi_enable()
{
  disabled = false;
  wifi_reconnect();
}

void wifi_end()
{
  disabled = true;
  WiFi.disconnect();
}

void wifi_reconnect()
{
  if (WiFi.isConnected())
    WiFi.disconnect();

  if (wifi_ssid != "" && !disabled)
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
}

void wifi_setCredentials(const char* ssid, const char* password)
{
  wifi_ssid = String(ssid);
  wifi_password = String(password);

  wifi_reconnect();
}

const char* wifi_getSSID()
{
  return wifi_ssid.c_str();
}

const char* wifi_getPassword()
{
  return wifi_password.c_str();
}

void wifi_setSSID(const char* ssid)
{
  wifi_ssid = String(ssid);
  wifi_reconnect();
}

void wifi_setPassword(const char* password)
{
  wifi_password = String(password);
  wifi_reconnect();
}
