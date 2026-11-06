#include <Arduino.h>
#include <WS2812FX.h>
#include <vector>
#include <algorithm>

#ifdef ESP8266
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <NeoPixelBus.h>
#include "secrets.h"
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

#define TIMER_MS 30000

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

// Bit-banged LED data can't run alongside Wi-Fi, so send it with the UART1
// hardware instead. That is fixed to GPIO2, and this must match LED_COLOR_ORDER.
NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart1Ws2812xMethod> strip(LED_COUNT);

bool wifiLogged = false;

// Each color channel draws up to 20mA, and the supply must also cover the
// board and the LEDs' idle draw. Frames that would exceed this budget are
// dimmed together, so sparse patterns can run brighter than a full white one.
#ifndef LED_MAX_MA
#define LED_MAX_MA 3300
#endif

void showStrip(void)
{
  if (strip.CanShow())
  {
    uint8_t *out = strip.Pixels();
    const uint8_t *in = ws2812fx.getPixels();
    size_t size = strip.PixelsSize();

    uint32_t total = 0;
    for (size_t i = 0; i < size; i++)
    {
      total += in[i];
    }
    uint32_t limit = (uint32_t)LED_MAX_MA * 255 / 20;
    if (total > limit)
    {
      uint32_t scale = (uint64_t)limit * 256 / total;
      for (size_t i = 0; i < size; i++)
      {
        out[i] = (in[i] * scale) >> 8;
      }
    }
    else
    {
      memcpy(out, in, size);
    }
    strip.Dirty();
    strip.Show();
  }
}
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
  // Connects in the background, so the lights still run without a network
  WiFi.mode(WIFI_STA);
  WiFi.hostname("diwali-lights");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  ArduinoOTA.setHostname("diwali-lights");
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.begin();
#endif

  ws2812fx.init();
#ifdef ESP8266
  // Has to come after init() so that GPIO2 ends up set for UART1
  strip.Begin();
  ws2812fx.setCustomShow(showStrip);
#endif
  ws2812fx.setBrightness(LED_BRIGHTNESS);
  ws2812fx.setSpeed(1000);
  // generate a random color
#ifdef ESP8266
  // A0 reads the same on every boot here, which would repeat the same order
  randomSeed(RANDOM_REG32);
#else
  randomSeed(analogRead(A0));
#endif
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

#ifdef ESP8266
  ArduinoOTA.handle();
  if (!wifiLogged && WiFi.status() == WL_CONNECTED)
  {
    LOG_PRINTLN("Wi-Fi connected: " + WiFi.localIP().toString());
    wifiLogged = true;
  }
#endif

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
