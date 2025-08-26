#include "vec2.h"
#include <cassert>

Vec2::Vec2(float x, float y) : x(x), y(y) {}

Vec2 Vec2::operator+(const Vec2 &other) const {
  return Vec2(x + other.x, y + other.y);
}

Vec2 Vec2::operator-(const Vec2 &other) const {
  return Vec2(x - other.x, y - other.y);
}

Vec2 Vec2::operator*(float scalar) const {
  return Vec2(x * scalar, y * scalar);
}

Vec2 Vec2::operator*(const Vec2 &other) const {
  return Vec2(x * other.x, y * other.y);
}

float Vec2::length() const { return sqrt(x * x + y * y); }

Vec2 Vec2::normalize() const {
  float len = length();
  assert(len > 0);
  return Vec2(x / len, y / len);
}