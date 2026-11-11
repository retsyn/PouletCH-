#include "items.h"
#include "globals.h"
#include "entity.h"
#include "ephemeral.h"
#include "sounds.h"

Balloon::Balloon() : Balloon(0, 0)
{
}

Balloon::Balloon(uint16_t new_x, uint8_t new_y)
{
    x = new_x;
    y = new_y;
    popped = true;

    sprite = balloon_plus_mask;
}

void Balloon::draw(uint16_t offset_x)
{

    if (!popped)
    {
        screen::draw(x - offset_x, y, sprite, 0);
    }
}

bool Balloon::collide(PlayerEntity *player)
{

    if ((x + BALLOON_SKIN_LFT < player->x + SPR_RGTSKIN) && (x + BALLOON_SKIN_RGT > player->x + SPR_LFTSKIN) && (y + BALLOON_SKIN_TOP < player->y + SPR_BOTSKIN) && (y + BALLOON_SKIN_BOT > player->y + SPR_TOPSKIN))
    {
        return true;
    }
    else
    {
        return false;
    }
}

Item::Item()
{
    x = 0;
    y = 0;
    gotten = true;
    grounded = false;
    type = 0;

    sprite = glasses_plus_mask;
}

void Item::update(Stage *stage, PlayerEntity *player)
{
    if(y >= 56){
        gotten = true;
        return;
    }

    // Item softly falls!
    if (!stage->is_solid(x + SPR_LFTSKIN, y + SPR_BOTSKIN + 1) && !stage->is_solid(x + SPR_RGTSKIN, y + SPR_BOTSKIN + 1))
    {
        y += 1;
        grounded = false;
    }
    else
    {
        grounded = true;
    }
}

void Item::draw(uint16_t offset_x)
{

    screen::draw(x - offset_x, y, sprite, 0);
}

void Item::giveitem(PlayerEntity *player, EphemeralRoster *ephem){

    switch (type)
    {
    case (PRIZE_TOQUE):
        player->toque = true;
        sfx_powerup();
        ephem->add(player->x, player->y, toqueword, 0, 0);
        break;

    case (PRIZE_GLASSES):
        player->flyboy = true;
        sfx_powerup();
        ephem->add(player->x, player->y, flyboy, 0, 0);
        player->sprite = aviator_plus_mask;
        break;

    case (PRIZE_1UP):
        player->lives += 1;
        sfx_oneup();
        break;

    default:
        break;
    }
}

bool Item::collide(PlayerEntity *player)
{

    if (!grounded)
    {
        return false;
    }
    if (player->y <= -16)
    {
        return false;
    }

    if (gotten)
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

ItemRoster::ItemRoster()
{

    for (uint8_t i = 0; i < ITEM_MAX; i++)
    {
        roster[i] = Item();
    }
}

void ItemRoster::emptyRoster(){
    // Empty the entire roster (While static memory remains.)
    for (uint8_t i = 0; i < ITEM_MAX; i++){
        roster[i].gotten = true;
    }
}


void ItemRoster::add(uint16_t new_x, uint8_t new_y, uint8_t new_type)
{

    uint8_t i = 0;
    for (i = 0; i < ITEM_MAX; i++)
    {
        if (roster[i].gotten == true)
        {
            break;
        }
    }

    if(i >= ITEM_MAX){
        i = 0; // We were gonna overflow, so now we just delete number one.
    }

    // Assign all the arg values to the chosen spot in the roster:
    roster[i].x = new_x;
    roster[i].y = new_y;
    roster[i].type = new_type;
    roster[i].gotten = false;
    roster[i].grounded = false;

    switch (new_type)
    {
    case PRIZE_GLASSES:
        roster[i].sprite = glasses_plus_mask;
        break;

    case PRIZE_TOQUE:
        roster[i].sprite = toquepowerup_plus_mask;
        break;

    case PRIZE_1UP:
        roster[i].sprite = oneup_plus_mask;
        break;

    default:
        roster[i].sprite = glasses_plus_mask;
        break;
    }
}

void ItemRoster::updateRoster(Stage *stage, PlayerEntity *player, uint16_t scroll, EphemeralRoster *ephem)
{

    for (uint8_t i = 0; i < ITEM_MAX; i++)
    {
        if (!roster[i].gotten)
        {
            roster[i].draw(scroll);
            roster[i].update(stage, player);
            if(roster[i].collide(player)){
                roster[i].gotten = true;
                roster[i].giveitem(player, ephem);
            }
        }
        else
        {
            continue;
        }
    }
}
