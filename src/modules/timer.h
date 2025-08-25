#ifndef TENSAI_TIMER_H
#define TENSAI_TIMER_H

#include <chrono>
#include <thread>

class Timer {
private:
  std::chrono::high_resolution_clock::time_point startTime;
  std::chrono::high_resolution_clock::time_point lastFrame;
  double deltaTime = 0.0;
  double totalTime = 0.0;
  int frameCount = 0;
  double fps = 0.0;
  double fpsUpdateTime = 0.0;

public:
  Timer();

  void update();

  double getDelta() const;
  double getTime() const;
  double getFPS() const;

  void sleep(double seconds);
};

#endif // TENSAI_TIMER_H