#pragma once
#include <iostream>
#include <string>
#include "sml.hpp"

namespace sml = boost::sml;

// Events
struct ConnectEvent {
  std::string port;
  int baudRate;
};
struct ConnectionSuccessEvent {};
struct ConnectionFailedEvent {};
struct SchemaReceivedEvent {};
struct DisconnectEvent {};
struct WelcomeTimerEvent {};

// Actions
struct OpenConnection {
  template <class TEvent>
  void operator()(const TEvent& event) {
    std::cout << "[SM] Action: Opening connection to " << event.port << std::endl;
  }
};

struct RequestSchema {
  void operator()() { std::cout << "[SM] Action: Requesting Schema from device..." << std::endl; }
};

struct CloseConnection {
  void operator()() { std::cout << "[SM] Action: Closing connection" << std::endl; }
};

// State Machine Definition
struct AppStateController {
  auto operator()() const noexcept {
    using namespace sml;
    using namespace sml::literals;

    /**
     * Transitions:
     * Welcome      -> Disconnected (on WelcomeTimerEvent)
     * Disconnected -> Connecting (on ConnectEvent)
     * Connecting   -> FetchingSchema (on ConnectionSuccessEvent)
     * FetchingSchema -> Connected (on SchemaReceivedEvent)
     * * -> Disconnected (on DisconnectEvent or ConnectionFailed)
     */
    return make_transition_table(*"Welcome"_s + event<WelcomeTimerEvent> = "Disconnected"_s,
                                 "Disconnected"_s + event<ConnectEvent> / OpenConnection{} = "Connecting"_s,
                                 "Connecting"_s + event<ConnectionSuccessEvent> / RequestSchema{} = "FetchingSchema"_s,
                                 "Connecting"_s + event<ConnectionFailedEvent> = "Disconnected"_s,

                                 "FetchingSchema"_s + event<SchemaReceivedEvent> = "Connected"_s,
                                 "FetchingSchema"_s + event<ConnectionFailedEvent> = "Disconnected"_s,
                                 "FetchingSchema"_s + event<DisconnectEvent> / CloseConnection{} = "Disconnected"_s,

                                 "Connected"_s + event<DisconnectEvent> / CloseConnection{} = "Disconnected"_s,
                                 "Connected"_s + event<ConnectionFailedEvent> = "Disconnected"_s);
  }
};
