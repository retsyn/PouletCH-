
#pragma once
#include <stdint.h>

namespace screen {

constexpr int16_t WIDTH  = 128;   ///< The game's screen, as on the Arduboy.
constexpr int16_t HEIGHT = 64;
constexpr int16_t SCREEN_Y = 32;  ///< Where its top row lands on the 128x128 panel.

constexpr int16_t HUD_Y = 12;
constexpr int16_t HUD_HEIGHT = 8;

constexpr uint8_t TRANSPARENT = 0;  ///< In sprites: not drawn.
constexpr uint8_t BACKGROUND  = 0;  ///< The screen's clear colour: PAL[0], #000000.

extern const uint16_t PAL[16];

void begin();
void clear(uint8_t colour = BACKGROUND);
void display();

void hud(bool on);

/// Draw frame `frame` of a sprite with its top-left corner at (x, y).
void draw(int16_t x, int16_t y, const uint8_t *sprite, uint8_t frame);

void drawFastVLine(int16_t x, int16_t y, int16_t h, uint8_t colour);
void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t colour);

}  // namespace screen
