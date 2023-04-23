#pragma once

#include <Arduino.h>

void begin_tcp_server();
void tcp_server_update();
bool tcp_message_available();
String tcp_message_read();
void tcp_message_send(String data);
