#ifndef DiwaliPatterns_h
#define DiwaliPatterns_h

#include <WS2812FX.h>
#include "CenterOutRainbow.h"

extern WS2812FX ws2812fx;

#define FRAME_MS 20
#define MAX_PARTICLES 256

#define DIYA_SPACING 12
#define DIYA_COUNT (LED_COUNT / DIYA_SPACING)
#define TORAN_LENGTH 30
#define RANGOLI_WIDTH 100
#define LOTUS_COUNT 7
#define METEOR_COUNT 5
#define ROCKET_COUNT 3

#define COLOR_COUNT(colors) (sizeof(colors) / sizeof(colors[0]))

struct Pixel
{
    uint8_t r, g, b;
};

struct Particle
{
    float pos, vel, gravity;
    uint8_t life, decay;
    uint32_t color;
};

struct Bloom
{
    int16_t center;
    uint8_t age, wait;
    uint32_t color;
};

struct Meteor
{
    float pos, vel;
    uint32_t color;
};

struct Rocket
{
    float pos, vel, target;
    uint8_t wait;
    bool flying;
};

static const uint32_t TORAN_COLORS[] = {0xFF4A00, 0xFFA000, 0xFFC800, 0xFFA000, 0xFF4A00, 0x008A1E};
static const uint32_t RANGOLI_COLORS[] = {0xE6007E, 0xFF6A00, 0xFFC400, 0x00B8A9, 0x6A1BD1};
static const uint32_t SILK_COLORS[] = {0xC8003C, 0xD0007A, 0x5A14C8, 0x0040D0, 0x00A090, 0xE8A000};
static const uint32_t LOTUS_COLORS[] = {0xFF1080, 0xE000B0, 0xFF3050, 0xB020FF};
static const uint32_t METEOR_COLORS[] = {0xFFB000, 0xFF2090, 0x00C8FF, 0xFF5000, 0x8040FF};
static const uint32_t FIREWORK_COLORS[] = {0xFF1040, 0xFFB000, 0x20FF40, 0x2060FF, 0xFF20C0, 0x00E0E0};
static const uint32_t SPARK_COLORS[] = {0xFFFFFF, 0xFFD060, 0xFFA020, 0xFF6000};

// Fades need more precision than reading pixels back from the strip gives at
// low brightness, so patterns draw here and copy the result to the strip
static Pixel frame[LED_COUNT];
static Particle particles[MAX_PARTICLES];

uint32_t frameCount(void)
{
    return ws2812fx.getSegmentRuntime()->counter_mode_call;
}

// Returns true on a pattern's first frame, after clearing what the last one left
bool beginFrame(void)
{
    if (frameCount() != 0)
    {
        return false;
    }
    memset(frame, 0, sizeof(frame));
    memset(particles, 0, sizeof(particles));
    return true;
}

uint16_t showFrame(void)
{
    for (uint16_t i = 0; i < LED_COUNT; i++)
    {
        ws2812fx.setPixelColor(i, frame[i].r, frame[i].g, frame[i].b);
    }
    return FRAME_MS;
}

void clearFrame(void)
{
    memset(frame, 0, sizeof(frame));
}

void fadeFrame(uint8_t keep)
{
    for (uint16_t i = 0; i < LED_COUNT; i++)
    {
        frame[i].r = (frame[i].r * keep) >> 8;
        frame[i].g = (frame[i].g * keep) >> 8;
        frame[i].b = (frame[i].b * keep) >> 8;
    }
}

uint8_t addChannel(uint8_t value, uint32_t amount, uint8_t level)
{
    uint16_t sum = value + (((amount & 0xFF) * (level + 1)) >> 8);
    return sum > 255 ? 255 : sum;
}

void addColor(int pos, uint32_t color, uint8_t level)
{
    if (pos < 0 || pos >= LED_COUNT)
    {
        return;
    }
    frame[pos].r = addChannel(frame[pos].r, color >> 16, level);
    frame[pos].g = addChannel(frame[pos].g, color >> 8, level);
    frame[pos].b = addChannel(frame[pos].b, color, level);
}

// Picks a smooth color from a looping list, pos 0-255 being once around it
uint32_t paletteColor(const uint32_t *colors, uint8_t count, uint8_t pos)
{
    uint16_t scaled = pos * count;
    uint8_t index = scaled >> 8;
    return ws2812fx.color_blend(colors[index], colors[(index + 1) % count], scaled & 0xFF);
}

float randomFloat(float low, float high)
{
    return low + (high - low) * ws2812fx.random8() / 255.0f;
}

void spawnParticle(float pos, float vel, float gravity, uint8_t decay, uint32_t color)
{
    for (uint16_t i = 0; i < MAX_PARTICLES; i++)
    {
        if (particles[i].life == 0)
        {
            particles[i] = {pos, vel, gravity, 255, decay, color};
            return;
        }
    }
}

void updateParticles(float drag)
{
    for (uint16_t i = 0; i < MAX_PARTICLES; i++)
    {
        Particle &p = particles[i];
        if (p.life == 0)
        {
            continue;
        }
        p.vel = p.vel * drag + p.gravity;
        p.pos += p.vel;
        p.life = p.life > p.decay ? p.life - p.decay : 0;
        if (p.pos < 0 || p.pos >= LED_COUNT)
        {
            p.life = 0;
        }
        addColor((int)p.pos, p.color, p.life);
    }
}

// A row of oil lamps, each flame flickering on its own
uint16_t diyas(void)
{
    static uint8_t flames[DIYA_COUNT];
    if (beginFrame())
    {
        memset(flames, 170, sizeof(flames));
    }
    clearFrame();
    for (uint16_t i = 0; i < DIYA_COUNT; i++)
    {
        int flame = flames[i] + ws2812fx.random8(25) - 12;
        if (ws2812fx.random8() < 3)
        {
            flame -= 70; // caught by a breeze
        }
        flames[i] = constrain(flame, 110, 255);

        int center = i * DIYA_SPACING + DIYA_SPACING / 2;
        uint32_t color = ws2812fx.color_blend(0xFF3000, 0xFF9A20, flames[i]);
        addColor(center, color, flames[i]);
        addColor(center - 1, color, flames[i] / 3);
        addColor(center + 1, color, flames[i] / 3);
        addColor(center - 2, color, flames[i] / 8);
        addColor(center + 2, color, flames[i] / 8);
    }
    return showFrame();
}

// A marigold and mango leaf garland swaying, with glints of gold
uint16_t toran(void)
{
    beginFrame();
    clearFrame();
    uint8_t sway = Adafruit_NeoPixel::sine8(frameCount() / 2);
    for (uint16_t i = 0; i < LED_COUNT; i++)
    {
        uint8_t pos = i * 256 / TORAN_LENGTH + sway;
        addColor(i, paletteColor(TORAN_COLORS, COLOR_COUNT(TORAN_COLORS), pos), 200);
    }
    if (ws2812fx.random8() < 90)
    {
        spawnParticle(random(LED_COUNT), 0, 0, 10, 0xFFE8A0);
    }
    updateParticles(1);
    return showFrame();
}

// Mirrored panels of color rings flowing out from each panel's center
uint16_t rangoli(void)
{
    beginFrame();
    clearFrame();
    uint32_t t = frameCount();
    for (uint16_t i = 0; i < LED_COUNT; i++)
    {
        int distance = abs(i % RANGOLI_WIDTH - RANGOLI_WIDTH / 2);
        uint8_t pos = distance * 5 - t / 2;
        uint8_t ring = Adafruit_NeoPixel::sine8(distance * 16 - t * 3);
        addColor(i, paletteColor(RANGOLI_COLORS, COLOR_COUNT(RANGOLI_COLORS), pos), 70 + ring * 185 / 255);
    }
    return showFrame();
}

// Lotus flowers opening at random places, then closing again
uint16_t lotus(void)
{
    static Bloom blooms[LOTUS_COUNT];
    if (beginFrame())
    {
        for (uint8_t i = 0; i < LOTUS_COUNT; i++)
        {
            blooms[i].age = 0;
            blooms[i].wait = ws2812fx.random8();
        }
    }
    clearFrame();
    for (uint8_t i = 0; i < LOTUS_COUNT; i++)
    {
        Bloom &bloom = blooms[i];
        if (bloom.wait > 0)
        {
            bloom.wait--;
            continue;
        }
        if (bloom.age == 0)
        {
            bloom.center = random(LED_COUNT);
            bloom.color = LOTUS_COLORS[ws2812fx.random8(COLOR_COUNT(LOTUS_COLORS))];
        }
        int radius = bloom.age / 8;
        uint8_t level = Adafruit_NeoPixel::sine8(192 + bloom.age);
        for (int offset = -radius; offset <= radius; offset++)
        {
            uint8_t edge = abs(offset) * 255 / (radius + 1);
            uint32_t color = ws2812fx.color_blend(0xFFE0C0, bloom.color, edge);
            addColor(bloom.center + offset, color, level * (255 - edge / 2) / 255);
        }
        if (++bloom.age == 0)
        {
            bloom.wait = ws2812fx.random8(150);
        }
    }
    return showFrame();
}

// Slow overlapping waves of jewel colors, like silk moving in the wind
uint16_t silk(void)
{
    beginFrame();
    clearFrame();
    uint32_t t = frameCount();
    for (uint16_t i = 0; i < LED_COUNT; i++)
    {
        uint8_t wave1 = Adafruit_NeoPixel::sine8(i * 2 + t);
        uint8_t wave2 = Adafruit_NeoPixel::sine8(i * 5 - t * 2);
        uint8_t pos = (wave1 + wave2) / 2 + t / 2;
        uint8_t level = 110 + Adafruit_NeoPixel::sine8(i * 3 + t * 2) * 145 / 255;
        addColor(i, paletteColor(SILK_COLORS, COLOR_COUNT(SILK_COLORS), pos), level);
    }
    return showFrame();
}

void launchMeteor(Meteor &meteor)
{
    bool forward = ws2812fx.random8(2);
    meteor.pos = forward ? 0 : LED_COUNT - 1;
    meteor.vel = randomFloat(0.5, 1.6) * (forward ? 1 : -1);
    meteor.color = METEOR_COLORS[ws2812fx.random8(COLOR_COUNT(METEOR_COLORS))];
}

// Colored shooting stars crossing in both directions, shedding sparks
uint16_t meteors(void)
{
    static Meteor shower[METEOR_COUNT];
    if (beginFrame())
    {
        for (uint8_t i = 0; i < METEOR_COUNT; i++)
        {
            launchMeteor(shower[i]);
            shower[i].pos = random(LED_COUNT);
        }
    }
    fadeFrame(222);
    for (uint8_t i = 0; i < METEOR_COUNT; i++)
    {
        Meteor &meteor = shower[i];
        float next = meteor.pos + meteor.vel;
        // Fast meteors move over one LED per frame, so fill in the gap
        int step = meteor.vel > 0 ? 1 : -1;
        for (int pos = (int)meteor.pos; pos != (int)next + step; pos += step)
        {
            addColor(pos, meteor.color, 255);
        }
        meteor.pos = next;
        if (ws2812fx.random8() < 60)
        {
            spawnParticle(meteor.pos, randomFloat(-0.3, 0.3), 0, 10, 0xFFE0A0);
        }
        if (meteor.pos < 0 || meteor.pos >= LED_COUNT)
        {
            launchMeteor(meteor);
        }
    }
    updateParticles(0.9);
    return showFrame();
}

void burstFirework(float pos)
{
    uint32_t color1 = FIREWORK_COLORS[ws2812fx.random8(COLOR_COUNT(FIREWORK_COLORS))];
    uint32_t color2 = FIREWORK_COLORS[ws2812fx.random8(COLOR_COUNT(FIREWORK_COLORS))];
    for (uint8_t i = 0; i < 36; i++)
    {
        spawnParticle(pos, randomFloat(-2.4, 2.4), 0, 3 + ws2812fx.random8(4), i % 2 ? color1 : color2);
    }
    for (uint8_t i = 0; i < 8; i++)
    {
        spawnParticle(pos, randomFloat(-1, 1), 0, 30, 0xFFFFFF);
    }
}

// Rockets climb from either end and burst into two-color showers
uint16_t fireworks(void)
{
    static Rocket rockets[ROCKET_COUNT];
    if (beginFrame())
    {
        for (uint8_t i = 0; i < ROCKET_COUNT; i++)
        {
            rockets[i].flying = false;
            rockets[i].wait = ws2812fx.random8(80);
        }
    }
    fadeFrame(190);
    for (uint8_t i = 0; i < ROCKET_COUNT; i++)
    {
        Rocket &rocket = rockets[i];
        if (rocket.wait > 0)
        {
            rocket.wait--;
            continue;
        }
        if (!rocket.flying)
        {
            bool forward = ws2812fx.random8(2);
            rocket.pos = forward ? 0 : LED_COUNT - 1;
            rocket.vel = randomFloat(2, 3.5) * (forward ? 1 : -1);
            rocket.target = random(LED_COUNT / 4, LED_COUNT * 3 / 4);
            rocket.flying = true;
        }
        rocket.pos += rocket.vel;
        addColor((int)rocket.pos, 0xFFC060, 200);
        if (rocket.vel > 0 ? rocket.pos >= rocket.target : rocket.pos <= rocket.target)
        {
            burstFirework(rocket.pos);
            rocket.flying = false;
            rocket.wait = 20 + ws2812fx.random8(100);
        }
    }
    updateParticles(0.96);
    return showFrame();
}

// Flower pot fountains spraying sparks that rise, slow and fall back
uint16_t anar(void)
{
    beginFrame();
    fadeFrame(170);
    float power = 1.2 + Adafruit_NeoPixel::sine8(frameCount() / 2) * 1.2 / 255;
    const int bases[] = {LED_COUNT / 4, LED_COUNT * 3 / 4};
    for (uint8_t i = 0; i < 2; i++)
    {
        for (int direction = -1; direction <= 1; direction += 2)
        {
            if (ws2812fx.random8(2))
            {
                uint32_t color = SPARK_COLORS[ws2812fx.random8(COLOR_COUNT(SPARK_COLORS))];
                spawnParticle(bases[i], direction * randomFloat(0.6, power), -direction * 0.03, 2 + ws2812fx.random8(2), color);
            }
        }
    }
    updateParticles(1);
    return showFrame();
}

// A burning sparkler travelling along the strip, throwing sparks both ways
uint16_t sparkler(void)
{
    static float head, speed;
    if (beginFrame())
    {
        head = 0;
        speed = 0.7;
    }
    fadeFrame(150);
    head += speed;
    if (head < 0 || head >= LED_COUNT)
    {
        speed = -speed;
        head += speed;
    }
    addColor((int)head, 0xFFFFFF, 255);
    addColor((int)head - 1, 0xFFD060, 200);
    addColor((int)head + 1, 0xFFD060, 200);
    for (uint8_t i = 0; i < 4; i++)
    {
        uint32_t color = SPARK_COLORS[ws2812fx.random8(COLOR_COUNT(SPARK_COLORS))];
        spawnParticle(head, randomFloat(-3.2, 3.2), 0, 10 + ws2812fx.random8(16), color);
    }
    updateParticles(0.9);
    return showFrame();
}

struct DiwaliPattern
{
    const char *name;
    uint16_t (*run)(void);
    uint8_t seconds; // how long it runs, enough to cross 600 LEDs
};

static const DiwaliPattern diwaliPatterns[] = {
    {"Diyas", diyas, 20},
    {"Toran", toran, 20},
    {"Rangoli", rangoli, 20},
    {"Lotus", lotus, 20},
    {"Silk", silk, 20},
    {"Center out rainbow", centerOutRainbow, 20},
    {"Meteors", meteors, 30},
    {"Fireworks", fireworks, 30},
    {"Anar", anar, 20},
    {"Sparkler", sparkler, 40},
};

#define DIWALI_PATTERN_COUNT COLOR_COUNT(diwaliPatterns)

#endif
