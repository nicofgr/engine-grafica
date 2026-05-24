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
u32  create_patch(vec3d position, float scale, u32 parentID, u32 LOD);
void draw_all_patches();
void rotate_patch(u32 patchID, versor rotation);

u32 create_patchtree(vec3d position, float scale, u32 maxLOD);
void draw_patchtree();

void draw_object(u32 object);
void move_object(u32 objectID, vec3d position);
void object_position_copy(u32 objectID, vec3d dest);
void position_update(u32 objectID, vec3d newPos);
void rotate_object(u32 objectID, versor rotation);

u32  create_material(Color_RGB ambient, Color_RGB diffuse, Color_RGB specular, float shininess, Color_RGB emission);
void change_material(u32 objectID, u32 materialID);

void camera_position_get(vec3 dest);
void engine_camera_position_set(vec3d position);

extern float delta_time;
extern int last_frame_time;

#endif

