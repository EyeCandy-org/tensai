#ifndef TENSAI_TRANSFORM_H
#define TENSAI_TRANSFORM_H

#include "vec2.h"
#include <SDL2/SDL.h>

class Transform {
public:
    Vec2 position{0, 0};
    Vec2 scale{1, 1};
    float rotation = 0.0f;
    Vec2 origin{0, 0};
    SDL_Point getSDLOrigin() const;
    double getRotationDegrees() const;
};

#endif // TENSAI_TRANSFORM_H