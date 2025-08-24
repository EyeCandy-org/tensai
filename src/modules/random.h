#ifndef TENSAI_RANDOM_H
#define TENSAI_RANDOM_H

#include <random>
#include <cstdint>
#include "../core/vec2.h"

class Random {
private:
    std::mt19937 generator;
    
public:
    Random();
    Random(uint32_t seed);
    
    void setSeed(uint32_t seed);
    
    int randomInt(int min, int max);
    float randomFloat(float min = 0.0f, float max = 1.0f);
    bool randomBool();
    Vec2 randomVec2(const Vec2& min = Vec2(0,0), const Vec2& max = Vec2(1,1));
};

#endif // TENSAI_RANDOM_H