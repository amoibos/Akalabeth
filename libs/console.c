#include "console.h"

static void scroll_text_area(void) {
    unsigned char buf[8];
    unsigned char row, col, i;

    for (row = SCROLL_ROW_START; row < SCROLL_ROW_END; ++row) {
        unsigned int dst_off = (unsigned int)row * 256;
        unsigned int src_off = dst_off + 256; /* row + 1 */
        for (col = 0; col <= SCROLL_COL_END; ++col) {
            unsigned int col_off = (unsigned int)col * 8;
            for (i = 0; i < 8; ++i)
                buf[i] = SG_readVRAMbyte(PGTADDRESS + src_off + col_off + i);
            SG_VRAMmemcpy_brief(PGTADDRESS + dst_off + col_off, buf, 8);
            for (i = 0; i < 8; ++i)
                buf[i] = SG_readVRAMbyte(CGTADDRESS + src_off + col_off + i);
            SG_VRAMmemcpy_brief(CGTADDRESS + dst_off + col_off, buf, 8);
        }
    }
    SG_blankArea(0, SCROLL_ROW_END, SCROLL_COL_END, SCROLL_ROW_END);
}

void wait(unsigned char duration) {

    for(unsigned char wait=0; wait < duration; ++wait) {
        waitForVBlank();
    }
}

// print_raw do not modify color information and do not use internal cursor positon
inline void print_raw(unsigned char x, unsigned char y, char str[]) {
    loadTiles(str, (y << 5) + x, 8);
}

inline void print_tile(unsigned char x, unsigned char y, unsigned short tileno) {
    setNextTileatXY(x, y);
    setTile(tileno);
}

void print_font(unsigned char x, unsigned char y, unsigned short tileno) {
    loadTiles(&font__tiles__bin[(tileno) << 3], (y << 5) + x, 8);
}

static void advance_scroll_line(void) {
    if (state.textconsole.y >= SCROLL_ROW_END)
        scroll_text_area();
    else
        ++state.textconsole.y;
    state.textconsole.x = 0;
}

static void print_wrap_char(unsigned char ch, unsigned char colors[8]) {
    loadTiles(&font__tiles__bin[ch << 3], (state.textconsole.y << 5) + state.textconsole.x, 8);
    loadPalette(colors, (state.textconsole.y << 5) + state.textconsole.x, 8);
    ++state.textconsole.x;
}

void print_wrap(const char *str) {
    unsigned char colors[8];
    const char *p;
    unsigned char word_len;

    if (state.textconsole.y < SCROLL_ROW_START)
        state.textconsole.y = SCROLL_ROW_START;

    colors[0] = (state.fgcolor << 4) + state.bgcolor;
    for (unsigned char i = 1; i < 8; ++i)
        colors[i] = colors[0];

    while (*str) {
        if (*str == '\n') {
            advance_scroll_line();
            ++str;
            continue;
        }

        /* measure next word */
        word_len = 0;
        p = str;
        while (*p && *p != ' ' && *p != '\n') {
            ++word_len;
            ++p;
        }

        /* wrap before word if it doesn't fit (but never on an empty line) */
        if (state.textconsole.x > 0 &&
            state.textconsole.x + word_len > PRINT_WRAP_MAX_X)
            advance_scroll_line();

        /* output word, hard-break if single word exceeds max width */
        while (word_len--) {
            if (state.textconsole.x >= PRINT_WRAP_MAX_X)
                advance_scroll_line();
            print_wrap_char((unsigned char)*str, colors);
            ++str;
        }

        /* consume trailing space (skip if at line end) */
        if (*str == ' ') {
            if (state.textconsole.x < PRINT_WRAP_MAX_X)
                print_wrap_char(' ', colors);
            ++str;
        }
    }
}

void print(const char *str) {
    unsigned char colors[8];

    colors[0] = (state.fgcolor << 4) + state.bgcolor;
    for (unsigned char tile_row = 1; tile_row < 8; ++tile_row)
        colors[tile_row] = colors[0];

    unsigned char wrap_x = TEXTCONSOLE_MAX_X;
    if (!state.fullscreen && player.floor > OVERWORLD_LEVEL && state.textconsole.x < PRINT_WRAP_MAX_X)
        wrap_x = PRINT_WRAP_MAX_X;
    for(; *str; ++str) {
        if ((state.textconsole.x >= wrap_x) || (*str == '\n')) {
            if (player.floor > OVERWORLD_LEVEL && state.textconsole.y >= SCROLL_ROW_END) {
                scroll_text_area();
                /* y bleibt auf SCROLL_ROW_END, neue Zeile ist jetzt leer */
            } else {
                state.textconsole.y = (state.textconsole.y + 1 < TEXTCONSOLE_MAX_Y) ? state.textconsole.y + 1 : state.textconsole.y;
            }
            state.textconsole.x = 0;
        }
        if (*str != '\n') {
            loadTiles(&font__tiles__bin[(*str) << 3], (state.textconsole.y << 5) + state.textconsole.x, 8);
            loadPalette(&colors, (state.textconsole.y << 5) + state.textconsole.x, 8);
            ++state.textconsole.x;
        }
    }
}


/*
void print_num(unsigned char * x, unsigned char * y, long num, short offse) {
    char buffer[10+1];
    char *str;

    str = buffer;
    SEGA_itoa(num, buffer);

    for(; *str; ++str) {
        if (*x >= TEXTCONSOLE_MAX_X) {
            *y = (*y + 1 < TEXTCONSOLE_MAX_Y) ? *y + 1 : *y;
            *x=0;
        }
        loadTiles(&font__tiles__bin[(*str + offset) << 3], (*y << 5) + *x, 8);
        ++x;
    }
}*/
