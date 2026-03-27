#include "global.h"

// current animation frame for a sprite
unsigned char animation_frame;
// save tile index where the sprite is located
signed short all_sprites[MAX_SPRITE];
// update sprite state
_Bool animation_refresh;
// current number of sprites
unsigned char sprites_no;

unsigned char previous_fps_seqment=0;

sPlayer player;
// option to disable audio
_Bool audio_enabled;
// for pausing the timer and animation
_Bool timer_enabled;
// counting passed seconds
unsigned short seconds;
// helper for counting seconds;
unsigned char fps;
//sum of level records
unsigned short totaltime=0;

sState state;

unsigned char dungeon_obj[DUNGEON_MAX+1][DUNGEON_MAX+1];
unsigned char dungeon_mon[DUNGEON_MAX+1][DUNGEON_MAX+1];
unsigned char terrain[TERRAIN_MAX-1][TERRAIN_MAX/2];

//quest to kill one monster with increasing difficulty level
eMonster quest_monster = mUNDEFINED;
_Bool    quest_done    = 0;

unsigned char draw_center_y;
const unsigned char draw_center_x = 139;

const unsigned char ITEM_PRICE[MAX_ITEMS] = {1, 8, 5, 6, 3, 15};

const unsigned char monster_y[] = {79, 47, 25, 16, 12, 10, 8, 7, 6, 5};

const unsigned char monster_pos[DUNGEON_MAX+1][4] = {
    {  0, 255,   0, 159},
    { 56, 222,  32, 126},
    { 95, 183,  54, 104},
    {110, 168,  63,  95},
    {117, 161,  67,  91},
    {121, 157,  69,  89},
    {124, 154,  71,  87},
    {126, 152,  72,  86},
    {128, 150,  73,  85},
    {129, 149,  74,  84},
    {130, 148,  74,  84}
};
const unsigned char side_door[DUNGEON_MAX+1][6] = {
    { 19,   37,  38,  45, 148, 137},
    { 69,   82,  55,  60, 119, 111},
    {100,  105,  66,  68, 101,  98},
    {112,  115,  70,  70,  94,  92},
    {118,  120,  72,  72,  90,  90},
    {122,  123,  74,  74,  88,  88},
    {125,  125,  75,  75,  87,  86},
    {127,  127,  76,  75,  86,  85},
    {128,  129,  75,  75,  85,  84},
    {129,  130,  76,  76,  84,  84},
    {  0,    0,   0,   0,   0,   0}
};

const unsigned char font_door[DUNGEON_MAX+1][4] = {
    {  0,   0,   0,   0},
    {111, 167,  46, 126},
    {124, 154,  62, 104},
    {129, 149,  68,  95},
    {132, 146,  71,  91},
    {133, 145,  72,  89},
    {134, 144,  73,  87},
    {135, 143,  74,  86},
    {135, 143,  75,  85},
    {136, 142,  76,  84},
    {136, 142,  76,  84}
};

const unsigned char floor_hole[DUNGEON_MAX+1][6] = {
    { 93, 185, 111, 167, 147, 137},
    {111, 167, 124, 154, 119, 111},
    {124, 154, 129, 149, 101,  98},
    {129, 149, 132, 146,  94,  92},
    {132, 146, 133, 145,  90,  90},
    {133, 145, 134, 144,  88,  88},
    {134, 144, 135, 143,  87,  86},
    {135, 143, 135, 143,  86,  85},
    {135, 143, 136, 142,  85,  84},
    {136, 142, 136, 142,  84,  84},
    {  0,   0,   0,   0,   0,   0}
};

const unsigned char ladder[DUNGEON_MAX+1][4] = {
    {124, 154,  12, 147},
    {130, 148,  40, 119},
    {134, 144,  58, 101},
    {136, 142,  65,  94},
    {136, 141,  69,  90},
    {137, 141,  71,  88},
    {137, 141,  72,  87},
    {137, 140,  73,  86},
    {138, 140,  74,  85},
    {138, 140,  75,  84},
    {  0,   0,   0,   0}
};

const char PRESS_ANY_KEY_TO[] = "Press any Key to Continue";
const char _GOLD[] = "GOLD=";
const char _HP[] = "H.P.=";
const char _FOOD[] = "FOOD=";

const char PLAYER_NAME[] = "Adventurer";

//original M$
const char MONSTERS[MAX_MONSTERS][16] = { ""
                            ,"SKELETON", "THIEF", "GIANT RAT", "ORC", "VIPE"
                            ,"CARRION CRAWLER", "GREMLIN", "MIMIC", "DAEMON", "BALROG"
                        };
//original W$
const char ITEMS[MAX_ITEMS][13]       = {    "FOOD",  "RAPIER",   "AXE",  "SHIELD",   "BOW & ARROWS",   "MAGIC AMULET"};
const char SHORT_ITEMS[MAX_ITEMS]     = {    'f',     'r',        'a',    's',        'b',                'm'};
//orginal C$
const char STATS[MAX_STATS][12]       = {   "HIT POINTS.", "STRENGTH...", "DEXTERITY..", "STAMINA....", "WISDOM.....", "GOLD......."};

const char PRICE_TABLE[MAX_ITEMS][18] = {   "1 FOR 10    N/A",  "8           1-10", "5           1-5", "6           1", "3           1-4", "15          ?????"};

//   iFOOD, iRAPIER, iAXE, iSHIELD, iBOW_AND_ARROWS, iAMULET
const unsigned char WEAPONS[MAX_ITEMS] = {0, 10, 5, 1, 4, 0};

// player position in dungeon
sPoint dunloc;

// player movement direction in dungeon
sSignedPoint movdir;

// player position in overworld
sPoint terrloc;

// monsters in the dungeon with their stats and location
sMonster monster[MAX_MONSTERS];

