#ifndef CenterOutRainbow_h
#define CenterOutRainbow_h

#include <WS2812FX.h>

extern WS2812FX ws2812fx;

uint16_t centerOutRainbow(void)
{
    WS2812FX::Segment *seg = ws2812fx.getSegment(); // get the current segment
    uint16_t seglen = seg->stop - seg->start + 1;
    ws2812fx.setBrightness(255); // force full brightness
    
}

#endif