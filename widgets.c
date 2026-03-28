#include "widgets.h"
#include "libbasic.h"
#include "libs/keyboard.h"

void (*timer_callback)(void);

/* Navigate a menu with directional keys, confirm with button 2.
   Start position is taken from state.textconsole.x/y.
   offset: distance in tiles between items (cursor at n*offset from start).
   horizontal: 1 = left/right navigation, 0 = up/down navigation.
   highlight=0: blinking ► sprite cursor (labels may be NULL).
   highlight=1: active label reprinted in yellow, inactive in normal color
                (labels must point to an array of count strings; label column
                 is start + i*offset + 1, one tile right of the cursor position). */
unsigned char menu(unsigned char count, unsigned char offset, _Bool horizontal,
                   _Bool highlight, const char * const *labels,
                   unsigned char initial_selection) {
    unsigned char selection = initial_selection;
    unsigned char start_x = state.textconsole.x;
    unsigned char start_y = state.textconsole.y;
    unsigned char old_color = state.fgcolor;

    if (highlight) {
        /* Initial highlight: color all items, mark active one white. */
        for (unsigned char i = 0; i < count; ++i) {
            state.fgcolor = (i == selection) ? SG_COLOR_WHITE : old_color;
            if (horizontal)
                locateyx(start_y, start_x + i * offset + 1);
            else
                locateyx(start_y + i * offset, start_x + 1);
            print(labels[i]);
        }
        state.fgcolor = old_color;
    } else {
        /* load ► (ASCII 16) from font into sprite tile slot 11 */
        SG_loadSpritePatterns((void *)&font__tiles__bin[16 << 3], 11, 8);
    }

    unsigned char blink_timer = 0;
    _Bool cursor_visible = 1;

    for (;;) {
        waitForVBlank();

        if (!highlight) {
            initSprites();
            if (cursor_visible) {
                if (horizontal)
                    addSprite((start_x + selection * offset) * 8, start_y * 8, 11, SG_COLOR_WHITE);
                else
                    addSprite(start_x * 8, (start_y + selection * offset) * 8, 11, SG_COLOR_WHITE);
            }
            finalizeSprites();
            copySpritestoSAT();

            if (++blink_timer >= (FRAME_RATE >> 2)) {
                blink_timer = 0;
                cursor_visible = !cursor_visible;
            }
        }

        scanKeyboardJoypad();
        unsigned int keys = map_b_to_a(keypressed());
        unsigned char prev = selection;

        if (horizontal) {
            if (keys & PORT_A_KEY_LEFT) {
                if (selection > 0)
                    --selection;
                cursor_visible = 1; blink_timer = 0;
            } else if (keys & PORT_A_KEY_RIGHT) {
                if (selection < count - 1)
                    ++selection;
                cursor_visible = 1; blink_timer = 0;
            }
        } else {
            if (keys & PORT_A_KEY_UP) {
                if (selection > 0)
                    --selection;
                cursor_visible = 1; blink_timer = 0;
            } else if (keys & PORT_A_KEY_DOWN) {
                if (selection < count - 1)
                    ++selection;
                cursor_visible = 1; blink_timer = 0;
            }
        }

        if (highlight && selection != prev) {
            /* Recolor previous item back to normal, new item to white. */
            state.fgcolor = old_color;
            if (horizontal)
                locateyx(start_y, start_x + prev * offset + 1);
            else
                locateyx(start_y + prev * offset, start_x + 1);
            print(labels[prev]);

            state.fgcolor = SG_COLOR_WHITE;
            if (horizontal)
                locateyx(start_y, start_x + selection * offset + 1);
            else
                locateyx(start_y + selection * offset, start_x + 1);
            print(labels[selection]);

            state.fgcolor = old_color;
        }

        if (keys & PORT_A_KEY_2) {
            state.fgcolor = old_color;
            if (!highlight) {
                initSprites();
                finalizeSprites();
                copySpritestoSAT();
            }
            return selection;
        }
    }
}

void SetTimerCallback (void (*theHandlerFunction)(void)) __z88dk_fastcall {
    timer_callback=theHandlerFunction;
}

unsigned int pressed_anything(void) {
    unsigned int button=0;
    unsigned int key=0;

    scanKeyboardJoypad();
    button = getKeyboardJoypadStatus();
    SG_getKeycodes(&key, 1);
    
    if (SG_queryPauseRequested() || (keytoa(key) == 'p')) {
        SG_resetPauseRequest();
        button |= CARTRIDGE_SLOT;
    }

    return button;
}

void print_img_compressed( const unsigned char *tiledata,
                const unsigned char *colordata,
                const unsigned short width, const unsigned char height,
                const unsigned char left, const unsigned char top,
                Effect effect,
                const unsigned short start_img_tiles) {

    loadZX7compressedTiles(tiledata, start_img_tiles);
    loadZX7compressedBGColors(colordata, start_img_tiles);

    signed char height8=height>>3, width8=width>>3;
    unsigned short tileno = 0;
    unsigned char aborted = 0;
    if (effect == EffectSpiral) {
        displayOn();
        signed char center_x, center_y;
        signed char x=0, y=0, x_off=width8>>1, y_off=height8>>1, dx=0, dy=-1, temp;
        unsigned short longside_squared = (width8 < height8) ? height8 * height8 : width8 * width8;
        for(unsigned short i=0; i < longside_squared; ++i) {
            if ((x >= -x_off) && (x <= x_off) && (y >= -y_off) && (y <= y_off)) {
                center_x = x + x_off, center_y = y + y_off;
                tileno = center_y * width8 + center_x + start_img_tiles;
                if ((center_y < height8) && (center_y >= 0))
                    print_tile(center_x + left, center_y + top, tileno);
                if (!aborted)
                    wait(1);

                if (pressed_anything())
                    aborted = 1;
            }
            if ((x == y) || ((x < 0) && (x == -y)) || ((x > 0) && (x == 1 - y))) {
                temp = dx;
                dx = -dy;
                dy = temp;
            }

            x += dx;
            y += dy;
        }
    } else { //linear
        for (unsigned short y=top; y < top + height8; ++y) {
            for(unsigned short x=left; x < left + width8; ++x) {
                print_tile(x, y, (unsigned char)(tileno + start_img_tiles));
                ++tileno;
            }
        }
    }
}


