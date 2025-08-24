#include "physics.h"

bool Physics::AABB::intersects(const AABB& other) const {
    return max.x >= other.min.x && min.x <= other.max.x &&
           max.y >= other.min.y && min.y <= other.max.y;
}

bool Physics::AABB::contains(const Vec2& point) const {
    return point.x >= min.x && point.x <= max.x &&
           point.y >= min.y && point.y <= max.y;
}

bool Physics::circleCircleCollision(const Vec2& pos1, float radius1, const Vec2& pos2, float radius2) {
    float dx = pos2.x - pos1.x;
    float dy = pos2.y - pos1.y;
    float distance = sqrt(dx*dx + dy*dy);
    return distance < (radius1 + radius2);
}

bool Physics::pointInCircle(const Vec2& point, const Vec2& center, float radius) {
    float dx = point.x - center.x;
    float dy = point.y - center.y;
    return (dx*dx + dy*dy) <= (radius*radius);
}

void Physics::updateBody(Body& body, float dt) {
    if (body.kinematic) return;
    
    body.velocity = body.velocity + body.acceleration * dt;
    body.velocity = body.velocity * (1.0f - body.friction * dt);
    body.position = body.position + body.velocity * dt;
    body.acceleration = Vec2(0, 0);
}

void Physics::applyForce(Body& body, const Vec2& force) {
    body.acceleration = body.acceleration + force * (1.0f / body.mass);
}

void Physics::resolveCollision(Body& body1, Body& body2, const Vec2& normal) {
    Vec2 relativeVelocity = body2.velocity - body1.velocity;
    float velAlongNormal = relativeVelocity.x * normal.x + relativeVelocity.y * normal.y;
    
    if (velAlongNormal > 0) return;
    
    float e = std::min(body1.restitution, body2.restitution);
    float j = -(1 + e) * velAlongNormal;
    j /= (1.0f / body1.mass) + (1.0f / body2.mass);
    
    Vec2 impulse = normal * j;
    body1.velocity = body1.velocity - impulse * (1.0f / body1.mass);
    body2.velocity = body2.velocity + impulse * (1.0f / body2.mass);
}