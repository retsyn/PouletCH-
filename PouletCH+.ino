#include <Arduino.h>
#include "globals.h"
#include "graphics.h"
#include "tilemap.h"
#include "entity.h"
#include "gizmos.h"
#include "items.h"
#include "ephemeral.h"
#include "sounds.h"

// ---- CHGame port ----
// The settings kept in flash (the Arduboy version kept sound on/off in
// EEPROM). The save pages are shared by every sketch ever uploaded; this
// magic is Poulet's own.
struct SaveData
{
    uint8_t sound_on;
};
static const uint32_t SAVE_MAGIC = save::magic("POUL");
static const uint8_t SAVE_VERSION = 1;
static SaveData save_data = {1};
static bool save_dirty = false;

// Wait for the next frame inside a blocking loop (the fades), still
// answering the debug protocol so the simulator can step through them.
// In lockstep (the simulator's scripts) it waits for the driver's next
// frames, as loop() would; running freely, the short pause is what moves
// the simulator's virtual clock on, and on the board it is only polling.
static void wait_frame()
{
    while (!chgame.nextFrame())
    {
        dbg::poll();
        if (chgame.lockstep == 0)
            dbg::waitInput();
        else
            delayMicroseconds(100);
    }
}

static GameState game_state = title_screen;

// 'universal frame' for animations not part of ents, a screen-ticker speed for wait times,
// record of the power_up_seq for which powerup will drop next, and where in the free-lives
// Aware scheme we are.
static uint8_t u_frame{0}, screen_ticker = 0, power_up_seq = 0, freelivesseq = 1;

// int freeMemory();

static Stage stage;

static uint8_t deathtime = DEATHTIME_MAX;
static uint8_t prize_seq = 0;

// Static Memory content:
static PlayerEntity player(ENT_POULET, 10.0f, 10.0f);
static uint16_t spawnstatus = 0; // Bit flags for what is spawned or not.
static Foe foe_roster[FOE_MAX];
static Balloon balloon_roster[BALLOON_MAX];
static EphemeralRoster ephemerals;
static ItemRoster items;

static Door door;
static Pole pole;

static int16_t scroll = 0;
static bool masterblink = true;
static uint8_t alttiles = 0;
static bool pending_game_over = false;

static void advance_master_frames();
static void next_stage();
static void fade_out();
static void fade_in();

// Object roster and allocations
static void allocate_foes();
static void init_foes();
static void update_foes();
static void allocate_balloons();

static void advance_stage();
static void show_title_screen();
static void show_ending_screen();
static void start_level();
static void setup_level();
static void die();
static void gameover();
static void kill_all_foes();
static void cleanup_spawns();
static bool spawn_foe(uint16_t newx, uint8_t newy, uint8_t etype);
static void spawn_balloon(uint16_t newx, uint8_t newy);
static void update_balloons();
static void draw_hud();
static void check_for_spawn(uint16_t scroll_x, int8_t tile_offset);
void show_debug_stage();

void setup()
{
    // Hardware stuff
    chgame.boot();              // buttons; START held 3 s returns to the SD menu
    dbg::begin("POUL 1.0");     // the debug protocol (simulator / debug builds only)
    screen::begin();            // the panel, 16 colours
    save::load(SAVE_MAGIC, SAVE_VERSION, save_data);
    sounds_begin(save_data.sound_on != 0);
    chgame.setFrameRate(60);

    // Static memory initializations:
    allocate_foes();
    allocate_balloons();
    //setup_level();

    // Game init stuff!
    door.x = (stage.exit_x * 8);
    door.y = (stage.exit_y * 8);
    //
    door.x = 1000;
    door.y = 40;
}

void loop()
{
    dbg::poll();
    if (!chgame.nextFrame())
        return;
    chgame.pollButtons();
    audio::update();
    screen::clear(14);
    masterblink = !masterblink;

    switch (game_state)
    {
    case title_screen:
        show_title_screen();
        advance_master_frames();
        if(stage.currentstage != 0){
            show_debug_stage();
        }

        if (chgame.justPressed(B_BUTTON | START_BUTTON))
        {            
            fade_out();
            screen::clear();

            screen_ticker = 0;
            game_state = interstitial;
        }

        if(chgame.justPressed(A_BUTTON)){
            audio::setOn(!audio::on());
            save_data.sound_on = audio::on();
            save_dirty = true;
        }

        // Stage select, kept to the stages that exist.
        if(chgame.justPressed(UP_BUTTON) && stage.currentstage < STAGE_COUNT - 1) stage.currentstage += 1;
        if(chgame.justPressed(DOWN_BUTTON) && stage.currentstage > 0) stage.currentstage -= 1;

        break;

    case victory:
        show_ending_screen();
        if (chgame.justPressed(A_BUTTON | B_BUTTON | START_BUTTON))
        {
            gameover(); // back to the title
        }
        break;


    case interstitial:
        if(screen_ticker == 0) { 
            sfx_start();
            }
        draw_hud();
        screen::draw(36, 30, stage_label, 0);
        draw_level(76, 30, stage.currentstage);
        screen_ticker += 1;
        if (screen_ticker >= SCREEN_TRANS_SPEED)
        {
            screen_ticker = 0;
            setup_level();
            game_state = in_play;
            alttiles = (stage.currentstage / 3) % 4;
        }
        break;

    case in_play:
        // Set up pole:
        if (stage.currentstage % 3 == 2)
        {
            pole.on = true;
            door.on = false;
        }
        else
        {
            pole.on = false;
            door.on = true;
        }

        // If dead!
        if (player.death)
        {
            deathtime -= 1;
            if (deathtime <= 0)
            {
                game_state = interstitial;
                deathtime = DEATHTIME_MAX;
                player.sprite = poulet_plus_mask;
                if(player.pending_game_over){
                    gameover();
                } else {
                    spawnstatus = 0;
                    kill_all_foes();
                    allocate_balloons();
                }
                setup_level();
            }
        }

        // Award lives?
        if (player.score >= freelivesseq * ONEUP_MILESTONE * (freelivesseq))
        {
            freelivesseq++;
            player.lives++;
            sfx_oneup();
            // Animated indicator of 1up using ephem system?
        }

        // Debug scroll?
        scroll = floor(player.x - 64);
        if (scroll < 0)
        {
            scroll = 0;
        }
        else if (scroll > 1024 - 128)
        {
            scroll = 1024 - 128;
        }
        stage.draw_level(scroll, alttiles);

        // pole.update(&player);
        pole.draw(scroll);

        door.update(&player);
        pole.update(&player);
        door.draw(scroll);
        stage.draw_coins(scroll);

        draw_hud();


        if (door.open)
        {
            advance_stage();
        }

        update_balloons();
        update_foes();

        if (stage.get_coin(uint16_t(player.x + SPR_LFTSKIN), uint16_t(player.y + SPR_TOPSKIN), uint16_t(player.x + SPR_RGTSKIN), uint16_t(player.y + SPR_BOTSKIN)))
        {
            player.score += 5;
            sfx_coin();
        }
        player.draw(scroll);
        player.control();
        if (player.physics(&stage))
            player.hitspike();

        items.updateRoster(&stage, &player, scroll, &ephemerals);
        if(ephemerals.updateRoster(scroll, player.x, player.y)){
            player.hitprojectile();
        }

        // Some in game stuff:
        if (player.y > 56)
        {
            player.vx = 0;
            player.vy = 0;
            if (!player.death)
            {
                die();
            }
        }

        // Work out what should spawn!
        cleanup_spawns();
        for (int8_t offset = 0; offset <= 3; offset++)
        {
            check_for_spawn(scroll, offset);
        }


        advance_master_frames();

        // If a flagpole was touched and the player is "celebrating"
        if (player.celebrate)
        {
            screen::draw(28, 30, stage_label, 0);
            screen::draw(66, 30, done_label, 0);
            screen_ticker += 1;
            if (screen_ticker > LEVEL_END_SPEED)
            {
                screen_ticker = 0;
                advance_stage();
            }
        }
        if(player.pending_game_over){
            screen::draw(28, 30, gameover_label, 0);
        }

        break;

    default:
        break;
    }

    // Flash writes wait for the panel to be idle.
    if (save_dirty)
    {
        gfx_wait();
        save_dirty = !save::store(SAVE_MAGIC, SAVE_VERSION, save_data);
    }
    screen::display();
}

static void advance_stage()
{
    fade_out();
    next_stage();
    if (stage.currentstage >= STAGE_COUNT)
    {
        // Past 8-3: there is no stage 9-1 to load.
        game_state = victory;
        player.celebrate = 0;
        return;
    }
    start_level();
    player.celebrate = 0;
}

static void advance_master_frames()
{
    static uint8_t master_ticker = TICKER_SPEED;
    // Advance the frames of all animations.
    master_ticker--;
    if (master_ticker <= 0)
    {
        u_frame++;
        master_ticker = TICKER_SPEED;
        stage.tile_anim = !stage.tile_anim;
    }
}

static void show_title_screen()
{
    screen::draw(7, 11, smallpoulet, 0);
    screen::draw(60, 16, titlecard, 0);
    screen::draw(74, 36, aplus, 0);
    if(audio::on()){
        screen::draw(112, 48, soundicon1, 0);
    } else {
        screen::draw(112, 48, soundicon2, 0);
    }
}

static void show_ending_screen(){
    screen::draw(7, 11, smallpoulet, 0);
}

static void next_stage()
{
    spawnstatus = 0;
    stage.currentstage += 1;
    door.open = false;
    player.x = 0;
    scroll = 0;
}

// A wipe to black, three columns a frame, over the game and the HUD.
static void fade_out()
{
    for (uint8_t i = 128; i > 3; i -= 3)
    {
        wait_frame();
        for (uint8_t layer = 0; layer < 2; layer++)
        {
            screen::hud(layer == 1);
            screen::drawFastVLine(i, 0, 128, screen::BACKGROUND);
            screen::drawFastVLine(i - 1, 0, 128, screen::BACKGROUND);
            screen::drawFastVLine(i - 2, 0, 128, screen::BACKGROUND);
        }
        screen::hud(false);

        screen::display();
    }
}

// The level revealed from the right, five columns a frame.
static void fade_in()
{
    for (uint8_t i = 1; i < 128; i += 5)
    {
        wait_frame();
        stage.draw_level(scroll, alttiles);
        stage.draw_coins(scroll);
        player.draw(scroll);
        screen::fillRect(0, 0, 128 - i, 64, screen::BACKGROUND);

        screen::display();
    }
}

static void allocate_balloons()
{
    for (uint8_t i = 0; i < BALLOON_MAX; i++)
    {
        balloon_roster[i] = Balloon(0, 0);
        balloon_roster[i].popped = true;
    }
}

static void allocate_foes()
{
    for (uint8_t i = 0; i < FOE_MAX; i++)
    {
        foe_roster[i] = Foe(ENT_FENNEC, 180, 10);
        foe_roster[i].x = 180 * i;
        foe_roster[i].spawned = false;
        foe_roster[i].dead = true;
    }
}

static bool spawn_foe(uint16_t newx, uint8_t newy, uint8_t etype)
{

    for (uint8_t i = 0; i < FOE_MAX; i++)
    {
        if (foe_roster[i].spawned == false)
        {
            foe_roster[i].x = newx;
            foe_roster[i].y = newy;
            foe_roster[i].spawned = true;
            foe_roster[i].dead = false;
            foe_roster[i].act = false;
            foe_roster[i].enttype = etype;
            foe_roster[i].assign_sprite();
            return true;
        }
    }
    return false;
}

static void update_foes()
{
    for (uint8_t i = 0; i < FOE_MAX; i++)
    {
        foe_roster[i].update(&stage, &player, &ephemerals);
        foe_roster[i].draw(scroll);

        // No need to operate on dead foes.
        if (foe_roster[i].dead)
        {
            continue;
        }

        if (foe_roster[i].x < (scroll - 24))
        {
            foe_roster[i].spawned = false;
            foe_roster[i].dead = true;
        }
        if (foe_roster[i].x > (scroll + 256))
        {
            foe_roster[i].spawned = false;
            foe_roster[i].dead = true;
        }

        if (foe_roster[i].collide(&player))
        {
            if (!player.attack)
            {
                if(player.toque){
                    prize_seq = 0;
                }
                player.takehit(&foe_roster[i]);
            }
            else
            {
                foe_roster[i].dead = true;
                // player.attack = false; // Attack persists until touching ground!
                player.vy = -KILL_BOUNCE;
                player.score += 25;
                sfx_stomp();
            }
        }
    }
}

static void spawn_balloon(uint16_t newx, uint8_t newy)
{

    for (uint8_t i = 0; i < BALLOON_MAX; i++)
    {
        if (balloon_roster[i].popped == true)
        {
            balloon_roster[i].x = newx;
            balloon_roster[i].y = newy;
            balloon_roster[i].popped = false;
            break;
        }
    }
}

static void update_balloons()
{
    // Draw them all on screen
    for (uint8_t i = 0; i < BALLOON_MAX; i++)
    {
        if (balloon_roster[i].popped)
        {
            continue;
        }
        balloon_roster[i].draw(scroll);

        if (balloon_roster[i].collide(&player))
        {
            ephemerals.add(balloon_roster[i].x, balloon_roster[i].y, pop, 0, 0);
            sfx_pop();

            switch (prize_seq)
            {
            case PRIZE_TOQUE:
                items.add(balloon_roster[i].x, balloon_roster[i].y, PRIZE_TOQUE);
                break;

            case PRIZE_GLASSES:
                items.add(balloon_roster[i].x, balloon_roster[i].y, PRIZE_GLASSES);
                break;

            case PRIZE_1UP:
                items.add(balloon_roster[i].x, balloon_roster[i].y, PRIZE_1UP);
                break;

            default:
                break;
            }

            balloon_roster[i].popped = true;
            prize_seq += 1;
            if (prize_seq > PRIZE_1UP)
            {
                prize_seq = PRIZE_TOQUE;
            }
        }
    }
}

// In the strip above the game (screen::HUD_Y), clear of the level.
static void draw_hud()
{
    screen::hud(true);
    draw_digits(player.score, 7, 80, 0);
    draw_lives(96, 0, player.lives);
    draw_level(1, 0, stage.currentstage);
    screen::hud(false);
}

static void start_level()
{
    scroll = 0;
    if (stage.currentstage % 3 == 0)
    {
        game_state = interstitial;
    }
    else
    {
        game_state = in_play;
    }

    kill_all_foes();

    setup_level();
    // Check for spawns in this tile and one to the right:
    check_for_spawn(scroll, 0);
    check_for_spawn(scroll, 1);
    check_for_spawn(scroll, 2);
    check_for_spawn(scroll, 3);

    if (game_state == in_play)
    {
        fade_in();
    }
}

static void setup_level()
{
    items.emptyRoster();
    stage.fill_coins();
    player.x = 8;
    player.y = 8;
    player.attack = false;
    player.skidding = false;
    player.vx = 0;
    player.vy = 0;
    player.death = false;
}

/*
int freeMemory()
{
    extern int __heap_start, *__brkval;
    int v;
    return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}
*/

static void die()
{
    player.death = true;
    sfx_die();
    if(player.lives > 0){
        player.lives -= 1;
    } else {
        player.pending_game_over = true;
    }
    player.toque = false;
    player.flyboy = false;
}

static void gameover(){
     
    screen_ticker = 0; 
    power_up_seq = 0; 
    freelivesseq = 1;
    game_state = title_screen;
    spawnstatus = 0;
    kill_all_foes();
    allocate_balloons();
    stage.currentstage = 0;
    door.open = false;
    player.x = 0;
    player.lives = START_LIVES;
    player.score = 0;
    scroll = 0;   
    player.pending_game_over = false;
}

static bool check_spawn_status(uint8_t position)
{
    uint16_t reversed_position = 15 - position;
    return (spawnstatus & (1 << reversed_position)) != 0;
}

static void set_spawn_status(bool newstate, uint16_t position)
{
    uint16_t reversed_position = 15 - position;
    if (newstate == true)
    {
        spawnstatus |= (1 << reversed_position);
    }
    else
    {
        spawnstatus &= ~(1 << reversed_position);
    }
}

static uint8_t spawn_type(uint8_t position)
{
    // Put some bounds checking here?
    if ((int8_t)position < 0)
    {
        return 0;
    }

    uint16_t slice_data = stages[stage.currentstage * 16 + position];
    return (slice_data >> 12) & 0x0F;
}

static bool spawn_high(uint8_t position)
{
    uint16_t slice_data = stages[stage.currentstage * 16 + position];
    return (slice_data >> 11) & 0x01;
}

static void cleanup_spawns()
{
    uint8_t meta_tile = (scroll / 64);
    for (uint8_t i = 0; i < 16; i++)
    {
        if (i < (meta_tile - 4) || i > (meta_tile + 6))
        {
            if(i > -1 && i < 16){
                set_spawn_status(false, i);
            }
        }
    }
}

static void kill_all_foes()
{

    for (uint8_t i = 0; i < FOE_MAX; i++)
    {
        foe_roster[i].dead = true;
        foe_roster[i].act = false;
        foe_roster[i].spawned = false;
    }
}

static void check_for_spawn(uint16_t scroll_x, int8_t tile_offset)
{
    uint8_t meta_tile = (scroll_x / 64) + tile_offset;

    uint16_t spawnheight = 0;

    if (check_spawn_status(meta_tile) == false)
    {
        uint16_t spawnx = ((meta_tile * 64) + 16);
        uint16_t target_spawn_x = spawnx;

        // Find where spawnx should be-- not on top of a holes or unsupported spikes:
        bool no_hole = false;
        // Sweep from centre to right...
        for (uint16_t i = 0; i < 16; i += 8){
            if((stage.is_solid(spawnx + i, 56) == 0) || stage.is_spike(spawnx + i, 56)) {
                // still a hole, keep going
            } else {
                target_spawn_x = spawnx + i;
                no_hole = true;
                break;
            }
        }
        if(!no_hole){
            target_spawn_x = spawnx; // Reset this for the test.
            for (uint16_t i = 0; i < 24; i += 8){
                if((stage.is_solid(spawnx - i, 56) == 0) || stage.is_spike(spawnx - i, 56)){
                    // still a hole
                } else {
                    target_spawn_x = spawnx - i;
                    no_hole = true;
                    break;
                }
            }
        }

        // If we've still placed it above a hole, we just let it happen.  Hey, it might be a bird anyways.
        if(no_hole){
            spawnx = target_spawn_x;
        }
            
        

        if (spawn_high(meta_tile) == true)
        {
            for (uint8_t i = 0; i < 40; i += 8)
            {
                if (stage.is_solid(spawnx, i + 16) == 0)
                {
                    spawnheight = i;
                    break;
                }
            }
        }
        else
        {
            for (uint8_t i = 40; i > 0; i -= 8)
            {
                if (stage.is_solid(spawnx, i + 16) == 0)
                {
                    spawnheight = i;
                    break;
                }
            }
        }

        bool spawn_ok = true;

        switch (spawn_type(meta_tile))
        {
        case SPAWN_BALLOON:
            spawn_balloon(spawnx, spawnheight);
            break;

        case SPAWN_FENNEC:
            spawn_ok = spawn_foe(spawnx, spawnheight, ENT_FENNEC);
            break;

        case SPAWN_GOOB:
            spawn_ok = spawn_foe(spawnx, spawnheight, ENT_GOOB);
            break;

        case SPAWN_BLOOB:
            spawn_ok = spawn_foe(spawnx, spawnheight, ENT_BLOOB);
            break;

        case SPAWN_DRAKE:
            spawn_ok = spawn_foe(spawnx, spawnheight, ENT_DRAKE);
            break;

        case SPAWN_2FENNEC:
            spawn_ok = spawn_foe(spawnx - 16, spawnheight, ENT_FENNEC);
            spawn_ok = spawn_foe(spawnx + 16, spawnheight, ENT_FENNEC);
            break;

        case SPAWN_2GOOB:
            spawn_ok = spawn_foe(spawnx - 16, spawnheight, ENT_GOOB);
            spawn_ok = spawn_foe(spawnx + 16, spawnheight, ENT_GOOB);
            break;

        default:
            break;
        }

        if(spawn_ok){
            set_spawn_status(true, meta_tile);
        }
    }
}

// The title's stage select, where the HUD's stage number goes.
void show_debug_stage(){
    screen::hud(true);
    draw_level(1, 0, stage.currentstage);
    screen::hud(false);
}