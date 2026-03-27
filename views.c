#include "views.h"
#include "data.h"
#include "bpe.h"
#include "assets/bpe_texts.h"

static const unsigned char title_rain[8] = {
    0x10, 0x18, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00
};

typedef struct {
    unsigned char x;
    unsigned char y;
    unsigned char speed;
} Raindrop;

const unsigned char title_torch[4][8] = {
    { 0x10, 0x38, 0x7C, 0xFE, 0xFE, 0x54, 0x54, 0x00 }, /* normal: single center spike */
    { 0x28, 0x7C, 0x7C, 0xFE, 0xFE, 0x54, 0x54, 0x00 }, /* tall:   double spike        */
    { 0x54, 0x7C, 0x7C, 0xFE, 0xFE, 0x54, 0x54, 0x00 }, /* tall:   triple spike        */
    { 0x00, 0x00, 0x3C, 0x7E, 0xFF, 0x54, 0x54, 0x00 }, /* squat:  low glowing flare   */
};
const unsigned char torch_seq[8] = { 0, 1, 0, 2, 0, 3, 1, 0 };
const unsigned char title_torch_color[4] = {
    SG_COLOR_DARK_YELLOW, SG_COLOR_DARK_YELLOW, SG_COLOR_DARK_YELLOW, SG_COLOR_MEDIUM_RED
};

const unsigned char title_flag[3][8] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x07, 0x00 }, /* wave up   */
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x1F, 0x00 }, /* flat      */
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x0F, 0x3C }, /* wave down */
};
const unsigned char flag_seq[4] = { 0, 1, 2, 1 };

unsigned int view_title(void) {
    unsigned int result=0;
    eScreens current_screen=ScreenTitle;

    displayOff();
    print_img_compressed(akalabeth__tiles__bin, akalabeth__palette__bin, SCREEN_MAX_X,  SCREEN_MAX_Y, 0, 0, EffectNone, 0);

    /* Pattern table layout:
       0-3 = torch (4 frames × 8 bytes)
       4-6 = flag  (3 × 8 bytes)
       7   = rain  (1 × 8 bytes) */
    SG_loadSpritePatterns((void *)title_torch, 0, 32);
    SG_loadSpritePatterns((void *)title_flag,  4, 24);
    SG_loadSpritePatterns((void *)title_rain,  RAIN_SPRITE_SLOT, 8);

    displayOn();
    PSGPlay(stones_psg);
    timer_enabled = 1;
    audio_enabled = 1;
    unsigned char color=2;
    unsigned char torch_timer=0, torch_frame=0;
    unsigned char flag_timer=0,  flag_frame=0;

    Raindrop drops[RAIN_COUNT];
    for (unsigned char i = 0; i < RAIN_COUNT; i++) {
        drops[i].x     = rnd8();
        drops[i].y     = (unsigned char)(i * (192 / RAIN_COUNT));
        drops[i].speed = (i & 1) ? 2 : 3;
    }

    wait(1);
    do {
        waitForVBlank();
        result = pressed_anything();
        if (result != 0)
            break;
        if (color == 15)
            color = 2;
        setBackdropColor(color++);

        /* Torch flicker: ~5 fps, ping-pong normal→tall→short→tall→... */
        if (++torch_timer >= (FRAME_RATE / 8)) {
            torch_timer = 0;
            if (++torch_frame >= 8)
                torch_frame = 0;
        }
        /* Flag wave: ~4 fps (4 frames, wave travels left→right) */
        if (++flag_timer >= (FRAME_RATE / 4)) {
            flag_timer = 0;
            if (++flag_frame >= 4)
                flag_frame = 0;
        }

        initSprites();
        /* Torch */
        addSprite(TITLE_TORCH_X, TITLE_TORCH_Y, torch_seq[torch_frame], title_torch_color[torch_seq[torch_frame]]);
        /* Flag */
        addSprite(TITLE_FLAG_X, TITLE_FLAG_Y, 4 + flag_seq[flag_frame], SG_COLOR_CYAN);
        /* Rain */
        for (unsigned char i = 0; i < RAIN_COUNT; i++) {
            drops[i].y += drops[i].speed;
            if (drops[i].y >= 192) {
                drops[i].y = 0;
                drops[i].x = rnd8();
            }
            addSprite(drops[i].x, drops[i].y, RAIN_SPRITE_SLOT, SG_COLOR_WHITE);
        }
        finalizeSprites();
        copySpritestoSAT();
    } while(1);

    initSprites();
    finalizeSprites();
    copySpritestoSAT();
    PSGStop();
    setBackdropColor(SG_COLOR_BLACK);
    return result;
}

void add_any_key_animation(void) {
    unsigned char output[TEXTCONSOLE_MAX_X+1];
    strcpy(output, PRESS_ANY_KEY_TO);

    char old_fgcolor = state.fgcolor;
    state.fgcolor = SG_COLOR_WHITE;

    unsigned char colors[8];
    colors[0] = (unsigned char)((state.fgcolor << 4) + state.bgcolor);
    for (unsigned char i = 1; i < 8; ++i)
        colors[i] = colors[0];

    unsigned char len = (unsigned char)strlen(output);
    unsigned char start_x = CENTER(output);
    unsigned char y = TEXTCONSOLE_MAX_Y - 1;

    for (unsigned char pos = 0; pos < len; ++pos) {
        loadPalette(colors, (y << 5) + start_x + pos, 8);
        add_animation(start_x + pos, y);
    }

    state.fgcolor = old_fgcolor;
}

void view_explain(void) {
    eScreens current_screen=ScreenExplain;

    blank_screen();
    reset_sprites();
    bpe_print(BPE_EXPLAIN);

    add_any_key_animation();    

    while(!pressed_anything()) {

        if (animation_refresh) {
            animate_quarterly(current_screen);
        }
        waitForVBlank();
    }
}

void view_explain2(void) {
    eScreens current_screen=ScreenExplain2;

    blank_screen();
    reset_sprites();
    bpe_print(BPE_EXPLAIN2);

    add_any_key_animation(); 

    while(!pressed_anything()) {

        if (animation_refresh) {
            animate_quarterly(current_screen);
        }
        waitForVBlank();
    }
}


void view_story(void) {
    eScreens current_screen=ScreenStory;

    blank_screen();
    reset_sprites();
    bpe_print(BPE_STORY);
    
    add_any_key_animation(); 

    do {
        if (pressed_anything() != 0)
            break;
        if (animation_refresh) {
            animate_quarterly(current_screen);
        }
        waitForVBlank();
    } while(1);
}

void view_controls(void) {
    eScreens current_screen=ScreenControls;

    blank_screen();
    reset_sprites();
    bpe_print(BPE_CONTROLS);

    add_any_key_animation();    

    while(!pressed_anything()) {

        if (animation_refresh) {
            animate_quarterly(current_screen);
        }
        waitForVBlank();
    }
}

void view_appendix(void) {
    eScreens current_screen=ScreenAppendix;

    blank_screen();
    reset_sprites();
    bpe_print(BPE_APPENDIX);

    add_any_key_animation();

    while(!pressed_anything()) {

        if (animation_refresh) {
            animate_quarterly(current_screen);
        }
        waitForVBlank();
    }
}
