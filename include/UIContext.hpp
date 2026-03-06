#pragma once
#include <string>
#include <vector>
#include "DeviceParameter.hpp"
#include "raylib.h"

// --- Build Configuration ---
#define ENABLE_UI_DELAYS 1
#ifdef ENABLE_UI_DELAYS
constexpr int kTransitionDelayMs = 500;
#else
constexpr int kTransitionDelayMs = 0;
#endif

struct DeviceLog {
  uint8_t level;
  std::string message;
  double timestamp;
};

struct ConnectionSettings {
  int currentPort = 0;
  int baudRateIndex = 0;
  bool portDropdownEdit = false;
  bool baudDropdownEdit = false;

  char portList[2048] = {0};
  std::vector<std::string> portPaths;
  const char* baudRates = "9600;19200;38400;57600;115200";

  bool isDropdownOpen() const { return portDropdownEdit || baudDropdownEdit; }
};

struct VisualSettings {
  int themeIndex = 0;
  bool themeDropdownEdit = false;
  int currentShader = 0;
  bool shaderDropdownEdit = false;
  int currentFont = 0;
  bool fontDropdownEdit = false;

  const char* themes = "TermX;Mocha;Macchiato;Frappe;Latte";
  int themeCount = 5;
  const char* shaders = "None;CRT;Bayer 2x2;Bayer 4x4";
  int shaderCount = 4;
  const char* fonts = "Default;Sans;Mono";
  int fontCount = 3;

  bool isDropdownOpen() const { return themeDropdownEdit || shaderDropdownEdit || fontDropdownEdit; }
};

struct DeviceState {
  std::vector<DeviceParameter> config;
  std::vector<DeviceLog> deviceLogs;
  std::string connectedDeviceName = "";
  Vector2 configScroll = {0, 0};
  Vector2 logScroll = {0, 0};
};

struct AppUIContext {
  ConnectionSettings connection;
  VisualSettings visual;
  DeviceState device;

  // Internal Timers/Flags
  double stateTransitionTime = 0.0;
  double welcomeTimer = 0.0;
  bool pendingSchemaResponse = false;

  bool anyDropdownOpen() const { return connection.isDropdownOpen() || visual.isDropdownOpen(); }
};
