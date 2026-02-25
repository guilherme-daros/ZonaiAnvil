#ifndef STYLE_TERMX_H
#define STYLE_TERMX_H

#include "style_terminal.h"

#define TERMX_STYLE_PROPS_COUNT  17

// Custom style name: TermX (Orange Terminal)
static const GuiStyleProp termxStyleProps[TERMX_STYLE_PROPS_COUNT] = {
    { 0, 0, (int)0x8d4a00ff },    // DEFAULT_BORDER_COLOR_NORMAL 
    { 0, 1, (int)0x161313ff },    // DEFAULT_BASE_COLOR_NORMAL 
    { 0, 2, (int)0xffa500ff },    // DEFAULT_TEXT_COLOR_NORMAL (Orange)
    { 0, 3, (int)0xffd700ff },    // DEFAULT_BORDER_COLOR_FOCUSED (Gold)
    { 0, 4, (int)0xbf7a2eff },    // DEFAULT_BASE_COLOR_FOCUSED 
    { 0, 5, (int)0xffefd5ff },    // DEFAULT_TEXT_COLOR_FOCUSED (PapayaWhip)
    { 0, 6, (int)0x5b3a19ff },    // DEFAULT_BORDER_COLOR_PRESSED 
    { 0, 7, (int)0xff8c00ff },    // DEFAULT_BASE_COLOR_PRESSED (DarkOrange)
    { 0, 8, (int)0x6f4a15ff },    // DEFAULT_TEXT_COLOR_PRESSED 
    { 0, 9, (int)0x3b2a22ff },    // DEFAULT_BORDER_COLOR_DISABLED 
    { 0, 10, (int)0x2c1e18ff },    // DEFAULT_BASE_COLOR_DISABLED 
    { 0, 11, (int)0x412d25ff },    // DEFAULT_TEXT_COLOR_DISABLED 
    { 0, 16, (int)0x00000010 },    // DEFAULT_TEXT_SIZE 
    { 0, 17, (int)0x00000000 },    // DEFAULT_TEXT_SPACING 
    { 0, 18, (int)0xfce9e3ff },    // DEFAULT_LINE_COLOR 
    { 0, 19, (int)0x150a05ff },    // DEFAULT_BACKGROUND_COLOR 
    { 0, 20, (int)0x00000008 },    // DEFAULT_TEXT_LINE_SPACING 
};

static void GuiLoadStyleTermX(void)
{
    for (int i = 0; i < TERMX_STYLE_PROPS_COUNT; i++)
    {
        GuiSetStyle(termxStyleProps[i].controlId, termxStyleProps[i].propertyId, termxStyleProps[i].propertyValue);
    }

    int terminalFontDataSize = 0;
    unsigned char *data = DecompressData(terminalFontData, TERMINAL_STYLE_FONT_ATLAS_COMP_SIZE, &terminalFontDataSize);
    Image imFont = { data, 512, 256, 1, 2 };

    Font font = { 0 };
    font.baseSize = 16;
    font.glyphCount = 189;
    font.texture = LoadTextureFromImage(imFont);
    UnloadImage(imFont);

    font.recs = (Rectangle *)RAYGUI_MALLOC(font.glyphCount*sizeof(Rectangle));
    memcpy(font.recs, terminalFontRecs, font.glyphCount*sizeof(Rectangle));

    font.glyphs = (GlyphInfo *)RAYGUI_MALLOC(font.glyphCount*sizeof(GlyphInfo));
    memcpy(font.glyphs, terminalFontGlyphs, font.glyphCount*sizeof(GlyphInfo));

    GuiSetFont(font);
    Rectangle fontWhiteRec = { 510, 254, 1, 1 };
    SetShapesTexture(font.texture, fontWhiteRec);
}

#endif // STYLE_TERMX_H
