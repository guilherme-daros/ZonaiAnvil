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

struct AppUIContext {
  // Selection state
  int currentPort = 0;
  int baudRateIndex = 0;
  int themeIndex = 0;  // TermX by default

  // Dropdown modes
  bool portDropdownEdit = false;
  bool baudDropdownEdit = false;
  bool themeDropdownEdit = false;
  bool crtEnabled = false;

  // Static lists
  char portList[2048] = {0};
  std::vector<std::string> portPaths;
  const char* baudRates = "9600;19200;38400;57600;115200";
  const char* themes = "TermX;Cyber;Dark;Terminal";
  int themeCount = 4;

  // Dynamic Device Config
  std::vector<DeviceParameter> config;
  std::vector<DeviceLog> deviceLogs;
  std::string connectedDeviceName = "";

  // Internal Timers/Flags
  double stateTransitionTime = 0.0;
  double welcomeTimer = 0.0;
  bool pendingSchemaResponse = false;
  Vector2 configScroll = {0, 0};
  Vector2 logScroll = {0, 0};

  bool anyDropdownOpen() const { return portDropdownEdit || baudDropdownEdit || themeDropdownEdit; }
};
