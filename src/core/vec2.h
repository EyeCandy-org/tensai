#ifndef TENSAI_VEC2_H
#define TENSAI_VEC2_H

#include <cmath>

class Vec2 {
public:
    float x, y;
    Vec2(float x = 0, float y = 0);
    Vec2 operator+(const Vec2& other) const;
    Vec2 operator-(const Vec2& other) const;
    Vec2 operator*(float scalar) const;
    Vec2 operator*(const Vec2& other) const;
    float length() const;
    Vec2 normalize() const;
};

#endif // TENSAI_VEC2_H