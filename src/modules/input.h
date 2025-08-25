#ifndef TENSAI_INPUT_H
#define TENSAI_INPUT_H

#include "../core/vec2.h"
#include <unordered_map>

class Input {
private:
  std::unordered_map<int, bool> currentKeys;
  std::unordered_map<int, bool> previousKeys;
  std::unordered_map<int, bool> currentMouse;
  std::unordered_map<int, bool> previousMouse;
  Vec2 mousePos{0, 0};
  Vec2 mouseWheel{0, 0};

public:
  void update();
  void setKey(int key, bool pressed);
  void setMouse(int button, bool pressed);
  void setMousePos(int x, int y);
  void setMouseWheel(int x, int y);
  bool isDown(int key) const;
  bool isPressed(int key) const;
  bool isReleased(int key) const;
  bool isMouseDown(int button) const;
  bool isMousePressed(int button) const;
  bool isMouseReleased(int button) const;
  Vec2 getMousePosition() const;
  Vec2 getMouseWheel() const;
};

#endif // TENSAI_INPUT_H