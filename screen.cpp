/*
 * screen.cpp
 *
 * The game's 128x64 screen in 16 colours (screen.h): drawn into CHGfx's
 * framebuffer, rows 32-95, and sent to the panel by DMA.
 */
#include <CHGame.h>
#include "screen.h"

namespace screen {

// The game's palette, RGB565. 0 is the background; in sprites, transparent.
const uint16_t PAL[16] = {
    0x0000,  /*  0  #000000  background / transparent */
    0x38E9,  /*  1  #3A1C4F */
    0x81ED,  /*  2  #803C6B */
    0xEBF4,  /*  3  #ED7EA3 */
    0xF719,  /*  4  #F6E1CC */
    0xFFFF,  /*  5  #FEFFFF */
    0x0B71,  /*  6  #0A6C8A */
    0x8631,  /*  7  #83C78A */
    0xBFF9,  /*  8  #BAFFC8 */
    0xF7EE,  /*  9  #F6FF75 */
    0xFCA0,  /* 10  #FF9500 */
    0xD241,  /* 11  #D34B08 */
    0x8147,  /* 12  #802839 */
    0x2214,  /* 13  #2643A3 */
    0x5334,  /* 14  #5265A3 */
    0xAD9A,  /* 15  #A8B2D1 */
};

namespace {

// Every drawing call goes through this: the framebuffer may still be on
// its way to the panel from the last display(). Free when it is not.
inline void ready() { gfx_wait(); }

// Where y = 0 lands on the panel, and the clip that goes with it: the game's
// band, or the HUD strip while hud(true).
int16_t originY = SCREEN_Y;

void applyClip() {
    if (originY == HUD_Y) gfx_setClip(0, HUD_Y, WIDTH, HUD_HEIGHT);
    else                  gfx_setClip(0, SCREEN_Y, WIDTH, HEIGHT);
}

}  // namespace

void begin() {
    // 16 bpp (RGB565): PAL reaches the panel exactly. 12 bpp would cut
    // every colour to 4 bits a channel (#3A1C4F would show as #331144).
    gfx_begin(GFX_DIV2, GFX_16BPP);
    gfx_setPalette(PAL, 16);
    gfx_resetClip();
    gfx_clear(BACKGROUND);
    gfx_flush();                                    // the whole panel, once
    originY = SCREEN_Y;
    applyClip();                                    // from here on, the band or the HUD strip
}

void clear(uint8_t colour) {
    ready();
    gfx_resetClip();
    gfx_fillRect(0, HUD_Y, WIDTH, HUD_HEIGHT, BACKGROUND);
    gfx_fillRect(0, SCREEN_Y, WIDTH, HEIGHT, colour);
    applyClip();
}

void display() {
    // One transfer from the HUD's top row to the band's bottom row. The rows
    // between them never change; sending them costs about 1 ms of DMA.
    gfx_flushRectAsync(0, HUD_Y, WIDTH, SCREEN_Y + HEIGHT - HUD_Y);
}

void hud(bool on) {
    originY = on ? HUD_Y : SCREEN_Y;
    applyClip();
}

void draw(int16_t x, int16_t y, const uint8_t *sprite, uint8_t frame) {
    ready();
    const int w = sprite[0];
    const int h = sprite[1];
    const uint8_t *pixels = sprite + 2 + (uint32_t)frame * (uint32_t)(h * ((w + 1) / 2));
    gfx_blit(pixels, x, y + originY, w, h, TRANSPARENT);
}

void drawFastVLine(int16_t x, int16_t y, int16_t h, uint8_t colour) {
    ready();
    gfx_vline(x, y + originY, h, colour);
}

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t colour) {
    ready();
    gfx_fillRect(x, y + originY, w, h, colour);
}

}  // namespace screen
