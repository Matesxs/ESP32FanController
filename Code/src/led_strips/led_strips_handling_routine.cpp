#include <Arduino.h>

#include "mic/data_share.h"

void render_led_strips()
{
  if (ledStripChannel1 != NULL)
    ledStripChannel1->render();

  if (ledStripChannel2 != NULL)
    ledStripChannel2->render();
}

void led_strips_task(void *)
{
  while (true)
  {
    render_led_strips();
    vTaskDelay(10);
  }
}

void begin_led_strips()
{
  xTaskCreateUniversal(led_strips_task, "led_strips", 4096, NULL, 1, NULL, 1);
}
