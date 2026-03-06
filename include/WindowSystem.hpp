#pragma once
#include "raylib.h"

namespace WindowSystem {

class Window {
 public:
  Window(int width, int height, const char* title);
  ~Window();

  // Disable copy
  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  bool shouldClose() const;
  void beginDrawing();
  void endDrawing();

  // Setup the offscreen render texture (target)
  void beginTextureMode();
  void endTextureMode();

  // Draw the target texture to the screen (with optional shader)
  void drawTarget(Shader shader, bool useShader = false);
  void drawTarget();  // No shader

  int getWidth() const;
  int getHeight() const;
  RenderTexture2D getTarget() const { return target; }

 private:
  void updateTarget();

  RenderTexture2D target;
  int screenWidth;
  int screenHeight;
};

}  // namespace WindowSystem
