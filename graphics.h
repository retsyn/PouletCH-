#pragma once

#define MIRROR 8
#define FOE_MIRROR 3

#include "globals.h"

extern const unsigned char foreground[];
extern const unsigned char foregroundb[];
extern const unsigned char foregroundc[];
extern const unsigned char foregroundd[];
extern const unsigned char poulet_plus_mask[];
extern const unsigned char aviator_plus_mask[];
//extern const uint8_t title[];
extern const unsigned char smallpoulet[];
extern const unsigned char titlecard[];
extern const unsigned char aplus[];



// Anim frame id's for Poulet
extern const unsigned char poulet_anim_idle[];
extern const unsigned char poulet_anim_walk[];
extern const unsigned char poulet_anim_jump_up[]; 
extern const unsigned char poulet_anim_jump_down[];
extern const unsigned char poulet_anim_attack[];
extern const unsigned char poulet_anim_death[];
extern const unsigned char poulet_anim_hover[];

// objects
extern const unsigned char doorspr[];
extern const unsigned char polemid[];
extern const unsigned char poletop[];


// Anim frame id's for all foes
extern const unsigned char foe_anim_walk[];
extern const unsigned char foe_anim_die[];

// Foes
extern const unsigned char fennec_plus_mask[];
extern const unsigned char goob_plus_mask[];
extern const unsigned char bloob_plus_mask[];
extern const unsigned char proj_plus_mask[];
extern const unsigned char drake_plus_mask[];


// The toque!
extern const unsigned char toque_plus_mask[];

// The digits
extern const unsigned char digits_sheet[];

// Other text
extern const uint8_t stage_label[];
extern const unsigned char done_label[];
extern const uint8_t gameover_label[];

// Items:
extern const unsigned char glasses_plus_mask[];
extern const unsigned char balloon_plus_mask[];
extern const unsigned char toquepowerup_plus_mask[];
extern const unsigned char oneup_plus_mask[];
extern const unsigned char toquetext_plus_mask[];

// Ephemeral effects:
extern const unsigned char pop_plus_mask[];
extern const unsigned char flyboy_plus_mask[];
extern const unsigned char soundicon1[];
extern const unsigned char soundicon2[];

void draw_digits(uint32_t value, uint8_t digits, uint8_t x, uint8_t y);
void draw_lives(uint8_t x, uint8_t y, uint8_t total);
void draw_level(uint8_t x, uint8_t y, uint8_t stage);
