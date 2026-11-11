#include "globals.h"
#include "sounds.h"

// The Arduboy version's ArduboyTones sequences, note for note, as CHGame
// audio:: effects. Each step is AUDIO_STEP(pitch Hz, 0 = no sweep, ms);
// AUDIO_REST(ms) is ArduboyTones' NOTE_REST. The engine keeps pitches in
// 20 Hz units and lengths in 2 ms units, so notes below C4 land up to a
// few percent off and 13 ms steps play for 14 ms.
//
// Every effect has the same priority, so a new one cuts off the one
// playing, as a new tones() call did with ArduboyTones.

// ArduboyTones' note frequencies (its NOTE_xxH "high volume" variants are
// the same pitches; the CHGame piezo has one volume).
// In a namespace: the core already defines A3, D4 ... as pin names.
namespace hz {
enum : uint16_t {
    CS3 = 139, DS3 = 156, E3 = 165, F3 = 175, FS3 = 185, G3 = 196, GS3 = 208,
    A3 = 220, AS3 = 233,
    C4 = 262, CS4 = 277, D4 = 294, F4 = 349, G4 = 392, GS4 = 415, A4 = 440, AS4 = 466,
    C5 = 523, CS5 = 554, D5 = 587, DS5 = 622, E5 = 659, F5 = 698, FS5 = 740,
    G5 = 784, GS5 = 831, A5 = 880, AS5 = 932,
    C6 = 1047, CS6 = 1109, D6 = 1175, DS6 = 1245, F6 = 1397, FS6 = 1480,
    G6 = 1568, GS6 = 1661, A6 = 1760, AS6 = 1865,
};
}  // namespace hz

#define N(note, ms) AUDIO_STEP(hz::note, 0, ms)
#define R(ms)       AUDIO_REST(ms)

AUDIO_STEPS(SEQ_JUMP) = {
    N(E5, 13), N(FS5, 13), N(G5, 13) };

AUDIO_STEPS(SEQ_COIN) = {
    N(F5, 26), N(C6, 26) };

AUDIO_STEPS(SEQ_STOMP) = {
    N(DS3, 13), N(A5, 13), N(GS3, 13), N(A3, 13),
    N(C4, 13), N(D4, 13), N(C6, 13) };

AUDIO_STEPS(SEQ_POP) = {
    N(CS3, 4), N(AS6, 4), N(GS6, 4), N(AS3, 4),
    N(CS3, 4), N(A4, 4), R(4), N(C5, 4),
    N(CS3, 4), N(G4, 4) };

AUDIO_STEPS(SEQ_POWERUP) = {
    N(G3, 26), N(E3, 26), N(G3, 26), N(C4, 26),
    N(G4, 26), N(E5, 26), N(C6, 26), N(G6, 26) };

AUDIO_STEPS(SEQ_ONEUP) = {
    N(DS6, 54), R(54), N(F6, 54), R(54),
    N(AS6, 54), R(54), N(G6, 54), R(163),
    N(GS6, 54) };

AUDIO_STEPS(SEQ_HURT) = {
    N(G3, 8), N(A6, 8), N(GS3, 8), N(GS6, 8),
    N(G3, 8), N(G6, 8), N(F3, 8), N(DS6, 8),
    N(E3, 8), N(CS6, 8) };

AUDIO_STEPS(SEQ_DIE) = {
    N(E5, 13), N(FS5, 13), N(G5, 13), N(CS6, 13),
    N(FS6, 13), N(A6, 13), N(G6, 13), N(D6, 13),
    N(GS5, 13), N(E5, 13), N(DS5, 13), N(D5, 13),
    N(DS5, 13) };

AUDIO_STEPS(SEQ_CLEAR) = {
    N(AS4, 64), R(64), N(C5, 64), R(64),
    N(AS4, 64), R(192), N(C5, 192), R(64),
    N(G4, 64), R(64), N(G4, 64), R(64),
    N(G4, 64), R(192), N(F4, 64), R(192),
    N(AS4, 64), R(64), N(C5, 64) };

AUDIO_STEPS(SEQ_START) = {
    N(DS6, 64), R(64), N(C6, 64), R(64),
    N(AS5, 64), R(192), N(G5, 64), R(192),
    N(F5, 64), R(192), N(C5, 64), R(64),
    N(DS5, 64), R(64), N(G5, 64), R(192),
    N(F5, 64) };

AUDIO_STEPS(SEQ_ATTACK) = {
    N(GS5, 17), N(CS5, 13), R(3), N(A4, 14),
    R(3), N(G4, 13), R(3), N(F4, 14),
    R(3), N(CS4, 13), R(3), N(GS3, 14) };

AUDIO_STEPS(SEQ_DOOR) = {
    N(AS6, 6), N(E3, 6), N(E5, 6), N(E3, 6),
    N(E5, 6), N(E3, 6), N(D5, 6), N(FS3, 6),
    R(38), N(AS6, 6), N(GS5, 6), N(FS6, 6),
    N(GS5, 6), N(FS6, 6), N(GS5, 6), N(FS6, 6),
    N(FS5, 6) };

#undef N
#undef R

enum class Sfx : uint8_t {
    Jump, Coin, Stomp, Pop, Powerup, Oneup, Hurt, Die, Clear, Start, Attack, Door, COUNT
};

static const audio::Effect SOUNDS[] = {
    AUDIO_EFFECT(SEQ_JUMP, 1),
    AUDIO_EFFECT(SEQ_COIN, 1),
    AUDIO_EFFECT(SEQ_STOMP, 1),
    AUDIO_EFFECT(SEQ_POP, 1),
    AUDIO_EFFECT(SEQ_POWERUP, 1),
    AUDIO_EFFECT(SEQ_ONEUP, 1),
    AUDIO_EFFECT(SEQ_HURT, 1),
    AUDIO_EFFECT(SEQ_DIE, 1),
    AUDIO_EFFECT(SEQ_CLEAR, 1),
    AUDIO_EFFECT(SEQ_START, 1),
    AUDIO_EFFECT(SEQ_ATTACK, 1),
    AUDIO_EFFECT(SEQ_DOOR, 1),
};
static_assert(sizeof SOUNDS / sizeof SOUNDS[0] == (size_t)Sfx::COUNT, "one effect per Sfx");

void sounds_begin(bool on) { audio::begin(SOUNDS, (uint8_t)Sfx::COUNT, on); }

void sfx_jump()    { audio::sfx(Sfx::Jump); }
void sfx_coin()    { audio::sfx(Sfx::Coin); }
void sfx_stomp()   { audio::sfx(Sfx::Stomp); }
void sfx_pop()     { audio::sfx(Sfx::Pop); }
void sfx_powerup() { audio::sfx(Sfx::Powerup); }
void sfx_oneup()   { audio::sfx(Sfx::Oneup); }
void sfx_hurt()    { audio::sfx(Sfx::Hurt); }
void sfx_die()     { audio::sfx(Sfx::Die); }
void sfx_clear()   { audio::sfx(Sfx::Clear); }
void sfx_start()   { audio::sfx(Sfx::Start); }
void sfx_attack()  { audio::sfx(Sfx::Attack); }
void sfx_door()    { audio::sfx(Sfx::Door); }
