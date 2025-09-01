//
// Created by Martin on 8. 7. 2025.
//

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <esp_task_wdt.h>

#include <WebSerialLite.h>
#include <string_helpers.h>

#include "datastores.h"
#include "settings.h"
#include "pins.h"
#include "helpers.h"

static AsyncWebServer server(80);
static AsyncEventSource events("/events");

static uint64_t lastYield = 0;

extern uint8_t detectedTemperatureSensors;
extern TemperatureData temperatureData[MAXIMUM_TEMPERATURE_SENSORS];

extern uint32_t avgFanRPM[FAN_COUNT];
extern FanControlData fanControlData[PWM_CHANNELS_COUNT];

extern double pwmToPerc(uint32_t pwm);
extern void enqueMessage(Print* source, const String& message);

constexpr char PROGMEM temperature_template[] = R"rawliteral(
<div class="card">
  <p><i class="fas fa-thermometer-half" style="color:#059e8a;"></i> Sensor %d</p><p><span class="reading"><span id="temp%d">%.1f</span> &deg;C</span></p>
</div>)rawliteral";

constexpr char PROGMEM fan_template[] = R"rawliteral(
<div class="card">
  <p><i class="fas fa-fan" style="color:#059e8a;"></i> Fan %d</p><p><span class="reading"><span id="fan%d-speed">%d</span> RPM</span><br><span class="reading"><span id="fan%d-power">%.1f</span> &percnt;</span><span class="reading"> (<span id="fan%d-power-target">%.1f</span> &percnt;)</span></p>
</div>)rawliteral";

constexpr char PROGMEM index_html[] = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v6.7.2/css/all.css" integrity="sha384-nRgPTkuX86pH8yjPJUAFuASXQSSl2/bBUiNV47vSYpKFxHJhbcrGnmlYpYJMeD7a" crossorigin="anonymous">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p { font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: #50B8B4; color: white; font-size: 1rem; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }
    .cards { max-width: 800px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); }
    .reading { font-size: 1.4rem; }
  </style>
</head>

<body>
  <div class="topnav">
    <h1>ESP32 Fan Controller</h1>
  </div>
  <div class="content small-padding">
    <div class="cards" id="temperatures">
      %TEMPERATURES%
    </div>
    <div class="cards small-padding" id="fans">
      %FANS%
    </div>
  </div>

  <style>
  .small-padding {
    padding:20px;
  }
  </style>

  <script>
  if (!!window.EventSource) {
   var source = new EventSource('/events');

   source.addEventListener('message', function(e) {
    console.log("message", e.data);
   }, false);

   source.addEventListener('open', function(e) {
    console.log("Events Connected");
   }, false);
   source.addEventListener('error', function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
   }, false);

   source.addEventListener('temperatures', function(e) {
    var data = JSON.parse(e.data);
    document.getElementById(data.id).innerHTML = data.value;
   }, false);

   source.addEventListener('fans', function(e) {
    var data = JSON.parse(e.data);
    document.getElementById(data.id + "-speed").innerHTML = data.speed;
    document.getElementById(data.id + "-power").innerHTML = data.power;
    document.getElementById(data.id + "-power-target").innerHTML = data.power_target;
   }, false);
  }
  </script>
</body>
</html>)rawliteral";

String processor(const String& variable)
{
  String finalString;

  if (variable == "TEMPERATURES")
  {
    for (uint8_t i = 0; i < detectedTemperatureSensors; i++)
    {
      finalString += formatString(temperature_template, i + 1, i, temperatureData[i].temperature);
      yieldIfNecessary(lastYield);
      esp_task_wdt_reset();
    }
  }
  else if (variable == "FANS")
  {
    for (size_t i = 0; i < FAN_COUNT; i++)
    {
      finalString += formatString(fan_template, i + 1, i, avgFanRPM[i], i, pwmToPerc(fanControlData[FAN_INDEX_TO_PWM_CHANNEL_INDEX[i]].pwmData.currentPwm), i, pwmToPerc(fanControlData[FAN_INDEX_TO_PWM_CHANNEL_INDEX[i]].pwmData.targetPwm));
      yieldIfNecessary(lastYield);
      esp_task_wdt_reset();
    }
  }

  return finalString;
}

void pollWebserver()
{
  for (uint8_t i = 0; i < detectedTemperatureSensors; i++)
  {
    JsonDocument tempData;
    tempData["id"] = formatString("temp%d", i);
    tempData["value"] = formatString("%.1f", temperatureData[i].temperature);

    String finalDataString;
    serializeJson(tempData, finalDataString);
    events.send(finalDataString,"temperatures",millis());

    yieldIfNecessary(lastYield);
    esp_task_wdt_reset();
  }

  for (size_t i = 0; i < FAN_COUNT; i++)
  {
    JsonDocument tempData;
    tempData["id"] = formatString("fan%d", i);
    tempData["speed"] = formatString("%d", avgFanRPM[i]);
    tempData["power"] = formatString("%.1f", pwmToPerc(fanControlData[FAN_INDEX_TO_PWM_CHANNEL_INDEX[i]].pwmData.currentPwm));
    tempData["power_target"] = formatString("%.1f", pwmToPerc(fanControlData[FAN_INDEX_TO_PWM_CHANNEL_INDEX[i]].pwmData.targetPwm));

    String finalDataString;
    serializeJson(tempData, finalDataString);
    events.send(finalDataString,"fans",millis());

    yieldIfNecessary(lastYield);
    esp_task_wdt_reset();
  }
}

static void recvMsg(uint8_t *data, size_t len)
{
  enqueMessage(&WebSerial, {data, len});
}

static void onConsoleConnect(AsyncWebSocketClient* client)
{
  client->printf("To get help use command \"HELP\"\n");
}

void webserverInit()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html, processor);
  });

  events.onConnect([](AsyncEventSourceClient *client){
    client->send("hello!", nullptr, millis(), 10000);
  });

  server.addHandler(&events);
  server.begin();

  // Start web console
  WebSerial.onMessage(recvMsg);
  WebSerial.onConnect(onConsoleConnect);

  WebSerial.begin(&server);
}
