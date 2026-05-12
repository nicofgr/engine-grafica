#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>
#include "types.h"

typedef struct Engine{
        void (*init)();
        void (*update)();
        void (*draw)();
}Engine;

void engine_run(Engine);
void engine_quit();
u32 create_sphere(vec3 position,  float radius, Color_RGBA color, float luminosity);
void draw_object(u32 object);
void move_object(u32 objectID, vec3 position);
void position_update(u32 objectID, vec3 newPos);

extern float delta_time;
extern int last_frame_time;

#endif

