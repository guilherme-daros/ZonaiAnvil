#include "CommunicationManager.hpp"
#include <chrono>
#include <thread>
#include "LinuxSerialPort.hpp"
#include "Log.hpp"
#include "MockSerialPort.hpp"

namespace CommunicationManager {

Manager::Manager(AppUIContext& ctx, AppSM& sm) : ctx(ctx), sm(sm) {
  realSerial = std::make_unique<LinuxSerialPort>();
  mockSerial = std::make_unique<MockSerialPort>();
}

Manager::~Manager() { disconnect(); }

std::vector<std::string> Manager::listPorts() {
  auto ports = realSerial->listPorts();
  ports.push_back("ttyMock1");
  ports.push_back("ttyMock2");
  ports.push_back("ttyMock3");
  return ports;
}

void Manager::update() {
  if (protocol) {
    protocol->update();
  }
}

void Manager::connect(const std::string& port, int baud) {
  using namespace boost::sml::literals;

  if (port.find("ttyMock") != std::string::npos) {
    activeComm = mockSerial.get();
  } else {
    activeComm = realSerial.get();
  }

  if (activeComm && activeComm->open(port, baud)) {
    ctx.device.connectedDeviceName = port;
    sm.process_event(ConnectEvent{port, baud});

    // Handle optional delay
    if (kTransitionDelayMs > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(kTransitionDelayMs));
    }

    sm.process_event(ConnectionSuccessEvent{});
    setupProtocol();
  }
}

void Manager::disconnect() {
  using namespace boost::sml::literals;
  if (activeComm) {
    activeComm->close();
  }
  activeComm = nullptr;
  protocol.reset();
  ctx.device.connectedDeviceName = "";
  sm.process_event(DisconnectEvent{});
}

bool Manager::isConnected() const { return activeComm && activeComm->isOpen(); }

void Manager::setupProtocol() {
  if (!activeComm || !activeComm->isOpen()) return;

  protocol = std::make_unique<ProtocolHandler>(activeComm);

  // Wire up listeners from old UIManager logic
  protocol->onSchemaReceived = [&](const std::vector<DeviceParameter>& s) {
    ctx.device.config = s;
    for (auto& p : ctx.device.config) {
      p.lastSentValue = p.value;
      p.lastSentString = p.stringValue;
    }
    protocol->requestAllValues();
    ctx.stateTransitionTime = GetTime();
    ctx.pendingSchemaResponse = true;  // Still using ctx to notify SM in main for now
  };

  protocol->onValuesReceived = [&](const std::vector<std::pair<uint8_t, float>>& v) {
    for (auto& val : v) {
      for (auto& p : ctx.device.config) {
        if (p.id == val.first) {
          p.value = val.second;
          p.lastSentValue = val.second;
        }
      }
    }
  };

  protocol->onWriteAck = [&](uint8_t id) {
    for (auto& p : ctx.device.config) {
      if (p.id == id) p.pending = false;
    }
  };

  protocol->onLogReceived = [&](uint8_t level, const std::string& msg) {
    ctx.device.deviceLogs.push_back({level, msg, (double)GetTime()});
    if (ctx.device.deviceLogs.size() > 100) ctx.device.deviceLogs.erase(ctx.device.deviceLogs.begin());
    ctx.device.logScroll.y = -1000000;  // Auto-scroll
  };

  protocol->requestSchema();
}

}  // namespace CommunicationManager
