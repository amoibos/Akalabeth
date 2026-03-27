#ifndef ENGINE2_H
#define ENGINE2_H

#include "engine.h"

/* sprite handling */
extern inline signed short set_sprite_data(unsigned char sprite, unsigned char x, unsigned char y);
extern sSpriteData get_sprite_data(signed short data);
extern void animate_quarterly(eScreens screen);
void add_animation(unsigned char x, unsigned char y);
void reset_sprites(void);

void timer(void);
void view_dungeon_map(void);

#define MAP_ORIGIN_ROW  11
#define MAP_ORIGIN_COL  10
#define MAP_INNER_SIZE   9   /* dungeon cells 1..9 */

#endif
