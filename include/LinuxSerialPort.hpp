#pragma once
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string>
#include <vector>
#include "ICommunication.hpp"

class LinuxSerialPort : public ICommunication {
 public:
  LinuxSerialPort();
  ~LinuxSerialPort() override;

  bool open(const std::string& port, int baudRate) override;
  void close() override;
  bool isOpen() const override;
  std::size_t write(const std::vector<uint8_t>& data) override;
  std::vector<uint8_t> read(std::size_t maxSize) override;
  std::vector<std::string> listPorts() override;

 private:
  int fd;
  struct termios tty;
  speed_t translateBaud(int baudRate);
};
