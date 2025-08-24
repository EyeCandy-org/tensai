#include "random.h"

Random::Random() : generator(std::random_device{}()) {}

Random::Random(uint32_t seed) : generator(seed) {}

void Random::setSeed(uint32_t seed) { generator.seed(seed); }

int Random::randomInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(generator);
}

float Random::randomFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(generator);
}

bool Random::randomBool() {
    return randomFloat() < 0.5f;
}

Vec2 Random::randomVec2(const Vec2& min, const Vec2& max) {
    return Vec2(randomFloat(min.x, max.x), randomFloat(min.y, max.y));
}