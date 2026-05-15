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

u32  create_sphere(vec3d position,  float radius);
void draw_object(u32 object);
void move_object(u32 objectID, vec3d position);
void object_position_copy(u32 objectID, vec3d dest);
void position_update(u32 objectID, vec3d newPos);

u32  create_material(Color_RGB ambient, Color_RGB diffuse, Color_RGB specular, float shininess, Color_RGB emission);
void change_material(u32 objectID, u32 materialID);

void camera_position_get(vec3 dest);

extern float delta_time;
extern int last_frame_time;

#endif

