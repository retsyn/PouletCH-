#include "globals.h"
#include "graphics.h"
#include "gizmos.h"
#include "entity.h"
#include "sounds.h"

void Door::update(PlayerEntity *player)
{
    if (!on)
        return;

    bool collided = false;
    // Check for collision with the player.
    if (player->y > 0)
    {
        if (((player->x + SPR_LFTSKIN) > x) && ((player->x + SPR_RGTSKIN) < x + w))
        {
            if (((player->y + SPR_TOPSKIN) > y) && ((player->y + SPR_BOTSKIN) < y + h))
            {
                collided = true;
            }
        }
    }

    if (collided)
    {
        open = true;
        sfx_door();
    }
}

Door::Door()
{
    x = 0;
    y = 0;
    on = false;
    open = false;
}

Door::Door(int16_t start_x, int16_t start_y)
{
    x = start_x;
    y = start_y;
    on = false;
    open = false;
}

void Door::draw(int16_t camera_offset)
{
    if (on)
    {
        screen::draw(x - camera_offset, y, doorspr, open);
    }
}

void Pole::draw(int16_t camera_offset)
{
    if (on)
    {
        for (uint8_t i = 24; i <= 48; i += 8)
        {
            screen::draw(POLE_X - camera_offset, i, polemid, 0);
        }
        screen::draw(POLE_X - camera_offset, 16, poletop, 0);
    }
}

void Pole::update(PlayerEntity *player){
    if(!on)
        return;
        
    if(player->x > POLE_X){
        if (!player->celebrate)
            sfx_clear();
        player->celebrate = 1;
    }
}
