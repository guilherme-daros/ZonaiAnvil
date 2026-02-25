#include "LinuxSerialPort.hpp"
#include <cstring>
#include <filesystem>
#include <iostream>

LinuxSerialPort::LinuxSerialPort() : fd(-1) {}

LinuxSerialPort::~LinuxSerialPort() { close(); }

bool LinuxSerialPort::open(const std::string& port, int baudRate) {
  if (isOpen()) close();

  fd = ::open(port.c_str(), O_RDWR | O_NOCTTY | O_SYNC | O_NONBLOCK);
  if (fd < 0) {
    perror("Error opening serial port");
    return false;
  }

  if (tcgetattr(fd, &tty) != 0) {
    perror("Error from tcgetattr");
    return false;
  }

  speed_t baud = translateBaud(baudRate);
  cfsetospeed(&tty, baud);
  cfsetispeed(&tty, baud);

  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;  // 8-bit chars
  tty.c_iflag &= ~IGNBRK;                      // disable break processing
  tty.c_lflag = 0;                             // no signaling chars, no echo,
                                               // no canonical processing
  tty.c_oflag = 0;                             // no remapping, no delays
  tty.c_cc[VMIN] = 0;                          // read doesn't block
  tty.c_cc[VTIME] = 5;                         // 0.5 seconds read timeout

  tty.c_iflag &= ~(IXON | IXOFF | IXANY);  // shut off xon/xoff ctrl

  tty.c_cflag |= (CLOCAL | CREAD);    // ignore modem controls,
                                      // enable reading
  tty.c_cflag &= ~(PARENB | PARODD);  // shut off parity
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CRTSCTS;

  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    perror("Error from tcsetattr");
    return false;
  }

  return true;
}

void LinuxSerialPort::close() {
  if (isOpen()) {
    ::close(fd);
    fd = -1;
  }
}

bool LinuxSerialPort::isOpen() const { return fd >= 0; }

std::size_t LinuxSerialPort::write(const std::vector<uint8_t>& data) {
  if (!isOpen()) return 0;
  ssize_t bytesWritten = ::write(fd, data.data(), data.size());
  return bytesWritten > 0 ? static_cast<std::size_t>(bytesWritten) : 0;
}

std::vector<uint8_t> LinuxSerialPort::read(std::size_t maxSize) {
  if (!isOpen()) return {};
  std::vector<uint8_t> buffer(maxSize);
  ssize_t bytesRead = ::read(fd, buffer.data(), maxSize);
  if (bytesRead > 0) {
    buffer.resize(bytesRead);
    return buffer;
  }
  return {};
}

std::vector<std::string> LinuxSerialPort::listPorts() {
  std::vector<std::string> ports;
  for (const auto& entry : std::filesystem::directory_iterator("/dev")) {
    std::string path = entry.path().string();
    if (path.find("/dev/ttyUSB") != std::string::npos || path.find("/dev/ttyACM") != std::string::npos) {
      ports.push_back(path);
    }
  }
  return ports;
}

speed_t LinuxSerialPort::translateBaud(int baudRate) {
  switch (baudRate) {
    case 9600:
      return B9600;
    case 19200:
      return B19200;
    case 38400:
      return B38400;
    case 57600:
      return B57600;
    case 115200:
      return B115200;
    default:
      return B9600;
  }
}
