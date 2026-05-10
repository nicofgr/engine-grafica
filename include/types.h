#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <cglm/cglm.h>

typedef uint32_t u32;

typedef struct Vec3_Array{
        vec3* array;
        u32   size;
}Vec3_Array;

typedef struct u32_Array{
        u32* array;
        u32  size;
}u32_Array;

typedef struct Color_RGBA{
        float R;
        float G;
        float B;
        float A;
}Color_RGBA;

typedef struct{
        char* string;
        unsigned int size;
} String;

typedef struct{
        Color_RGBA red;
        Color_RGBA green;
        Color_RGBA orange;
        Color_RGBA star;
} ColorPallete;

void Vec3Array_Push(Vec3_Array*, vec3);
#endif
