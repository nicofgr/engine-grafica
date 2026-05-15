#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "types.h"

// My defines
#define SCREEN_WIDTH   800
#define SCREEN_HEIGHT  600
#define TRUE  1
#define FALSE 0
#define TARGET_FPS 60
#define FRAME_TARGET_TIME 1000.0/TARGET_FPS
#define TARGET_PPS 60.0
#define PPS_TARGET_TIME 1000.0/TARGET_PPS
#define FOV 70
#define GOLDEN_RATIO 1.61803398875

typedef struct{
        Color_RGB red;
        Color_RGB green;
        Color_RGB orange;
        Color_RGB black;
        Color_RGB star;
} ColorPallete;

extern const ColorPallete color;

#endif
