#include "transform.h"
#include <cmath>

SDL_Point Transform::getSDLOrigin() const {
    return {(int)origin.x, (int)origin.y};
}

double Transform::getRotation() const {
    if (useDegrees) {
        return rotation;
    }
    return rotation * 180.0 / M_PI;
}

void Transform::setRotation(float angleInRadians) {
    rotation = angleInRadians;
    useDegrees = false;
}

void Transform::setRotationDegrees(float angleInDegrees) {
    rotation = angleInDegrees;
    useDegrees = true;
}
