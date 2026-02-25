#pragma once
#include <functional>
#include <memory>
#include <vector>
#include "DeviceParameter.hpp"
#include "ICommunication.hpp"
#include "Protocol.hpp"

class ProtocolHandler {
 public:
  explicit ProtocolHandler(ICommunication* comm);

  // Master Methods (Requesting)
  void sendPing();
  void requestSchema();
  void requestAllValues();
  void writeValue(uint8_t id, float value);
  void writeString(uint8_t id, const std::string& value);
  void writeAll(const std::vector<float>& values);

  // General Methods
  void sendPacket(Protocol::Command cmd, const std::vector<uint8_t>& payload = {});
  void update();

  // Callbacks for Master Role (UI)
  std::function<void(const std::vector<DeviceParameter>&)> onSchemaReceived;
  std::function<void(const std::vector<std::pair<uint8_t, float>>&)> onValuesReceived;
  std::function<void(uint8_t)> onWriteAck;

  // Callbacks for Slave Role (Simulator/Device)
  std::function<void(Protocol::Command, const std::vector<uint8_t>&)> onCommandReceived;

 private:
  void processPacket(const Protocol::PacketHeader& header, const std::vector<uint8_t>& payload);

  ICommunication* comm;
  std::vector<uint8_t> rxBuffer;
};
