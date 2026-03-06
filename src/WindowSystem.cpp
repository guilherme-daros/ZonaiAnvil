#include "WindowSystem.hpp"
#include "Log.hpp"

namespace WindowSystem {

Window::Window(int width, int height, const char* title) : screenWidth(width), screenHeight(height) {
  Log::App::Info() << "Initializing Window: " << title;

  // Disable Raylib terminal logging
  SetTraceLogLevel(LOG_NONE);

  // Set some default flags - we keep resizable since the user wants it!
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(width, height, title);
  SetTargetFPS(60);

  target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
}

Window::~Window() {
  Log::App::Info() << "Closing Window System";
  UnloadRenderTexture(target);
  CloseWindow();
}

bool Window::shouldClose() const { return WindowShouldClose(); }

void Window::updateTarget() {
  if (IsWindowResized()) {
    UnloadRenderTexture(target);
    target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
  }
}

void Window::beginDrawing() {
  updateTarget();
  BeginDrawing();
  ClearBackground(BLACK);
}

void Window::endDrawing() { EndDrawing(); }

void Window::beginTextureMode() { BeginTextureMode(target); }

void Window::endTextureMode() { EndTextureMode(); }

void Window::drawTarget(Shader shader, bool useShader) {
  if (useShader) {
    BeginShaderMode(shader);
    DrawTextureRec(target.texture, (Rectangle){0, 0, (float)target.texture.width, (float)-target.texture.height},
                   (Vector2){0, 0}, WHITE);
    EndShaderMode();
  } else {
    DrawTextureRec(target.texture, (Rectangle){0, 0, (float)target.texture.width, (float)-target.texture.height},
                   (Vector2){0, 0}, WHITE);
  }
}

void Window::drawTarget() {
  DrawTextureRec(target.texture, (Rectangle){0, 0, (float)target.texture.width, (float)-target.texture.height},
                 (Vector2){0, 0}, WHITE);
}

int Window::getWidth() const { return GetScreenWidth(); }
int Window::getHeight() const { return GetScreenHeight(); }

}  // namespace WindowSystem
