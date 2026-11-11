/*
 * tilemap.h
 * Created Date: 2026-07-17
 * Author: Matthew Riche
 * 
 */
#pragma once

#include "globals.h"

// (moved from stages.h)
extern const uint16_t stages[];

// Stages in stages[] (tilemap.cpp), 16 slices each: 1-1 to 8-3.
#define STAGE_COUNT 24

#define MAP_WIDTH 128
#define MAP_MEM 1024

#define COIN_ARRAY 96

#define TILE_SPIKE 4
#define TILE_EMPTY 0
#define TILE_CASTLEWALL 5
#define TILE_CHOCWALL 8
#define TILE_CLASSIC 1
#define TILE_CASTLEBLOCK 2
#define TILE_CHOCBLOCK 3
#define TILE_COIN1 9
#define TILE_COIN2 10
#define LAST_SOLID 3

#define TILE_ANIM_SPEED 20



extern const uint8_t stage1_1[];
extern const uint8_t stage1_2[];
extern const uint8_t stage1_3[];

extern const unsigned char meta_tiles[];

class Stage
{

public:
    uint8_t exit_x;
    uint8_t exit_y;

    uint8_t tile_anim = 0;
    uint16_t currentstage = 0;

    uint8_t coins[COIN_ARRAY];

    void draw_level(uint16_t offset_x, uint8_t alt_tiles);
    void fill_coins();
    void draw_coins();
    void draw_coins(uint16_t offset_x);
    bool get_coin(uint16_t x, uint16_t y, uint16_t x2, uint16_t y2);
    bool check_coin(uint16_t x, uint16_t y);
    bool is_solid(uint16_t x, int8_t y);
    bool is_spike(uint16_t x, int8_t y);
    uint8_t unpack_tile(uint16_t x, uint8_t y);

};


