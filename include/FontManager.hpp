#pragma once
#include <map>
#include <string>
#include "raylib.h"

namespace FontManager {

enum class FontType {
    Default = 0,
    Sans,
    Mono,
    Mecha
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

    Font getFont(FontType type);
    Shader getSDFShader() const { return sdfShader; }

private:
    Manager();
    ~Manager();

    void init();
    void loadFromMemory(FontType type, const unsigned char* data, int dataSize);
    void unloadAll();

    std::map<FontType, Font> fonts;
    Shader sdfShader;
    bool initialized = false;
};

} // namespace FontManager
