#pragma once
#include "raylib.h"

namespace ThemeManager {

enum class ThemeType {
    TermX = 0,
    Mocha,
    Macchiato,
    Frappe,
    Latte
};

struct Theme {
    const char* name;
    
    // Default colors
    Color borderNormal;
    Color baseNormal;
    Color textNormal;
    
    Color borderFocused;
    Color baseFocused;
    Color textFocused;
    
    Color borderPressed;
    Color basePressed;
    Color textPressed;
    
    Color borderDisabled;
    Color baseDisabled;
    Color textDisabled;
    
    Color background;
    Color line;
};

// Returns the theme data for a given type
const Theme& getTheme(ThemeType type);

} // namespace ThemeManager
