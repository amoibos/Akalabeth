#include "engine2.h"
#include "SGlib/SGlib.h"
#include "engine.h"
#include "fbuffer.h"
#include "global.h"
#include "libbasic.h"
#include "libs/console.h"
#include "libs/keyboard.h"
#include "monster.h"
#include "widgets.h"
#include "bpe.h"
#include "assets/bpe_texts.h"

/* Shared itoa scratch buffer - 7 bytes is enough for any 16-bit integer.
   Never used concurrently (all callers are non-reentrant). */
static char s_itoa_buf[7];

void reset_sprites(void) {

    animation_frame = 0;
    for(unsigned char pos=0; pos < MAX_SPRITE; ++pos)
        all_sprites[pos] = NO_MOTION;
    sprites_no = 0;
    animation_refresh = 0;
}

/*
    save pseudo sprite
*/

inline signed short set_sprite_data(unsigned char sprite, unsigned char x, unsigned char y) {

    return ((signed short)(sprite << 10)) | (x << 5) | y;
}

void add_animation(unsigned char x, unsigned char y) {

    if (sprites_no < MAX_SPRITE)
        all_sprites[sprites_no] = set_sprite_data(sprites_no, x, y);
    ++sprites_no;
}

void timer(void) {
    unsigned fps_segment;

    if (timer_enabled) {
        if (++fps == FRAME_RATE) {
            ++seconds;
            fps = 0;
        }
    }

    if (timer_enabled && seconds) {
        fps_segment = (fps + 1) / (FRAME_RATE >> 3);
        if (previous_fps_seqment != fps_segment) {
            animation_refresh = 1;
            previous_fps_seqment = fps_segment;
        } else
            animation_refresh = 0;
    }

    if (audio_enabled) {
        PSGSFXFrame();
        PSGFrame();
    }
}

unsigned int wait_for_key(unsigned int keys) {
    unsigned int key=0;

    while (getKeyboardJoypadPressed() || keypressed()) {
        wait(1);
        scanKeyboardJoypad();
    }
    while (1) {
        wait(1);
        if (animation_refresh && timer_callback) {
            animation_refresh = 0;
            timer_callback();
        }

        SG_getKeycodes(&key, 1);
        if (SG_queryPauseRequested() || (keytoa(key) == 'p')) {
            SG_resetPauseRequest();
            return 0;
        }
        unsigned int pressed = read_input_pressed();
        if (pressed) {
            wait(1);
            if (!keys || (pressed & keys))
                return pressed;
        }
    }
}

void print_stat(const char * text, int value) {
    char output[7];
    unsigned char old_color = state.fgcolor;

    print(text);
    state.fgcolor = SG_COLOR_WHITE;
    if (value < 10)
        print("0");
    print(SEGA_itoa(value, output));
    state.fgcolor = old_color;
}

void character_creation(void) {
    static const unsigned int lucky_choices[5] = { 42, 1980, 6174, 30000, 65535 };
    static const char * const lucky_labels[5] = { "42", "1980", "6174", "30000", "65535" };
    static const char * const lop_labels[10] = { "01","02","03","04","05","06","07","08","09","10" };
    eScreens current=ScreenQuestions;
    unsigned char lineno=0;
   
    blank_screen();
    reset_sprites();
    player.has_quest = 0;

    locateyx(lineno, 0);
    bpe_print(BPE_CHOOSE_LUCKY);
    ++lineno;
    locateyx(lineno, 0);
    player.lucky_number = lucky_choices[menu(5, 6, HORIZONTAL, HIGHLIGHT_ON, lucky_labels, 2)];
    randomize(player.lucky_number);
    ++lineno;
    ++lineno;

    locateyx(lineno, 0);
    bpe_print(BPE_LEVEL_OF_PLAY);
    ++lineno;
    locateyx(lineno, 0);
    player.level_of_play = menu(10, 3, HORIZONTAL, HIGHLIGHT_ON, lop_labels, 0) + 1;
    ++lineno;
    ++lineno;

    // character values — print static labels once (they never change on reroll)
    static const char * const reroll_labels[] = {"ACCEPT", "REROLL"};
    for (unsigned char ll = 0; ll < MAX_STATS; ++ll) {
        locateyx(lineno + ll, 0);
        print(STATS[ll]);
    }
    locateyx(lineno + 7, 0);
    bpe_print(BPE_PLAY_WITH_THESE);
    do {
        // All STATS labels are exactly 11 chars; only overwrite the 2-digit value
        char output[7];
        unsigned char old_color = state.fgcolor;
        state.fgcolor = SG_COLOR_WHITE;
        for (unsigned char ll = 0; ll < MAX_STATS; ++ll) {
            player.stats[ll] = rnd_stat();
            locateyx(lineno + ll, 11);
            if (player.stats[ll] < 10)
                print("0");
            print(SEGA_itoa(player.stats[ll], output));
        }
        state.fgcolor = old_color;
        locateyx(lineno + 8, 14); //14 due to "qualities?" word length + extra space
    } while (menu(2, 9, HORIZONTAL, HIGHLIGHT_ON, reroll_labels, 1) != 0);

    bpe_print(BPE_FIGHTER_OR_MAGE);
    locateyx(-1, 2); //2 due space before "Fighter" and 15 due space for mage
    player.playerclass = (menu(2, 14, HORIZONTAL, HIGHLIGHT_OFF, 0, 0) == 0) ? pcFIGHTER : pcMAGE;
}

void assign_quest(void) {
    bpe_print(BPE_ASSIGN_QUEST);
    locateyx(-1, 10);
    print("CONTINUE");
    locateyx(-1, state.textconsole.x - 10);
    menu(1, 1, VERTICAL, HIGHLIGHT_OFF, 0, 0);
    for (unsigned char x = 0; x < MAX_STATS; ++x)
        ++player.stats[x];
    blank_screen();
}

int view_lord_british_castle(void) {
    char *output = s_itoa_buf;
    eScreens current = ScreenCastle;
    unsigned char old_color = state.fgcolor;

    blank_screen();
    locateyx(0, 0);
    PSGPlay(lordbrit_psg);
    timer_enabled = 1;
    audio_enabled = 1;

    if (!player.has_quest) {
        bpe_print(BPE_CASTLE_WELCOME);
        state.fgcolor = SG_COLOR_MAGENTA;
        print(PLAYER_NAME);
        state.fgcolor = old_color;
        bpe_print(BPE_CASTLE_ADVENTURE);
        static const char * const yn_labels[] = { "YES", "NO" };
        unsigned char choice = menu(2, 5, HORIZONTAL, HIGHLIGHT_ON, yn_labels, 0);
        if (choice != 0) {
            bpe_print(BPE_CASTLE_BEGONE);
            locateyx(-1, 7);
            menu(1, 1, VERTICAL, HIGHLIGHT_OFF, 0, 0);
            return 1090;
        }
        locateyx(-1, 0);
        bpe_print(BPE_CASTLE_FIRST_QUEST);
        quest_monster = (eMonster)(player.stats[sSTAMINA] / 3);
        quest_done    = 0;
        state.fgcolor = SG_COLOR_MAGENTA;
        print(MONSTERS[quest_monster]);
        state.fgcolor = old_color;
        player.has_quest = 1;
        assign_quest();
        return 1090;
    }

    if (quest_monster != mUNDEFINED && !quest_done) {
        print("\n\n");
        state.fgcolor = SG_COLOR_MAGENTA;
        print(PLAYER_NAME);
        state.fgcolor = old_color;
        bpe_print(BPE_CASTLE_RETURN);
        state.fgcolor = SG_COLOR_MAGENTA;
        print(MONSTERS[quest_monster]);
        state.fgcolor = old_color;
        bpe_print(BPE_QUEST_GO);
        wait_for_key(NO_SPECIFIC_KEY);
        blank_screen();
        return 1090;
    }

    bpe_print(BPE_AAHH);
    state.fgcolor = SG_COLOR_MAGENTA;
    print(PLAYER_NAME);
    state.fgcolor = old_color;
    bpe_print(BPE_QUEST_DONE);

    if (quest_monster == mBALROG) {
        blank_screen();
        locateyx(3, 0);
        strcpy(output, "           ");
        strcat(output, "LORD ");
        strcat(output, PLAYER_NAME);
        state.fgcolor = SG_COLOR_MAGENTA;
        print(output);
        state.fgcolor = old_color;
        bpe_print(BPE_QUEST_WORTHY);
        if (player.level_of_play == 10) {
            bpe_print(BPE_QUEST_CALIFORNIA);
        } else {
            bpe_print(BPE_QUEST_FOOLHEARTY);
            state.fgcolor = SG_COLOR_MAGENTA;
            print(SEGA_itoa(player.level_of_play + 1, output));
            state.fgcolor = old_color;
        }
    } else {
        bpe_print(BPE_QUEST_NOT_ENOUGH);
        quest_monster = (eMonster)(quest_monster + 1);
        quest_done    = 0;
        bpe_print(BPE_NOW_KILL);
        state.fgcolor = SG_COLOR_MAGENTA;
        print(MONSTERS[quest_monster]);
        state.fgcolor = old_color;
    }
    assign_quest();
    return 1090;
}

//60080
void view_stats(_Bool refresh_only) {
    char *output = s_itoa_buf;
    unsigned char old_color = state.fgcolor;

    //reduce flickering, update only values which could be changed
    if (!refresh_only) {
        state.fgcolor = SG_COLOR_TRANSPARENT;
        for (signed char y=8; y >= 0; --y) {
            locateyx(y, 0);
            blank_right_from_cursor(NO_DELAY);
        }

        state.fgcolor = SG_COLOR_MAGENTA;
        bpe_print(BPE_STATS_WEAPONS);
    }
    for(unsigned char x=0; x < MAX_ITEMS; ++x) {
        locateyx(3 + x, 0);
        if (!refresh_only) {
            state.fgcolor = SG_COLOR_CYAN;
            //show name of stat
            print(STATS[x]);
        }
        //show player stat
        locateyx(-1, 11);
        state.fgcolor = SG_COLOR_DARK_BLUE;
        unsigned char slen = strlen(SEGA_itoa(player.stats[x], output));
        print(output);
        if (slen < 5)
            blankArea(11 + slen, 3 + x, 15, 3 + x);

        //show amount of item
        locateyx(-1, 17);
        unsigned char ilen = strlen(SEGA_itoa(player.inventory[x], output));
        print(output);
        print("-");
        if (ilen < 3)
            blankArea(17 + ilen + 1, 3 + x, 20, 3 + x);

        //show item name
        if (!refresh_only) {
            locateyx(-1, 32 - strlen(ITEMS[x]));
            state.fgcolor = SG_COLOR_CYAN;
            print(ITEMS[x]);
        }
    }


    state.fgcolor = old_color;
}

/* ---- Dungeon map (shown during pause in dungeon) ----------------------- */

void view_dungeon_map(void) {
    char itoa_buf[8];
    char ch_buf[2];
    unsigned char old_color = state.fgcolor;
    unsigned char x, y;

    ch_buf[1] = '\0';
    unsigned char border_color = (player.floor == FIRST_DUNGEON_LEVEL) ? SG_COLOR_BLACK : SG_COLOR_GRAY;

    /* header */
    state.fgcolor = SG_COLOR_MAGENTA;
    locateyx(MAP_ORIGIN_ROW - 1, MAP_ORIGIN_COL);
    print("MAP LVL ");
    print(SEGA_itoa(player.floor, itoa_buf));
  
    /* rows including outer walls (DUNGEON_MIN..DUNGEON_MAX = 0..10) */
    for (y = DUNGEON_MIN; y <= DUNGEON_MAX; ++y) {
        locateyx(MAP_ORIGIN_ROW + y, MAP_ORIGIN_COL);

        for (x = DUNGEON_MIN; x <= DUNGEON_MAX; ++x) {
            unsigned char cell = dungeon_obj[x][y];
            char ch;
            unsigned char color;

            if (x == dunloc.x && y == dunloc.y) {
                /* player: direction arrow */
                if (movdir.x == 1)
                    ch = '>';
                else if (movdir.x == -1)
                    ch = '<';
                else if (movdir.y == 1)
                    ch = 'v';
                else
                    ch = '^';
                color = SG_COLOR_LIGHT_YELLOW;
            } else if (dungeon_mon[x][y] != 0) {
                ch = '!'; color = SG_COLOR_MEDIUM_RED;
            } else {
                switch (cell) {
                    case oWALL:        ch = '#'; color = SG_COLOR_GRAY;          break;
                    case oTRAP:        ch = 'T'; color = SG_COLOR_DARK_GREEN;   break;
                    case oSECRET_DOOR: ch = 'S'; color = SG_COLOR_LIGHT_BLUE;   break;
                    case oDOOR:        ch = '+'; color = SG_COLOR_CYAN;         break;
                    case oCHEST:       ch = '$'; color = SG_COLOR_DARK_YELLOW;  break;
                    case oLADDER_DOWN: ch = 'v'; color = SG_COLOR_LIGHT_GREEN;  break;
                    case oLADDER_UP:   ch = '^'; color = SG_COLOR_LIGHT_GREEN;  break;
                    case oPIT:         ch = 'O'; color = SG_COLOR_MEDIUM_RED;   break;
                    default:           ch = ' '; color = border_color;          break;
                }
            }
            state.fgcolor = color;
            ch_buf[0] = ch;
            print(ch_buf);
        }
    }
    
    /* legend — left-aligned at col 22 (one gap right of map border at col 20) */
#define LEGEND_COL 22
    locateyx(MAP_ORIGIN_ROW + 1, LEGEND_COL);
    state.fgcolor = SG_COLOR_MEDIUM_RED;  print("!");
    state.fgcolor = SG_COLOR_WHITE;       print("=MONSTER");

    locateyx(MAP_ORIGIN_ROW + 2, LEGEND_COL);
    state.fgcolor = SG_COLOR_CYAN;        print("+");
    state.fgcolor = SG_COLOR_WHITE;       print("=DOOR");

    locateyx(MAP_ORIGIN_ROW + 3, LEGEND_COL);
    state.fgcolor = SG_COLOR_DARK_YELLOW; print("$");
    state.fgcolor = SG_COLOR_WHITE;       print("=CHEST");

    locateyx(MAP_ORIGIN_ROW + 4, LEGEND_COL);
    state.fgcolor = SG_COLOR_LIGHT_BLUE;  print("S");
    state.fgcolor = SG_COLOR_WHITE;       print("=SECRET");

    locateyx(MAP_ORIGIN_ROW + 5, LEGEND_COL);
    state.fgcolor = SG_COLOR_DARK_GREEN;  print("T");
    state.fgcolor = SG_COLOR_WHITE;       print("=TRAP");

    locateyx(MAP_ORIGIN_ROW + 6, LEGEND_COL);
    state.fgcolor = SG_COLOR_MEDIUM_RED;  print("O");
    state.fgcolor = SG_COLOR_WHITE;       print("=PIT");

    locateyx(MAP_ORIGIN_ROW + 8, LEGEND_COL);
    state.fgcolor = SG_COLOR_LIGHT_GREEN; print("^");
    state.fgcolor = SG_COLOR_WHITE;       print("=UP");

    locateyx(MAP_ORIGIN_ROW + 9, LEGEND_COL);
    state.fgcolor = SG_COLOR_LIGHT_GREEN; print("v");
    state.fgcolor = SG_COLOR_WHITE;       print("=DOWN");
#undef LEGEND_COL

    state.fgcolor = old_color;
}

void show_toast(char *text) {
    unsigned char start_x = state.textconsole.x;
    unsigned char start_y = state.textconsole.y;
    print(text);
    wait(FRAME_RATE);
    unsigned char end_y = state.textconsole.y;
    for (unsigned char y = start_y; y <= end_y; ++y) {
        locateyx(y, (y == start_y) ? start_x : 0);
        while (state.textconsole.x < TEXTCONSOLE_MAX_X)
            print(" ");
    }
    locateyx(start_y, start_x);
}

//60200
void view_shop(void) {
    unsigned char line=11;
    eScreens current_screen=ScreenShop;
    unsigned old_color;

    SetTimerCallback(0);
    blank_screen();
    reset_sprites();

    // show table header
    state.fgcolor = SG_COLOR_MAGENTA;
    locateyx(16, 2);
    bpe_print(BPE_SHOP_HEADER);

    // list items
    state.fgcolor = SG_COLOR_CYAN;
    for(unsigned char y = 0; y < MAX_ITEMS; ++y) {
        locateyx(18 + y, 0);
        print(PRICE_TABLE[y]);
        locateyx(-1, 20);
        print(ITEMS[y]);
    }

    old_color = state.fgcolor;
    state.fgcolor = SG_COLOR_WHITE;
    locateyx(9, 17);
    print("QUIT");
    locateyx(line, 0);
    bpe_print(BPE_SHOP_WELCOME);
    state.fgcolor = old_color;
    PSGPlay(shop_psg);
    timer_enabled = 1;
    audio_enabled = 1;

    unsigned char selected = 0;
    _Bool first_draw = 1;
    do {
        view_stats(!first_draw);
        first_draw = 0;
        locateyx(line + 1, 0);
        state.fgcolor = SG_COLOR_WHITE;
        bpe_print(BPE_SHOP_BUY);
        state.fgcolor = old_color;
        unsigned char x_pos = state.textconsole.x;
        unsigned char price=0;
     
        locateyx(3, 15);
        selected = menu(MAX_ITEMS + 1, 1, VERTICAL, HIGHLIGHT_OFF, 0, selected);
        if (selected == MAX_ITEMS)
            break;

        price = ITEM_PRICE[selected];

        if (price == 0) {
            locateyx(line + 2, 0);
            show_toast("I\'M SORRY WE DON\'T HAVE THAT.");
        } else {
            if (price > player.stats[sGOLD]) {
                locateyx(line + 2, 0);
                show_toast("M'LORD THOU CAN NOT AFFORD THAT ITEM.");
            } else {
                if (selected == iFOOD)
                    player.inventory[iFOOD] += 10;
                else
                    player.inventory[selected] += 1;

                player.stats[sGOLD] -= price;
                PSGSFXPlay((void*)gold_psg, SFX_CHANNEL0);
            }
        }
        locateyx(line + 1, x_pos);
        blank_right_from_cursor(NO_DELAY);
    } while(1);
    PSGStop();
    locateyx(line + 2, 0);
    show_toast("Bye");
    wait(FRAME_RATE);
}

//_0060868
void blank_right_from_cursor(unsigned char delay) {
    unsigned char cy=state.textconsole.y;
    unsigned char cx=state.textconsole.x;

    for (unsigned char x=cx+1; x < TEXTCONSOLE_MAX_X; ++x) {
        print(" ");
        if (delay)
            wait(delay);
    }
    locateyx(cy ,cx);
}

