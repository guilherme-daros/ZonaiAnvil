#include "FontManager.hpp"
#include <iostream>
#include "Log.hpp"

// Generated headers
#include "IBMPlexMono-Regular_ttf.h"
#include "IBMPlexSans-Regular_ttf.h"
#include "Mecha_ttf.h"
#include "sdf_fs.h"

namespace FontManager {

Manager::Manager() {}

Manager::~Manager() {
    unloadAll();
}

void Manager::init() {
    if (initialized) return;
    Log::App::Info() << "Initializing Font Manager";

  std::string sdfSource((const char*)resources_shaders_sdf_fs, resources_shaders_sdf_fs_len);
  sdfShader = LoadShaderFromMemory(0, sdfSource.c_str());

  // Load Fonts
  loadFromMemory(FontType::Mono, resources_fonts_IBMPlexMono_Regular_ttf, resources_fonts_IBMPlexMono_Regular_ttf_len);
  loadFromMemory(FontType::Sans, resources_fonts_IBMPlexSans_Regular_ttf, resources_fonts_IBMPlexSans_Regular_ttf_len);
  loadFromMemory(FontType::Mecha, resources_fonts_Mecha_ttf, resources_fonts_Mecha_ttf_len);

  // Alias Default to Mecha (or Sans if you prefer, but Mecha is what styles use)
  fonts[FontType::Default] = fonts[FontType::Mecha];

  initialized = true;
}

void Manager::loadFromMemory(FontType type, const unsigned char* data, int dataSize) {
  int baseSize = 64;
  int fontCharsCount = 95;

  Font font = LoadFontFromMemory(".ttf", data, dataSize, baseSize, 0, fontCharsCount);
  SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);

  fonts[type] = font;
}

void Manager::unloadAll() {
  // Only unload original instances once
  UnloadFont(fonts[FontType::Mono]);
  UnloadFont(fonts[FontType::Sans]);
  UnloadFont(fonts[FontType::Mecha]);

  fonts.clear();
  if (sdfShader.id > 0) UnloadShader(sdfShader);
}

Font Manager::getFont(FontType type) {
  auto it = fonts.find(type);
  if (it != fonts.end()) {
    return it->second;
  }
  return GetFontDefault();
}

}  // namespace FontManager
