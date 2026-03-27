#ifndef GLOBAL_H
#define GLOBAL_H

#include "data.h"

#define MAX_ITEMS       ((6))
#define MAX_STATS       ((6))
#define MAX_MONSTERS    ((11))

#define TERRAIN_MIN     ((0))
#define TERRAIN_MAX     ((20))
#define DUNGEON_MIN     ((0))
#define DUNGEON_MAX     ((10))

typedef enum {
    pcMAGE, pcFIGHTER
} ePlayerClass;

typedef struct {
    ePlayerClass playerclass;
    int stats[MAX_STATS];
    int inventory[MAX_ITEMS];
    unsigned int lucky_number;
    unsigned char level_of_play;
    // current floor, overworld is 0, in a dungeon it is > 0
    signed char floor;
    // set to 1 once the player has accepted a quest from Lord British
    _Bool has_quest;
    // current screen context (ScreenOverworld or ScreenDungeon)
    eScreens current_screen;
} sPlayer;

typedef enum {
    iFOOD, iRAPIER, iAXE, iSHIELD, iBOW_AND_ARROWS, iAMULET
} eItem;

typedef enum {
    sHIT_POINTS, sSTRENGTH, sDEXTERITY, sSTAMINA, sWISDOM, sGOLD
} eStats;

typedef enum {
    mUNDEFINED
    ,mSKELETON, mTHIEF, mGIANT_RAT, mORC, mVIPER
    ,mCARRION_CRAWLER, mGREMLIN, mMIMIC, mDAEMON, mBALROG
} eMonster;

typedef enum {
    tEMPTY, tMOUNTAINS, tTREES, tTOWN, tDUNGEON, tCASTLE
} eTerrain;

typedef enum {
    oEMPTY, oWALL, oTRAP=2, oSECRET_DOOR=3, oDOOR=4, oCHEST=5, oLADDER_DOWN=7, oLADDER_UP=8, oPIT=9, oOBJECT_MAX
} eObjects;

typedef struct {
    _Bool alive;
    unsigned char hp;
    sPoint location;
} sMonster;

#define HIGHLIGHT_OFF           ((0))
#define HIGHLIGHT_ON            ((1))

#define VERTICAL                ((0))
#define HORIZONTAL              ((1))

#define OVERWORLD_LEVEL         ((0))
#define FIRST_DUNGEON_LEVEL     ((1))

#define NO_SPECIFIC_KEY         ((0))

#define abs(x)   ((x) < 0 ? -(x) : (x))
#define sqr(x)   ((x) * (x))

// current animation frame for a sprite
extern unsigned char animation_frame;
// save tile index where the sprite is located
extern signed short all_sprites[MAX_SPRITE];
// update sprite state
extern _Bool animation_refresh;
// current number of sprites
extern unsigned char sprites_no;

extern unsigned char previous_fps_seqment;

extern sPlayer player;
// option to disable audio
extern _Bool audio_enabled;
// for pausing the timer and animation
extern _Bool timer_enabled;
// counting passed seconds
extern unsigned short seconds;
// helper for counting seconds;
extern unsigned char fps;
//sum of level records
extern unsigned short totaltime;

extern sState state;

extern unsigned char dungeon_obj[DUNGEON_MAX+1][DUNGEON_MAX+1];
extern unsigned char dungeon_mon[DUNGEON_MAX+1][DUNGEON_MAX+1];
/* Nibble-packed terrain: two eTerrain values per byte (values 0-5 fit in 4 bits).
   Low nibble = even (y-1), high nibble = odd (y-1).
   Border (x=0, x=20, y=0, y=20) is implicit tMOUNTAINS — not stored.
   Array stores inner field x=1..19, y=1..19: terrain[19][10] = 190 bytes. */
extern unsigned char terrain[TERRAIN_MAX-1][TERRAIN_MAX/2];

#define terrain_get(x, y) \
    ((x) == TERRAIN_MIN || (x) == TERRAIN_MAX || \
     (y) == TERRAIN_MIN || (y) == TERRAIN_MAX    \
        ? tMOUNTAINS                              \
        : ((((y)-1)&1) ? (terrain[(x)-1][((y)-1)>>1] >> 4)    \
                        : (terrain[(x)-1][((y)-1)>>1] & 0x0F)))

#define terrain_set(x, y, v) do { \
    if ((x) != TERRAIN_MIN && (x) != TERRAIN_MAX && \
        (y) != TERRAIN_MIN && (y) != TERRAIN_MAX) {  \
        if (((y)-1)&1) \
            terrain[(x)-1][((y)-1)>>1] = (terrain[(x)-1][((y)-1)>>1] & 0x0Fu) | (unsigned char)((unsigned char)(v) << 4); \
        else \
            terrain[(x)-1][((y)-1)>>1] = (terrain[(x)-1][((y)-1)>>1] & 0xF0u) | ((unsigned char)(v) & 0x0Fu); \
    } \
} while(0)

extern eMonster quest_monster;
extern _Bool    quest_done;

extern const unsigned char monster_pos[][3+1];
extern const unsigned char side_door[][5+1];
extern const unsigned char font_door[][3+1];
extern const unsigned char floor_hole[][5+1];
extern const unsigned char ladder[][3+1];

extern unsigned char draw_center_y;
extern const unsigned char draw_center_x;

extern const unsigned char ITEM_PRICE[MAX_ITEMS];


extern const char MONSTERS[][16];
extern const char ITEMS[][13];
extern const char STATS[][12];
extern const char SHORT_ITEMS[MAX_ITEMS];
extern const char PRICE_TABLE[][18];
extern const unsigned char WEAPONS[MAX_ITEMS];

extern sPoint dunloc;
extern sSignedPoint movdir;
extern sPoint terrloc;

extern sMonster monster[MAX_MONSTERS];

#endif
