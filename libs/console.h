#ifndef CONSOLE_H
#define CONSOLE_H

#ifdef PLATFORM_SMS
#include "../SMSlib/SMSlib.h"
//#endif
#else
//#ifdef PLATFORM_SG
#include "../SGlib/SGlib.h"
#include "../assets/font.h"
#endif

#include "../data.h"
#include "../global.h"
#include "../libs/strings.h"

#define FRAME_RATE      ((60))

#define SCREEN_MAX_Y    ((192U))
#define SCREEN_MAX_X    ((256U))

#define TEXTCONSOLE_MAX_Y   ((24))
#define TEXTCONSOLE_MAX_X   ((32))

void clear_screen(void);

#ifdef PLATFORM_SMS
#define mapROMBank(x) SMS_mapROMBank(x)
#else //elif PLATFORM_SG || PLATFORM_SC
	#ifdef ALLOW_MAPPER
		#define mapROMBank(x) SG_mapROMBank(x)
	#else
		#define mapROMBank(x)	/**/
	#endif
#endif

//#define print(str)    print_str(&state.textconsole.x, &state.textconsole.y, (const char*)str, 0)

#define loadTiles SG_loadTilePatterns
#define loadPalette SG_loadTileColours
#define setNextTileatXY SG_setNextTileatXY
#define setTile SG_setTile
#define getKeysHeld SG_getKeysHeld
#define displayOn SG_displayOn
#define displayOff SG_displayOff
#define get_tile SG_getTileatXY
#define keypressed SG_getKeysPressed
#define keyreleased SG_getKeysReleased
#define readkey SG_getKeysStatus
#define waitForVBlank SG_waitForVBlank
#define setFrameInterruptHandler SG_setFrameInterruptHandler
#define initSprites SG_initSprites
#define addSprite SG_addSprite
#define finalizeSprites SG_finalizeSprites
#define loadZX7compressedTiles SG_loadZX7compressedBGTiles
#define loadZX7compressedBGColors SG_loadZX7compressedBGColors
#define scanKeyboardJoypad	SG_scanKeyboardJoypad
#define getKeyboardJoypadPressed SG_getKeyboardJoypadPressed
#define getKeyboardJoypadStatus SG_getKeyboardJoypadStatus
#define copySpritestoSAT SG_copySpritestoSAT
#define putPixel SG_putPixel
#define setBackdropColor SG_setBackdropColor
#define blankArea SG_blankArea

#define SCROLL_ROW_START    ((unsigned char)20)
#define SCROLL_ROW_END      ((unsigned char)23)
#define SCROLL_COL_END      ((unsigned char)19)
#define PRINT_WRAP_MAX_X    ((unsigned char)20)

void wait(unsigned char duration);

void print_tile(unsigned char x, unsigned char y, unsigned short tileno);
void print_font(unsigned char x, unsigned char y, unsigned short tileno);
void print(const char *str);
void print_wrap(const char *str);
//void print_num(unsigned char * x, unsigned char * y, long num, short offset);
void print_raw(unsigned char x, unsigned char y, char str[]);

#endif
