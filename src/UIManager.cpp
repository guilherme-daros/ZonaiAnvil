#include "UIManager.hpp"
#include "Log.hpp"
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include "DeviceParameter.hpp"
#include "ICommunication.hpp"
#include "ProtocolHandler.hpp"

#include "raylib.h"

// Raylib 5.0 compatibility for raygui
#ifndef TextToFloat
#define TextToFloat(text) (float)atof(text)
#endif
#ifndef TextToInteger
#define TextToInteger(text) atoi(text)
#endif

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "styles/style_frappe.h"
#include "styles/style_latte.h"
#include "styles/style_macchiato.h"
#include "styles/style_mocha.h"
#include "styles/style_termx.h"

namespace UIManager {

void ApplyTheme(int index) {
  Log::App::Debug() << "Applying theme index: " << index;
  GuiLoadStyleDefault();
  switch (index) {
    case 0:
      GuiLoadStyleTermX();
      break;
    case 1:
      GuiLoadStyleMocha();
      break;
    case 2:
      GuiLoadStyleMacchiato();
      break;
    case 3:
      GuiLoadStyleFrappe();
      break;
    case 4:
      GuiLoadStyleLatte();
      break;
    default:
      GuiLoadStyleTermX();
      break;
  }
}

void UpdateStateLogic(AppUIContext& ctx, AppSM& sm, std::unique_ptr<ProtocolHandler>& protocol,
                      ICommunication* activeComm) {
  using namespace boost::sml::literals;

  if (sm.is("Welcome"_s)) {
    if (ctx.welcomeTimer == 0.0) ctx.welcomeTimer = GetTime();
    if (GetKeyPressed() != 0 || (GetTime() - ctx.welcomeTimer > 5.0)) {  // 5 seconds max or any key
      sm.process_event(WelcomeTimerEvent{});
    }
    return;
  }

  // 1. Logic & State Transitions
  if (ctx.pendingSchemaResponse && (GetTime() - ctx.stateTransitionTime) > (kTransitionDelayMs / 1000.0)) {
    sm.process_event(SchemaReceivedEvent{});
    ctx.pendingSchemaResponse = false;
  }

  // Handle Protocol Creation/Reset on State Change
  if (sm.is("FetchingSchema"_s) && !protocol && activeComm && activeComm->isOpen()) {
    protocol = std::make_unique<ProtocolHandler>(activeComm);
    protocol->onSchemaReceived = [&](const std::vector<DeviceParameter>& s) {
      ctx.config = s;
      for (auto& p : ctx.config) {
        p.lastSentValue = p.value;
        p.lastSentString = p.stringValue;
      }
      protocol->requestAllValues();
      ctx.stateTransitionTime = GetTime();
      ctx.pendingSchemaResponse = true;
    };
    protocol->onValuesReceived = [&](const std::vector<std::pair<uint8_t, float>>& v) {
      for (auto& val : v) {
        for (auto& p : ctx.config) {
          if (p.id == val.first) {
            p.value = val.second;
            p.lastSentValue = val.second;
          }
        }
      }
    };
    protocol->onWriteAck = [&](uint8_t id) {
      for (auto& p : ctx.config) {
        if (p.id == id) p.pending = false;
      }
    };
    protocol->onLogReceived = [&](uint8_t level, const std::string& msg) {
      ctx.deviceLogs.push_back({level, msg, GetTime()});
      if (ctx.deviceLogs.size() > 100) ctx.deviceLogs.erase(ctx.deviceLogs.begin());
      ctx.logScroll.y = -1000000;  // Auto-scroll
    };
    protocol->requestSchema();
  }

  if (sm.is("Disconnected"_s) && protocol) {
    protocol.reset();
    ctx.config.clear();
    ctx.deviceLogs.clear();
  }
}

void HandleInput(AppUIContext& ctx) {
  if (ctx.portDropdownEdit) {
    if (IsKeyPressed(KEY_DOWN)) ctx.currentPort = (ctx.currentPort + 1) % (int)ctx.portPaths.size();
    if (IsKeyPressed(KEY_UP))
      ctx.currentPort = (ctx.currentPort - 1 + (int)ctx.portPaths.size()) % (int)ctx.portPaths.size();
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) ctx.portDropdownEdit = false;
  } else if (ctx.baudDropdownEdit) {
    if (IsKeyPressed(KEY_DOWN)) ctx.baudRateIndex = (ctx.baudRateIndex + 1) % 5;
    if (IsKeyPressed(KEY_UP)) ctx.baudRateIndex = (ctx.baudRateIndex - 1 + 5) % 5;
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) ctx.baudDropdownEdit = false;
  } else if (ctx.themeDropdownEdit) {
    if (IsKeyPressed(KEY_DOWN)) {
      ctx.themeIndex = (ctx.themeIndex + 1) % ctx.themeCount;
      ApplyTheme(ctx.themeIndex);
    }
    if (IsKeyPressed(KEY_UP)) {
      ctx.themeIndex = (ctx.themeIndex - 1 + ctx.themeCount) % ctx.themeCount;
      ApplyTheme(ctx.themeIndex);
    }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) ctx.themeDropdownEdit = false;
  }
}

void DrawWelcomeScreen(AppUIContext& ctx) {
  const char* title = "ZonaiAnvil";
  int fontSize = 60;
  int textWidth = MeasureText(title, fontSize);
  DrawText(title, GetScreenWidth() / 2 - textWidth / 2, GetScreenHeight() / 2 - fontSize / 2 - 20, fontSize,
           GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));

  const char* subTitle = "Press any key to continue";
  int subFontSize = 20;
  int subTextWidth = MeasureText(subTitle, subFontSize);
  DrawText(subTitle, GetScreenWidth() / 2 - subTextWidth / 2, GetScreenHeight() / 2 + fontSize / 2 + 10, subFontSize,
           GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
}

void DrawSidebar(AppUIContext& ctx, AppSM& sm, ProtocolHandler* protocol, ICommunication* realComm,
                 ICommunication* mockComm, ICommunication*& activeComm) {
  using namespace boost::sml::literals;

  float screenHeight = (float)GetScreenHeight();
  GuiGroupBox((Rectangle){10, 10, 200, screenHeight - 20}, "Connection");

  if (sm.is("Disconnected"_s)) {
    if (GuiButton((Rectangle){20, 210, 180, 40}, "Connect")) {
      if (ctx.currentPort < (int)ctx.portPaths.size()) {
        std::string selectedPort = ctx.portPaths[ctx.currentPort];
        int baud = 9600;
        switch (ctx.baudRateIndex) {
          case 1:
            baud = 19200;
            break;
          case 2:
            baud = 38400;
            break;
          case 3:
            baud = 57600;
            break;
          case 4:
            baud = 115200;
            break;
        }

        if (selectedPort.find("ttyMock") != std::string::npos)
          activeComm = mockComm;
        else
          activeComm = realComm;

        if (activeComm && activeComm->open(selectedPort, baud)) {
          ctx.connectedDeviceName = selectedPort;
          sm.process_event(ConnectEvent{selectedPort, baud});
          if (kTransitionDelayMs > 0) std::this_thread::sleep_for(std::chrono::milliseconds(kTransitionDelayMs));
          sm.process_event(ConnectionSuccessEvent{});
        }
      }
    }
  } else {
    if (GuiButton((Rectangle){20, 210, 180, 40}, "Disconnect")) {
      if (activeComm) activeComm->close();
      ctx.connectedDeviceName = "";
      sm.process_event(DisconnectEvent{});
    }
  }

  const char* statusStr = "STATUS: Disconnected";
  if (sm.is("Connecting"_s))
    statusStr = "STATUS: Connecting...";
  else if (sm.is("FetchingSchema"_s))
    statusStr = "STATUS: Syncing...";
  else if (sm.is("Connected"_s))
    statusStr = "STATUS: Connected";
  GuiLabel((Rectangle){20, 255, 180, 20}, statusStr);

  // 2. Buttons & Labels (Fixed positions)
  if (ctx.anyDropdownOpen()) GuiLock();
  if (GuiButton((Rectangle){20, 370, 180, 30}, ctx.crtEnabled ? "CRT Effect: ON" : "CRT Effect: OFF")) {
    ctx.crtEnabled = !ctx.crtEnabled;
  }
  GuiUnlock();

  // 3. Dropdowns (Drawn last to appear on top)
  if (ctx.portDropdownEdit || ctx.baudDropdownEdit) GuiLock();
  GuiLabel((Rectangle){20, 290, 180, 20}, "UI Theme:");
  int pt = ctx.themeIndex;
  if (GuiDropdownBox((Rectangle){20, 310, 180, 30}, ctx.themes, &ctx.themeIndex, ctx.themeDropdownEdit)) {
    ctx.themeDropdownEdit = !ctx.themeDropdownEdit;
  }
  if (ctx.themeIndex != pt) ApplyTheme(ctx.themeIndex);
  GuiUnlock();

  if (ctx.portDropdownEdit || ctx.themeDropdownEdit) GuiLock();
  GuiLabel((Rectangle){20, 90, 180, 20}, "Baud Rate:");
  if (GuiDropdownBox((Rectangle){20, 110, 180, 30}, ctx.baudRates, &ctx.baudRateIndex, ctx.baudDropdownEdit)) {
    ctx.baudDropdownEdit = !ctx.baudDropdownEdit;
  }
  GuiUnlock();

  if (ctx.baudDropdownEdit || ctx.themeDropdownEdit) GuiLock();
  GuiLabel((Rectangle){20, 30, 180, 20}, "Serial Port:");
  if (GuiDropdownBox((Rectangle){20, 50, 180, 30}, ctx.portList, &ctx.currentPort, ctx.portDropdownEdit)) {
    ctx.portDropdownEdit = !ctx.portDropdownEdit;
  }
  GuiUnlock();
}

void DrawConfigPanel(AppUIContext& ctx, AppSM& sm, ProtocolHandler* protocol) {
  using namespace boost::sml::literals;

  float screenWidth = (float)GetScreenWidth();
  float screenHeight = (float)GetScreenHeight();
  float panelWidth = screenWidth - 220 - 10;
  float logPanelHeight = 150.0f;
  float configPanelHeight = screenHeight - logPanelHeight - 30;

  const char* configTitle = ctx.connectedDeviceName.empty()
                                ? "Configuration"
                                : TextFormat("Configuration [%s]", ctx.connectedDeviceName.c_str());
  GuiGroupBox((Rectangle){220, 10, panelWidth, configPanelHeight}, configTitle);

  if (sm.is("Connected"_s)) {
    float availableWidth = panelWidth - 40;
    float itemWidth = 300.0f;
    float itemHeight = 80.0f;
    int itemsPerRow = (int)(availableWidth / itemWidth);
    if (itemsPerRow < 1) itemsPerRow = 1;

    int totalRows = ctx.config.empty() ? 0 : (int)((ctx.config.size() + itemsPerRow - 1) / itemsPerRow);
    float totalContentHeight = totalRows * itemHeight + 20;

    Rectangle scrollBounds = {230, 40, panelWidth - 20, configPanelHeight - 80};
    Rectangle contentBounds = {0, 0, panelWidth - 40, totalContentHeight};
    Rectangle view = {0, 0, 0, 0};
    GuiScrollPanel(scrollBounds, NULL, contentBounds, &ctx.configScroll, &view);

    BeginScissorMode((int)view.x, (int)view.y, (int)view.width, (int)view.height);

    for (size_t i = 0; i < ctx.config.size(); ++i) {
      auto& param = ctx.config[i];
      int row = (int)(i / itemsPerRow);
      int col = (int)(i % itemsPerRow);

      float posX = col * itemWidth + 40;
      float posY = row * itemHeight + 10;

      float drawX = posX + ctx.configScroll.x + scrollBounds.x;
      float drawY = posY + ctx.configScroll.y + scrollBounds.y;

      if (drawY + itemHeight > scrollBounds.y && drawY < scrollBounds.y + scrollBounds.height) {
        if (param.pending) GuiLock();
        if (param.type == Protocol::ParamType::kToggle) {
          bool val = (param.value > 0.5f);
          bool oldVal = val;
          GuiToggle((Rectangle){drawX, drawY + 20, 150, 30},
                    param.pending ? TextFormat("%s...", param.name.c_str()) : param.name.c_str(), &val);
          if (val != oldVal) {
            param.value = val ? 1.0f : 0.0f;
            param.lastSentValue = param.value;
            param.pending = true;
            if (protocol) protocol->writeValue(param.id, param.value);
          }
        } else if (param.type == Protocol::ParamType::kSlider) {
          GuiLabel((Rectangle){drawX, drawY, 200, 20},
                   param.pending ? TextFormat("%s (sending...)", param.name.c_str()) : param.name.c_str());
          Rectangle sliderRect = {drawX, drawY + 20, 180, 20};
          GuiSlider(sliderRect, TextFormat("%.1f", param.min), TextFormat("%.1f", param.max), &param.value, param.min,
                    param.max);
          if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && (std::abs(param.value - param.lastSentValue) > 0.001f)) {
            param.pending = true;
            param.lastSentValue = param.value;
            if (protocol) protocol->writeValue(param.id, param.value);
          }
        } else if (param.type == Protocol::ParamType::kNumeric) {
          GuiLabel((Rectangle){drawX, drawY, 200, 20},
                   param.pending ? TextFormat("%s (sending...)", param.name.c_str()) : param.name.c_str());
          int val = (int)param.value;
          if (GuiValueBox((Rectangle){drawX, drawY + 20, 100, 30}, NULL, &val, (int)param.min, (int)param.max,
                          param.editMode)) {
            param.editMode = !param.editMode;
            if (!param.editMode && (float)val != param.lastSentValue) {
              param.value = (float)val;
              param.lastSentValue = param.value;
              param.pending = true;
              if (protocol) protocol->writeValue(param.id, param.value);
            }
          }
          if (param.editMode) param.value = (float)val;
        } else if (param.type == Protocol::ParamType::kString) {
          GuiLabel((Rectangle){drawX, drawY, 200, 20},
                   param.pending ? TextFormat("%s (sending...)", param.name.c_str()) : param.name.c_str());
          char buffer[128] = {0};
          std::strncpy(buffer, param.stringValue.c_str(), sizeof(buffer) - 1);
          if (GuiTextBox((Rectangle){drawX, drawY + 20, 200, 30}, buffer, 128, param.editMode)) {
            param.editMode = !param.editMode;
            if (!param.editMode && param.stringValue != buffer) {
              param.stringValue = buffer;
              param.lastSentString = param.stringValue;
              param.pending = true;
              if (protocol) protocol->writeString(param.id, param.stringValue);
            }
          }
          if (param.editMode) param.stringValue = buffer;
        }
        if (param.pending) GuiUnlock();
      }
    }
    EndScissorMode();
    if (GuiButton((Rectangle){230, configPanelHeight - 40, 120, 30}, "Refresh All")) {
      if (protocol) protocol->requestAllValues();
    }
  } else {
    const char* message = "Please connect to a device";
    if (sm.is("FetchingSchema"_s))
      message = "Retrieving device configuration...";
    else if (sm.is("Connecting"_s))
      message = "Establishing connection...";
    int fontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);
    float msgWidth = (float)MeasureText(message, fontSize);
    GuiLabel((Rectangle){220 + panelWidth / 2 - msgWidth / 2, configPanelHeight / 2, msgWidth + 20, 20}, message);
  }

  // Device Logs Panel
  float logPanelY = configPanelHeight + 20;
  GuiGroupBox((Rectangle){220, logPanelY, panelWidth, logPanelHeight}, "Device Logs");

  float logContentHeight = ctx.deviceLogs.size() * 20.0f + 10.0f;
  Rectangle logScrollBounds = {230, logPanelY + 20, panelWidth - 20, logPanelHeight - 30};
  Rectangle logContentBounds = {0, 0, panelWidth - 40, logContentHeight};
  Rectangle logView = {0, 0, 0, 0};

  GuiScrollPanel(logScrollBounds, NULL, logContentBounds, &ctx.logScroll, &logView);

  BeginScissorMode((int)logView.x, (int)logView.y, (int)logView.width, (int)logView.height);
  for (size_t i = 0; i < ctx.deviceLogs.size(); ++i) {
    auto& log = ctx.deviceLogs[i];
    float drawY = logPanelY + 25 + i * 20 + ctx.logScroll.y;

    if (drawY + 20 > logScrollBounds.y && drawY < logScrollBounds.y + logScrollBounds.height) {
      Color color = GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL));
      if (log.level == 1)
        color = YELLOW;
      else if (log.level >= 2)
        color = RED;

      DrawText(TextFormat("[%.2f] %s", log.timestamp, log.message.c_str()), (int)logScrollBounds.x + 5, (int)drawY, 10,
               color);
    }
  }
  EndScissorMode();
}
}  // namespace UIManager
