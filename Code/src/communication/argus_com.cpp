#include "argus_com.h"

#include "temperature_reader.h"
#include "mic/data_share.h"
#include "settings.h"

AMCOM amCom;
uint8_t buffer[20];

void begin_argus_com()
{
  amCom.setDetails(ARGUS_DEVICE_ID, min(temp_reader_sensor_count(), static_cast<uint8_t>(4)), 2);
}

void processArgusCommand()
{
  amCom.delay(50);

  if (amCom.queueCount() > 0)
  {
    uint32_t qdata = amCom.queuePop();
    uint8_t cmd = qdata & 0xFF;
    uint8_t channel;
    switch (cmd)
    {
    case AMAC_CMD::CmdGetTemp:
      buffer[0] = cmd;
      buffer[1] = min(temp_reader_sensor_count(), static_cast<uint8_t>(4));
      for (uint8_t i = 0; i < min(temp_reader_sensor_count(), static_cast<uint8_t>(4)); i++)
      {
        int16_t temperature = static_cast<int16_t>(temperatures[i] * 10);
        buffer[2 + i * 2] = temperature >> 8;
        buffer[3 + i * 2] = temperature & 0xFF;
      }
      amCom.send(buffer, 2 + 2 * min(temp_reader_sensor_count(), static_cast<uint8_t>(4)));
      break;
    case AMAC_CMD::CmdGetFanRpm:
      buffer[0] = cmd;
      buffer[1] = 2;
      for (uint8_t i = 0; i < 2; i++)
      {
        uint16_t rpm = 0;
        for (size_t y = (FAN_COUNT / 2) * i; y < (FAN_COUNT / 2) * (i + 1); y++)
          rpm = max(fanData[y].rpm, rpm);

        buffer[2 + i * 2] = rpm >> 8;
        buffer[3 + i * 2] = rpm & 0xFF;
      }
      amCom.send(buffer, 6);
      break;
    case AMAC_CMD::CmdGetFanPwm:
      channel = (qdata >> 8) & 0xFF;
      buffer[0] = cmd;
      buffer[1] = channel;
      buffer[2] = static_cast<uint8_t>(fanData[static_cast<size_t>((FAN_COUNT / 2) * channel)].currentPower);
      amCom.send(buffer, 3);
      break;
    case AMAC_CMD::CmdSetFanPwm:
    {
      channel = (qdata >> 8) & 0xFF;
      uint8_t pwm = (qdata >> 16) & 0xFF;
      if (pwm <= 100 && pwm >= 0)
      {
        for (size_t i = (FAN_COUNT / 2) * channel; i < (FAN_COUNT / 2) * (channel + 1); i++)
        {
          fanData[i].lerper.set(static_cast<double>(pwm), FAN_SPEED_RAMP_TIME_MS);
          fanData[i].manual = true;
        }
        buffer[0] = cmd; // ok code
      }
      else
      {
        buffer[0] = 0xFF;
      }
      amCom.send(buffer, 1);
      break;
    }
    case AMAC_CMD::CmdEEReadByte:
    {
      buffer[0] = 0xFF;
      amCom.send(buffer, 1);
      break;
    }
    case AMAC_CMD::CmdEEWriteByte:
    {
      buffer[0] = 0xFF;
      amCom.send(buffer, 1);
      break;
    }
    default:
      break;
    }
  }
}
