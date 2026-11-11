#include "graphics.h"
#include "ephemeral.h"
#include "globals.h"

Ephemeral::Ephemeral()
{
    x = 0;
    y = 0;
    frame = 0;
    anim_timer = EPHEM_ANIM_SPEED;
    done = true;
    text = false;
    sprite = pop_plus_mask;
}

void Ephemeral::make(uint16_t new_x, uint8_t new_y, uint8_t ephemType, int16_t new_vx, int16_t new_vy)
{

    // Assign new sprite if needed:
    // Will be a switch case if more ephemera are added.
    switch (ephemType)
    {
    case pop:
        sprite = pop_plus_mask;
        text = false;
        break;

    case toqueword:
        sprite = toquetext_plus_mask;
        text = true;
        break;

    case flyboy:
        sprite = flyboy_plus_mask;
        text = true;
        break;

    case proj:
        sprite = proj_plus_mask;

        text = false;
        break;

    default:
        break;
    }

    frame = 0;
    done = false;
    anim_timer = EPHEM_ANIM_SPEED;

    x = new_x << 4;
    y = new_y << 4;
    vx = new_vx;
    vy = new_vy;
}

void Ephemeral::draw(uint16_t offset_x)
{
    if (!done)
    {
        uint16_t screen_x = (x >> 4) - offset_x;
        uint16_t screen_y = (y >> 4);
        screen::draw(screen_x, screen_y, sprite, frame);
    }
}

void Ephemeral::animate()
{
    if (done)
    {
        // If the done flag is set, this ephem is "gone."
        return;
    }

    // Nonzero velocity means this is projectile
    if (vx != 0 || vy != 0)
    {
        return;
    }

    if (anim_timer > 0)
    {
        anim_timer--;
    }
    else
    {
        if (vy != 0 || vx != 0)
        {
        }
        else
        {
            if (!text)
            {
                frame++;
                anim_timer = EPHEM_ANIM_SPEED;
                if (frame >= EPHEM_END_FRAME)
                {
                    done = true;
                }
            }
            else
            {
                y -= 1 << 4;
                anim_timer = EPHEM_ANIM_SPEED;
                frame = 0;
                if (y <= (10 << 4))
                {
                    done = true;
                }
            }
        }
    }
}

void Ephemeral::proj_update(uint16_t offset_x)
{
    // Move by velocity:
    // TODO: This is a problem as nothign will actually accumulate below the fraction
    x += vx;
    y += vy;

    uint16_t screen_x = ((x >> 4) - offset_x);
    uint16_t screen_y = ((y >> 4));
    if (screen_x < 0 || screen_x > 128 || screen_y < 0 || screen_y > 64)
    {
        done = true;
    }
}

bool Ephemeral::collide(uint16_t playerx, uint8_t playery)
{

    uint16_t px = (x >> 4);
    uint16_t py = (y >> 4);
    return ((px < playerx + SPR_PRGTSKIN) &&
            (px + 8 > playerx + SPR_PLFTSKIN) &&
            (py < playery + SPR_PBOTSKIN) &&
            (py + 8 > playery + SPR_PTOPSKIN));
}

EphemeralRoster::EphemeralRoster()
{

    for (uint8_t i = 0; i < EPHEM_MAX; i++)
    {
        roster[i] = Ephemeral();
    }
}

void EphemeralRoster::add(uint16_t new_x, uint8_t new_y, uint8_t type, int16_t vx, int16_t vy)
{

    uint8_t i = 0;
    for (i = 0; i < EPHEM_MAX - 1; i++)
    {
        if (roster[i].done)
        {
            break;
        }
    }

    roster[i].make(new_x, new_y, type, vx, vy);
}

// This is a bool since it's also checking if hit by a projectile...
bool EphemeralRoster::updateRoster(uint16_t scroll, uint16_t playerx, uint8_t playery)
{
    bool hit = false;

    for (uint8_t i = 0; i < EPHEM_MAX; i++)
    {
        if (!roster[i].done)
        {
            roster[i].draw(scroll);
            if (roster[i].vx != 0 || roster[i].vy != 0)
            {
                roster[i].proj_update(scroll);
                if (roster[i].collide(playerx, playery))
                {
                    hit = true;
                    
                }
            }
            else
            {
                roster[i].animate();
            }
        }
        else
        {
            continue;
        }
    }

    return hit;
}

void EphemeralRoster::shoot_projectile(int16_t sx, int16_t sy, int16_t tx, int16_t ty, uint8_t speed)
{

    int16_t dx = tx - sx;
    int16_t dy = ty - sy;

    // Cheeky lil normalization faker:
    dx = (dx > 0) - (dx < 0);
    dy = (dy > 0) - (dy < 0);

    int16_t proj_vx = (int16_t)(dx * speed);
    int16_t proj_vy = (int16_t)(dy * speed);

    add(sx, sy, proj, proj_vx, proj_vy);
}
