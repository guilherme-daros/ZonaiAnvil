#pragma once
#include "AppStateMachine.hpp"
#include "CommunicationManager.hpp"
#include "FontManager.hpp"
#include "ProtocolHandler.hpp"
#include "UIContext.hpp"

namespace UIManager {

void ApplyTheme(int index);
void ApplyFont(Font font, int fontSize);
void UpdateStateLogic(AppUIContext& ctx, AppSM& sm);
void HandleInput(AppUIContext& ctx);

// Main entry point for drawing the entire UI
void Draw(AppUIContext& ctx, AppSM& sm, CommunicationManager::Manager& comms);

}  // namespace UIManager
