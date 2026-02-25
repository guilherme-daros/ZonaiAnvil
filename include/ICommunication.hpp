#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

class ICommunication {
 public:
  virtual ~ICommunication() = default;
  virtual bool open(const std::string& port, int baudRate) = 0;
  virtual void close() = 0;
  virtual bool isOpen() const = 0;
  virtual std::size_t write(const std::vector<uint8_t>& data) = 0;
  virtual std::vector<uint8_t> read(std::size_t maxSize) = 0;
  virtual std::vector<std::string> listPorts() = 0;
};
