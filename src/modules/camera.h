#ifndef TENSAI_CAMERA_H
#define TENSAI_CAMERA_H

#include "../core/vec2.h"
#include <cmath>

class Camera {
public:
    Vec2 position{0, 0};
    float rotation = 0.0f;
    Vec2 scale{1, 1};
    
    void translate(const Vec2& offset);
    void rotate(float angle);
    void zoom(float factor);
    void lookAt(const Vec2& target);
    
    Vec2 worldToScreen(const Vec2& worldPos, int screenWidth, int screenHeight) const;
    Vec2 screenToWorld(const Vec2& screenPos, int screenWidth, int screenHeight) const;
};

#endif // TENSAI_CAMERA_H