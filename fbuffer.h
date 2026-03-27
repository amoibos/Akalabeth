#ifndef FBUFFER_H
#define FBUFFER_H

#include "global.h"
#include "libs/console.h"

inline void point(unsigned char x, unsigned char y);
void line(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2);
void line_end(unsigned char x2, unsigned char y2);
void rectangle(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char color, _Bool filled);
void blank_screen_keep_off(void);
void blank_screen(void);
#endif
