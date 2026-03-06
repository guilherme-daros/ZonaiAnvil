#pragma once
#include <map>
#include <string>
#include "raylib.h"

namespace ShaderManager {

enum class ShaderType {
    None = 0,
    CRT,
    Bayer2x2,
    Bayer4x4
};

struct ShaderInstance {
    Shader shader;
    int renderSizeLoc;
    bool loaded = false;
};

class Manager {
public:
    static Manager& instance() {
        static Manager inst;
        if (!inst.initialized) inst.init();
        return inst;
    }

    // Disable copy and move
    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;
    Manager(Manager&&) = delete;
    Manager& operator=(Manager&&) = delete;

    void updateUniforms(float width, float height);
    void begin(ShaderType type);
    void end();
    
    Shader getShader(ShaderType type);

private:
    Manager();
    ~Manager();

    void init();
    void loadFromMemory(ShaderType type, const unsigned char* data);
    void unloadAll();

    std::map<ShaderType, ShaderInstance> shaders;
    bool initialized = false;
};

} // namespace ShaderManager
