#include "engine.h"
#include "constants.h"

u32 sphere;
u32 sphere2;

void init(){
        vec3 position = {0.0f, 0.0f, 0.0f};
        float radius = 1.0f;

        sphere = create_sphere(position, radius, color.star);
        sphere2 = create_sphere((vec3){10.0f, 0.0f, 0.0f}, 0.5f, color.red);
}

void update(){
        //move_entity(sphere2, (vec3){0.0f, sin(last_frame_time), 0.0f});
        position_update(sphere2, (vec3){10.0f, 2*sin(last_frame_time/1000.0), 0.0f});
}

void draw(){
        draw_entity(sphere);
        draw_entity(sphere2);
}

Engine engine = {
                 .init = &init,
                 .update = &update,
                 .draw = &draw
                };

int main(int argc, char** argv) {
        engine_run(engine);
        return 0;
}
