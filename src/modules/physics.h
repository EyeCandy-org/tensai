#ifndef TENSAI_PHYSICS_H
#define TENSAI_PHYSICS_H

#include "../core/vec2.h"
#include <algorithm>
#include <cmath>

class Physics {
public:
  struct Body {
    Vec2 position{0, 0};
    Vec2 velocity{0, 0};
    Vec2 acceleration{0, 0};
    float mass = 1.0f;
    float friction = 0.0f;
    float restitution = 1.0f;
    bool kinematic = false;
  };

  struct AABB {
    Vec2 min, max;

    bool intersects(const AABB &other) const;
    bool contains(const Vec2 &point) const;
  };

  static bool circleCircleCollision(const Vec2 &pos1, float radius1,
                                    const Vec2 &pos2, float radius2);
  static bool pointInCircle(const Vec2 &point, const Vec2 &center,
                            float radius);
  static void updateBody(Body &body, float dt);
  static void applyForce(Body &body, const Vec2 &force);
  static void resolveCollision(Body &body1, Body &body2, const Vec2 &normal);
};

#endif // TENSAI_PHYSICS_H