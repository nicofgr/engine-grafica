#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <types.h>
#include <mesh.h>

void renderer_init();
void renderer_quit();

// Camera
void camera_move(vec3 direction, float speed);
void camera_rotate(float, float);
void camera_print_coords();
float* camera_get_position();

void renderer_draw_model(const u32 modelID, vec3d position, vec3 scale);
u32  renderer_duplicate_model(u32 modelID);
//u32 renderer_create_sphere();
u32  renderer_get_sphere();

void renderer_draw();
void render_text(const char* text, float x, const float y, const float scale, const Color_RGB color);
void renderer_draw_GUI();

// Materials
u32  renderer_create_material(Color_RGB ambient, Color_RGB diffuse, Color_RGB specular, float shininess, Color_RGB emission);
void renderer_change_material(u32 modelID, u32 materialID);

#endif
