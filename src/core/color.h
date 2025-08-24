#ifndef TENSAI_COLOR_H
#define TENSAI_COLOR_H

#include <SDL2/SDL.h>

class Color {
public:
    Uint8 r, g, b, a;
    Color(Uint8 r = 255, Uint8 g = 255, Uint8 b = 255, Uint8 a = 255);
};

#endif // TENSAI_COLOR_H