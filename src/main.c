//
//#include "cglm/affine-pre.h"
//#include "cglm/affine-pre.h"
/**
#include "cglm/affine.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/util.h"
#include "cglm/vec3.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h> // for wait time
#include <cglm/cglm.h>
**/
#include "engine.h"
#include "constants.h"

void swapFloat(float* x1 , float* x2){
        float aux = *x1;
        *x1 = *x2;
        *x2 = aux;
}

int main(int argc, char** argv) {

        int quit = FALSE;
        int counter = 0;

        init();
        
        while(quit == FALSE){
                input(&quit);
                update(&counter);
                draw(counter);
                //quit = TRUE;
        }
        engine_quit();
        return 0;
}
