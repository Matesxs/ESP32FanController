#include "settings.h"

#include "mic/debug.h"
#include "communication/message_processing.h"

#ifdef WIFI_CONNECTION
#ifdef WIFI_CONTROL
#include "communication/tcp_server.h"
#endif
#endif

#ifdef COM_SERIAL
#ifdef USE_ARGUS_INTERFACE
#include "communication/argus_com.h"
#endif
#endif

void communication_task(void *)
{
  while (true)
  {
#ifdef COM_SERIAL
#ifdef USE_ARGUS_INTERFACE
    processArgusCommand();
#else
    if (COM_SERIAL.available())
    {
      String data = COM_SERIAL.readStringUntil('\n');
      data.trim();

      DEBUG("[Communication Serial] Received message: %s\r\n", data.c_str());

      COM_SERIAL.print(processComMessage(data));
    }
#endif
#endif

#ifdef WIFI_CONNECTION
#ifdef WIFI_CONTROL
    tcp_server_update();

    if (tcp_message_available())
    {
      String data = tcp_message_read();
      tcp_message_send(processComMessage(data));
    }
#endif
#endif

    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

void begin_communication()
{
  xTaskCreateUniversal(communication_task, "comm", 4096, NULL, 1, NULL, 0);
}
