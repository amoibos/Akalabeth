#ifndef VIEWS_H
#define VIEWS_H

#include "data.h"
#include "assets/images.h"
#include "libs/console.h"
#include "libs/strings.h"

#include "PSGlib/PSGlib.h"
#include "assets/audio.h"

#include "animation.h"
#include "engine.h"
#include "engine2.h"
#include "widgets.h"
#include "fbuffer.h"

#define CENTER(x) ((TEXTCONSOLE_MAX_X >> 1) - (strlen(x) >> 1))

#define RAIN_COUNT       20
#define RAIN_SPRITE_SLOT  7
#define TITLE_TORCH_X  (unsigned char)(16 * 8 + 1)
#define TITLE_TORCH_Y  (7 * 8 + 2)
#define TITLE_FLAG_X   (6 * 8)
#define TITLE_FLAG_Y   (4 * 8)

unsigned int view_title(void);
void view_explain(void);
void view_explain2(void);
void view_story(void);
void view_controls(void);
void view_appendix(void);
void add_any_key_animation(void);

void blank_screen(void);

extern const char PRESS_ANY_KEY_TO[];

#endif
