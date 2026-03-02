#pragma once
#include <string>
#include <string_view>
#include "Log.hpp"
#include "sml.hpp"

namespace sml = boost::sml;

struct SmlLogger {
  template <class SM, class TEvent>
  void log_process_event(const TEvent&) {
    if (isInternal(sml::aux::get_type_name<TEvent>())) return;
    Log::StateMachine::Debug() << "process_event : " << sml::aux::get_type_name<TEvent>();
  }

  template <class SM, class TGuard, class TEvent>
  void log_guard(const TGuard&, const TEvent&, bool result) {
    if (isInternal(sml::aux::get_type_name<TGuard>())) return;
    Log::StateMachine::Debug() << "guard: " << sml::aux::get_type_name<TGuard>() << " "
                               << (result ? "[OK]" : "[Reject]");
  }

  template <class SM, class TAction, class TEvent>
  void log_action(const TAction&, const TEvent&) {
    if (isInternal(sml::aux::get_type_name<TAction>())) return;
    Log::StateMachine::Debug() << "action: " << sml::aux::get_type_name<TAction>();
  }

  template <class SM, class TSrcState, class TDstState>
  void log_state_change(const TSrcState& src, const TDstState& dst) {
    if (isInternal(src.c_str()) || isInternal(dst.c_str())) return;
    Log::StateMachine::Debug() << "transition: " << src.c_str() << " -> " << dst.c_str();
  }

 private:
  static bool isInternal(std::string_view name) {
    return name.find("boost::ext::sml") != std::string_view::npos || name.find("boost::sml") != std::string_view::npos;
  }
};

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
    Log::StateMachine::Info() << "Opening connection to " << event.port;
  }
};

struct RequestSchema {
  void operator()() { Log::StateMachine::Info() << "Requesting Schema from device..."; }
};

struct CloseConnection {
  void operator()() { Log::StateMachine::Info() << "Closing connection"; }
};

// State Machine Definition
struct AppStateController {
  auto operator()() const noexcept {
    using namespace sml;
    using namespace sml::literals;

    /**
     * Transitions:
     * Welcome        -> Disconnected   (on WelcomeTimerEvent)
     * Disconnected   -> Connecting     (on ConnectEvent)
     * Connecting     -> FetchingSchema (on ConnectionSuccessEvent)
     * FetchingSchema -> Connected      (on SchemaReceivedEvent)
     * *              -> Disconnected   (on DisconnectEvent or ConnectionFailed)
     */
    // clang-format off
    return make_transition_table(
      *"Welcome"_s        + event<WelcomeTimerEvent>                          = "Disconnected"_s,
       "Disconnected"_s   + event<ConnectEvent>           / OpenConnection{}  = "Connecting"_s,
       "Connecting"_s     + event<ConnectionSuccessEvent> / RequestSchema{}   = "FetchingSchema"_s,
       "Connecting"_s     + event<ConnectionFailedEvent>                      = "Disconnected"_s,
       "FetchingSchema"_s + event<SchemaReceivedEvent>                        = "Connected"_s,
       "FetchingSchema"_s + event<ConnectionFailedEvent>                      = "Disconnected"_s,
       "FetchingSchema"_s + event<DisconnectEvent>        / CloseConnection{} = "Disconnected"_s,
       "Connected"_s      + event<DisconnectEvent>        / CloseConnection{} = "Disconnected"_s,
       "Connected"_s      + event<ConnectionFailedEvent>                      = "Disconnected"_s);
  }
    // clang-format on 
};

using AppSM = boost::sml::sm<AppStateController, boost::sml::logger<SmlLogger>>;
