#include "ProtocolHandler.hpp"
#include "Log.hpp"
#include <cstring>

ProtocolHandler::ProtocolHandler(ICommunication* comm) : comm(comm) {}

void ProtocolHandler::sendPing() { sendPacket(Protocol::Command::kPing); }
void ProtocolHandler::requestSchema() { sendPacket(Protocol::Command::kGetSchema); }
void ProtocolHandler::requestAllValues() { sendPacket(Protocol::Command::kReadAll); }

void ProtocolHandler::writeValue(uint8_t id, float value) {
  std::vector<uint8_t> payload;
  payload.push_back(id);
  uint8_t bytes[4];
  std::memcpy(bytes, &value, 4);
  payload.insert(payload.end(), bytes, bytes + 4);
  sendPacket(Protocol::Command::kWriteValue, payload);
}

void ProtocolHandler::writeAll(const std::vector<float>& values) {
  std::vector<uint8_t> payload;
  payload.push_back(static_cast<uint8_t>(values.size()));
  for (float v : values) {
    uint8_t bytes[4];
    std::memcpy(bytes, &v, 4);
    payload.insert(payload.end(), bytes, bytes + 4);
  }
  sendPacket(Protocol::Command::kWriteAll, payload);
}

void ProtocolHandler::update() {
  if (!comm || !comm->isOpen()) return;

  // Read new data into buffer
  std::vector<uint8_t> newData = comm->read(512);
  rxBuffer.insert(rxBuffer.end(), newData.begin(), newData.end());

  // Parse buffer for packets
  while (rxBuffer.size() >= sizeof(Protocol::PacketHeader)) {
    // Find start byte
    if (rxBuffer[0] != Protocol::kStartByte) {
      rxBuffer.erase(rxBuffer.begin());
      continue;
    }

    const auto* header = reinterpret_cast<const Protocol::PacketHeader*>(rxBuffer.data());
    std::size_t totalSize = sizeof(Protocol::PacketHeader) + header->length + 1;  // +1 for checksum

    if (rxBuffer.size() < totalSize) break;  // Wait for more data

    // Extract payload
    std::vector<uint8_t> payload(rxBuffer.begin() + sizeof(Protocol::PacketHeader), rxBuffer.begin() + totalSize - 1);
    uint8_t receivedChecksum = rxBuffer[totalSize - 1];

    if (receivedChecksum == Protocol::CalculateChecksum(payload)) {
      processPacket(*header, payload);
    } else {
      Log::Protocol::Error() << "Checksum mismatch for command 0x" << std::hex << (int)header->command;
    }

    rxBuffer.erase(rxBuffer.begin(), rxBuffer.begin() + totalSize);
  }
}

void ProtocolHandler::sendPacket(Protocol::Command cmd, const std::vector<uint8_t>& payload) {
  if (!comm || !comm->isOpen()) return;

  Protocol::PacketHeader header;
  header.startByte = Protocol::kStartByte;
  header.command = static_cast<uint8_t>(cmd);
  header.length = static_cast<uint16_t>(payload.size());

  std::vector<uint8_t> packet;
  uint8_t* hPtr = reinterpret_cast<uint8_t*>(&header);
  packet.insert(packet.end(), hPtr, hPtr + sizeof(header));
  packet.insert(packet.end(), payload.begin(), payload.end());
  packet.push_back(Protocol::CalculateChecksum(payload));

  comm->write(packet);
}

void ProtocolHandler::writeString(uint8_t id, const std::string& value) {
  std::vector<uint8_t> payload;
  payload.push_back(id);
  payload.push_back(static_cast<uint8_t>(value.length()));
  payload.insert(payload.end(), value.begin(), value.end());
  sendPacket(Protocol::Command::kWriteValue, payload);
}

void ProtocolHandler::processPacket(const Protocol::PacketHeader& header, const std::vector<uint8_t>& payload) {
  Protocol::Command cmd = static_cast<Protocol::Command>(header.command);

  // If we are in slave mode (simulator), trigger the command callback
  if (onCommandReceived) {
    onCommandReceived(cmd, payload);
  }

  // Master mode logic (handling responses from device)
  switch (cmd) {
    case Protocol::Command::kGetSchema: {
      if (payload.empty()) return;
      std::vector<DeviceParameter> schema;
      uint8_t count = payload[0];
      std::size_t offset = 1;
      for (int i = 0; i < count; i++) {
        DeviceParameter p;
        p.id = payload[offset++];
        p.type = static_cast<Protocol::ParamType>(payload[offset++]);
        uint8_t nameLen = payload[offset++];
        p.name = std::string(reinterpret_cast<const char*>(&payload[offset]), nameLen);
        offset += nameLen;
        std::memcpy(&p.min, &payload[offset], 4);
        offset += 4;
        std::memcpy(&p.max, &payload[offset], 4);
        offset += 4;
        p.value = p.min;  // Default
        schema.push_back(p);
      }
      if (onSchemaReceived) onSchemaReceived(schema);
      break;
    }
    case Protocol::Command::kReadAll: {
      if (payload.empty()) return;
      std::vector<std::pair<uint8_t, float>> values;
      uint8_t count = payload[0];
      std::size_t offset = 1;
      for (int i = 0; i < count; i++) {
        uint8_t id = payload[offset++];
        float val;
        std::memcpy(&val, &payload[offset], 4);
        offset += 4;
        values.push_back({id, val});
      }
      if (onValuesReceived) onValuesReceived(values);
      break;
    }
    case Protocol::Command::kWriteValue: {
      if (payload.size() >= 1) {
        if (onWriteAck) onWriteAck(payload[0]);
      }
      break;
    }
    case Protocol::Command::kPing:
      Log::Protocol::Info() << "Ping/Ack received.";
      break;
    case Protocol::Command::kLog: {
      if (payload.size() >= 1) {
        uint8_t level = payload[0];
        std::string msg(reinterpret_cast<const char*>(&payload[1]), payload.size() - 1);
        if (onLogReceived) onLogReceived(level, msg);
      }
      break;
    }
  }
}
