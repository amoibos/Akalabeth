#ifndef KEYBOARD_H
#define KEYBOARD_H

typedef enum eInputType {
    InputTypeUndefined,
    InputTypeNumerical,
    InputTypeNumericalExtended,
    InputTypeAlphaNumerical
} InputType;

#define KC_CURSOR_UP                ((0x01))
#define KC_CURSOR_DOWN              ((0x02))
#define KC_CURSOR_LEFT              ((0x03))
#define KC_CURSOR_RIGHT             ((0x04))
#define KC_RETURN                   ((0x0D))
#define KC_SPACE                    ((0x20))
#define KC_SHIFT                    ((0x1B))
#define KC_BACKSPACE                ((0x09))

#define MAX_KEYS                    ((44))
extern const unsigned short KEYCODES[MAX_KEYS];
extern const unsigned char  KEYS[MAX_KEYS];

unsigned char keytoa(unsigned short keycode);

#endif
