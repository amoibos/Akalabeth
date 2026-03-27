#include <stddef.h>
#include "engine.h"
#include "PSGlib/PSGlib.h"
#include "global.h"
#include "libs/console.h"
#include "libs/strings.h"
#include "monster.h"
#include "widgets.h"
#include "engine2.h"
#include "bpe.h"
#include "assets/bpe_texts.h"

/* ---------- current music tracking -------------------------------------- */

extern void *PSGMusicStart;

static void play_song(const void *song) {
    if (PSGMusicStart != song
        || PSGGetStatus() != PSG_PLAYING)
    {
        PSGPlay((void *)song);
    }
}

/* ---------- string constants -------------------------------------------- */

static const char STR_HUH[] = "HUH?\n";  /* too short to BPE-compress */

/* ---------- dungeon ambient animations ---------------------------------- */


/* Spider: 3 frames × 8 bytes — neutral, legs-up, legs-down */
static const unsigned char s_spider_pattern[SPIDER_FRAMES * 8] = {
    /* frame 0: neutral — legs level */
    0x42, 0xE7, 0x3C, 0x3C, 0xE7, 0x42, 0x00, 0x00,
    /* frame 1: legs angled up — spider pulling itself up */
    0x81, 0x42, 0x3C, 0x3C, 0x24, 0x18, 0x00, 0x00,
    /* frame 2: legs angled down — spider extending downward */
    0x18, 0x24, 0x3C, 0x3C, 0x42, 0x81, 0x00, 0x00,
};
/* Thread: single center pixel per row */
static const unsigned char s_thread_pattern[8] = {
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10
};

static struct {
    unsigned char side;
    unsigned char spider_y;
    unsigned char spider_frame;
    unsigned char spider_tick;
    _Bool spider_going_down;
    _Bool active;
} s_dungeon_anim;

static _Bool is_ladder_in_view(void) {
    for (unsigned char di = 0; di <= DUNGEON_MAX; ++di) {
        unsigned char cell = dungeon_obj[dunloc.x + movdir.x * di][dunloc.y + movdir.y * di];
        if (cell == oLADDER_DOWN || cell == oLADDER_UP)
            return 1;
        if (cell == oWALL || cell == oSECRET_DOOR || cell == oDOOR)
            break;
    }
    return 0;
}

static void anim_dungeon_callback(void) {
    unsigned char x = s_dungeon_anim.side ? ANIM_X_RIGHT : ANIM_X_LEFT;
    initSprites();
    if (!s_dungeon_anim.active) {
        finalizeSprites();
        copySpritestoSAT();
        return;
    }
    if (++s_dungeon_anim.spider_tick >= SPIDER_ANIM_DIVIDER) {
        s_dungeon_anim.spider_tick = 0;
        if (++s_dungeon_anim.spider_frame >= SPIDER_FRAMES)
            s_dungeon_anim.spider_frame = 0;
        if (s_dungeon_anim.spider_going_down) {
            s_dungeon_anim.spider_y += SPIDER_STEP;
            if (s_dungeon_anim.spider_y >= SPIDER_END_Y)
                s_dungeon_anim.spider_going_down = 0;
        } else {
            if (s_dungeon_anim.spider_y > SPIDER_STEP)
                s_dungeon_anim.spider_y -= SPIDER_STEP;
            else {
                /* reached top again — done */
                s_dungeon_anim.active = 0;
            }
        }
    }
    /* thread: stack 8-px segments from ceiling down to spider */
    for (unsigned char ty = 0; ty < s_dungeon_anim.spider_y; ty += 8)
        addSprite(x, ty, THREAD_SLOT, SG_COLOR_WHITE);
    addSprite(x, s_dungeon_anim.spider_y, SPIDER_SLOT + s_dungeon_anim.spider_frame, SG_COLOR_WHITE);
    finalizeSprites();
    copySpritestoSAT();
}

/* ----------------------------------------------------------------------- */

static void print_labeled_int(const char *label, int val) {
    char buf[24];
    char num[7];
    strcpy(buf, label);
    strcat(buf, SEGA_itoa(val, num));
    print(buf);
}

static void bpe_labeled_int(const unsigned char *bpe_label, int val) {
    char num[7];
    bpe_print(bpe_label);
    print(SEGA_itoa(val, num));
}

void generate_environment(void) {
    randomize((unsigned int)player.lucky_number * 6361u + 1u);
    // border is implicit tMOUNTAINS via terrain_get — no initialization needed
    for (unsigned char x = 1; x <= TERRAIN_MAX - 1; ++x) {
        for (unsigned char y = 1; y <= TERRAIN_MAX - 1; ++y) {
            unsigned char r = rnd8();

            // INT(RND^5*4.5) approximiert durch Schwellenwerte
            unsigned char tv;
            if (r < TERRAIN_THRESH_EMPTY)
                tv = tEMPTY;
            else if (r < TERRAIN_THRESH_MOUNTAINS)
                tv = tMOUNTAINS;
            else if (r < TERRAIN_THRESH_TREES)
                tv = tTREES;
            else if (r < TERRAIN_THRESH_TOWN)
                tv = tTOWN;
            else
                tv = tDUNGEON;
            // 50% Chance: Town zu Empty (BASIC Zeile 41)
            if ((tv == tTOWN) && rnd_bool())
                tv = tEMPTY;
            terrain_set(x, y, tv);
        }
    }

    terrain_set(rnd_range(19) + 1, rnd_range(19) + 1, tCASTLE);
    terrloc.x = rnd_range(19) + 1;
    terrloc.y = rnd_range(19) + 1;
    terrain_set(terrloc.x, terrloc.y, tTOWN);
}


void show_death_screen(void) {
    unsigned char output[TEXTCONSOLE_MAX_X + 1];
    unsigned char line=8;
    unsigned char old_color = state.fgcolor;
    _Bool old_fullscreen = state.fullscreen;
    state.fgcolor = SG_COLOR_MEDIUM_RED;
    state.fullscreen = 1;

    blankArea(0, 7, TEXTCONSOLE_MAX_X - 1, 16);
    strcpy(output, "WE MOURN THE PASSING OF");
    locateyx(line++, CENTER(output));
    print(output);
    strcpy(output, PLAYER_NAME);
    strcat(output, " AND HIS COMPUTER");
    locateyx(line++ , CENTER(output));
    print(output);

    strcpy(output,"TO INVOKE A MIRACLE");
    locateyx(line++ , CENTER(output));
    print(output);
    strcpy(output, "OF RESURRECTION\n");
    locateyx(line++ , CENTER(output));
    print(output);

    ++line;
    char num[6];
    unsigned char sec = (unsigned char)(seconds % 60);
    strcpy(output, "TIME: ");
    strcat(output, SEGA_itoa(seconds / 60, num));
    strcat(output, ":");
    if (sec < 10)
        strcat(output, "0");
    strcat(output, SEGA_itoa(sec, num));
    locateyx(line++, CENTER(output));
     state.fgcolor = SG_COLOR_WHITE;
    print(output);
    
    strcpy(output, "RESURRECTION");
    locateyx(++line, CENTER(output));
    print(output);
    locateyx(-1, CENTER(output) - 2);
    menu(1, 1, VERTICAL, HIGHLIGHT_OFF, 0, 0);
    state.fullscreen = old_fullscreen;
    state.fgcolor = old_color;
}

void draw_mountain(unsigned char x1, unsigned char y1) {
    line(x1 + 10, y1 + 50, x1 + 10, y1 + 40);
    line_end(x1 + 20, y1 + 30);
    line_end(x1 + 40, y1 + 30);
    line_end(x1 + 40, y1 + 50);
    line(x1, y1 + 10, x1 + 10, y1 + 10);
    line(x1 + 50, y1 + 10, x1 + 40, y1 + 10);
    line(x1, y1 + 40, x1 + 10, y1 + 40);
    line(x1 + 40, y1 + 40, x1 + 50, y1 + 40);
    line(x1 + 10, y1, x1 + 10, y1 + 20);
    line_end(x1 + 20, y1 + 20);
    line_end(x1 + 20, y1 + 30);
    line_end(x1 + 30, y1 + 30);
    line_end(x1 + 30, y1 + 10);
    line_end(x1 + 40, y1 + 10);
    line_end(x1 + 40, y1);
}

void draw_trees(unsigned char x1, unsigned char y1) {
    line(x1 + 20, y1 + 20, x1 + 30, y1 + 20);
    line_end(x1 + 30, y1 + 30);
    line_end(x1 + 20, y1 + 30);
    line_end(x1 + 20, y1 + 20);
}

void draw_town(unsigned char x1, unsigned char y1) {
    line(x1 + 10, y1 + 10, x1 + 20, y1 + 10);
    line_end(x1 + 20, y1 + 40);
    line_end(x1 + 10, y1 + 40);
    line_end(x1 + 10, y1 + 30);
    line_end(x1 + 40, y1 + 30);
    line_end(x1 + 40, y1 + 40);
    line_end(x1 + 30, y1 + 40);
    line_end(x1 + 30, y1 + 10);
    line_end(x1 + 40, y1 + 10);
    line_end(x1 + 40, y1 + 20);
    line_end(x1 + 10, y1 + 20);
    line_end(x1 + 10, y1 + 10);
}

void draw_dungeon_entry(unsigned char x1, unsigned char y1) {
    line(x1 + 20, y1 + 20, x1 + 30, y1 + 30);
    line(x1 + 20, y1 + 30, x1 + 30, y1 + 20);
}

void draw_castle(unsigned char x1, unsigned char y1) {
    line(x1, y1, x1 + 50, y1);
    line_end(x1 + 50, y1 + 50);
    line_end(x1, y1 + 50);
    line_end(x1, y1);
    line(x1 + 10, y1 + 10, x1 + 10, y1 + 40);
    line_end(x1 + 40, y1 + 40);
    line_end(x1 + 40, y1 + 10);
    line_end(x1 + 10, y1 + 10);
    line_end(x1 + 40, y1 + 40);
    line(x1 + 10, y1 + 40, x1 + 40, y1 + 10);
}

void draw_player(void) {
    unsigned char old_color = state.fgcolor;
    state.fgcolor = SG_COLOR_LIGHT_YELLOW;
    line(138, 75, 142, 75);
    line(140, 73, 140, 77);
    state.fgcolor = old_color;
}

/* Pixel extent of the 3x3 terrain grid:
   x: 65..215 px  → tile cols  8..26  (19 cols)
   y:  0..150 px  → tile rows  0..18  (19 rows) */
#define OVERWORLD_TILE_X1   8
#define OVERWORLD_TILE_X2   26
#define OVERWORLD_TILE_Y2   18

void draw_overworld(void) {
    displayOff();
    if (player.current_screen == ScreenOverworld) {
        /* Overworld-to-overworld move: only the 3x3 terrain area changed.
           Cols 0-7 and 27-31 are untouched; rows 19-23 are managed by the gameloop. */
        blankArea(OVERWORLD_TILE_X1, 0, OVERWORLD_TILE_X2, OVERWORLD_TILE_Y2);
    } else {
        /* Entering overworld from another screen (dungeon, menus).
           Clear full width through row 19 to remove any leftover graphics/text. */
        blankArea(0, 0, TEXTCONSOLE_MAX_X - 1, OVERWORLD_TILE_Y2 + 1);
        state.textconsole.x = state.textconsole.y = 0;
        state.screen.x = state.screen.y = 0;
        s_dungeon_anim.active = 0;
        SetTimerCallback(0);
        initSprites();
        finalizeSprites();
        copySpritestoSAT();
        play_song(overworld_psg);
        timer_enabled = 1;
        audio_enabled = 1;
    }
    displayOn();
    player.current_screen = ScreenOverworld;
    draw_player();

    unsigned char old_color = state.fgcolor;
    for (signed char y = -1; y <= 1; ++y) {
        unsigned char y1 = (unsigned char)(y + 1) * 50;
        for (signed char x = -1; x <= 1; ++x) {
            unsigned char x1 = 65 + (unsigned char)(x + 1) * 50;
            switch (terrain_get(terrloc.x + x, terrloc.y + y)) {
                case tMOUNTAINS:
                    state.fgcolor = SG_COLOR_GRAY;
                    draw_mountain(x1, y1);
                    break;
                case tTREES:
                    state.fgcolor = SG_COLOR_MEDIUM_GREEN;
                    draw_trees(x1, y1);
                    break;
                case tTOWN:
                    state.fgcolor = SG_COLOR_WHITE;
                    draw_town(x1, y1);
                    break;
                case tDUNGEON:
                    state.fgcolor = SG_COLOR_MEDIUM_RED;
                    draw_dungeon_entry(x1, y1);
                    break;
                case tCASTLE:
                    state.fgcolor = old_color;
                    draw_castle(x1, y1);
                    break;
            }
        }
    }
    state.fgcolor = old_color;
}

void create_monster(void) {
    //nm removed, not used
    for(unsigned char x=1; x < MAX_MONSTERS; ++x) {
        monster[x].alive = 0;
        monster[x].hp = x + 3 + player.floor;
        if ((x - 2 > player.floor) || rnd_pct(60))
            break;
        _Bool again=1;
        while(again) {
            monster[x].location.x = (unsigned char)(rnd_range(9) + 1);
            monster[x].location.y = (unsigned char)(rnd_range(9) + 1);
            again = (
                        (dungeon_obj[monster[x].location.x][monster[x].location.y] != 0) ||
                        (dungeon_mon[monster[x].location.x][monster[x].location.y] != 0) ||
                        (monster[x].location.x == dunloc.x) && (monster[x].location.y == dunloc.y)
                    );
        }
        dungeon_mon[monster[x].location.x][monster[x].location.y] = x;
        dungeon_obj[monster[x].location.x][monster[x].location.y] = oEMPTY;
        monster[x].alive = 1;
        monster[x].hp = (x << 1) + (player.floor << 1) * player.level_of_play;
    }
}

void draw_front_wall(unsigned char di) {
    unsigned char l1 = monster_pos[di][0], r1 = monster_pos[di][1];
    unsigned char t1 = monster_pos[di][2], b1 = monster_pos[di][3];
    line(l1, t1, r1, t1);
    line_end(r1, b1);
    line_end(l1, b1);
    line_end(l1, t1);
}

void draw_front_door(unsigned char di) {
    line(font_door[di][0], font_door[di][3], font_door[di][0], font_door[di][2]);
    line_end(font_door[di][1], font_door[di][2]);
    line_end(font_door[di][1], font_door[di][3]);
}

void draw_side_walls(unsigned char di, unsigned char left, unsigned char right) {
    unsigned char l1 = monster_pos[di][0],   r1 = monster_pos[di][1];
    unsigned char t1 = monster_pos[di][2],   b1 = monster_pos[di][3];
    unsigned char l2 = monster_pos[di+1][0], r2 = monster_pos[di+1][1];
    unsigned char t2 = monster_pos[di+1][2], b2 = monster_pos[di+1][3];
    if ((left == oWALL) || (left == oSECRET_DOOR) || (left == oDOOR)) {
        line(l1, t1, l2, t2);
        line(l1, b1, l2, b2);
    }
    if ((right == oWALL) || (right == oSECRET_DOOR) || (right == oDOOR)) {
        // at di=0, the right perspective line intersects x=255 at y≈13 (top) and y≈145 (bottom)
        // extrapolated from monster_pos[1] and [2]; t1=0/b1=159 are correct only for the left wall
        line(r1, (di == 0) ? 13 : t1, r2, t2);
        line(r1, (di == 0) ? 145 : b1, r2, b2);
    }
}

void draw_side_doors(unsigned char di, unsigned char left, unsigned char right) {
    if (left == oDOOR) {
        if (di > 0) {
            line(side_door[di][0], side_door[di][4], side_door[di][0], side_door[di][2]);
            line_end(side_door[di][1], side_door[di][3]);
            line_end(side_door[di][1], side_door[di][5]);
        } else {
            line(0, side_door[di][2] - 3, side_door[di][1], side_door[di][3]);
            line_end(side_door[di][1], side_door[di][5]);
        }
    }
    if (right == oDOOR) {
        // di=0: mirror=278 (inner edge 278-37=241, within right-side zone 222..255)
        // mirror=255 was wrong: gave x=218, left of side wall r2=222
        if (di > 0) {
            line(DOOR_MIRROR_AXIS - side_door[di][0], side_door[di][4], DOOR_MIRROR_AXIS - side_door[di][0], side_door[di][2]);
            line_end(DOOR_MIRROR_AXIS - side_door[di][1], side_door[di][3]);
            line_end(DOOR_MIRROR_AXIS - side_door[di][1], side_door[di][5]);
        } else {
            line(255, side_door[di][2] - 3, DOOR_MIRROR_AXIS - side_door[di][1], side_door[di][3]);
            line_end(DOOR_MIRROR_AXIS - side_door[di][1], side_door[di][5]);
        }
    }
}

void draw_open_passages(unsigned char di, unsigned char left, unsigned char right) {
    unsigned char l1 = monster_pos[di][0],   r1 = monster_pos[di][1];
    unsigned char t1 = monster_pos[di][2],   b1 = monster_pos[di][3];
    unsigned char l2 = monster_pos[di+1][0], r2 = monster_pos[di+1][1];
    unsigned char t2 = monster_pos[di+1][2], b2 = monster_pos[di+1][3];
    if (di == 0) { l1 = 8; r1 = 247; }
    if (!((left == oSECRET_DOOR) || (left == oWALL) || (left == oDOOR))) {
        if (di != 0) 
            line(l1, t1, l1, b1);
        line(l1, t2, l2, t2);
        line_end(l2, b2);
        line_end(l1, b2);
    }
    if (!((right == oSECRET_DOOR) || (right == oWALL) || (right == oDOOR))) {
        if (di != 0) 
            line(r1, t1, r1, b1);
        line(r1, t2, r2, t2);
        line_end(r2, b2);
        line_end(r1, b2);
    }
}

void draw_floor_ceiling_features(unsigned char di, unsigned char center) {
    if ((center == oLADDER_DOWN) || (center == oPIT)) {
        line(floor_hole[di][0], floor_hole[di][4], floor_hole[di][2], floor_hole[di][5]);
        line_end(floor_hole[di][3], floor_hole[di][5]);
        line_end(floor_hole[di][1], floor_hole[di][4]);
        line_end(floor_hole[di][0], floor_hole[di][4]);
    }
    if (center == oLADDER_UP) {
        line(floor_hole[di][0], 158 - floor_hole[di][4], floor_hole[di][2], 158 - floor_hole[di][5]);
        line_end(floor_hole[di][3], 158 - floor_hole[di][5]);
        line_end(floor_hole[di][1], 158 - floor_hole[di][4]);
        line_end(floor_hole[di][0], 158 - floor_hole[di][4]);
    }
    if ((center == oLADDER_DOWN) || (center == oLADDER_UP)) {
        line(ladder[di][0], ladder[di][3], ladder[di][0], ladder[di][2]);
        line(ladder[di][1], ladder[di][2], ladder[di][1], ladder[di][3]);
        unsigned char y1 = (ladder[di][3] * 4 + ladder[di][2]) / 5;
        unsigned char y2 = (ladder[di][3] * 3 + ladder[di][2] * 2) / 5;
        unsigned char y3 = (ladder[di][3] * 2 + ladder[di][2] * 3) / 5;
        unsigned char y4 = (ladder[di][3] + ladder[di][2] * 4) / 5;
        line(ladder[di][0], y1, ladder[di][1], y1);
        line(ladder[di][0], y2, ladder[di][1], y2);
        line(ladder[di][0], y3, ladder[di][1], y3);
        line(ladder[di][0], y4, ladder[di][1], y4);
    }
    if ((di > 0) && (center == oCHEST)) {
        draw_mimic(di);
        locateyx(19, 0);
        print("CHEST!\n");
    }
}

void draw_monster_at_depth(unsigned char curr_monster, unsigned char di) {
    if (curr_monster <= mUNDEFINED) {
        blankArea(0, 19, TEXTCONSOLE_MAX_X - 1, 19);
        return;
    }
    draw_center_y = 79 + monster_y[di];
    locateyx(19, 0);
    if (curr_monster == mMIMIC)
        print("CHEST!");
    else
        print(MONSTERS[curr_monster]);
    blank_right_from_cursor(NO_DELAY);
    print("\n");
    if (di != 0)
        draw_monster((eMonster)curr_monster, di);
}

static void draw_compass(void) {
    /* single direction letter centered on row 0 (col 15 of 32) */
    unsigned char old_color = state.fgcolor;
    char dir[2] = { 0, 0 };

    if (movdir.x == 1)       dir[0] = 'E';
    else if (movdir.x == -1) dir[0] = 'W';
    else if (movdir.y == -1) dir[0] = 'N';
    else                     dir[0] = 'S';

    state.fgcolor = SG_COLOR_LIGHT_YELLOW;
    locateyx(0, 15);
    print(dir);
    state.fgcolor = old_color;
}

void draw_dungeon(void) {
    if (player.current_screen != ScreenDungeon) {
        play_song(dungeon_psg);
        timer_enabled = 1;
        audio_enabled = 1;
    }
    player.current_screen = ScreenDungeon;
    static const unsigned char dungeon_colors[9] = {
        SG_COLOR_GRAY,        /* floor 1 */
        SG_COLOR_DARK_GREEN,  /* floor 2 */
        SG_COLOR_DARK_YELLOW, /* floor 3 */
        SG_COLOR_DARK_BLUE,   /* floor 4 */
        SG_COLOR_CYAN,        /* floor 5 */
        SG_COLOR_DARK_RED,    /* floor 6 */
        SG_COLOR_MAGENTA,     /* floor 7 */
        SG_COLOR_MEDIUM_RED,  /* floor 8 */
        SG_COLOR_BLACK,       /* floor 9+ */
    };
    unsigned char floor_idx = player.floor >= 9 ? 8 : player.floor - 1;
    setBackdropColor(dungeon_colors[floor_idx]);
    displayOff();
    blankArea(0, 0, TEXTCONSOLE_MAX_X - 1, 19);

    for (unsigned char di = 0; ; ++di) {
        unsigned char center = dungeon_obj[dunloc.x + movdir.x * di][dunloc.y + movdir.y * di];
        unsigned char left   = dungeon_obj[dunloc.x + movdir.x * di + movdir.y][dunloc.y + movdir.y * di - movdir.x];
        unsigned char right  = dungeon_obj[dunloc.x + movdir.x * di - movdir.y][dunloc.y + movdir.y * di + movdir.x];
        unsigned char curr_monster = dungeon_mon[dunloc.x + movdir.x * di][dunloc.y + movdir.y * di];

        _Bool stop = 0;
        if (di != 0) {
            if ((center == oWALL) || (center == oSECRET_DOOR) || (center == oDOOR)) {
                draw_front_wall(di);
                if (center == oDOOR)
                    draw_front_door(di);
                stop = 1;
            }
        }

        if (!stop) {
            draw_side_walls(di, left, right);
            draw_side_doors(di, left, right);
            draw_open_passages(di, left, right);
            draw_floor_ceiling_features(di, center);
        }

        draw_monster_at_depth(curr_monster, di);

        if (stop)
            break;
    }

    /* load sprite patterns for ambient animations */
    SG_loadSpritePatterns((void *)s_spider_pattern, SPIDER_SLOT, SPIDER_FRAMES * 8);
    SG_loadSpritePatterns((void *)s_thread_pattern, THREAD_SLOT, 8);

    if (player.floor == FIRST_DUNGEON_LEVEL && is_ladder_in_view()) {
        s_dungeon_anim.active            = 1;
        s_dungeon_anim.side              = rnd_bool();
        s_dungeon_anim.spider_y          = SPIDER_START_Y;
        s_dungeon_anim.spider_going_down = 1;
        SetTimerCallback(anim_dungeon_callback);
    } else {
        s_dungeon_anim.active = 0;
        SetTimerCallback(0);
        initSprites();
        finalizeSprites();
        copySpritestoSAT();
    }

    draw_compass();
    displayOn();
}

void fill_dungeon(void) {
    randomize(player.lucky_number - terrloc.x * 10 - terrloc.y * 1000 + player.floor * 31);

    for (unsigned char x=1; x < DUNGEON_MAX; ++x)
        for (unsigned char y=1; y <= 9; ++y) {
            dungeon_obj[x][y] = oEMPTY;
            dungeon_mon[x][y] = 0;
        }

    for (unsigned char x=0; x <= DUNGEON_MAX; ++x) {
        dungeon_obj[x][DUNGEON_MIN] = dungeon_obj[x][DUNGEON_MAX] =
            dungeon_obj[DUNGEON_MIN][x] = dungeon_obj[DUNGEON_MAX][x] = oWALL;
        dungeon_mon[x][DUNGEON_MIN] = dungeon_mon[x][DUNGEON_MAX] =
            dungeon_mon[DUNGEON_MIN][x] = dungeon_mon[DUNGEON_MAX][x] = 0;
    }

    for (unsigned char x=2; x <= 8; x+=2)
        for (unsigned char y=1; y <= 9; ++y) {
            dungeon_obj[x][y] = oWALL;
            dungeon_mon[x][y] = 0;
        }

    for (unsigned char x=2; x <= 8; x+=2)
        for (unsigned char y=1; y < DUNGEON_MAX; y+=2) {
            if (rnd_pct(5))
                dungeon_obj[x][y] = oTRAP;
            if (rnd_pct(5))
                dungeon_obj[y][x] = oTRAP;
            if (rnd_pct(40))
                dungeon_obj[y][x] = oSECRET_DOOR;
            if (rnd_pct(40))
                dungeon_obj[x][y] = oSECRET_DOOR;
            if (rnd_pct(40))
                dungeon_obj[x][y] = oDOOR;
            if (rnd_pct(40))
                dungeon_obj[y][x] = oDOOR;
            if (rnd_pct(3))
                dungeon_obj[y][x] = oPIT;
            if (rnd_pct(3))
                dungeon_obj[x][y] = oPIT;
            if (rnd_pct(6))
                dungeon_obj[x][y] = oCHEST;
            if (rnd_pct(6))
                dungeon_obj[y][x] = oCHEST;
        }

    dungeon_obj[2][1] = oEMPTY;
    dungeon_obj[7][3] = (player.floor % 2 == 0) ? oLADDER_DOWN  : oLADDER_UP;
    dungeon_obj[3][7] = (player.floor % 2 == 0) ? oLADDER_UP    : oLADDER_DOWN;

    if (player.floor == FIRST_DUNGEON_LEVEL) {
        dungeon_obj[1][1] = oLADDER_UP;
        dungeon_obj[7][3] = oEMPTY;
    }

    create_monster();
}

void fight_monster(void) {
    for(unsigned char mm=1; mm < MAX_MONSTERS; ++mm) {
        if (!monster[mm].alive)
            continue;
        int dist_sq = sqr(dunloc.x - monster[mm].location.x) + sqr(dunloc.y - monster[mm].location.y);
        _Bool is_hostile = monster[mm].hp < player.floor * player.level_of_play;
        _Bool should_attack = 0;

        if (!is_hostile) {
            if (dist_sq <= 1)
                should_attack = 1;
            else if ((mm == 8) && (dist_sq < 9))
                continue;
        }

        if (!should_attack) {
            signed char x1 = sgn(dunloc.x - monster[mm].location.x);
            signed char y1 = sgn(dunloc.y - monster[mm].location.y);

            if (is_hostile) 
                x1 = -x1;
            y1 = -y1;

            // Versuche Y-Bewegung, sonst X-Bewegung
            _Bool moved = 0;
            if (y1 != 0) {
                unsigned char d = dungeon_obj[monster[mm].location.x][monster[mm].location.y + y1];
                if (!((d == oWALL) || dungeon_mon[monster[mm].location.x][monster[mm].location.y + y1] || (d == oTRAP))) {
                    x1 = 0;
                    moved = 1;
                }
            }
            if (!moved) {
                y1 = 0;
                if (x1 != 0) {
                    unsigned char d = dungeon_obj[monster[mm].location.x + x1][monster[mm].location.y];
                    if (!((d == oWALL) || dungeon_mon[monster[mm].location.x + x1][monster[mm].location.y] || (d == oTRAP)))
                        moved = 1;
                    else
                        x1 = 0;
                }
            }

            if (moved) {
                dungeon_mon[monster[mm].location.x][monster[mm].location.y] = 0;
                if ((monster[mm].location.x + x1 == dunloc.x) && (monster[mm].location.y + y1 == dunloc.y))
                    continue;
                monster[mm].location.x += x1;
                monster[mm].location.y += y1;
                dungeon_mon[monster[mm].location.x][monster[mm].location.y] = mm;
                dungeon_obj[monster[mm].location.x][monster[mm].location.y] = oEMPTY;
                continue;
            }

            if (is_hostile && dist_sq <= 1)
                should_attack = 1;
            else if (is_hostile)
                monster[mm].hp += mm + player.floor;
        }

        if (!should_attack) 
            continue;

        // 50% probality to steal if monster is a thief or a gremlin and player has something to steal
        _Bool do_steal = ((mm == mTHIEF) || (mm == mGREMLIN)) && !rnd_bool();
        if (!do_steal) {
            locateyx(20, 0);
            bpe_print(BPE_BEING_ATTACKED);
            print("BY A ");
            print(MONSTERS[mm]);
            if ((rnd_range(20)) - sgn(player.inventory[iSHIELD]) - player.stats[sSTAMINA] + mm + player.floor >= OVERWORLD_LEVEL) {
                PSGSFXPlay((void*)hit_psg, SFX_CHANNEL2);
                print("HIT");
                player.stats[sHIT_POINTS] -= (rnd_range(mm) + player.floor);
            } else {
                print("MISSED");
            }
        } else if (mm == mGREMLIN) {
            player.inventory[iFOOD] /= 2;
            bpe_print(BPE_GREMLIN_FOOD);
        } else {
            unsigned char loot;
            while(1) {
                loot = rnd_range(MAX_ITEMS);
                if (player.inventory[loot] >= 1)
                    break;
            }
            bpe_print(BPE_THIEF_STOLE);
            print(ITEMS[loot]);
            --player.inventory[loot];
        }

        wait_half_second();
    }
}

void show_player_stat(void) {
    unsigned char old_color = state.fgcolor;

    locateyx(21, 22);
    if (player.inventory[iFOOD] < 10)
        state.fgcolor = SG_COLOR_MAGENTA;
    print_labeled_int(_FOOD, player.inventory[iFOOD]);
    state.fgcolor = old_color;
    blank_right_from_cursor(NO_DELAY);

    locateyx(22, 22);
    print_labeled_int(_HP, player.stats[sHIT_POINTS]);
    blank_right_from_cursor(NO_DELAY);

    locateyx(23, 22);
    print_labeled_int(_GOLD, player.stats[sGOLD]);
    blank_right_from_cursor(NO_DELAY);

    locateyx(20, 0);
}

_Bool player_use_item(eItem item) {
    _Bool ret = (player.inventory[item] > 0);
    if (!ret)
        bpe_print(BPE_NOT_OWNED);

    return ret;
}

void handle_forward(void) {
    if (player.floor != OVERWORLD_LEVEL) {
        if ((dungeon_obj[dunloc.x + movdir.x][dunloc.y + movdir.y] != oWALL) &&
            (dungeon_mon[dunloc.x + movdir.x][dunloc.y + movdir.y] == 0)) {
            dunloc.x += movdir.x;
            dunloc.y += movdir.y;
        }
        print("FORWARD\n");

        if (dungeon_obj[dunloc.x][dunloc.y] == oTRAP) {
            PSGSFXPlay((void*)trap_psg, SFX_CHANNEL2);
            bpe_print(BPE_TRAP);
            player.stats[sHIT_POINTS] -= (rnd_range(player.floor) + 3);
            ++player.floor;
            print_labeled_int("FALLING TO LEVEL ", player.floor);
            fill_dungeon();
            wait_half_second();
            blankArea(0, 20, TEXTCONSOLE_MAX_X - 1, TEXTCONSOLE_MAX_Y - 1);
            return;
        }
        unsigned char z = 0;
        if (dungeon_obj[dunloc.x][dunloc.y] == oCHEST) {
            dungeon_obj[dunloc.x][dunloc.y] = oEMPTY;
            bpe_print(BPE_GOLD);
            z = (rnd_range(5 * player.floor) + player.floor);
            char gold_buf[7];
            print(SEGA_itoa(z, gold_buf));
            bpe_print(BPE_PIECES_SUFFIX);
            player.stats[sGOLD] += z;
        }
        if (z > 0) {
            z = rnd_range(6);
            print("AND A ");
            print(ITEMS[z]);
            print("\n");
            ++player.inventory[z];
            wait_half_second();
        }
    } else {
        print("NORTH\n");
        if (terrain_get(terrloc.x, terrloc.y - 1) == tMOUNTAINS) {
            bpe_print(BPE_YOU_CANT_PASS);
            PSGSFXPlay((void*)blocked_psg, SFX_CHANNEL2);
        } else
            --terrloc.y;
    }
}

void handle_enter_exit(unsigned char *lk) {
    if (player.floor != OVERWORLD_LEVEL) {
        if ((dungeon_obj[dunloc.x][dunloc.y] == oLADDER_DOWN) || (dungeon_obj[dunloc.x][dunloc.y] == oPIT)) {
            ++player.floor;
            PSGSFXPlay((void*)fall_psg, SFX_CHANNEL2);
            print_labeled_int("GO DOWN TO LEVEL ", player.floor);
            print("\n");
            fill_dungeon();
        } else {
            if (dungeon_obj[dunloc.x][dunloc.y] != oLADDER_UP) {
                print(STR_HUH);
            } else {
                if (player.floor == FIRST_DUNGEON_LEVEL) {
                    --player.floor;
                    setBackdropColor(SG_COLOR_BLACK);
                    bpe_print(BPE_LEAVE_DUNGEON);
                } else {
                    --player.floor;
                    bpe_labeled_int(BPE_GO_UP_TO_LEVEL, player.floor);
                    print("\n");
                    fill_dungeon();
                }
                if (player.floor == OVERWORLD_LEVEL) {
                    char lk_num[11];
                    bpe_print(BPE_THOU_HAST_GAINED);
                    print(SEGA_itoa(*lk, lk_num));
                    bpe_print(BPE_HIT_POINTS);
                    player.stats[sHIT_POINTS] += *lk;
                    *lk = 0;
                }
            }
        }
    } else {
        if (terrain_get(terrloc.x, terrloc.y) == tTOWN) {
            player.current_screen = ScreenUndefined;
            view_shop();
            return;
        }
        if ((terrain_get(terrloc.x, terrloc.y) == tDUNGEON) && (player.floor == OVERWORLD_LEVEL)) {
            print(  "GO DUNGEON\n");
            player.floor = FIRST_DUNGEON_LEVEL;
            fill_dungeon();
            movdir.x = 1;
            movdir.y = 0;
            dunloc.x = 1;
            dunloc.y = 1;
            return;
        }

        if (terrain_get(terrloc.x, terrloc.y) == tCASTLE) {
            player.current_screen = ScreenUndefined;
            if (view_lord_british_castle() == 1090)
                return;
        }

        print(STR_HUH);
    }
}

static unsigned char s_last_axe_sel = 0;

signed char find_ranged_target(unsigned char attack_value) {
    signed char mn = 0;
    _Bool skip_scan = 0;
    if (attack_value == WEAPONS[iAXE]) {
        blankArea(0, 15, TEXTCONSOLE_MAX_X - 1, 17);
        locateyx(15, 0);
        print("AXE:");
        locateyx(16, 2); print("SWING");
        locateyx(17, 2); print("THROW");
        locateyx(16, 0);
        {
            unsigned char axe_sel = menu(2, 1, VERTICAL, HIGHLIGHT_OFF, 0, s_last_axe_sel);
            s_last_axe_sel = axe_sel;
            if (axe_sel == 0) {
                mn = dungeon_mon[dunloc.x + movdir.x][dunloc.y + movdir.y];
                skip_scan = 1;
            } else {
                print("THROW\n");
                --player.inventory[iAXE];
            }
        }
        blankArea(0, 15, TEXTCONSOLE_MAX_X - 1, 17);
    }
    if (!skip_scan) {
        for (unsigned char y = 1; y <= 5; ++y) {
            if ((dunloc.x + movdir.x * y < 1) || (dunloc.x + movdir.x * y > 9) || (dunloc.y + movdir.y * y > 9) || (dunloc.y + movdir.y * y < 0))
                break;
            mn = dungeon_mon[dunloc.x + movdir.x * y][dunloc.y + movdir.y * y];
            if (mn > 0)
                break;
        }
    }
    return mn;
}

void resolve_combat(signed char mn, unsigned char attack_value, unsigned char *lk) {
    if ((mn < 1) || (player.stats[sDEXTERITY] - rnd_range(25) < mn + player.floor)) {
        print("YOU MISSED\n");
    } else {
        char num[7];
        print("HIT!!! \n");
        attack_value = (rnd_range(attack_value) + player.stats[sSTRENGTH] / 5);

        monster[mn].hp -= monster[mn].hp < attack_value ? monster[mn].hp : attack_value;

        print(MONSTERS[mn]);
        bpe_print(BPE_MONSTER_HP);
        print(SEGA_itoa(monster[mn].hp, num));
        print("\n");
        if (monster[mn].hp == 0) {
            bpe_print(BPE_THOU_KILLED);
            print(MONSTERS[mn]);
            bpe_print(BPE_THOU_RECEIVE);
            attack_value = mn + player.floor;
            print(SEGA_itoa(attack_value, num));
            bpe_print(BPE_PIECES);
            player.stats[sGOLD] += attack_value;
            dungeon_mon[monster[mn].location.x][monster[mn].location.y] = 0;
            monster[mn].alive = 0;
        }
        *lk += (int)(mn * (player.floor >> 1));
        if ((eMonster)mn == quest_monster)
            quest_done = 1;
    }
}

static unsigned char s_last_weapon_sel = 0xFF;

_Bool handle_attack(unsigned char *lk) {
    signed char mn = 0;
    unsigned char attack_value = 0;
    unsigned char weapon_item;
    unsigned char sel;
    _Bool retry;
    _Bool do_combat = 1;
    if (s_last_weapon_sel == 0xFF) {
        s_last_weapon_sel = 0;
        for (unsigned char wi = iRAPIER; wi <= iAMULET; ++wi) {
            if (player.inventory[wi] > 0) {
                s_last_weapon_sel = wi - iRAPIER + 1;
                break;
            }
        }
    }
    do {
        retry = 0;
        print("ATTACK\n");
        blankArea(0, 15, TEXTCONSOLE_MAX_X - 1, 21);
        locateyx(15, 0);
        bpe_print(BPE_CHOOSE_WEAPON);
        locateyx(16, 2);
        print("HANDS");
        for (unsigned char wi = iRAPIER; wi <= iAMULET; ++wi) {
            locateyx(17 + (wi - iRAPIER), 2);
            print(ITEMS[wi]);
        }
        locateyx(16, 0);
        sel = menu(iAMULET - iRAPIER + 2, 1, VERTICAL, HIGHLIGHT_OFF, 0, s_last_weapon_sel);
        weapon_item = (sel == 0) ? iFOOD : (sel - 1 + iRAPIER);
        blankArea(0, 15, TEXTCONSOLE_MAX_X - 1, 21);

        if (weapon_item == iFOOD) {
            /* HANDS — no weapon, attack_value stays 0 */
            print("HANDS\n");
        } else switch (weapon_item) {
            case iRAPIER:
                if (player.playerclass == pcMAGE) {
                    bpe_print(BPE_NO_RAPIER);
                    retry = 1; break;
                }
                attack_value = WEAPONS[iRAPIER];
                print(ITEMS[iRAPIER]); print("\n");
                if (!player_use_item(iRAPIER))
                    retry = 1;
                break;
            case iAXE:
                attack_value = WEAPONS[iAXE];
                print(ITEMS[iAXE]);
                print("\n");
                if (!player_use_item(iAXE))
                    retry = 1;
                break;
            case iSHIELD:
                attack_value = WEAPONS[iSHIELD];
                print(ITEMS[iSHIELD]);
                print("\n");
                if (!player_use_item(iSHIELD))
                    retry = 1;
                break;
            case iBOW_AND_ARROWS:
                if (player.playerclass == pcMAGE) {
                    bpe_print(BPE_NO_BOW);
                    retry = 1; break;
                }
                attack_value = WEAPONS[iBOW_AND_ARROWS];
                print(ITEMS[iBOW_AND_ARROWS]);
                print("\n");
                if (!player_use_item(iBOW_AND_ARROWS))
                    retry = 1;
                break;
            case iAMULET: {
                unsigned int amulet_key;
                print(ITEMS[iAMULET]); print("\n");
                if (!player_use_item(iAMULET)) {
                    retry = 1;
                    break;
                }
                if (player.playerclass == pcFIGHTER) {
                    amulet_key = rnd_range(4) + 1 + '0';
                } else {
                    blankArea(0, 15, TEXTCONSOLE_MAX_X - 1, 21);
                    locateyx(15, 0);
                    bpe_print(BPE_AMULET_CHOOSE);
                    locateyx(16, 2); print("1-LADDER-UP");
                    locateyx(17, 2); print("2-LADDER-DN");
                    locateyx(18, 2); print("3-KILL");
                    locateyx(19, 2); print("4-BAD??");
                    locateyx(16, 0);
                    amulet_key = menu(4, 1, VERTICAL, HIGHLIGHT_OFF, 0, 0) + '1';
                    blankArea(0, 15, TEXTCONSOLE_MAX_X - 1, 21);
                    if (rnd_pct(25)) {
                        bpe_print(BPE_LAST_CHARGE);
                        --player.inventory[iAMULET];
                    }
                }
                do_combat = 0;
                switch (amulet_key) {
                    case '1':
                        print("LADDER UP\n");
                        dungeon_obj[dunloc.x][dunloc.y] = oLADDER_UP;
                        break;
                    case '2':
                        print("LADDER DOWN\n");
                        dungeon_obj[dunloc.x][dunloc.y] = oLADDER_DOWN;
                        break;
                    case '3':
                        bpe_print(BPE_MAGIC_ATTACK);
                        attack_value = 10 + player.floor;
                        do_combat = 1;
                        break;
                    case '4':
                        switch (rnd_range(3) + 1) {
                            case 1:
                                bpe_print(BPE_BEEN_TURNED);
                                bpe_print(BPE_INTO_TOAD);
                                for (unsigned char z2 = sSTRENGTH; z2 <= sWISDOM; ++z2)
                                    player.stats[z2] = 3;
                                break;
                            case 2:
                                bpe_print(BPE_BEEN_TURNED);
                                bpe_print(BPE_INTO_LIZARD);
                                for (unsigned char y = sHIT_POINTS; y <= sWISDOM; ++y)
                                    player.stats[y] = player.stats[y] * 5 / 2;
                                break;
                            case 3:
                                print("BACKFIRE\n");
                                player.stats[sHIT_POINTS] /= 2;
                                break;
                        }
                        break;
                }
                break;
            }
        }
    } while (retry);
    s_last_weapon_sel = sel;

    if (do_combat) {
        if ((attack_value == WEAPONS[iAXE]) || (attack_value == WEAPONS[iBOW_AND_ARROWS])
                || (attack_value > WEAPONS[iRAPIER]))
            mn = find_ranged_target(attack_value);
        else
            mn = dungeon_mon[dunloc.x + movdir.x][dunloc.y + movdir.y];
        resolve_combat(mn, attack_value, lk);
        wait_half_second();
        return 1;
    }
    return 0;
}

void post_turn(void) {
    if (player.floor == OVERWORLD_LEVEL) {
        player.inventory[iFOOD] -= 1;
    } else {
        static unsigned char dungeon_steps = 0;
        if (++dungeon_steps >= 10) {
            dungeon_steps = 0;
            player.inventory[iFOOD] -= 1;
        }
    }
    if (player.inventory[iFOOD] < 0) {
        player.stats[sHIT_POINTS] = 0;
        bpe_print(BPE_STARVED);
    } else
        show_player_stat();
    if (player.stats[sHIT_POINTS] <= 0) {
        show_death_screen();
        return;
    }
    if (player.floor == OVERWORLD_LEVEL)
        blankArea(0, 20, TEXTCONSOLE_MAX_X - 1, TEXTCONSOLE_MAX_Y - 1);
    fight_monster();
    if (player.stats[sHIT_POINTS] <= 0) {
        show_death_screen();
        return;
    }

    show_player_stat();
}

void gameloop(void) {
    unsigned char lk = 0;
    unsigned int key;

    player.floor = OVERWORLD_LEVEL;
    player.current_screen = ScreenUndefined;
    draw_overworld();

    for (;;) {
        while (1) {
            //wait(FRAME_RATE / 2);
            if (player.floor == OVERWORLD_LEVEL) {
                blankArea(0, 20, TEXTCONSOLE_MAX_X - 1,
                    TEXTCONSOLE_MAX_Y - 1);
                locateyx(21, 0);
                bpe_print(BPE_COMMAND_PROMPT);
                blank_right_from_cursor(NO_DELAY);
                show_player_stat();
                locateyx(21, 9);  /* strlen("COMMAND? ") */
            } else {
                show_player_stat();
                locateyx(SCROLL_ROW_END, 0);
                bpe_print(BPE_COMMAND_PROMPT);
            }

            key = wait_for_key(NO_SPECIFIC_KEY);
            if (!key) { //pressed pause key
                setBackdropColor(SG_COLOR_BLACK);
                state.fgcolor = SG_COLOR_CYAN;
                state.fullscreen = 1;
                blank_screen();
                view_stats(STATS_SHOW_ALL);
                locateyx(9, 0);
                state.fgcolor = SG_COLOR_MAGENTA;
                print("QUEST:");
                state.fgcolor = SG_COLOR_CYAN;
                if (!player.has_quest) {
                    bpe_print(BPE_VISIT_BRITISH);
                } else if (quest_monster != mUNDEFINED
                    && !quest_done)
                {
                    print(" SLAY A ");
                    print(MONSTERS[quest_monster]);
                } else {
                    bpe_print(BPE_RETURN_BRITISH);
                }
                blank_right_from_cursor(NO_DELAY);
                if (player.floor > OVERWORLD_LEVEL)
                    view_dungeon_map();
                SetTimerCallback(0);
                initSprites();
                finalizeSprites();
                copySpritestoSAT();
                reset_sprites();
                add_any_key_animation();
                unsigned char old_color = state.fgcolor;
                state.fgcolor = SG_COLOR_WHITE;
                while (!pressed_anything()) {
                    if (animation_refresh)
                        animate_quarterly(ScreenAppendix);
                    waitForVBlank();
                }
                state.fgcolor = old_color;
                state.fullscreen = 0;
                player.current_screen = ScreenUndefined;
                blankArea(0, 20, TEXTCONSOLE_MAX_X - 1,
                    TEXTCONSOLE_MAX_Y - 1);
                if (s_dungeon_anim.active)
                    SetTimerCallback(anim_dungeon_callback);
                break;
            } else if (key & PORT_A_KEY_UP) {
                handle_forward(); break;
            } else if (key & PORT_A_KEY_RIGHT) {
                if (player.floor == OVERWORLD_LEVEL) {
                    print("EAST\n");
                    if (terrain_get(terrloc.x + 1, terrloc.y)
                        == tMOUNTAINS)
                    {
                        bpe_print(BPE_YOU_CANT_PASS);
                        PSGSFXPlay((void*)blocked_psg,
                            SFX_CHANNEL2);
                    } else
                        ++terrloc.x;
                } else {
                    bpe_print(BPE_TURN_RIGHT);
                    if (movdir.x != 0) {
                        movdir.y = movdir.x;
                        movdir.x = 0;
                    } else {
                        movdir.x = -movdir.y;
                        movdir.y = 0;
                    }
                }
                break;
            } else if (key & PORT_A_KEY_LEFT) {
                if (player.floor == OVERWORLD_LEVEL) {
                    print("WEST\n");
                    if (terrain_get(terrloc.x - 1, terrloc.y)
                        == tMOUNTAINS)
                    {
                        bpe_print(BPE_YOU_CANT_PASS);
                        PSGSFXPlay((void*)blocked_psg,
                            SFX_CHANNEL2);
                    } else
                        --terrloc.x;
                } else {
                    bpe_print(BPE_TURN_LEFT);
                    if (movdir.x != 0) {
                        movdir.y = -movdir.x;
                        movdir.x = 0;
                    } else {
                        movdir.x = movdir.y;
                        movdir.y = 0;
                    }
                }
                break;
            } else if (key & PORT_A_KEY_DOWN) {
                if (player.floor == OVERWORLD_LEVEL) {
                    print("SOUTH\n");
                    if (terrain_get(terrloc.x, terrloc.y + 1)
                        == tMOUNTAINS)
                    {
                        PSGSFXPlay((void*)blocked_psg,
                            SFX_CHANNEL2);
                        bpe_print(BPE_YOU_CANT_PASS);
                    } else
                        ++terrloc.y;
                } else {
                    bpe_print(BPE_TURN_AROUND);
                    movdir.x = -movdir.x;
                    movdir.y = -movdir.y;
                }
                break;
            } else if (key & PORT_A_KEY_1) {
                handle_enter_exit(&lk); break;
            } else if (key & PORT_A_KEY_2) {
                if (player.floor != OVERWORLD_LEVEL) {
                    if (handle_attack(&lk))
                        break;
                    break;
                }
                continue;
            }
        }

        /* post-turn processing */
        for (;;) {
            post_turn();

            if (player.stats[sHIT_POINTS] <= 0)
                return;

            if (player.floor >= OVERWORLD_LEVEL) {
                if (player.floor == OVERWORLD_LEVEL)
                    draw_overworld();
                else
                    draw_dungeon();
                break;
            }
            if (dungeon_obj[dunloc.x][dunloc.y]
                != oLADDER_UP)
            {
                print(STR_HUH);
                continue;
            }
            if (player.floor == FIRST_DUNGEON_LEVEL) {
                --player.floor;
                setBackdropColor(SG_COLOR_BLACK);
                bpe_print(BPE_LEAVE_DUNGEON);
            } else {
                --player.floor;
                bpe_labeled_int(BPE_GO_UP_TO_LEVEL,
                    player.floor);
                print("\n");
                fill_dungeon();
            }
            if (player.floor == OVERWORLD_LEVEL) {
                char lk_buf[7];
                bpe_print(BPE_THOU_HAST_GAINED);
                print(SEGA_itoa(lk, lk_buf));
                bpe_print(BPE_HIT_POINTS);
                player.stats[sHIT_POINTS] += lk;
                lk = 0;
            }
        }
    }
}

/* ---------- pregame dissolve animation ---------------------------------- */

/* 1-second pause before dissolve starts */
#define DISSOLVE_WAIT       ((unsigned char)(1 * FRAME_RATE))
#define DISSOLVE_FIRST_ROW  12
#define DISSOLVE_ROWS       6       /* rows 12..17 */
#define DISSOLVE_COLS       TEXTCONSOLE_MAX_X   /* 32 */
#define DISSOLVE_TOTAL      (DISSOLVE_ROWS * DISSOLVE_COLS) /* 192 */
#define DISSOLVE_STEP       77      /* coprime to 192 */
#define DISSOLVE_PER_FRAME  3

static void pregame_dissolve(void) {
    unsigned char pos = 0;

    for (unsigned char w = 0; w < DISSOLVE_WAIT; ++w) {
        waitForVBlank();
        if (pressed_anything())
            return;
    }

    for (unsigned char i = 0; i < DISSOLVE_TOTAL; ++i) {
        print_font(pos & 0x1F,
            DISSOLVE_FIRST_ROW + (pos >> 5), ' ');
        pos += DISSOLVE_STEP;
        if (pos >= DISSOLVE_TOTAL)
            pos -= DISSOLVE_TOTAL;
        if (i % DISSOLVE_PER_FRAME == DISSOLVE_PER_FRAME - 1) {
            waitForVBlank();
            if (pressed_anything())
                return;
        }
    }
}

void pregame(void) {
    blank_screen();
    locateyx(15, 6);
    bpe_print(BPE_CREDITS_1);
    locateyx(16, 6);
    bpe_print(BPE_CREDITS_2);

    locateyx(17, 1);
    bpe_print(BPE_CREDITS_3);


    locateyx(12, 6);   /* CENTER("WELCOME TO AKALABETH") = 32/2 - 20/2 */
    bpe_print(BPE_TITLE);

    locateyx(13, 9);   /* CENTER("WORLD OF DOOM!") = 32/2 - 14/2 */
    bpe_print(BPE_SUBTITLE);

    pregame_dissolve();

    gameloop();
}
