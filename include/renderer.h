#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <types.h>
#include <model.h>

void renderer_init();
void camera_move(vec3);
void camera_rotate(float, float);
void draw_object(const Color_RGBA color, Model modelo);
void object_create(Model* model);
void renderer_draw(Model* model);
void camera_print_coords();

#endif
