#pragma once


#define START_LIVES 2

#define LEFT_BOUND 0
#define RIGHT_BOUND 1024

#define ONEUP_MILESTONE 1000

#define DEATHTIME_MAX 255

#define KILL_BOUNCE 1.25
#define SKID_MAX 8

#define COIN_PTS 2

#define FOE_MAX 5
#define BALLOON_MAX 3
#define ITEM_MAX 2

#define EPHEM_ANIM_SPEED 3
#define EPHEM_TEXT_SPEED 10
#define EPHEM_MAX 5
#define EPHEM_END_FRAME 4

#define LEVEL_END_SPEED 250


// CHGame: buttons and frame pacing (chgame), sound (audio::), saves (save::),
// the panel (CHGfx). The game draws through screen.h: 128x64 in 16 colours.
#include <CHGame.h>
#include "screen.h"

#define TICKER_SPEED 30
#define SCREEN_TRANS_SPEED 120

// (moved from game.h)
enum GameState{
    title_screen = 0,
    instruct,
    in_play,
    interstitial,
    game_over,
    victory
};
