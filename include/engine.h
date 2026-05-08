#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>
#include <cglm/cglm.h>
#include <glad/glad.h>
#include <SDL2/SDL.h>

typedef uint32_t u32;

typedef struct Vec3_Array{
        vec3* array;
        u32   size;
}Vec3_Array;

typedef struct u32_Array{
        u32* array;
        u32  size;
}u32_Array;

typedef struct Model{
        Vec3_Array vertices;
        u32_Array  faces;
}Model;

typedef struct Color_RGBA{
        float R;
        float G;
        float B;
        float A;
}Color_RGBA;


// My defines
#define SCREEN_WIDTH   800
#define SCREEN_HEIGHT  600
#define TRUE  1
#define FALSE 0
#define TARGET_FPS 100
#define FRAME_TARGET_TIME 1000/TARGET_FPS
#define FOV 70
#define GOLDEN_RATIO 1.61803398875


#endif

