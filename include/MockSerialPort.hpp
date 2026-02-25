#pragma once
#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>
#include <vector>
#include "ICommunication.hpp"
#include "Protocol.hpp"

struct MockParam {
  uint8_t id;
  uint8_t type;
  std::string name;
  float value;
  float min;
  float max;
  std::string stringValue;
};

struct DelayedResponse {
  std::chrono::steady_clock::time_point readyTime;
  std::vector<uint8_t> data;
};

class MockSerialPort : public ICommunication {
 public:
  MockSerialPort() : isOpenFlag(false), activePortName("") {}

  bool open(const std::string& port, int baudRate) override {
    if (port.find("ttyMock") != std::string::npos) {
      isOpenFlag = true;
      activePortName = port;

      params.clear();
      if (port == "ttyMock1") {
        params = {{0, static_cast<uint8_t>(Protocol::ParamType::kToggle), "Mock1 Power", 1.0f, 0.0f, 1.0f, ""},
                  {1, static_cast<uint8_t>(Protocol::ParamType::kSlider), "Mock1 Level", 50.0f, 0.0f, 100.0f, ""}};
      } else if (port == "ttyMock2") {
        params = {{10, static_cast<uint8_t>(Protocol::ParamType::kToggle), "Mock2 Turbo", 0.0f, 0.0f, 1.0f, ""},
                  {11, static_cast<uint8_t>(Protocol::ParamType::kToggle), "Mock2 LED", 1.0f, 0.0f, 1.0f, ""},
                  {12, static_cast<uint8_t>(Protocol::ParamType::kSlider), "Mock2 Speed", 25.0f, 0.0f, 200.0f, ""},
                  {13, static_cast<uint8_t>(Protocol::ParamType::kNumeric), "Mock2 Goal", 150.0f, 0.0f, 500.0f, ""}};
      } else if (port == "ttyMock3") {
        params = {{20, static_cast<uint8_t>(Protocol::ParamType::kToggle), "Tgl 1", 0.0f, 0.0f, 1.0f, ""},
                  {21, static_cast<uint8_t>(Protocol::ParamType::kToggle), "Tgl 2", 1.0f, 0.0f, 1.0f, ""},
                  {22, static_cast<uint8_t>(Protocol::ParamType::kSlider), "Sld 1", 10.0f, 0.0f, 100.0f, ""},
                  {23, static_cast<uint8_t>(Protocol::ParamType::kSlider), "Sld 2", 80.0f, 0.0f, 100.0f, ""},
                  {24, static_cast<uint8_t>(Protocol::ParamType::kNumeric), "Num 1", 123.0f, 0.0f, 1000.0f, ""},
                  {25, static_cast<uint8_t>(Protocol::ParamType::kNumeric), "Num 2", 456.0f, 0.0f, 1000.0f, ""},
                  {26, static_cast<uint8_t>(Protocol::ParamType::kString), "Str 1", 0.0f, 0.0f, 0.0f, "Hello"},
                  {27, static_cast<uint8_t>(Protocol::ParamType::kString), "Str 2", 0.0f, 0.0f, 0.0f, "World"}};
      }

      std::cout << "[MOCK] Connected to " << port << " with " << params.size() << " parameters." << std::endl;
      return true;
    }
    return false;
  }

  void close() override {
    if (isOpenFlag) {
      std::cout << "[MOCK] Disconnected from " << activePortName << "." << std::endl;
      isOpenFlag = false;
      delayedResponses.clear();
    }
  }

  bool isOpen() const override { return isOpenFlag; }

  std::size_t write(const std::vector<uint8_t>& data) override {
    if (!isOpenFlag || data.size() < sizeof(Protocol::PacketHeader)) return 0;

    const auto* header = reinterpret_cast<const Protocol::PacketHeader*>(data.data());
    if (header->startByte != Protocol::kStartByte) return 0;

    Protocol::Command cmd = static_cast<Protocol::Command>(header->command);

    std::vector<uint8_t> payload;
    switch (cmd) {
      case Protocol::Command::kPing:
        payload = {0x01};
        break;

      case Protocol::Command::kGetSchema: {
        payload.push_back(static_cast<uint8_t>(params.size()));
        for (const auto& p : params) {
          payload.push_back(p.id);
          payload.push_back(p.type);
          payload.push_back(static_cast<uint8_t>(p.name.size()));
          payload.insert(payload.end(), p.name.begin(), p.name.end());
          uint8_t minMax[8];
          std::memcpy(minMax, &p.min, 4);
          std::memcpy(minMax + 4, &p.max, 4);
          payload.insert(payload.end(), minMax, minMax + 8);
        }
        break;
      }

      case Protocol::Command::kReadAll: {
        payload.push_back(static_cast<uint8_t>(params.size()));
        for (const auto& p : params) {
          payload.push_back(p.id);
          if (p.type == static_cast<uint8_t>(Protocol::ParamType::kString)) {
            payload.push_back(static_cast<uint8_t>(p.stringValue.size()));
            payload.insert(payload.end(), p.stringValue.begin(), p.stringValue.end());
          } else {
            uint8_t valBytes[4];
            std::memcpy(valBytes, &p.value, 4);
            payload.insert(payload.end(), valBytes, valBytes + 4);
          }
        }
        break;
      }

      case Protocol::Command::kWriteValue: {
        uint8_t id = 0xFF;
        if (data.size() >= sizeof(Protocol::PacketHeader) + 1) {
          id = data[sizeof(Protocol::PacketHeader)];
          for (auto& p : params) {
            if (p.id == id) {
              if (p.type == static_cast<uint8_t>(Protocol::ParamType::kString)) {
                uint8_t strLen = data[sizeof(Protocol::PacketHeader) + 1];
                p.stringValue =
                    std::string(reinterpret_cast<const char*>(&data[sizeof(Protocol::PacketHeader) + 2]), strLen);
                std::cout << "[MOCK:" << activePortName << "] Param " << (int)id << " updated to \"" << p.stringValue
                          << "\" (ACK in 500ms)" << std::endl;
              } else {
                float newVal;
                std::memcpy(&newVal, &data[sizeof(Protocol::PacketHeader) + 1], 4);
                p.value = newVal;
                std::cout << "[MOCK:" << activePortName << "] Param " << (int)id << " updated to " << newVal
                          << " (ACK in 500ms)" << std::endl;
              }
              break;
            }
          }
        }
        payload = {id};
        break;
      }
      default:
        break;
    }

    if (!payload.empty() || cmd == Protocol::Command::kPing) {
      queueDelayedResponse(cmd, payload);
    }

    return data.size();
  }

  std::vector<uint8_t> read(std::size_t maxSize) override {
    auto now = std::chrono::steady_clock::now();
    auto it = delayedResponses.begin();
    while (it != delayedResponses.end()) {
      if (now >= it->readyTime) {
        activeBuffer.insert(activeBuffer.end(), it->data.begin(), it->data.end());
        it = delayedResponses.erase(it);
      } else {
        ++it;
      }
    }

    if (activeBuffer.empty()) return {};
    std::size_t size = std::min(maxSize, activeBuffer.size());
    std::vector<uint8_t> chunk(activeBuffer.begin(), activeBuffer.begin() + size);
    activeBuffer.erase(activeBuffer.begin(), activeBuffer.begin() + size);
    return chunk;
  }

  std::vector<std::string> listPorts() override { return {"ttyMock1", "ttyMock2", "ttyMock3"}; }

 private:
  void queueDelayedResponse(Protocol::Command cmd, const std::vector<uint8_t>& payload) {
    Protocol::PacketHeader header;
    header.startByte = Protocol::kStartByte;
    header.command = static_cast<uint8_t>(cmd);
    header.length = static_cast<uint16_t>(payload.size());

    std::vector<uint8_t> fullPacket;
    uint8_t* hPtr = reinterpret_cast<uint8_t*>(&header);
    fullPacket.insert(fullPacket.end(), hPtr, hPtr + sizeof(header));
    fullPacket.insert(fullPacket.end(), payload.begin(), payload.end());
    fullPacket.push_back(Protocol::CalculateChecksum(payload));

    auto readyTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);
    delayedResponses.push_back({readyTime, fullPacket});
  }

  bool isOpenFlag;
  std::string activePortName;
  std::vector<MockParam> params;
  std::vector<uint8_t> activeBuffer;
  std::vector<DelayedResponse> delayedResponses;
};
