# ESP32 PWM Fan Controller

![Image of final board](/media/20230403_162902.jpg)

## Description
### This project is still in development so lot of things can be untested or not working at all!

```text
ESP32 10 channel pwm fan controller with control over tcp sockets and serial, 
temperature readout from onewire temperature probes (unlimited count, board have 2 ports), 
fan control routine (automatic - 20 points temperature curves with option of multiple 
temperature inputs and one external temperature value set by control interfaces, manual) 
for 10 separated channels with rpm readout from tacho sensor and led strip control routine with 
predefined effects (some of them use the temperature probes to be able to display temperature).
```

## Features

* 10 Channels for PWM fans
  * Automatic or manual fan speed control
  * Automatic control with 20 point curves with option for multi source temperature input for each fan
* 2 Channels for one wire temperature probes
* 2 Channels for RGB adressable led strips
  * Predefined effects and some even reacts to temperature
  * Support for both 5V and 12V warriants
* Integration for Argus Monitor
* Power by molex or barrel jack (selectable by jumper when stepdown is present)

## Todo

- [x] Publish Schematics for board

## References

[ESP32 Docs](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/) \
[PlatformIO](https://platformio.org) \
[NeoPixelBus](https://github.com/Makuna/NeoPixelBus) \
[Argus Monitor](https://www.argusmonitor.com) \
[Argus Monitor Controller example](https://github.com/openfancontrol/arguscontroller) \
[DallasTemperature](https://github.com/milesburton/Arduino-Temperature-Control-Library) \
[ArduinoJson](https://github.com/bblanchon/ArduinoJson)
