#pragma once

#include "logger/Logger.hpp"

namespace Log {
using Protocol = sb::logger::Logger<"Protocol">;
using App = sb::logger::Logger<"App">;
using StateMachine = sb::logger::Logger<"StateMachine">;
using SerialMock = sb::logger::Logger<"Mock">;
}  // namespace Log
