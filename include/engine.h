#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>
#include "cglm/cglm.h"
#include "types.h"

typedef struct Engine{
        void (*init)();
        void (*update)();
        void (*draw)();
}Engine;

void engine_run(Engine);
void engine_quit();
u32 create_sphere(vec3 position,  float radius, Color_RGBA color);
void draw_entity(u32 object);

extern float delta_time;

#endif

