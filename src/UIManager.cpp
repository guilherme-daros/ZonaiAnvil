#include "UIManager.hpp"
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include "DeviceParameter.hpp"
#include "ICommunication.hpp"
#include "Log.hpp"
#include "ProtocolHandler.hpp"
#include "ThemeManager.hpp"
#include "FontManager.hpp"

#include "raylib.h"

// Raylib 5.0 compatibility for raygui
#ifndef TextToFloat
#define TextToFloat(text) (float)atof(text)
#endif
#ifndef TextToInteger
#define TextToInteger(text) atoi(text)
#endif

// Define raygui implementation in ONE translation unit
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

namespace UIManager {

void ApplyTheme(int index) {
  ThemeManager::ThemeType type = static_cast<ThemeManager::ThemeType>(index);
  const ThemeManager::Theme& t = ThemeManager::getTheme(type);
  
  Log::App::Debug() << "Applying theme: " << t.name;

  GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL,   ColorToInt(t.borderNormal));
  GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL,     ColorToInt(t.baseNormal));
  GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL,     ColorToInt(t.textNormal));
  
  GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED,  ColorToInt(t.borderFocused));
  GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED,    ColorToInt(t.baseFocused));
  GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED,    ColorToInt(t.textFocused));
  
  GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED,  ColorToInt(t.borderPressed));
  GuiSetStyle(DEFAULT, BASE_COLOR_PRESSED,    ColorToInt(t.basePressed));
  GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED,    ColorToInt(t.textPressed));
  
  GuiSetStyle(DEFAULT, BORDER_COLOR_DISABLED, ColorToInt(t.borderDisabled));
  GuiSetStyle(DEFAULT, BASE_COLOR_DISABLED,   ColorToInt(t.baseDisabled));
  GuiSetStyle(DEFAULT, TEXT_COLOR_DISABLED,   ColorToInt(t.textDisabled));
  
  GuiSetStyle(DEFAULT, BACKGROUND_COLOR,      ColorToInt(t.background));
  GuiSetStyle(DEFAULT, LINE_COLOR,            ColorToInt(t.line));
  
  GuiSetStyle(DEFAULT, TEXT_SIZE, 18);
  GuiSetStyle(DEFAULT, TEXT_SPACING, 1);
}

void ApplyFont(Font font, int fontSize) {
  if (font.texture.id == 0) return;
  GuiSetFont(font);
  GuiSetStyle(DEFAULT, TEXT_SIZE, fontSize);
}

void UpdateStateLogic(AppUIContext& ctx, AppSM& sm) {
  using namespace boost::sml::literals;

  if (sm.is("Welcome"_s)) {
    if (ctx.welcomeTimer == 0.0) ctx.welcomeTimer = GetTime();
    if (GetKeyPressed() != 0 || (GetTime() - ctx.welcomeTimer > 5.0)) {
      sm.process_event(WelcomeTimerEvent{});
    }
    return;
  }

  if (ctx.pendingSchemaResponse && (GetTime() - ctx.stateTransitionTime) > (kTransitionDelayMs / 1000.0)) {
    sm.process_event(SchemaReceivedEvent{});
    ctx.pendingSchemaResponse = false;
  }
}

void HandleInput(AppUIContext& ctx) {
  auto& conn = ctx.connection;
  auto& visual = ctx.visual;

  if (conn.portDropdownEdit) {
    if (IsKeyPressed(KEY_DOWN)) conn.currentPort = (conn.currentPort + 1) % (int)conn.portPaths.size();
    if (IsKeyPressed(KEY_UP))
      conn.currentPort = (conn.currentPort - 1 + (int)conn.portPaths.size()) % (int)conn.portPaths.size();
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) conn.portDropdownEdit = false;
  } else if (conn.baudDropdownEdit) {
    if (IsKeyPressed(KEY_DOWN)) conn.baudRateIndex = (conn.baudRateIndex + 1) % 5;
    if (IsKeyPressed(KEY_UP)) conn.baudRateIndex = (conn.baudRateIndex - 1 + 5) % 5;
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) conn.baudDropdownEdit = false;
  } else if (visual.themeDropdownEdit) {
    if (IsKeyPressed(KEY_DOWN)) {
      visual.themeIndex = (visual.themeIndex + 1) % visual.themeCount;
      ApplyTheme(visual.themeIndex);
    }
    if (IsKeyPressed(KEY_UP)) {
      visual.themeIndex = (visual.themeIndex - 1 + visual.themeCount) % visual.themeCount;
      ApplyTheme(visual.themeIndex);
    }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) visual.themeDropdownEdit = false;
  } else if (visual.shaderDropdownEdit) {
    if (IsKeyPressed(KEY_DOWN)) visual.currentShader = (visual.currentShader + 1) % visual.shaderCount;
    if (IsKeyPressed(KEY_UP)) visual.currentShader = (visual.currentShader - 1 + visual.shaderCount) % visual.shaderCount;
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) visual.shaderDropdownEdit = false;
  } else if (visual.fontDropdownEdit) {
    if (IsKeyPressed(KEY_DOWN)) visual.currentFont = (visual.currentFont + 1) % visual.fontCount;
    if (IsKeyPressed(KEY_UP)) visual.currentFont = (visual.currentFont - 1 + visual.fontCount) % visual.fontCount;
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) visual.fontDropdownEdit = false;
  }
}

static void DrawWelcomeScreen(AppUIContext& ctx) {
  const char* title = "ZonaiAnvil";
  float fontSize = 60.0f;
  float spacing = 2.0f;
  Font font = GuiGetFont();
  Vector2 textWidth = MeasureTextEx(font, title, fontSize, spacing);
  
  DrawTextEx(font, title, 
             (Vector2){(float)GetScreenWidth() / 2 - textWidth.x / 2, (float)GetScreenHeight() / 2 - fontSize / 2 - 20}, 
             fontSize, spacing, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));

  const char* subTitle = "Press any key to continue";
  float subFontSize = 20.0f;
  Vector2 subTextWidth = MeasureTextEx(font, subTitle, subFontSize, spacing);
  
  DrawTextEx(font, subTitle, 
             (Vector2){(float)GetScreenWidth() / 2 - subTextWidth.x / 2, (float)GetScreenHeight() / 2 + fontSize / 2 + 10}, 
             subFontSize, spacing, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
}

static void DrawSidebar(AppUIContext& ctx, AppSM& sm, CommunicationManager::Manager& comms) {
  using namespace boost::sml::literals;
  auto& conn = ctx.connection;
  auto& visual = ctx.visual;
  auto& fontMgr = FontManager::Manager::instance();

  float screenHeight = (float)GetScreenHeight();
  GuiGroupBox((Rectangle){10, 10, 200, screenHeight - 20}, "Settings");

  // 1. Static Button (Connect/Disconnect)
  if (sm.is("Disconnected"_s)) {
    if (GuiButton((Rectangle){20, 210, 180, 40}, "Connect")) {
      if (conn.currentPort < (int)conn.portPaths.size()) {
        std::string selectedPort = conn.portPaths[conn.currentPort];
        int baud = 9600;
        switch (conn.baudRateIndex) {
          case 1: baud = 19200; break;
          case 2: baud = 38400; break;
          case 3: baud = 57600; break;
          case 4: baud = 115200; break;
        }
        comms.connect(selectedPort, baud);
      }
    }
  } else {
    if (GuiButton((Rectangle){20, 210, 180, 40}, "Disconnect")) {
      comms.disconnect();
      ctx.device.config.clear();
      ctx.device.deviceLogs.clear();
    }
  }

  const char* statusStr = "STATUS: Disconnected";
  if (sm.is("Connecting"_s)) statusStr = "STATUS: Connecting...";
  else if (sm.is("FetchingSchema"_s)) statusStr = "STATUS: Syncing...";
  else if (sm.is("Connected"_s)) statusStr = "STATUS: Connected";
  GuiLabel((Rectangle){20, 255, 180, 20}, statusStr);

  // --- Dropdowns drawn BOTTOM-TO-TOP for correct layering ---

  // 1. Font (Bottom-most Y=440)
  if (ctx.anyDropdownOpen() && !visual.fontDropdownEdit) GuiLock();
  GuiLabel((Rectangle){20, 420, 180, 20}, "UI Font:");
  int pf = visual.currentFont;
  if (GuiDropdownBox((Rectangle){20, 440, 180, 30}, visual.fonts, &visual.currentFont, visual.fontDropdownEdit)) {
    visual.fontDropdownEdit = !visual.fontDropdownEdit;
  }
  if (visual.currentFont != pf) {
      ApplyFont(fontMgr.getFont(static_cast<FontManager::FontType>(visual.currentFont)), 18);
  }
  GuiUnlock();

  // 2. Shader (Y=380)
  if (ctx.anyDropdownOpen() && !visual.shaderDropdownEdit) GuiLock();
  GuiLabel((Rectangle){20, 360, 180, 20}, "Shader Effect:");
  if (GuiDropdownBox((Rectangle){20, 380, 180, 30}, visual.shaders, &visual.currentShader, visual.shaderDropdownEdit)) {
    visual.shaderDropdownEdit = !visual.shaderDropdownEdit;
  }
  GuiUnlock();

  // 3. Theme (Y=310)
  if (ctx.anyDropdownOpen() && !visual.themeDropdownEdit) GuiLock();
  GuiLabel((Rectangle){20, 290, 180, 20}, "UI Theme:");
  int pt = visual.themeIndex;
  if (GuiDropdownBox((Rectangle){20, 310, 180, 30}, visual.themes, &visual.themeIndex, visual.themeDropdownEdit)) {
    visual.themeDropdownEdit = !visual.themeDropdownEdit;
  }
  if (visual.themeIndex != pt) {
      ApplyTheme(visual.themeIndex);
      ApplyFont(fontMgr.getFont(static_cast<FontManager::FontType>(visual.currentFont)), 18);
  }
  GuiUnlock();

  // 4. Baud Rate (Y=110)
  if (ctx.anyDropdownOpen() && !conn.baudDropdownEdit) GuiLock();
  GuiLabel((Rectangle){20, 90, 180, 20}, "Baud Rate:");
  if (GuiDropdownBox((Rectangle){20, 110, 180, 30}, conn.baudRates, &conn.baudRateIndex, conn.baudDropdownEdit)) {
    conn.baudDropdownEdit = !conn.baudDropdownEdit;
  }
  GuiUnlock();

  // 5. Serial Port (Top-most Y=50)
  if (ctx.anyDropdownOpen() && !conn.portDropdownEdit) GuiLock();
  GuiLabel((Rectangle){20, 30, 180, 20}, "Serial Port:");
  if (GuiDropdownBox((Rectangle){20, 50, 180, 30}, conn.portList, &conn.currentPort, conn.portDropdownEdit)) {
    conn.portDropdownEdit = !conn.portDropdownEdit;
  }
  GuiUnlock();
}

static void DrawConfigPanel(AppUIContext& ctx, AppSM& sm, ProtocolHandler* protocol) {
  using namespace boost::sml::literals;
  auto& device = ctx.device;

  float screenWidth = (float)GetScreenWidth();
  float screenHeight = (float)GetScreenHeight();
  float panelWidth = screenWidth - 220 - 10;
  float logPanelHeight = 150.0f;
  float configPanelHeight = screenHeight - logPanelHeight - 30;

  const char* configTitle = device.connectedDeviceName.empty()
                                ? "Configuration"
                                : TextFormat("Configuration [%s]", device.connectedDeviceName.c_str());
  GuiGroupBox((Rectangle){220, 10, panelWidth, configPanelHeight}, configTitle);

  if (sm.is("Connected"_s)) {
    float availableWidth = panelWidth - 40;
    float itemWidth = 300.0f;
    float itemHeight = 80.0f;
    int itemsPerRow = (int)(availableWidth / itemWidth);
    if (itemsPerRow < 1) itemsPerRow = 1;

    int totalRows = device.config.empty() ? 0 : (int)((device.config.size() + itemsPerRow - 1) / itemsPerRow);
    float totalContentHeight = totalRows * itemHeight + 20;

    Rectangle scrollBounds = {230, 40, panelWidth - 20, configPanelHeight - 80};
    Rectangle contentBounds = {0, 0, panelWidth - 40, totalContentHeight};
    Rectangle view = {0, 0, 0, 0};
    GuiScrollPanel(scrollBounds, NULL, contentBounds, &device.configScroll, &view);

    BeginScissorMode((int)view.x, (int)view.y, (int)view.width, (int)view.height);

    for (size_t i = 0; i < device.config.size(); ++i) {
      auto& param = device.config[i];
      int row = (int)(i / itemsPerRow);
      int col = (int)(i % itemsPerRow);

      float posX = col * itemWidth + 40;
      float posY = row * itemHeight + 10;

      float drawX = posX + device.configScroll.x + scrollBounds.x;
      float drawY = posY + device.configScroll.y + scrollBounds.y;

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

  float logContentHeight = device.deviceLogs.size() * 20.0f + 10.0f;
  Rectangle logScrollBounds = {230, logPanelY + 20, panelWidth - 20, logPanelHeight - 30};
  Rectangle logContentBounds = {0, 0, panelWidth - 40, logContentHeight};
  Rectangle logView = {0, 0, 0, 0};

  GuiScrollPanel(logScrollBounds, NULL, logContentBounds, &device.logScroll, &logView);

  BeginScissorMode((int)logView.x, (int)logView.y, (int)logView.width, (int)logView.height);
  for (size_t i = 0; i < device.deviceLogs.size(); ++i) {
    auto& log = device.deviceLogs[i];
    float drawY = logPanelY + 25 + i * 20 + device.logScroll.y;

    if (drawY + 20 > logScrollBounds.y && drawY < logScrollBounds.y + logScrollBounds.height) {
      Color color = GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL));
      if (log.level == 1) color = YELLOW;
      else if (log.level >= 2) color = RED;

      float spacing = (float)GuiGetStyle(DEFAULT, TEXT_SPACING);
      float fontSize = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
      DrawTextEx(GuiGetFont(), TextFormat("[%.2f] %s", log.timestamp, log.message.c_str()), 
                 (Vector2){(float)logScrollBounds.x + 5, (float)drawY}, fontSize, spacing, color);
    }
  }
  EndScissorMode();
}

void Draw(AppUIContext& ctx, AppSM& sm, CommunicationManager::Manager& comms) {
  using namespace boost::sml::literals;
  
  ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

  if (sm.is("Welcome"_s)) {
    DrawWelcomeScreen(ctx);
  } else {
    DrawConfigPanel(ctx, sm, comms.getProtocol());
    DrawSidebar(ctx, sm, comms);
  }
}

}  // namespace UIManager
