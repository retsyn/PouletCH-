#include "globals.h"
#include "entity.h"
#include "sounds.h"
#include <math.h>

// Precalc framelengths, per PlayerAnimState--
static const uint8_t FRAMELEN[] = {0, 3, 1, 0, 0, 0, 0};

Entity::Entity(uint8_t newtype, float start_x, float start_y)
{

    x = start_x;
    y = start_y;
    type = newtype;

    anim_ticker = SPR_WAIT;
    anim_state = 0;
    anim_frame = 0;

    // initial bools
    flying = false;
    blinkon = true;
    death = false;
    grounded = false;
    flip = false;
    attack = false;
    blinking = false;
    celebrate = false;

    // Get sprite to point to the spot in progmem.
    switch (type)
    {
    case ENT_POULET:
        sprite = poulet_plus_mask;
        break;

    default:
        break;
    }

    vx = 0.0f;
    vy = 0.0f;
}

bool Entity::physics(Stage *in_stage)
{
    bool vert_collide = 0;
    bool horiz_collide = 0;
    bool hitspike = false;

    // Cap horizontal movement speed on ground:
    if (grounded && (!attack))
    {
        if (vx > PLAYER_TOPSPEED)
            vx = PLAYER_TOPSPEED;
        if (vx < -PLAYER_TOPSPEED)
            vx = -PLAYER_TOPSPEED;
    }

    // Apply gravity!
    if (!flying)
    {
        vy += PHYS_GRAVITY;
    }

    // Check where velocity will put this ent next:
    int16_t nx = (x + vx);
    int16_t ny = (y + vy);

    if (vy > 0)
    {

        // Check for downward collision by iterating through pixels travelled;
        for (int i = floor(y); i <= floor(ny) + 1; i++)
        {
            if (in_stage->is_spike(int16_t(floor(x) + SPR_SLFTSKIN), int16_t(i + SPR_SBOTSKIN)) || in_stage->is_spike(int16_t(floor(x) + SPR_SRGTSKIN), int16_t(i + SPR_SBOTSKIN)))
            {
                hitspike = true;

            }
            if (in_stage->is_solid(int16_t(floor(x) + SPR_LFTSKIN), int16_t(i + SPR_BOTSKIN)) || in_stage->is_solid(int16_t(floor(x) + SPR_RGTSKIN), int16_t(i + SPR_BOTSKIN)))
            {
                vy = 0;
                y = i - 1;
                vert_collide = 1;
                grounded = true;
                break;
            }
            else
            {
                grounded = false;
            }
        }
    }

    else if (vy < 0)
    {
        // Check for upward collision by iterating through pixels travelled;
        for (int i = y; i >= int(ny); i--)
        {
            if (in_stage->is_spike(int16_t(x + SPR_SLFTSKIN + 1), int16_t(i + SPR_STOPSKIN)) || in_stage->is_spike(int16_t(x + SPR_SRGTSKIN - 1), int16_t(i + SPR_STOPSKIN)))
            {
                hitspike = true;
            }

            if (in_stage->is_solid(int16_t(x + SPR_LFTSKIN + 1), int16_t(i + SPR_TOPSKIN)) || in_stage->is_solid(int16_t(x + SPR_RGTSKIN - 1), int16_t(i + SPR_TOPSKIN)))
            {
                vy = 0;
                y = i + 1;
                vert_collide = 1;
                break;
            }
        }
    }
    if (!vert_collide)
        y += vy;

    if (vx > 0)
    {
        // Check for right collision...
        for (int i = floor(x); i <= floor(nx) + 1; i++)
        {
            if (in_stage->is_spike(int16_t(i + SPR_SRGTSKIN), int16_t(y + SPR_STOPSKIN + 1)) || in_stage->is_spike(int16_t(i + SPR_SRGTSKIN), int16_t(y + SPR_SBOTSKIN - 1)))
            {
                hitspike = true;
            }

            if (in_stage->is_solid(int16_t(i + SPR_RGTSKIN), int16_t(y + SPR_TOPSKIN + 1)) || in_stage->is_solid(int16_t(i + SPR_RGTSKIN), int16_t(y + SPR_BOTSKIN - 1)))
            {
                vx = 0;
                x = i - 1;
                horiz_collide = 1;
                break;
            }
        }
    }
    else if (vx < 0)
    {
        // Check for left collision...
        for (int i = floor(x); i >= floor(nx); i--)
        {
            if (in_stage->is_spike(int16_t(i + SPR_SLFTSKIN), int16_t(y + SPR_STOPSKIN + 1)) || in_stage->is_spike(int16_t(i + SPR_SLFTSKIN), int16_t(y + SPR_SBOTSKIN - 1)))
            {
                hitspike = true;
            }

            if (in_stage->is_solid(int16_t(i + SPR_LFTSKIN), int16_t(y + SPR_TOPSKIN + 1)) || in_stage->is_solid(int16_t(i + SPR_LFTSKIN), int16_t(y + SPR_BOTSKIN - 1)))
            {
                vx = 0;
                x = i + 1;
                horiz_collide = 1;
                break;
            }
        }
    }
    if (!horiz_collide)
        x += vx;

    if ((grounded && (!attack)) || celebrate)
    {
        if (vx > 0)
        {
            vx -= PHYS_FRICTION;
        }
        else if (vx < 0)
        {
            vx += PHYS_FRICTION;
        }
        if (fabsf(vx) < PHYS_FRICTION)
        {
            vx = 0;
        }
    }

    // Restrict to the playfield.
    if (x < LEFT_BOUND - SPR_LFTSKIN)
    {
        x = LEFT_BOUND - SPR_LFTSKIN;
    }
    else if (x + SPR_RGTSKIN > RIGHT_BOUND - 1)
    {
        x = RIGHT_BOUND - SPR_RGTSKIN - 1;
    }

    return hitspike;
}

PlayerEntity::PlayerEntity(uint8_t newtype, float start_x, float start_y) : Entity(newtype, start_x, start_y)
{
    toque = 0;
    flyboy = 0;
}

void PlayerEntity::control()
{
    if (death)
    {
        return;
    }

    if (celebrate)
    {
        jump_buffer = 1;
    }

    // IFrames stuff:
    if (blinking)
    {
        iframes--;
        if (iframes <= 0)
        {
            blinking = false;
            blinkon = true;
        }
    }

    // Movement
    if (!celebrate)
    {
        if (chgame.pressed(LEFT_BUTTON))
        {
            if (vx > -PLAYER_TOPSPEED)
                vx -= accel;
            flip = true;
        }
        if (chgame.pressed(RIGHT_BUTTON))
        {
            if (vx < PLAYER_TOPSPEED)
                vx += accel;
            flip = false;
        }

        // Jumping
        if (chgame.justPressed(B_BUTTON))
        {
            if (grounded || (coyote_buffer > 0 && vy >= 0))
            {
                vy = -PLAYER_JUMPPOWER;
                grounded = false;
                attack = false;
                coyote_buffer = 0;
                sfx_jump();
            }
            else
            {
                jump_buffer = PLAYER_JUMP_BUFFER_TIME;
            }
        }

        // Attacking
        if (!grounded && chgame.justPressed(A_BUTTON))
        {
            attack = true;
            sfx_attack();
            if (!flip)
            {
                vx = HORIZ_ATTACK_SPEED;
            }
            else
            {
                vx = -HORIZ_ATTACK_SPEED;
            }
            vy = VERT_ATTACK_SPEED;
            skidding = SKID_MAX;
        }
    }

    // Jump buffer logic-- to be triggered if the buffer count is nonzero and the button is held.
    if (jump_buffer > 0)
    {
        if (grounded)
        {
            if (chgame.pressed(B_BUTTON) || celebrate)
            {
                vy = -PLAYER_JUMPPOWER;
                if (!celebrate)
                    sfx_jump();
                grounded = false;
                attack = false;
                coyote_buffer = 0;
            }
        }
        jump_buffer--;
    }

    // Coyote time assignment:
    if (grounded)
        coyote_buffer = PLAYER_COYOTE_TIME;
    if (!grounded)
    {
        if (coyote_buffer > 0)
            coyote_buffer--;
    }

    if (attack)
    {
        if (grounded)
        {
            skidding--;
            if (skidding <= 0)
                attack = false;
        }
    }

    // Little physics manipulation outside the physics module so we don't have to override:
    if (!flyboy)
    {
        if (vy < 0 and chgame.pressed(B_BUTTON))
        {
            vy -= JUMP_JUICE;
        }
    }
    else
    {
        if (vy < 0 and chgame.pressed(B_BUTTON))
        {
            vy -= JUMP_JUICE * 1.1;
        }
        else if (vy >= 0 and chgame.pressed(B_BUTTON))
        {
            vy -= JUMP_JUICE;
            if (vy > HOVER_SPEED)
            {
                vy = HOVER_SPEED;
            }
        }
    }
}

void PlayerEntity::draw(int16_t offset_x)
{

    if (y <= -16)
    {
        return;
    }

    // If blinkrate is on:
    if (blinking)
    {
        blinkon = !blinkon;
        if (blinkon)
            return;
    }

    // Anim state determination:
    if (grounded)
    {
        if (fabsf(vx) > 0.1)
        {
            anim_state = walking;
        }
        else
        {
            anim_state = idle;
        }
    }
    else
    {
        if (vy < 0)
        {
            anim_state = jumping_up;
        }
        else
        {
            if (!flyboy)
            {
                anim_state = jumping_down;
            }
            else
            {
                anim_state = hovering;
            }
        }
    }

    if (attack)
    {
        anim_state = attacking;
    }

    // Advance animation frames:
    anim_ticker--;
    if (anim_ticker <= 0)
    {
        anim_frame++;
        anim_ticker = SPR_WAIT;
    };
    // If the frame exceeds the current animation; refer to the pre-calced anim lengths.
    if (anim_frame > FRAMELEN[anim_state])
    {
        anim_frame = 0;
    };

    if (death)
    {
        anim_state = deading;
    }

    if (toque && flyboy)
    {
        screen::draw(x - offset_x + SPR_LFTSKIN, y + 2 - (anim_frame % 2), toque_plus_mask, 0);
    }

    switch (anim_state)
    {
    case idle:
        screen::draw(x - offset_x, y, sprite, poulet_anim_idle[anim_frame] + (MIRROR * int(flip)));
        break;
    case walking:
        screen::draw(x - offset_x, y, sprite, poulet_anim_walk[anim_frame] + (MIRROR * int(flip)));
        break;
    case jumping_up:
        // Special hack for drawing toque first only on this anim:
        if (toque)
            screen::draw(x - offset_x + SPR_LFTSKIN, y + 1, toque_plus_mask, 0);
        screen::draw(x - offset_x, y, sprite, poulet_anim_jump_up[anim_frame] + (MIRROR * int(flip)));
        break;
    case jumping_down:
        screen::draw(x - offset_x, y, sprite, poulet_anim_jump_down[anim_frame] + (MIRROR * int(flip)));
        break;
    case attacking:
        screen::draw(x - offset_x, y, sprite, poulet_anim_attack[anim_frame] + (MIRROR * int(flip)));
        break;
    case deading:
        screen::draw(x - offset_x, y, sprite, poulet_anim_death[anim_frame] + (MIRROR * int(flip)));
        break;
    case hovering:
        screen::draw(x - offset_x, y, sprite, poulet_anim_hover[anim_frame] + (MIRROR * int(flip)));
        break;

    default:
        break;
    }

    if (toque && (anim_state != jumping_up) && !flyboy)
    {
        screen::draw(x - offset_x + SPR_LFTSKIN, y + 2 - (anim_frame % 2), toque_plus_mask, 0);
    }
}

void PlayerEntity::hitspike()
{
    if (iframes > 0)
    {
        return;
    }
    vy = -2.5;
    power_down();
}

void PlayerEntity::hitprojectile(){
    if(iframes >0){
        return;
    }
    vy = -0.8;
    power_down();
}

void PlayerEntity::power_down()
{
    bool lethal = true;

    if (!toque)
    {
        if (flyboy)
        {
            flyboy = false;
            sprite = poulet_plus_mask;
            lethal = false;
        }
    }
    else
    {
        toque = false;
        lethal = false;
    }

    if (lethal)
    {
        death = true;
        if(lives > 0){
            lives -= 1;
        } else {
            pending_game_over = true;
        }
        sfx_die();
    }
    else
    {
        sfx_hurt();
    }

    blinking = true;
    iframes = PLAYER_IFRAMES;
}

void PlayerEntity::takehit(Foe *hitter)
{
    // Iframes will prevent a hit.
    if (iframes > 0)
    {
        return;
    }

    vy = -1.0;
    if (hitter->x > x)
    {
        vx = -2;
    }
    else if (hitter->x < x)
    {
        vx = 2;
    }

    power_down();
}

Foe::Foe(uint8_t newtype, uint16_t start_x, uint8_t start_y)
{
    x = start_x;
    y = start_y;

    spawned = 0;
    dead = 1;
    flip = 0;
    anim_bit = 0;
    act = 0;
    think = 0;

    enttype = newtype;
    assign_sprite();
}

void Foe::assign_sprite()
{
    switch (enttype)
    {
    case (ENT_FENNEC):
        sprite = fennec_plus_mask;
        break;

    case (ENT_GOOB):
        sprite = goob_plus_mask;
        break;

    case (ENT_BLOOB):
        sprite = bloob_plus_mask;
        break;

    case (ENT_DRAKE):
        sprite = drake_plus_mask;
        break;

    default:
        break;
    }
}

void Foe::draw(int16_t offset_x)
{

    if (!spawned)
        return;

    if (dead)
    {
        if (timer % 2 == 0)
        {
            screen::draw(x - offset_x, y, sprite, 2 + (FOE_MIRROR * int(flip)));
        }
        return;
    }

    if (act == 1)
    {
        screen::draw(x - offset_x, y, sprite, 2 + (FOE_MIRROR * int(flip)));
        return;
    }

    screen::draw(x - offset_x, y, sprite, int(anim_bit) + (FOE_MIRROR * int(flip)));
}

void Foe::update(Stage *stage, PlayerEntity *player, EphemeralRoster *ephemerals)
{
    bool advance = false;

    // Kill what does off screen:
    if (x < -8 || y >= 56 || x > 1042)
    {
        spawned = false;
        dead = true;
        return;
    }

    // Shitty gravity
    if (enttype != ENT_BLOOB)
    {
        if (!stage->is_solid(x + SPR_LFTSKIN, y + SPR_BOTSKIN + 1) && !stage->is_solid(x + SPR_RGTSKIN, y + SPR_BOTSKIN + 1))
        {
            y += 1;
        }
    }

    timer += 1;

    if (dead)
    {
        if (timer >= DEATH_ANIM)
        {
            spawned = false;
        }
        return;
    }

    switch (enttype)
    {
    case (ENT_BLOOB):

    // Same as Fennec.
    case (ENT_FENNEC):
    case (ENT_DRAKE):
        if (timer >= SPEED_FENNEC)
        {
            advance = true;
            anim_bit = !anim_bit;
            timer = 0;
        }
        break;

    case (ENT_GOOB):
        if (timer >= SPEED_FENNEC)
        {
            advance = true;
            anim_bit = !anim_bit;
            timer = 0;
        }
        break;

    default:
        break;
    }

    if (advance)
    {
        think += 1;

        switch (enttype)
        {
        case (ENT_BLOOB):
            if (think > BLOOB_DROPRATE)
            {
                think = 0;
                ephemerals->shoot_projectile(x + 4, y + SPR_BOTSKIN - 2, x + 4, 64, 5);
            }

        case (ENT_FENNEC):

            if (flip)
            {
                if (!stage->is_solid(x + SPR_LFTSKIN, y + SPR_BOTSKIN) && !stage->is_solid(x + SPR_LFTSKIN, y + SPR_TOPSKIN))
                {
                    x -= 1;
                }
                else
                {
                    flip = false;
                }
            }
            else
            {
                if (!stage->is_solid(x + SPR_RGTSKIN, y + SPR_BOTSKIN) && !stage->is_solid(x + SPR_RGTSKIN, y + SPR_TOPSKIN))
                {
                    x += 1;
                }
                else
                {
                    flip = true;
                }
            }
            break;

        case (ENT_DRAKE):
            if (think > BLOOB_DROPRATE)
            {

                uint16_t xdir = 0;
                if (player->x < x)
                {
                    flip = true;
                    xdir = 0;
                }
                else
                {
                    xdir = 1024;
                    flip = false;
                }

                if (act == 0)
                    ephemerals->shoot_projectile(x + 4, y + 8, xdir, y + 8, 7);
                act = 1;
                if (think > (BLOOB_DROPRATE + BLOOB_DROPRATE / 4))
                {
                    think = 0;
                    act = 0;
                }
            }
            break;

        case (ENT_GOOB):
            if (player->x < x)
            {
                flip = true;
            }
            else
            {
                flip = false;
            }
            break;

        default:
            break;
        }
    }
}

bool Foe::collide(PlayerEntity *player)
{
    if (player->y <= -16)
    {
        return false;
    }

    if (dead)
    {
        return false;
    }

    if ((x + SPR_LFTSKIN < player->x + SPR_RGTSKIN) && (x + SPR_RGTSKIN > player->x + SPR_LFTSKIN) && (y + SPR_TOPSKIN < player->y + SPR_BOTSKIN) && (y + SPR_BOTSKIN > player->y + SPR_TOPSKIN))
    {
        return true;
    }
    else
    {
        return false;
    }
}
