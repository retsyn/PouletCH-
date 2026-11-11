#pragma once

#include "entity.h"

#define POLE_X 950


class Door
{

public:
    int16_t x;
    int16_t y;
    Door();
    Door(int16_t start_x, int16_t start_y);
    void update(PlayerEntity *player);
    void draw(int16_t offset_x);
    bool open : 1;
    bool on : 1;
    int8_t w = 16;
    int8_t h = 16;

};

class Pole
{
public:
    void update(PlayerEntity *player);
    void draw(int16_t offset_x);
    bool on = 0;
};
