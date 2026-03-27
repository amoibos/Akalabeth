#ifndef LIBBASIC_H
#define LIBBASIC_H

#include "global.h"
#include "data.h"

void locateyx(signed char y, signed char x);

#define randomize(x)  (state.seed = (unsigned int)(x))
unsigned char rnd8(void);

// rnd_pct(p)   - gibt true mit ~p% Wahrscheinlichkeit (p in 0..100)
// rnd_range(n) - Zufallszahl in [0, n-1]
// rnd_bool()   - 50/50 Entscheidung
#define rnd_pct(p)    (rnd8() < (unsigned char)((p) * 256 / 100))
#define rnd_range(n)  (rnd8() % (unsigned char)(n))
#define rnd_bool()    (rnd8() & 0x80)
// rnd_stat() - Equivalent zu BASIC INT(SQR(RND)*21+4): Werte [4..24], hohe Werte haeufiger
#define rnd_stat()    (isqrt16((unsigned int)rnd8() << 8) * 21 / 256 + 4)

unsigned char isqrt16(unsigned int n);
#define sgn(x)  ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))
extern sState state;

#endif
