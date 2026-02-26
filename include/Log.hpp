#pragma once

#include "logger/Logger.hpp"

namespace Log {
using Protocol = sb::logger::Logger<"Protocol">;
using App = sb::logger::Logger<"App">;
using SM = sb::logger::Logger<"SM">;
using Mock = sb::logger::Logger<"Mock">;
} // namespace Log
