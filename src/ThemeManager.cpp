#include "ThemeManager.hpp"

namespace ThemeManager {

static const Theme mocha = {
    "Mocha",
    { 108, 112, 134, 255 }, { 49, 50, 68, 255 },   { 205, 214, 244, 255 }, // Normal
    { 180, 190, 254, 255 }, { 69, 71, 90, 255 },   { 205, 214, 244, 255 }, // Focused
    { 137, 180, 250, 255 }, { 88, 91, 112, 255 },  { 205, 214, 244, 255 }, // Pressed
    { 69, 71, 90, 255 },    { 24, 24, 37, 255 },   { 108, 112, 134, 255 }, // Disabled
    { 30, 30, 46, 255 },    { 108, 112, 134, 255 }                         // Background, Line
};

static const Theme macchiato = {
    "Macchiato",
    { 110, 115, 141, 255 }, { 54, 58, 79, 255 },   { 202, 211, 245, 255 }, // Normal
    { 183, 189, 248, 255 }, { 73, 77, 100, 255 },  { 202, 211, 245, 255 }, // Focused
    { 138, 173, 244, 255 }, { 91, 96, 120, 255 },  { 202, 211, 245, 255 }, // Pressed
    { 73, 77, 100, 255 },   { 30, 32, 48, 255 },   { 110, 115, 141, 255 }, // Disabled
    { 36, 39, 58, 255 },    { 110, 115, 141, 255 }                         // Background, Line
};

static const Theme frappe = {
    "Frappe",
    { 115, 121, 148, 255 }, { 65, 69, 89, 255 },   { 198, 208, 245, 255 }, // Normal
    { 186, 187, 241, 255 }, { 81, 87, 109, 255 },  { 198, 208, 245, 255 }, // Focused
    { 140, 170, 238, 255 }, { 98, 104, 128, 255 }, { 198, 208, 245, 255 }, // Pressed
    { 81, 87, 109, 255 },   { 41, 44, 60, 255 },   { 115, 121, 148, 255 }, // Disabled
    { 48, 52, 70, 255 },    { 115, 121, 148, 255 }                         // Background, Line
};

static const Theme latte = {
    "Latte",
    { 156, 160, 176, 255 }, { 204, 208, 218, 255 }, { 76, 79, 105, 255 },  // Normal
    { 114, 135, 253, 255 }, { 188, 192, 204, 255 }, { 76, 79, 105, 255 },  // Focused
    { 30, 102, 245, 255 },  { 172, 176, 190, 255 }, { 76, 79, 105, 255 },  // Pressed
    { 188, 192, 204, 255 }, { 230, 233, 239, 255 }, { 156, 160, 176, 255 }, // Disabled
    { 239, 241, 245, 255 }, { 156, 160, 176, 255 }                          // Background, Line
};

static const Theme termx = {
    "TermX",
    { 141, 74, 0, 255 },    { 22, 19, 19, 255 },    { 255, 165, 0, 255 },  // Normal
    { 255, 215, 0, 255 },   { 191, 122, 46, 255 },  { 255, 239, 213, 255 }, // Focused
    { 91, 58, 25, 255 },    { 255, 140, 0, 255 },   { 111, 74, 21, 255 },  // Pressed
    { 59, 42, 34, 255 },    { 44, 30, 24, 255 },    { 65, 45, 37, 255 },   // Disabled
    { 21, 10, 5, 255 },     { 252, 233, 227, 255 }                         // Background, Line
};

const Theme& getTheme(ThemeType type) {
    switch (type) {
        case ThemeType::Mocha: return mocha;
        case ThemeType::Macchiato: return macchiato;
        case ThemeType::Frappe: return frappe;
        case ThemeType::Latte: return latte;
        default: return termx;
    }
}

} // namespace ThemeManager
