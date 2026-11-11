#pragma once

#define BALLOON_SKIN_LFT 3
#define BALLOON_SKIN_RGT 12
#define BALLOON_SKIN_TOP 2
#define BALLOON_SKIN_BOT 14

#define PRIZE_TOQUE 0
#define PRIZE_GLASSES 1
#define PRIZE_1UP 2


#include "graphics.h"
#include "entity.h"
#include "globals.h"
#include "ephemeral.h"

class Balloon
{

public:
    uint16_t x = 0;
    uint8_t y = 0;
    bool popped : 1;

    const unsigned char *sprite;

    Balloon();
    Balloon(uint16_t start_x, uint8_t start_y);
    void draw(uint16_t offset_x);
    bool collide(PlayerEntity *player);
};

class Item
{

public:
    const unsigned char *sprite;

    uint16_t x;
    uint8_t y;

    bool gotten : 1;
    bool grounded : 1;
    uint8_t type : 2;

    Item();
    void update(Stage *stage, PlayerEntity *player);
    void draw(uint16_t offset_x);
    bool collide(PlayerEntity *player);
    void giveitem(PlayerEntity *player, EphemeralRoster *ephem);
};


class ItemRoster{

public:
    Item roster[ITEM_MAX];

    ItemRoster();
    ~ItemRoster() {};
    void add(uint16_t new_x, uint8_t new_y, uint8_t type);
    void emptyRoster();
    void updateRoster(Stage *stage, PlayerEntity *player, uint16_t scroll, EphemeralRoster *ephem);
};
