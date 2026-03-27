#include "PSGlib/PSGlib.h"
#include "SGlib/SGlib.h"
#include "fbuffer.h"
#include "global.h"
#include "engine.h"
#include "views.h"

static void init_bmp_mode(void) {
    displayOff();
    state.fgcolor = SG_COLOR_CYAN;
    state.bgcolor = SG_COLOR_TRANSPARENT;
    state.fullscreen = 1;
    SG_initBMPmode(state.bgcolor, state.fgcolor);
    blank_screen();
    displayOn();
}

void main(void) {
    setFrameInterruptHandler(timer);
    randomize(1);
    while(1) {
        unsigned int result = view_title();
        init_bmp_mode();
        PSGPlay(intro_psg);
        if (result == CARTRIDGE_SLOT) { /* pause pressed, show help screens */
            view_explain();
            view_explain2();
            view_controls();
            view_appendix();
        }

        view_story();
        character_creation();
        generate_environment();
        PSGStop();
        view_shop();
        pregame();
    }
}
