#include <pwmControl.h>

#include <esp32-hal.h>
#include <driver/ledc.h>
#include <soc/soc_caps.h>

#include <custom_debug.h>

namespace PWMControl
{
  #ifdef SOC_LEDC_SUPPORT_HS_MODE
  #define NUMBER_OF_CHANNELS (SOC_LEDC_CHANNEL_NUM<<1)
  #define NUMBER_OF_TIMERS (SOC_TIMER_GROUP_TOTAL_TIMERS<<1)
  #else
  #define NUMBER_OF_CHANNELS (SOC_LEDC_CHANNEL_NUM)
  #define NUMBER_OF_TIMERS (SOC_TIMER_GROUP_TOTAL_TIMERS)
  #endif

  struct TimerData
  {
    uint8_t enabled:1 = false;
    uint8_t resolution = 0;
    uint8_t frequency = 0;
  };

  struct ChannelData
  {
    size_t attachedTimer = NUMBER_OF_TIMERS;
  };

  TimerData timerData[NUMBER_OF_TIMERS];
  ChannelData channelData[NUMBER_OF_CHANNELS];

  uint32_t setupTimer(const uint8_t timer, const uint8_t resolution, const uint32_t freq)
  {
    if (timer >= NUMBER_OF_TIMERS)
    {
      LOG_ERROR("Invalid timer (%u), total count is %u", timer, NUMBER_OF_TIMERS);
      return 0;
    }

    if (resolution > SOC_LEDC_TIMER_BIT_WIDE_NUM)
    {
      LOG_ERROR("Invalid resolution (%u), maximum is %u", resolution, SOC_LEDC_TIMER_BIT_WIDE_NUM);
      return 0;
    }

    const ledc_mode_t mode = timer >= SOC_TIMER_GROUP_TOTAL_TIMERS ? LEDC_HIGH_SPEED_MODE : LEDC_LOW_SPEED_MODE;

    ledc_timer_config_t timer_config;
    timer_config.speed_mode = mode;
    timer_config.clk_cfg = ledc_clk_cfg_t::LEDC_AUTO_CLK;
    timer_config.duty_resolution = static_cast<ledc_timer_bit_t>(resolution);
    timer_config.timer_num = static_cast<ledc_timer_t>(timer % SOC_TIMER_GROUP_TOTAL_TIMERS);
    timer_config.freq_hz = freq;

    if(ledc_timer_config(&timer_config) != ESP_OK)
    {
      LOG_ERROR("Failed to setup PWM timer");
      return 0;
    }

    timerData[timer].frequency = freq;
    timerData[timer].resolution = resolution;
    timerData[timer].enabled = true;
    return ledc_get_freq(mode, static_cast<ledc_timer_t>(timer % SOC_TIMER_GROUP_TOTAL_TIMERS));
  }

  uint8_t getResolution(const uint8_t timer)
  {
    if (timer >= NUMBER_OF_TIMERS)
    {
      LOG_ERROR("Invalid timer (%u), total count is %u", timer, NUMBER_OF_TIMERS);
      return 0;
    }

    if (!timerData[timer].enabled)
    {
      LOG_ERROR("Timer %u not initialized", timer);
      return 0;
    }

    return timerData[timer].resolution;
  }

  uint32_t getMaxValue(const uint8_t timer)
  {
    if (timer >= NUMBER_OF_TIMERS)
    {
      LOG_ERROR("Invalid timer (%u), total count is %u", timer, NUMBER_OF_TIMERS);
      return 0;
    }

    if (!timerData[timer].enabled)
    {
      LOG_ERROR("Timer %u not initialized", timer);
      return 0;
    }

    return static_cast<uint32_t>(pow(2, timerData[timer].resolution) - 1);
  }

  uint32_t readFreq(const uint8_t timer)
  {
    if (timer >= NUMBER_OF_TIMERS)
    {
      LOG_ERROR("Invalid timer (%u), total count is %u", timer, NUMBER_OF_TIMERS);
      return 0;
    }

    return timerData[timer].frequency;
  }

  void attachPin(const uint8_t timer, const uint8_t channel, const uint8_t pin, const bool invert)
  {
    if (timer >= NUMBER_OF_TIMERS)
    {
      LOG_ERROR("Invalid timer (%u), total count is %u", timer, NUMBER_OF_TIMERS);
      return;
    }

    if (!timerData[timer].enabled)
    {
      LOG_ERROR("Timer %u not initialized", timer);
      return;
    }

    if (channel >= NUMBER_OF_CHANNELS)
    {
      LOG_ERROR("Invalid channel (%u), total count is %u", channel, SOC_LEDC_CHANNEL_NUM);
      return;
    }

    const ledc_mode_t mode = timer >= SOC_TIMER_GROUP_TOTAL_TIMERS ? LEDC_HIGH_SPEED_MODE : LEDC_LOW_SPEED_MODE;
    if ((mode == LEDC_HIGH_SPEED_MODE && channel < SOC_LEDC_CHANNEL_NUM) || (mode == LEDC_LOW_SPEED_MODE && channel >= SOC_LEDC_CHANNEL_NUM))
    {
      LOG_ERROR("Connecting low speed channel (%u) to high speed timer (%u) or high speed channel to low speed timer", channel, timer);
      LOG_ERROR("High speed timers start at index %u and high speed channels start at index %u", SOC_TIMER_GROUP_TOTAL_TIMERS, SOC_LEDC_CHANNEL_NUM);
      return;
    }

    // Detach if its attached
    detachPin(pin);

    const uint32_t duty = ledc_get_duty(mode, static_cast<ledc_channel_t>(channel % SOC_LEDC_CHANNEL_NUM));

    ledc_channel_config_t channel_config;
    channel_config.speed_mode = mode;
    channel_config.intr_type = ledc_intr_type_t::LEDC_INTR_DISABLE;
    channel_config.timer_sel = static_cast<ledc_timer_t>(timer % SOC_TIMER_GROUP_TOTAL_TIMERS);
    channel_config.duty = duty;
    channel_config.hpoint = 0;
    channel_config.gpio_num = pin;
    channel_config.channel = static_cast<ledc_channel_t>(channel % SOC_LEDC_CHANNEL_NUM);
    channel_config.flags.output_invert = invert;

    ledc_channel_config(&channel_config);

    channelData[channel].attachedTimer = timer;
  }

  void detachPin(const uint8_t pin)
  {
    pinMatrixOutDetach(pin, false, false);
  }

  uint32_t read(const uint8_t channel)
  {
    if (channel >= NUMBER_OF_CHANNELS)
    {
      LOG_ERROR("Invalid channel (%u), total count is %u", channel, NUMBER_OF_CHANNELS);
      return 0;
    }

    if (channelData[channel].attachedTimer >= NUMBER_OF_TIMERS)
    {
      LOG_ERROR("Channel (%u) not attached to any timer", channel);
      return 0;
    }

    const ledc_mode_t mode = channelData[channel].attachedTimer >= SOC_TIMER_GROUP_TOTAL_TIMERS ? LEDC_HIGH_SPEED_MODE : LEDC_LOW_SPEED_MODE;
    const uint32_t max_duty = (1 << timerData[channelData[channel].attachedTimer].resolution) - 1;
    return std::min(max_duty, ledc_get_duty(mode, static_cast<ledc_channel_t>(channel % SOC_LEDC_CHANNEL_NUM)));
  }

  void write(const uint8_t channel, uint32_t duty)
  {
    if (channel >= NUMBER_OF_CHANNELS)
    {
      LOG_ERROR("Invalid channel (%u), total count is %u", channel, NUMBER_OF_CHANNELS);
      return;
    }

    if (channelData[channel].attachedTimer >= NUMBER_OF_TIMERS)
    {
      LOG_ERROR("Channel (%u) not attached to any timer", channel);
      return;
    }

    const uint32_t max_duty = (1 << timerData[channelData[channel].attachedTimer].resolution) - 1;

    if (duty > max_duty)
    {
      LOG_ERROR("Invalid duty (%u) for channel %u, maximum value is %u", duty, channel, max_duty);
      return;
    }

    // DEBUG("Set %u to %u (max %u)", channel, duty, max_duty);

    // Duty fix
    if((duty == max_duty) && (max_duty != 1))
      duty = max_duty + 1;

    // DEBUG("Set fixed %u to %u (max %u)", channel, duty, max_duty);

    const ledc_mode_t mode = channelData[channel].attachedTimer >= SOC_TIMER_GROUP_TOTAL_TIMERS ? LEDC_HIGH_SPEED_MODE : LEDC_LOW_SPEED_MODE;
    ledc_set_duty(mode, static_cast<ledc_channel_t>(channel % SOC_LEDC_CHANNEL_NUM), duty);
    ledc_update_duty(mode, static_cast<ledc_channel_t>(channel % SOC_LEDC_CHANNEL_NUM));
  }
}
