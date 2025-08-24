#include "transform.h"

SDL_Point Transform::getSDLOrigin() const { return {(int)origin.x, (int)origin.y}; }

double Transform::getRotationDegrees() const { return rotation * 180.0 / M_PI; }