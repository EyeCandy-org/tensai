#ifndef TENSAI_GRAPHICS_H
#define TENSAI_GRAPHICS_H

#include "../core/color.h"
#include "../core/transform.h"
#include "../core/vec2.h"
#include "../resources/font.h"
#include "../resources/texture.h"
#include "camera.h"
#include <SDL2/SDL.h>
#include <algorithm>
#include <memory>
#include <vector>

class Graphics {
private:
  SDL_Renderer *renderer;
  Camera camera;
  Color currentColor{255, 255, 255, 255};
  std::shared_ptr<Font> currentFont;
  float lineWidth = 1.0f;

public:
  Graphics(SDL_Renderer *r);

  void setColor(const Color &color);
  void setFont(std::shared_ptr<Font> font);
  void setLineWidth(float width);

  void clear(const Color &color = Color(0, 0, 0, 255));
  void present();

  void drawPoint(const Vec2 &pos);
  void drawLine(const Vec2 &start, const Vec2 &end);
  void drawRect(const Vec2 &pos, const Vec2 &size, bool filled = false);
  void drawCircle(const Vec2 &center, float radius, bool filled = false);
  void drawEllipse(const Vec2 &center, const Vec2 &radii, bool filled = false);
  void drawTexture(std::shared_ptr<Texture> texture, const Transform &transform,
                   const Color &tint = Color(255, 255, 255, 255));
  void drawText(const std::string &text, const Vec2 &pos,
                const Color &color = Color(255, 255, 255, 255));
  void drawPolygon(const std::vector<Vec2> &vertices, bool filled = false);

  Camera &getCamera();
  void setCamera(const Camera &cam);

  void pushMatrix();
  void popMatrix();
  void translate(const Vec2 &offset);
  void rotate(float angle);
  void scale(const Vec2 &scale);
};

#endif // TENSAI_GRAPHICS_H