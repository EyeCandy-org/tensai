#include "input.h"

void Input::update() {
  previousKeys = currentKeys;
  previousMouse = currentMouse;
  mouseWheel = Vec2(0, 0);
}

void Input::setKey(int key, bool pressed) { currentKeys[key] = pressed; }

void Input::setMouse(int button, bool pressed) {
  currentMouse[button] = pressed;
}

void Input::setMousePos(int x, int y) { mousePos = Vec2(x, y); }

void Input::setMouseWheel(int x, int y) { mouseWheel = Vec2(x, y); }

bool Input::isDown(int key) const {
  auto it = currentKeys.find(key);
  return it != currentKeys.end() && it->second;
}

bool Input::isPressed(int key) const {
  auto curr = currentKeys.find(key);
  auto prev = previousKeys.find(key);
  bool currPressed = curr != currentKeys.end() && curr->second;
  bool prevPressed = prev != previousKeys.end() && prev->second;
  return currPressed && !prevPressed;
}

bool Input::isReleased(int key) const {
  auto curr = currentKeys.find(key);
  auto prev = previousKeys.find(key);
  bool currPressed = curr != currentKeys.end() && curr->second;
  bool prevPressed = prev != previousKeys.end() && prev->second;
  return !currPressed && prevPressed;
}

bool Input::isMouseDown(int button) const {
  auto it = currentMouse.find(button);
  return it != currentMouse.end() && it->second;
}

bool Input::isMousePressed(int button) const {
  auto curr = currentMouse.find(button);
  auto prev = previousMouse.find(button);
  bool currPressed = curr != currentMouse.end() && curr->second;
  bool prevPressed = prev != previousMouse.end() && prev->second;
  return currPressed && !prevPressed;
}

bool Input::isMouseReleased(int button) const {
  auto curr = currentMouse.find(button);
  auto prev = previousMouse.find(button);
  bool currPressed = curr != currentMouse.end() && curr->second;
  bool prevPressed = prev != previousMouse.end() && prev->second;
  return !currPressed && prevPressed;
}

Vec2 Input::getMousePosition() const { return mousePos; }

Vec2 Input::getMouseWheel() const { return mouseWheel; }