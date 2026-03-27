#include "bpe.h"

#include "assets/bpe_table.h"
#include "libs/console.h"

/*
 * Maximum stack depth for the recursive expansion.
 * With 184 rules the tree can nest deeper; 48 bytes is a safe upper bound.
 */
#define BPE_STACK_SIZE 48

/*
 * Flush buffer: accumulate decoded characters and call print() in chunks
 * to reduce per-character function-call overhead.
 */
#define BPE_BUF_SIZE 32

void bpe_print(const unsigned char *data) {
    char buf[BPE_BUF_SIZE + 1];
    unsigned char stack[BPE_STACK_SIZE];
    unsigned char sp = 0;
    unsigned char bp = 0;
    unsigned char tok;

    while (1) {
        /* Pop from expansion stack, or read next byte from stream */
        if (sp > 0) {
            tok = stack[--sp];
        } else {
            tok = *data++;
            if (tok == BPE_END_MARKER)
                break;
        }

        if (tok < BPE_TOKEN_BASE) {
            /* Literal character: remap dense id back to original byte */
            buf[bp++] = (char)bpe_remap[tok];
            if (bp >= BPE_BUF_SIZE) {
                buf[bp] = '\0';
                print(buf);
                bp = 0;
            }
        } else {
            /* BPE token: push right child, then left (LIFO → left first) */
            unsigned char idx = tok - BPE_TOKEN_BASE;
            if (sp + 1 < BPE_STACK_SIZE) {
                stack[sp++] = bpe_table[idx][1];
                stack[sp++] = bpe_table[idx][0];
            }
        }
    }

    /* Flush remaining characters */
    if (bp > 0) {
        buf[bp] = '\0';
        print(buf);
    }
}
