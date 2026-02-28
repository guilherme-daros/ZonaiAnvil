#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>
#include "raylib.h"

#include "raygui.h"

#include "AppStateMachine.hpp"
#include "LinuxSerialPort.hpp"
#include "MockSerialPort.hpp"
#include "ProtocolHandler.hpp"
#include "UIContext.hpp"
#include "UIManager.hpp"
#include "sml.hpp"

int main() {
  using namespace boost::sml::literals;
  const int screenWidth = 800;
  const int screenHeight = 600;

  Log::StateMachine::logging_level = sb::logger::Level::Debug;
  Log::App::logging_level = sb::logger::Level::Debug;

  Log::App::Info() << "Starting ZonaiAnvil Application";
  SetTraceLogLevel(LOG_NONE);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(screenWidth, screenHeight, "ZonaiAnvil");
  SetTargetFPS(60);

  // Load Shader and setup Render Texture
  Shader crtShader = LoadShader(0, "apps/ZonaiAnvil/crt.fs");
  int renderSizeLoc = GetShaderLocation(crtShader, "renderSize");
  RenderTexture2D target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

  auto realSerial = std::make_unique<LinuxSerialPort>();
  auto mockSerial = std::make_unique<MockSerialPort>();

  std::unique_ptr<ProtocolHandler> protocol;
  ICommunication *activeComm = nullptr;
  AppUIContext ctx;
  UIManager::ApplyTheme(ctx.themeIndex);

  // Scan real ports and add Mocks
  ctx.portPaths = realSerial->listPorts();
  ctx.portPaths.push_back("ttyMock1");
  ctx.portPaths.push_back("ttyMock2");
  ctx.portPaths.push_back("ttyMock3");

  for (size_t i = 0; i < ctx.portPaths.size(); ++i) {
    strcat(ctx.portList, ctx.portPaths[i].c_str());
    if (i < ctx.portPaths.size() - 1) strcat(ctx.portList, ";");
  }

  SmlLogger smlLogger;
  AppSM sm{smlLogger};

  while (!WindowShouldClose()) {
    // Handle Resizing of Render Texture
    if (IsWindowResized()) {
      UnloadRenderTexture(target);
      target = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    }

    if (protocol) protocol->update();

    UIManager::UpdateStateLogic(ctx, sm, protocol, activeComm);
    UIManager::HandleInput(ctx);

    // Update shader uniforms
    float renderSize[2] = {(float)GetScreenWidth(), (float)GetScreenHeight()};
    SetShaderValue(crtShader, renderSizeLoc, renderSize, SHADER_UNIFORM_VEC2);

    // Draw to Texture
    BeginTextureMode(target);
    ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

    if (sm.is("Welcome"_s)) {
      UIManager::DrawWelcomeScreen(ctx);
    } else {
      if (ctx.anyDropdownOpen()) GuiLock();
      UIManager::DrawConfigPanel(ctx, sm, protocol.get());
      UIManager::DrawSidebar(ctx, sm, protocol.get(), realSerial.get(), mockSerial.get(), activeComm);
      GuiUnlock();
    }
    EndTextureMode();

    // Draw Texture to Screen
    BeginDrawing();
    ClearBackground(BLACK);
    if (ctx.crtEnabled) {
      BeginShaderMode(crtShader);
      DrawTextureRec(target.texture, (Rectangle){0, 0, (float)target.texture.width, (float)-target.texture.height},
                     (Vector2){0, 0}, WHITE);
      EndShaderMode();
    } else {
      DrawTextureRec(target.texture, (Rectangle){0, 0, (float)target.texture.width, (float)-target.texture.height},
                     (Vector2){0, 0}, WHITE);
    }
    EndDrawing();
  }

  Log::App::Info() << "Closing ZonaiAnvil Application";

  UnloadShader(crtShader);
  UnloadRenderTexture(target);
  CloseWindow();
  return 0;
}
