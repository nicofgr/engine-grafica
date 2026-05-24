#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <cglm/cglm.h>

typedef uint32_t u32;
typedef double vec3d[3];

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

typedef struct Color_RGB{
        float R;
        float G;
        float B;
}Color_RGB;

typedef struct String{
        char* string;
        u32   size;
} String;

void Vec3Array_Push(Vec3_Array*, vec3);
static inline void vec3d_copy(vec3d src, vec3d dest){
        dest[0] = src[0];
        dest[1] = src[1];
        dest[2] = src[2];
}
static inline void vec3d_add(vec3d a, vec3d b, vec3d dest){
        dest[0] = a[0] + b[0];
        dest[1] = a[1] + b[1];
        dest[2] = a[2] + b[2];
}

static inline void vec3d_sub(vec3d a, vec3d b, vec3d dest){
        dest[0] = a[0] - b[0];
        dest[1] = a[1] - b[1];
        dest[2] = a[2] - b[2];
}

static inline void vec3d_to_vec3(vec3d src, vec3 dest){
        dest[0] = src[0];
        dest[1] = src[1];
        dest[2] = src[2];
}
static inline void vec3_to_vec3d(vec3 src, vec3d dest){
        dest[0] = src[0];
        dest[1] = src[1];
        dest[2] = src[2];
}

static inline void vec3d_zero(vec3d v){
        v[0] = 0;
        v[1] = 0;
        v[2] = 0;
}
static inline void vec3d_print(vec3d vec){
        printf("(%.2f %.2f %.2f)\n", vec[0], vec[1], vec[2]);
}


#endif
