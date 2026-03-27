#include "libbasic.h"

void locateyx(signed char y, signed char x) {

    if (y >= 0)
        state.textconsole.y = (unsigned char)y;
    if (x >= 0)
        state.textconsole.x = (unsigned char)x;
    //else
    //    state.textconsole.x = 0;
}


unsigned char rnd8(void) {
    state.seed ^= state.seed << 7;
    state.seed ^= state.seed >> 9;
    state.seed ^= state.seed << 8;
    return state.seed;
}


unsigned char isqrt16(unsigned int n) {
    unsigned char result = 0;
    unsigned char bit = 128;
    do {
        unsigned char candidate = result | bit;
        if ((unsigned int)candidate * candidate <= n)
            result = candidate;
        bit >>= 1;
    } while (bit);
    return result;
}

