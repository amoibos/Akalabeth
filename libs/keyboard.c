#include "keyboard.h"

const unsigned short KEYCODES[MAX_KEYS] = {
      4, 16392,  8200,  8196,  8194,
  12292, 16388, 20484,   128, 24580,
     64,  4160, 24584, 20488,  4224,
   8320,     2, 12290,  4100, 16386,
  24578, 12296,  4098,  4104, 20482,
      8,
   8448,     1,  4097,  8193, 12289,
  16385, 20481, 24577,   256,  4352,
   4112, 20544, 12304, 26624,
  24640, 16416, 20512, 24608
};

const unsigned char KEYS[MAX_KEYS] = {
  'a','b','c','d','e','f','g','h','i','j',
  'k','l','m','n','o','p','q','r','s','t',
  'u','v','w','x','y','z',
  '0','1','2','3','4','5','6','7','8','9',
  KC_SPACE, KC_RETURN, KC_BACKSPACE, KC_SHIFT,
  KC_CURSOR_UP, KC_CURSOR_DOWN, KC_CURSOR_LEFT, KC_CURSOR_RIGHT
};

unsigned char keytoa(unsigned short keycode) {
    for(unsigned char i=0; i < MAX_KEYS; ++i)
        if (keycode == KEYCODES[i])
            return KEYS[i];
    return 0;
}
