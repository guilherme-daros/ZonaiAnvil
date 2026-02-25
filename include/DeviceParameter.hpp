#pragma once
#include <cstdint>
#include <string>
#include "Protocol.hpp"

struct DeviceParameter {
  uint8_t id;
  Protocol::ParamType type;
  std::string name;
  float value;
  float min;
  float max;
  std::string stringValue;

  // UI State
  bool pending = false;
  float lastSentValue = 0.0f;
  std::string lastSentString;
  bool editMode = false;
};
