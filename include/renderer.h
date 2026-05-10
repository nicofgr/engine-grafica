#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <types.h>
#include <model.h>

void renderer_init();
void renderer_quit();

void camera_move(vec3);
void camera_rotate(float, float);
void camera_print_coords();

void renderer_draw_object(const Color_RGBA color, u32 ID, vec3 position, vec3 scale);
u32 renderer_create_sphere();
void renderer_draw();

#endif
