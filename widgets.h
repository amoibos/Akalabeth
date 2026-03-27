#ifndef WIDGETS_H
#define WIDGETS_H

#include "libs/console.h"
#include "libs/strings.h"
#include "data.h"

typedef enum eEffect {
    EffectNone,
    EffectSpiral
} Effect;

typedef enum eMainMenu {
     MainMenuUndefined
    ,MainMenuNewGame
    ,MainMenuLevelSelect
   // ,MainMenuHelp
    ,MainMenuCredits
    //,MainMenuCongratulation
} MainMenu;




typedef enum eMenuMode {
    MenuModeUndefined,
    MenuModeCenter,
    MenuModeLeft
} MenuMode;

#define NUMERIC                     "0123456789"
#define EXTENDED_NUMERIC NUMERIC    "+-."
#define ALPHA_NUMERIC               "ABCDEFGHIJKLMNOPQRSTUVWXYZ"\
                                    "abcdefghijklmnopqrstuvwxyz"\
                                    EXTENDED_NUMERIC

unsigned int pressed_anything(void);
unsigned char menu(unsigned char count, unsigned char offset, _Bool horizontal,
                   _Bool highlight, const char * const *labels,
                   unsigned char initial_selection);

extern void (*timer_callback)(void);
extern _Bool audio_enabled;
extern _Bool timer_enabled;
extern unsigned short seconds;
extern unsigned char fps;
extern _Bool animation_refresh;

void print_img_compressed(	const unsigned char *tiledata,
				const unsigned char *colordata,
				const unsigned short width, const unsigned char height,
                const unsigned char left, const unsigned char top,
                Effect effect,
                const unsigned short start_img_tiles);

extern void SetTimerCallback(void (*theHandlerFunction)(void)) __z88dk_fastcall;
extern unsigned int wait_for_key(unsigned int keys);

/* Mappt Port-2- und Tastatur-Bits auf Port-1-Äquivalente */
static inline unsigned int map_b_to_a(unsigned int k) {
    if (k & PORT_B_KEY_UP)    k |= PORT_A_KEY_UP;
    if (k & PORT_B_KEY_DOWN)  k |= PORT_A_KEY_DOWN;
    if (k & PORT_B_KEY_LEFT)  k |= PORT_A_KEY_LEFT;
    if (k & PORT_B_KEY_RIGHT) k |= PORT_A_KEY_RIGHT;
    if (k & PORT_B_KEY_1)     k |= PORT_A_KEY_1;
    if (k & PORT_B_KEY_2)     k |= PORT_A_KEY_2;
    return k;
}

#endif
