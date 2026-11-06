#include <Arduino.h>
#include <WS2812FX.h>
#include <vector>
#include <algorithm>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif

// Defaults can be overridden per board with build_flags in platformio.ini
#ifndef LED_COUNT
#define LED_COUNT 193
#endif
#ifndef LED_PIN
#define LED_PIN 11
#endif
#ifndef LED_COLOR_ORDER
#define LED_COLOR_ORDER NEO_RGB
#endif
#ifndef LED_BRIGHTNESS
#define LED_BRIGHTNESS 255
#endif

#define TIMER_MS 20000

// Uncomment the following line to enable debug logging
#define DEBUG

#ifdef DEBUG
#define LOG_INIT() Serial.begin(9600)
#define LOG_PRINTLN(msg) Serial.println(msg)
#else
#define LOG_INIT()       // No-op
#define LOG_PRINTLN(msg) // No-op
#endif

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, LED_COLOR_ORDER + NEO_KHZ800);

#ifdef ESP8266
// These need more memory than the Arduino boards have
#include "DiwaliPatterns.h"
#endif

unsigned long last_change = 0;
unsigned long now = 0;
uint8_t selectedMode = 0;
int colorWheelValues[] = {0, 50, 100, 150, 200, 250};

std::vector<int> usedModes;

// Picks a random number below count, using all of them before any repeats
int pickUnused(std::vector<int> &used, int count)
{
  if ((int)used.size() >= count)
  {
    used.clear();
  }

  int picked;
  do
  {
    picked = random(0, count);
  } while (std::find(used.begin(), used.end(), picked) != used.end());
  used.push_back(picked);
  return picked;
}

void setup()
{
  LOG_INIT();
  LOG_PRINTLN("Setup started");

#ifdef ESP8266
  // Wi-Fi interrupts disturb the WS2812 signal timing and it isn't used here
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
#endif

  ws2812fx.init();
  ws2812fx.setBrightness(LED_BRIGHTNESS);
  ws2812fx.setSpeed(1000);
  // generate a random color
  randomSeed(analogRead(A0));
  int randomIndex = random(0, 6);
  uint32_t color = ws2812fx.color_wheel(colorWheelValues[randomIndex]);
  LOG_PRINTLN("Color: " + String(color));
  ws2812fx.setColor(color);
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.start();
}

void loop()
{
  now = millis();

  ws2812fx.service();

  if (now - last_change > TIMER_MS)
  {
    int randomIndex = random(0, 6);
    uint32_t color = ws2812fx.color_wheel(colorWheelValues[randomIndex]);
    LOG_PRINTLN("Color: " + String(color));
    ws2812fx.setColor(color);

    // Select a new mode that hasn't been used yet
    int builtInCount = ws2812fx.getModeCount() - 8;
#ifdef ESP8266
    // The Diwali patterns share the shuffle, numbered after the built-in modes
    int picked = pickUnused(usedModes, builtInCount + DIWALI_PATTERN_COUNT);
    if (picked >= builtInCount)
    {
      const DiwaliPattern &pattern = diwaliPatterns[picked - builtInCount];
      LOG_PRINTLN("Pattern: " + String(pattern.name));
      ws2812fx.setCustomMode(pattern.run);
      ws2812fx.setMode(FX_MODE_CUSTOM);
      last_change = now;
      return;
    }
    selectedMode = picked;
#else
    selectedMode = pickUnused(usedModes, builtInCount);
#endif

    // selectedMode = (ws2812fx.getMode() + 1) % (ws2812fx.getModeCount() - 8);
    LOG_PRINTLN("Mode: " + String(selectedMode));
    ws2812fx.setMode(selectedMode);
    last_change = now;
  }
}
