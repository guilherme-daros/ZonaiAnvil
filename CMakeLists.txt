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
    src/ShaderManager.cpp
    src/WindowSystem.cpp
    src/CommunicationManager.cpp
    src/FontManager.cpp
    src/ThemeManager.cpp
)

# Function to embed resources as C headers
function(embed_resource RESOURCE_PATH OUTPUT_VAR)
    get_filename_component(RESOURCE_NAME ${RESOURCE_PATH} NAME_WE)
    get_filename_component(RESOURCE_EXT ${RESOURCE_PATH} EXT)
    string(SUBSTRING ${RESOURCE_EXT} 1 -1 RESOURCE_EXT_CLEAN)
    set(HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/generated/${RESOURCE_NAME}_${RESOURCE_EXT_CLEAN}.h")
    
    add_custom_command(
        OUTPUT "${HEADER_FILE}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/generated"
        COMMAND xxd -i "${RESOURCE_PATH}" > "${HEADER_FILE}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${RESOURCE_PATH}"
        COMMENT "Embedding resource ${RESOURCE_PATH}"
    )
    list(APPEND ${OUTPUT_VAR} "${HEADER_FILE}")
    set(${OUTPUT_VAR} ${${OUTPUT_VAR}} PARENT_SCOPE)
endfunction()

set(EMBEDDED_HEADERS "")
embed_resource("resources/shaders/crt.fs" EMBEDDED_HEADERS)
embed_resource("resources/shaders/bayer2x2.fs" EMBEDDED_HEADERS)
embed_resource("resources/shaders/bayer4x4.fs" EMBEDDED_HEADERS)
embed_resource("resources/shaders/sdf.fs" EMBEDDED_HEADERS)
embed_resource("resources/fonts/IBMPlexMono-Regular.ttf" EMBEDDED_HEADERS)
embed_resource("resources/fonts/IBMPlexSans-Regular.ttf" EMBEDDED_HEADERS)
embed_resource("resources/fonts/Mecha.ttf" EMBEDDED_HEADERS)

add_subdirectory(third_party/sb-logger)

# Main Application
add_executable(ZonaiAnvil
    apps/ZonaiAnvil/main.cpp
    src/UIManager.cpp
    ${COMMON_SOURCES}
    ${EMBEDDED_HEADERS}
)

target_include_directories(ZonaiAnvil PRIVATE
    include
    third_party
    resources
    "${CMAKE_CURRENT_BINARY_DIR}/generated"
)

target_link_libraries(ZonaiAnvil PRIVATE raylib sb::logger m pthread dl rt X11)
