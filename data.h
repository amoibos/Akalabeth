#ifndef DATA_H
#define DATA_H

#define VERSION_STRING              "0.8.2"

#ifdef DEMO
#define VERSION                     "DEMO " VERSION_STRING
#else
#define VERSION                     VERSION_STRING
#endif

#define NO_MOTION                   ((-1))

#define MAX_SPRITE                  ((28))


typedef enum eScreens {
     ScreenUndefined
    ,ScreenTitle
    ,ScreenControls
    ,ScreenExplain
    ,ScreenExplain2
    ,ScreenStory
    ,ScreenShop
    ,ScreenQuestions
    ,ScreenCastle
    ,ScreenAppendix
    ,ScreenDungeon
    ,ScreenOverworld
} eScreens;

typedef struct {
    unsigned char x;
    unsigned char y;
} sPoint;

typedef struct {
    signed char x;
    signed char y;
} sSignedPoint;

typedef struct {
    char fgcolor;
    char bgcolor;
    sPoint textconsole;
    sPoint screen;
    //used to random
    unsigned int seed;
    _Bool fullscreen;   /* 1 = always use full console width in print() */
} sState;

typedef struct {
    sPoint location;
    signed char index;
} sSpriteData;

#define FILLED              ((1U))
#define REFRESH_STATS_ONLY  ((1U))
#define STATS_SHOW_ALL      ((0U))
#define DELAY_1FRAME        ((1U))
#define NO_DELAY            ((0U))

#endif
