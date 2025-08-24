#include "timer.h"

Timer::Timer() : startTime(std::chrono::high_resolution_clock::now()), lastFrame(startTime) {}

void Timer::update() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    deltaTime = std::chrono::duration<double>(currentTime - lastFrame).count();
    totalTime = std::chrono::duration<double>(currentTime - startTime).count();
    lastFrame = currentTime;
    frameCount++;
    
    fpsUpdateTime += deltaTime;
    if (fpsUpdateTime >= 1.0) {
        fps = frameCount / fpsUpdateTime;
        frameCount = 0;
        fpsUpdateTime = 0.0;
    }
}

double Timer::getDelta() const { return deltaTime; }

double Timer::getTime() const { return totalTime; }

double Timer::getFPS() const { return fps; }

void Timer::sleep(double seconds) {
    auto duration = std::chrono::duration<double>(seconds);
    std::this_thread::sleep_for(duration);
}