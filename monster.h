#ifndef MONSTER_H
#define MONSTER_H

#include "global.h"

/* Shape command codes for the data-driven renderer in monster.c */
#define SC_MOVE     0
#define SC_DRAW     1
#define SC_DOT      2
#define SC_GNDMOVE  3   /* move to ground-relative y (79 - yy[di]) */
#define SC_GNDDRAW  4   /* draw to ground-relative y (79 - yy[di]) */
#define SC_END      127

void draw_skeleton(unsigned char di);
void draw_thief(unsigned char di);
void draw_giant_rat(unsigned char di);
void draw_orc(unsigned char di);
void draw_viper(unsigned char di);
void draw_carrion_crawler(unsigned char di);
void draw_gremlin(unsigned char di);
void draw_mimic(unsigned char di);
void draw_daemon(unsigned char di);
void draw_balrog(unsigned char di);
void draw_monster(eMonster enemy, unsigned char di);

#endif
