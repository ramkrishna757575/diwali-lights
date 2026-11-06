#ifndef CenterOutRainbow_h
#define CenterOutRainbow_h

#include <WS2812FX.h>

extern WS2812FX ws2812fx;

uint16_t centerOutRainbow(void)
{
    WS2812FX::Segment *seg = ws2812fx.getSegment(); // get the current segment
    WS2812FX::Segment_runtime *segrt = ws2812fx.getSegmentRuntime();
    int seglen = seg->stop - seg->start + 1;

    for (int i = 0; i < seglen; i++)
    {
        int distance = abs(i - seglen / 2);
        ws2812fx.setPixelColor(seg->start + i, ws2812fx.color_wheel((distance * 2 - segrt->counter_mode_step) & 0xFF));
    }
    segrt->counter_mode_step += 2;
    return 20;
}

#endif
