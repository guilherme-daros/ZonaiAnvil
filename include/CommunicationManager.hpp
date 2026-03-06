#pragma once
#include <memory>
#include <string>
#include <vector>
#include "AppStateMachine.hpp"
#include "ICommunication.hpp"
#include "ProtocolHandler.hpp"
#include "UIContext.hpp"

namespace CommunicationManager {

class Manager {
 public:
  Manager(AppUIContext& ctx, AppSM& sm);
  ~Manager();

  // Disable copy
  Manager(const Manager&) = delete;
  Manager& operator=(const Manager&) = delete;

  void update();
  void connect(const std::string& port, int baud);
  void disconnect();
  bool isConnected() const;

  ProtocolHandler* getProtocol() const { return protocol.get(); }
  ICommunication* getActiveComm() const { return activeComm; }

  // For listing ports
  std::vector<std::string> listPorts();

  // Internal access for mocks
  ICommunication* getRealSerial() const { return realSerial.get(); }
  ICommunication* getMockSerial() const { return mockSerial.get(); }

 private:
  void setupProtocol();

  AppUIContext& ctx;
  AppSM& sm;

  std::unique_ptr<ICommunication> realSerial;
  std::unique_ptr<ICommunication> mockSerial;
  ICommunication* activeComm = nullptr;

  std::unique_ptr<ProtocolHandler> protocol;
  bool pendingSchemaResponse = false;
};

}  // namespace CommunicationManager
