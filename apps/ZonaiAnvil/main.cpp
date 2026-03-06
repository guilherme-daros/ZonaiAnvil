#include <cstdlib>
#include <cstring>
#include <vector>

#include "AppStateMachine.hpp"
#include "CommunicationManager.hpp"
#include "FontManager.hpp"
#include "Log.hpp"
#include "ShaderManager.hpp"
#include "UIContext.hpp"
#include "UIManager.hpp"
#include "WindowSystem.hpp"
#include "sml.hpp"

int main() {
  using namespace boost::sml::literals;

  Log::StateMachine::logging_level = sb::logger::Level::Debug;
  Log::App::logging_level = sb::logger::Level::Debug;

  // Initialize Core Systems
  WindowSystem::Window window(800, 600, "ZonaiAnvil");

  auto& shaderManager = ShaderManager::Manager::instance();
  auto& fontManager = FontManager::Manager::instance();

  AppUIContext ctx;
  SmlLogger smlLogger;
  AppSM sm{smlLogger};

  CommunicationManager::Manager comms(ctx, sm);
  UIManager::ApplyTheme(ctx.visual.themeIndex);
  UIManager::ApplyFont(fontManager.getFont(FontManager::FontType::Default), 18);

  // Initial port scan
  ctx.connection.portPaths = comms.listPorts();
  for (size_t i = 0; i < ctx.connection.portPaths.size(); ++i) {
    strcat(ctx.connection.portList, ctx.connection.portPaths[i].c_str());
    if (i < ctx.connection.portPaths.size() - 1) strcat(ctx.connection.portList, ";");
  }

  while (!window.shouldClose()) {
    comms.update();
    UIManager::UpdateStateLogic(ctx, sm);
    UIManager::HandleInput(ctx);

    shaderManager.updateUniforms((float)window.getWidth(), (float)window.getHeight());

    // Draw UI to offscreen texture
    window.beginTextureMode();
    UIManager::Draw(ctx, sm, comms);
    window.endTextureMode();

    // Final screen pass
    window.beginDrawing();
    if (ctx.visual.currentShader > 0) {
      window.drawTarget(shaderManager.getShader(static_cast<ShaderManager::ShaderType>(ctx.visual.currentShader)),
                        true);
    } else {
      window.drawTarget();
    }
    window.endDrawing();
  }

  return 0;
}
