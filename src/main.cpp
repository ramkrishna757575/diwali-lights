#include <Arduino.h>
#include <WS2812FX.h>

#define LED_COUNT 193
#define LED_PIN 11

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
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

unsigned long last_change = 0;
unsigned long now = 0;
uint8_t last_mode = 0;
uint8_t selectedMode = last_mode;
int colorWheelValues[] = {0, 50, 100, 150, 200, 250};

void setup()
{
  LOG_INIT();
  LOG_PRINTLN("Setup started");

  ws2812fx.init();
  ws2812fx.setBrightness(255);
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
    randomSeed(analogRead(A0));
    int randomIndex = random(0, 6);
    uint32_t color = ws2812fx.color_wheel(colorWheelValues[randomIndex]);
    LOG_PRINTLN("Color: " + String(color));
    ws2812fx.setColor(color);
    /* while (selectedMode == last_mode)
    {
      randomSeed(analogRead(A0));
      // exclude the last 8 custom modes
      selectedMode = random(0, ws2812fx.getModeCount() - 8);
    }
    last_mode = selectedMode; */
    selectedMode = (ws2812fx.getMode() + 1) % (ws2812fx.getModeCount() - 8);
    LOG_PRINTLN("Mode: " + String(selectedMode));
    ws2812fx.setMode(selectedMode);
    last_change = now;
  }
}
