#ifndef RENDERER_H
#define RENDERER_H

#include <cglm/cglm.h>
#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <types.h>
#include <model.h>

void renderer_init();
void camera_move(vec3);
void camera_rotate(float, float);
void draw_object(const Color_RGBA color, GLenum mode, GLuint VAO, Model modelo);

#endif
