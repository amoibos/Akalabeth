#ifndef ENGINE_H
#define ENGINE_H

//#include "SGlib/SGlib.h"
//#include "PSGlib/PSGlib.h"

#include "libs/strings.h"
#include "libs/console.h"

#include "global.h"
#include "views.h"
#include "libbasic.h"
#include "fbuffer.h"

// l1+r1 for di>0 (56+222, 95+183, ...); mirror axis for right-side doors
#define DOOR_MIRROR_AXIS 278

#define TERRAIN_THRESH_EMPTY      190  // r < 190 → ~74%
#define TERRAIN_THRESH_MOUNTAINS  216  // r < 216 → +10%
#define TERRAIN_THRESH_TREES      234  // r < 234 → +7%
#define TERRAIN_THRESH_TOWN       250  // r < 250 → +6.25%, Dungeon: rest ~2.3%

#define SPIDER_SLOT         8   /* 3 frames occupy slots 8, 9, 10 */
#define SPIDER_FRAMES       3
#define THREAD_SLOT         11
#define SPIDER_START_Y      2
#define SPIDER_END_Y        68
#define SPIDER_STEP         3
#define SPIDER_ANIM_DIVIDER 2   /* move spider every N callback ticks */
#define ANIM_X_LEFT         22
#define ANIM_X_RIGHT        215

void generate_environment(void);
#define wait_half_second()  wait(FRAME_RATE >> 1)
void gameloop(void);
void pregame(void);

extern const unsigned char monster_y[];

extern void draw_monster(eMonster enemy, unsigned char di);
extern unsigned int wait_for_key(unsigned int keys);
extern void print_stat(const char * text, int value);
extern void character_creation(void);
extern int view_lord_british_castle(void);
extern void view_shop(void);
extern void blank_right_from_cursor(unsigned char delay);
extern void view_stats(_Bool refresh_only);

extern unsigned char animation_frame;
extern signed short all_sprites[MAX_SPRITE];
extern unsigned char sprites_no;
extern _Bool animation_refresh;
extern unsigned char previous_fps_seqment;
extern sState state;

extern unsigned char dungeon_obj[DUNGEON_MAX+1][DUNGEON_MAX+1];
extern unsigned char dungeon_mon[DUNGEON_MAX+1][DUNGEON_MAX+1];
extern unsigned char terrain[TERRAIN_MAX-1][TERRAIN_MAX/2];


extern const char PRESS_RETURN_TO[];

extern const char _FOOD[];
extern const char _GOLD[];
extern const char _HP[];
extern const char PLAYER_NAME[];

#endif
