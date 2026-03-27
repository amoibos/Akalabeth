#include "monster.h"
#include "engine.h"   /* yy[], pe[][] */
#include "fbuffer.h"  /* line(), line_end(), point() */
#include "libs/console.h"
#include "libs/strings.h"

/* compact notation for shape tables */
#define M(x,y)    SC_MOVE,   (signed char)(x),(signed char)(y)
#define L(x,y)    SC_DRAW,   (signed char)(x),(signed char)(y)
#define P(x,y)    SC_DOT,    (signed char)(x),(signed char)(y)
#define GNM(x,y)  SC_GNDMOVE,(signed char)(x),(signed char)(y)
#define GNL(x,y)  SC_GNDDRAW,(signed char)(x),(signed char)(y)

static const signed char shape_orc[] = {
    M(  0,   0), L(-15,   0),
    L( -8,  -8), L( -8, -15), L(-15, -23), L(-15, -15), L(-23, -23),
    L(-23, -45), L(-15, -53), L( -8, -53), L(-15, -68), L( -8, -75), L(  0, -75),
    M(  0,   0), L( 15,   0),
    L(  8,  -8), L(  8, -15), L( 15, -23), L( 15, -15), L( 23, -23),
    L( 23, -45), L( 15, -53), L(  8, -53), L( 15, -68), L(  8, -75), L(  0, -75),
    M(-15, -68), L( 15, -68),
    M( -8, -53), L(  8, -53),
    M(-23, -15), L(  8, -45),
    M( -8, -68), L(  0, -60), L(  8, -68), L(  8, -60), L( -8, -60), L( -8, -68),
    M(  0, -38), L( -8, -38), L(  8, -53), L(  8, -45), L( 15, -45), L(  0, -30), L(  0, -38),
    SC_END
};

static const signed char shape_viper[] = {
    M(-10, -15), L(-10, -30),
    L(-15, -20), L(-15, -15), L(-15,   0), L( 15,   0), L( 15, -15), L(-15, -15),
    M(-15, -10), L( 15, -10),
    M(-15,  -5), L( 15,  -5),
    M(  0, -15), L( -5, -20), L( -5, -35), L(  5, -35), L(  5, -20), L( 10, -15),
    M( -5, -20), L(  5, -20),
    M( -5, -25), L(  5, -25),
    M( -5, -30), L(  5, -30),
    M(-10, -35), L(-10, -40), L( -5, -45), L(  5, -45), L( 10, -40), L( 10, -35),
    M(-10, -40), L(  0, -45), L( 10, -40),
    M( -5, -40), L(  5, -40), L( 15, -30), L(  0, -40), L(-15, -30), L( -5, -40),
    SC_END
};

static const signed char shape_daemon[] = {
    M(-14, -46), L(-12, -37), L(-20, -32), L(-30, -32), L(-22, -24),
    L(-40, -17), L(-40,  -7), L(-38,  -5), L(-40,  -3), L(-40,   0),
    L(-36,   0), L(-34,  -2), L(-32,   0), L(-28,   0),
    L(-28,  -3), L(-30,  -5), L(-28,  -7), L(-28, -15), L(  0, -27),
    M( 14, -46), L( 12, -37), L( 20, -32), L( 30, -32), L( 22, -24),
    L( 40, -17), L( 40,  -7), L( 38,  -5), L( 40,  -3), L( 40,   0),
    L( 36,   0), L( 34,  -2), L( 32,   0), L( 28,   0),
    L( 28,  -3), L( 30,  -5), L( 28,  -7), L( 28, -15), L(  0, -27),
    M(  6, -48), L( 38, -41), L( 40, -42), L( 18, -56), L( 12, -56),
    L( 10, -57), L(  8, -56), L( -8, -56), L(-10, -58), L( 14, -58),
    L( 16, -59), L(  8, -63), L(  6, -63), L(  2, -70), L(  2, -63),
    L( -2, -63), L( -2, -70), L( -6, -63), L( -8, -63), L(-16, -59),
    L(-14, -58), L(-10, -57), L(-12, -56), L(-18, -56), L(-36, -47),
    L(-36, -39), L(-28, -41), L(-28, -46), L(-20, -50), L(-18, -50),
    L(-14, -46),
    M(-28, -41), L( 30, -55),
    M( 28, -58), L( 22, -56), L( 22, -53), L( 28, -52), L( 34, -54),
    L( 20, -50), L( 26, -47),
    M( 10, -58), L( 10, -61), L(  4, -58),
    M(-10, -58), L(-10, -61), L( -4, -58),
    M( 40,  -9), L( 50, -12), L( 40,  -7),
    M( -8, -25), L(  6,  -7), L( 28,  -7), L( 28,  -9), L( 20,  -9), L(  6, -25),
    SC_END
};

static const signed char shape_balrog[] = {
    M(  6, -60), L( 30, -90), L( 60, -30), L( 60, -10), L( 30, -40), L( 15, -40),
    M( -6, -60), L(-30, -90), L(-60, -30), L(-60, -10), L(-30, -40), L(-15, -40),
    M(  0, -25), L(  6, -25), L( 10, -20), L( 12, -10), L( 10,  -6),
    L( 10,   0), L( 14,   0), L( 15,  -5), L( 16,   0), L( 20,   0),
    L( 20,  -6), L( 18, -10), L( 18, -20), L( 15, -30), L( 15, -45),
    L( 40, -60), L( 40, -70), L( 10, -55), L(  6, -60), L( 10, -74),
    L(  6, -80), L(  4, -80), L(  3, -82), L(  2, -80), L(  0, -80),
    M(  0, -25), L( -6, -25), L(-10, -20), L(-12, -10), L(-10,  -6),
    L(-10,   0), L(-14,   0), L(-15,  -5), L(-16,   0), L(-20,   0),
    L(-20,  -6), L(-18, -10), L(-18, -20), L(-15, -30), L(-15, -45),
    L(-40, -60), L(-40, -70), L(-10, -55), L( -6, -60), L(-10, -74),
    L( -6, -80), L( -4, -80), L( -3, -82), L( -2, -80), L(  0, -80),
    M( -6, -25), L(  0,  -6), L( 10,   0), L(  4,  -8), L(  6, -25),
    M(-40, -64), L(-40, -90), L(-52, -80), L(-52, -40),
    M( 40, -86), L( 38, -92), L( 42, -92), L( 40, -86), L( 40, -50),
    M(  4, -70), L(  6, -74),
    M( -4, -70), L( -6, -74),
    M(  0, -64), L(  0, -60),
    SC_END
};

static const signed char shape_skeleton[] = {
    M(-23,   0), L(-15,   0), L(-15, -15), L( -8, -30), L(  8, -30), L( 15, -15), L( 15,   0), L( 23,   0),
    M(  0, -26), L(  0, -65),
    M( -3, -45), L(  3, -45),
    M( -2, -38), L(  2, -38),
    M( -5, -53), L(  5, -53),
    M(-23, -45), L(-30, -53), L(-23, -56), L(-23, -45), L( -8, -38),
    M(-15, -45), L( -8, -60), L(  8, -60), L( 15, -45),
    M( 15, -42), L( 15, -57),
    M( 12, -45), L( 20, -45),
    M(  0, -80), L( -5, -80), L( -8, -75), L( -5, -65), L(  5, -65), L(  5, -68),
    L( -5, -68), L( -5, -65), L(  5, -65), L(  8, -75), L(  5, -80), L(  0, -80),
    P( -6, -71), P( -5, -71), P( -6, -72), P( -5, -72),
    P(  5, -71), P(  6, -71), P(  5, -72), P(  6, -72),
    SC_END
};

static const signed char shape_thief[] = {
    M(  0, -56), L(  0,  -8), L( 10,   0), L( 30,   0), L( 30, -45), L( 10, -64),
    L(  0, -56), L(-10, -64), L(-30, -45), L(-30,   0), L(-10,   0), L(  0,  -8),
    M(-10, -64), L(-10, -75), L(  0, -83), L( 10, -75), L(  0, -79),
    L(-10, -75), L(  0, -60), L( 10, -75), L( 10, -64),
    SC_END
};

static const signed char shape_giant_rat[] = {
    M(  5, -30), L(  0, -25), L( -5, -30), L(-15,  -5), L(-10,   0), L( 10,   0),
    L( 15,  -5), L( 20,  -5), L( 10,   0), L( 15,  -5), L(  5, -30), L( 10, -40),
    L(  3, -35), L( -3, -35), L(-10, -40), L( -5, -30),
    SC_END
};

static const signed char shape_gremlin[] = {
    M(  5, -10), L( -5, -10), L(  0, -15), L( 10, -20), L(  5, -15), L(  5, -10),
    M(  5, -10), L(  7,  -6), L(  5,  -3), L( -5,  -3), L( -7,  -6), L( -5, -10),
    M(  2,  -3), L(  5,   0), L(  8,   0),
    M( -2,  -3), L( -5,   0), L( -8,   0),
    P(  3,  -8), P( -3,  -8),
    M(  3,  -5), L( -3,  -5),
    SC_END
};

/* Carrion Crawler: body outline uses ground level y (79 - yy[di]),
   all other coordinates use standard b + dy/di baseline. */
static const signed char shape_carrion_crawler[] = {
    GNM(-20,  0), L(-20, -88), L(-10, -83), L( 10, -83), L( 20, -88),
    GNL( 20,  0), GNL(-20,  0),
    M(-20, -88), L(-30, -83), L(-30, -78),
    M( 20, -88), L( 30, -83),
    M( 40, -83), L( 30, -83),
    M(-15, -86), L(-20, -83), L(-20, -78), L(-30, -73), L(-30, -68), L(-20, -63),
    M(-10, -83), L(-10, -58), L(  0, -50),
    M( 10, -83), L( 10, -78), L( 20, -73), L( 20, -40),
    M( 15, -85), L( 20, -78), L( 30, -76), L( 30, -60),
    M(  0, -83), L(  0, -73), L( 10, -68), L( 10, -63), L(  0, -58),
    SC_END
};

/* Mimic: all coordinates relative to pe[di][3] (floor baseline),
   x center = 139 = c.  Caller passes pe[di][3] as y_base. */
static const signed char shape_mimic[] = {
    M(-10,   0), L(-10, -10), L( 10, -10), L( 10,   0), L(-10,   0),
    M(-10, -10), L( -5, -15), L( 15, -15), L( 15,  -5), L( 10,   0),
    M( 10, -10), L( 15, -15),
    SC_END
};

void draw_shape(const signed char *s, unsigned char y_base, unsigned char di) {
    unsigned char gnd_y = (unsigned char)(79 - monster_y[di]);
    while (*s != SC_END) {
        signed char cmd   = *s++;
        signed char dx    = *s++;
        signed char dy    = *s++;
        unsigned char x   = (di == 1) ? (unsigned char)(139 + dx)
                                      : (unsigned char)(139 + dx / di);
        unsigned char y_off = (di == 1) ? (unsigned char)dy
                                        : (unsigned char)(dy / di);
        unsigned char y   = ((cmd == SC_GNDMOVE) || (cmd == SC_GNDDRAW))
                          ? (unsigned char)(gnd_y + y_off)
                          : (unsigned char)(y_base + y_off);
        switch (cmd) {
            case SC_MOVE:
            case SC_GNDMOVE:
                state.screen.x = x;
                state.screen.y = y;
                break;
            case SC_DOT:
                point(x, y);
                break;
            default:
                line_end(x, y);
                break;
        }
    }
}

void draw_skeleton(unsigned char di) {
    draw_shape(shape_skeleton, draw_center_y, di);
}

void draw_thief(unsigned char di) {
    draw_shape(shape_thief, draw_center_y, di);
}

void draw_giant_rat(unsigned char di) {
    draw_shape(shape_giant_rat, draw_center_y, di);
}

void draw_orc(unsigned char di) {
    draw_shape(shape_orc, draw_center_y, di);
}

void draw_viper(unsigned char di) {
    draw_shape(shape_viper, draw_center_y, di);
}

void draw_carrion_crawler(unsigned char di) {
    draw_shape(shape_carrion_crawler, draw_center_y, di);
}

void draw_gremlin(unsigned char di) {
    draw_shape(shape_gremlin, draw_center_y, di);
}

void draw_mimic(unsigned char di) {
    draw_shape(shape_mimic, monster_pos[di][3], di);
}

void draw_daemon(unsigned char di) {
    draw_shape(shape_daemon, draw_center_y, di);
}

void draw_balrog(unsigned char di) {
    draw_shape(shape_balrog, draw_center_y, di);
}

void draw_monster(eMonster enemy, unsigned char di) {
    switch(enemy) {
        case mUNDEFINED:
            break;
        case mSKELETON:
            draw_skeleton(di);
            break;
        case mTHIEF:
            draw_thief(di);
            break;
        case mGIANT_RAT:
            draw_giant_rat(di);
            break;
        case mORC:
            draw_orc(di);
            break;
        case mVIPER:
            draw_viper(di);
            break;
        case mCARRION_CRAWLER:
            draw_carrion_crawler(di);
            break;
        case mGREMLIN:
            draw_gremlin(di);
            break;
        case mMIMIC:
            draw_mimic(di);
            break;
        case mDAEMON:
            draw_daemon(di);
            break;
        case mBALROG:
            draw_balrog(di);
            break;
    }
}
