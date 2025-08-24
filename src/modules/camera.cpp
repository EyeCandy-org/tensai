#include "camera.h"

void Camera::translate(const Vec2& offset) { position = position + offset; }

void Camera::rotate(float angle) { rotation += angle; }

void Camera::zoom(float factor) { scale = scale * factor; }

void Camera::lookAt(const Vec2& target) { position = target; }

Vec2 Camera::worldToScreen(const Vec2& worldPos, int screenWidth, int screenHeight) const {
    Vec2 translated = worldPos - position;
    float cos_r = cos(-rotation);
    float sin_r = sin(-rotation);
    Vec2 rotated(translated.x * cos_r - translated.y * sin_r,
                 translated.x * sin_r + translated.y * cos_r);
    Vec2 scaled(rotated.x * scale.x, rotated.y * scale.y);
    return Vec2(scaled.x + screenWidth/2, scaled.y + screenHeight/2);
}

Vec2 Camera::screenToWorld(const Vec2& screenPos, int screenWidth, int screenHeight) const {
    Vec2 centered(screenPos.x - screenWidth/2, screenPos.y - screenHeight/2);
    Vec2 unscaled(centered.x / scale.x, centered.y / scale.y);
    float cos_r = cos(rotation);
    float sin_r = sin(rotation);
    Vec2 rotated(unscaled.x * cos_r - unscaled.y * sin_r,
                 unscaled.x * sin_r + unscaled.y * cos_r);
    return rotated + position;
}