#pragma once

void wifi_setCredentials(const char* ssid, const char* password);
void wifi_setup();
void wifi_enable();
void wifi_end();
void wifi_reconnect();

const char* wifi_getSSID();
const char* wifi_getPassword();
void wifi_setSSID(const char* ssid);
void wifi_setPassword(const char* password);
