#include "fbuffer.h"

inline void reorder(unsigned char *x1, unsigned char *y1, unsigned char *x2, unsigned char *y2) {
    unsigned char temp;
    temp = *x1;
    *x1 = *x2;
    *x2 = temp;
    temp = *y1;
    *y1 = *y2;
    *y2 = temp;
}

void bresenham(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2) {
    if (x1 > x2)
        reorder(&x1, &y1, &x2, &y2);

    int dx = x2 - x1;
    int dy = y2 - y1;

    int dx1 = (dx < 0) ? -dx : dx;
    int dy1 = (dy < 0) ? -dy : dy;

    unsigned int  addr   = PGTADDRESS + SG_get_Tile_address(x1, y1);
    unsigned char mask   = 0x80 >> (x1 & 7);
    unsigned char x_frac = x1 & 7;
    unsigned char y_frac = y1 & 7;

    if (dy1 <= dx1) {
        /* Horizontal-dominant: x always increases, y may go up or down */
        int twoDy   = dy1 << 1;
        int twoDyDx = (dy1 - dx1) << 1;
        int p       = (dy1 << 1) - dx1;
        _Bool y_down = (dy > 0);
        int x = x1, xEnd = x2;

        SG_setPixelAddr(addr, mask, state.fgcolor);

        while (x < xEnd) {
            x++;
            /* step x+1 */
            if (x_frac == 7) { x_frac = 0; addr += 8; mask = 0x80; }
            else              { x_frac++;               mask >>= 1;  }

            if (p >= 0) {
                /* step y */
                if (y_down) {
                    if (y_frac == 7) { y_frac = 0; addr += 249; }
                    else             { y_frac++;    addr++;       }
                } else {
                    if (y_frac == 0) { y_frac = 7; addr -= 249; }
                    else             { y_frac--;    addr--;       }
                }
                p += twoDyDx;
            } else {
                p += twoDy;
            }
            SG_setPixelAddr(addr, mask, state.fgcolor);
        }
    } else {
        /* Vertical-dominant: y always increases (swap if y1 > y2) */
        int twoDx   = dx1 << 1;
        int twoDxDy = (dx1 - dy1) << 1;
        int p       = (dx1 << 1) - dy1;
        int x, y, yEnd;
        _Bool x_inc;

        if (y1 > y2) {
            x = x2; y = y2; yEnd = y1;
            addr   = PGTADDRESS + SG_get_Tile_address(x2, y2);
            mask   = 0x80 >> (x2 & 7);
            x_frac = x2 & 7;
            y_frac = y2 & 7;
            x_inc  = 0;   /* after y-swap: traverse x from x2 down toward x1 */
        } else {
            x = x1; y = y1; yEnd = y2;
            x_inc  = 1;   /* normal: x increases from x1 toward x2 */
        }

        SG_setPixelAddr(addr, mask, state.fgcolor);

        while (y < yEnd) {
            y++;
            /* y always increases */
            if (y_frac == 7) { y_frac = 0; addr += 249; }
            else             { y_frac++;    addr++;       }

            if (p >= 0) {
                if (x_inc) {
                    if (x_frac == 7) { x_frac = 0; addr += 8; mask = 0x80; }
                    else              { x_frac++;               mask >>= 1;  }
                } else {
                    if (x_frac == 0) { x_frac = 7; addr -= 8; mask = 0x01; }
                    else              { x_frac--;               mask <<= 1;  }
                }
                p += twoDxDy;
            } else {
                p += twoDx;
            }
            SG_setPixelAddr(addr, mask, state.fgcolor);
        }
    }
}


inline void point(unsigned char x, unsigned char y) {

    SG_setPixel(x, y, state.fgcolor);
}

void line(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2) {

    bresenham(x1, y1, x2, y2);
    state.screen.x = x2;
    state.screen.y = y2;
}

void line_end(unsigned char x2, unsigned char y2) {

    bresenham(state.screen.x, state.screen.y, x2, y2);
    state.screen.x = x2;
    state.screen.y = y2;
}

void rectangle(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char color, _Bool filled) {

    unsigned char old_color = state.fgcolor;
    state.fgcolor = color;
    //draw border
    bresenham(x1, y1, x2, y1);
    bresenham(x1, y2, x2, y2);
    bresenham(x1, y1, x1, y2);
    bresenham(x2, y1, x2, y2);

    if (filled) {
        for (unsigned char y=y1; y <= y2; ++y)
            bresenham(x1 + 1, y1 + 1, x2 - 1, y1 + 1);
    }
    state.screen.x = x2;
    state.screen.y = y2;
    state.fgcolor = old_color;
}

void blank_screen_keep_off(void) {
    displayOff();
    blankArea(0, 0, TEXTCONSOLE_MAX_X - 1, TEXTCONSOLE_MAX_Y - 1);
    state.textconsole.x = state.textconsole.y = 0;
    state.screen.x = state.screen.y = 0;
}

void blank_screen(void) {
    blank_screen_keep_off();
    displayOn();
}
