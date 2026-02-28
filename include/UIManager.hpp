#pragma once
#include "AppStateMachine.hpp"
#include "ICommunication.hpp"
#include "ProtocolHandler.hpp"
#include "UIContext.hpp"

namespace UIManager {
void ApplyTheme(int index);
void UpdateStateLogic(AppUIContext& ctx, AppSM& sm, std::unique_ptr<ProtocolHandler>& protocol,
                      ICommunication* activeComm);
void HandleInput(AppUIContext& ctx);
void DrawWelcomeScreen(AppUIContext& ctx);
void DrawSidebar(AppUIContext& ctx, AppSM& sm, ProtocolHandler* protocol, ICommunication* realComm,
                 ICommunication* mockComm, ICommunication*& activeComm);
void DrawConfigPanel(AppUIContext& ctx, AppSM& sm, ProtocolHandler* protocol);
}  // namespace UIManager
