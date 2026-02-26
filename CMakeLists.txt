cmake_minimum_required(VERSION 3.16)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
project(ZonaiAnvil VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch Raylib
include(FetchContent)
FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 5.0
)
FetchContent_MakeAvailable(raylib)

# Common sources
set(COMMON_SOURCES
    src/LinuxSerialPort.cpp
    src/ProtocolHandler.cpp
)

add_subdirectory(third_party/sb-logger)

# Main Application
add_executable(ZonaiAnvil
    apps/ZonaiAnvil/main.cpp
    src/UIManager.cpp
    ${COMMON_SOURCES}
)

target_include_directories(ZonaiAnvil PRIVATE
    include
    third_party
)

target_link_libraries(ZonaiAnvil PRIVATE raylib sb::logger m pthread dl rt X11)
