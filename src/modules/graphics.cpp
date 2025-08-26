#include "graphics.h"
Graphics::Graphics(SDL_Renderer *r) : renderer(r) {}
void Graphics::setColor(const Color &color) {
  currentColor = color;
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void Graphics::setFont(std::shared_ptr<Font> font) { currentFont = font; }
void Graphics::setLineWidth(float width) { lineWidth = width; }
void Graphics::clear(const Color &color) {
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
  SDL_RenderClear(renderer);
}

void Graphics::present() { SDL_RenderPresent(renderer); }
void Graphics::drawPoint(const Vec2 &pos) {
  SDL_RenderDrawPoint(renderer, (int)pos.x, (int)pos.y);
}

void Graphics::drawLine(const Vec2 &start, const Vec2 &end) {
  if (lineWidth <= 1.0f) {
    SDL_RenderDrawLine(renderer, (int)start.x, (int)start.y, (int)end.x,
                       (int)end.y);
  } else {
    Vec2 dir = (end - start).normalize();
    Vec2 perp(-dir.y, dir.x);
    Vec2 offset = perp * (lineWidth / 2.0f);
    SDL_Point points[4] = {
        {(int)(start.x + offset.x), (int)(start.y + offset.y)},
        {(int)(start.x - offset.x), (int)(start.y - offset.y)},
        {(int)(end.x - offset.x), (int)(end.y - offset.y)},
        {(int)(end.x + offset.x), (int)(end.y + offset.y)}};

    SDL_RenderDrawLines(renderer, points, 5);
  }
}

void Graphics::drawRect(const Vec2 &pos, const Vec2 &size, bool filled) {
  SDL_Rect rect = {(int)pos.x, (int)pos.y, (int)size.x, (int)size.y};
  if (filled) {
    SDL_RenderFillRect(renderer, &rect);
  } else {
    SDL_RenderDrawRect(renderer, &rect);
  }
}

void Graphics::drawCircle(const Vec2 &center, float radius, bool filled) {
  int segments = std::max(8, (int)(radius * 0.5f));
  std::vector<SDL_Point> points;
  for (int i = 0; i <= segments; i++) {
    float angle = 2.0f * M_PI * i / segments;
    int x = (int)(center.x + radius * cos(angle));
    int y = (int)(center.y + radius * sin(angle));
    points.push_back({x, y});
  }

  if (filled) {
    for (int y = (int)-radius; y <= (int)radius; y++) {
      for (int x = (int)-radius; x <= (int)radius; x++) {
        if (x * x + y * y <= radius * radius) {
          SDL_RenderDrawPoint(renderer, (int)center.x + x, (int)center.y + y);
        }
      }
    }
  } else {
    SDL_RenderDrawLines(renderer, points.data(), points.size());
  }
}

void Graphics::drawEllipse(const Vec2 &center, const Vec2 &radii, bool filled) {
  int segments = std::max(16, (int)((radii.x + radii.y) * 0.25f));
  std::vector<SDL_Point> points;
  for (int i = 0; i <= segments; i++) {
    float angle = 2.0f * M_PI * i / segments;
    int x = (int)(center.x + radii.x * cos(angle));
    int y = (int)(center.y + radii.y * sin(angle));
    points.push_back({x, y});
  }

  if (filled) {
    for (int i = 1; i < segments; i++) {
      SDL_Point triangle[3] = {
          {(int)center.x, (int)center.y}, points[i - 1], points[i]};
      SDL_RenderDrawLines(renderer, triangle, 4);
    }
  } else {
    SDL_RenderDrawLines(renderer, points.data(), points.size());
  }
}

void Graphics::drawTexture(std::shared_ptr<Texture> texture,
                           const Transform &transform, const Color &tint) {
  if (!texture || !texture->texture)
    return;
  SDL_SetTextureColorMod(texture->texture, tint.r, tint.g, tint.b);
  SDL_SetTextureAlphaMod(texture->texture, tint.a);
  SDL_Rect dst = {
      (int)(transform.position.x - transform.origin.x * transform.scale.x),
      (int)(transform.position.y - transform.origin.y * transform.scale.y),
      (int)(texture->width * transform.scale.x),
      (int)(texture->height * transform.scale.y)};

  if (transform.rotation == 0.0f) {
    SDL_RenderCopy(renderer, texture->texture, nullptr, &dst);
  } else {
    SDL_Point center = transform.getSDLOrigin();
    SDL_RenderCopyEx(renderer, texture->texture, nullptr, &dst,
                     transform.getRotation(), &center, SDL_FLIP_NONE);
  }
}

void Graphics::drawText(const std::string &text, const Vec2 &pos,
                        const Color &color) {
  if (!currentFont || !currentFont->font)
    return;
  SDL_Color sdlColor = {color.r, color.g, color.b, color.a};
  SDL_Surface *surface =
      TTF_RenderText_Solid(currentFont->font, text.c_str(), sdlColor);
  if (!surface)
    return;
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (texture) {
    SDL_Rect dst = {(int)pos.x, (int)pos.y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
  }
  SDL_FreeSurface(surface);
}

void Graphics::drawPolygon(const std::vector<Vec2> &vertices, bool filled) {
  if (vertices.size() < 3)
    return;
  if (filled) {
    float minY = vertices[0].y;
    float maxY = vertices[0].y;
    for (const auto &v : vertices) {
      if (v.y < minY)
        minY = v.y;
      if (v.y > maxY)
        maxY = v.y;
    }

    for (int y = (int)minY; y <= (int)maxY; y++) {
      std::vector<float> intersections;
      for (size_t i = 0; i < vertices.size(); i++) {
        const Vec2 &p1 = vertices[i];
        const Vec2 &p2 = vertices[(i + 1) % vertices.size()];
        if ((p1.y <= y && p2.y > y) || (p2.y <= y && p1.y > y)) {
          if (p1.y == p2.y)
            continue;
          float intersectX = p1.x + (y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
          intersections.push_back(intersectX);
        }
      }
      std::sort(intersections.begin(), intersections.end());
      for (size_t i = 0; i + 1 < intersections.size(); i += 2) {
        SDL_RenderDrawLine(renderer, (int)intersections[i], y,
                           (int)intersections[i + 1], y);
      }
    }
  } else {
    std::vector<SDL_Point> points;
    for (const auto &v : vertices) {
      points.push_back({(int)v.x, (int)v.y});
    }
    points.push_back(points[0]);
    SDL_RenderDrawLines(renderer, points.data(), points.size());
  }
}

Camera &Graphics::getCamera() { return camera; }
void Graphics::setCamera(const Camera &cam) { camera = cam; }
void Graphics::pushMatrix() {}
void Graphics::popMatrix() {}
void Graphics::translate(const Vec2 &offset) { camera.translate(offset * -1); }
void Graphics::rotate(float angle) { camera.rotate(angle * -1); }
void Graphics::scale(const Vec2 &scale) {
  camera.scale = camera.scale * Vec2(1.0f / scale.x, 1.0f / scale.y);
}
