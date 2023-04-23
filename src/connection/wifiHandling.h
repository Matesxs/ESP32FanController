#pragma once

void wifi_set_credentials(const char* ssid, const char* password);
void wifi_setup(const char* ssid = "", const char* password = "");
void wifi_end();
