//
// Created by Martin on 7. 7. 2025.
//

#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

void wifiInit();
void wifiReconnect();
void wifiSetCredentials(const char* ssid, const char* password = nullptr);
void wifiEnable();
void wifiDisable();

#endif //WIFI_HANDLER_H
