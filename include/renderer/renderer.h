#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <types.h>
#include <mesh.h>

void renderer_init();
void renderer_quit();

// Camera
void camera_move(vec3 direction, float speed);
void camera_rotate(int dx, int dy, float sensitivity);
void camera_print_coords();
void camera_copy_position(vec3 dest);
void camera_move_to_origin();
void camera_change_up(vec3 direction);
void renderer_set_camera_world_pos(vec3d position);

void renderer_draw_model(const u32 modelID, vec3d position, vec3 scale);
u32  renderer_duplicate_model(u32 modelID);
//u32 renderer_create_sphere();
u32  renderer_get_sphere();

void renderer_draw();
void renderer_draw_finish();
void render_text(const char* text, float x, const float y, const float scale, const Color_RGB color);
void renderer_draw_GUI();
void renderer_draw_point(vec3 position, Color_RGB color, float size);
void renderer_draw_point_setup();
float renderer_gpu_time();

// Materials
u32  renderer_create_material(Color_RGB ambient, Color_RGB diffuse, Color_RGB specular, float shininess, Color_RGB emission);
void renderer_change_material(u32 modelID, u32 materialID);

// Lights
void renderer_update_light_position(vec3 position);

#endif
