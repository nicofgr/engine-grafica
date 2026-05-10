#include "engine.h"
#include "constants.h"

u32 sphere;

void init(){
        vec3 position = {0.0f, 0.0f, 0.0f};
        float radius = 1.0f;

        sphere = create_sphere(position, radius, color.star);
}

void update(){
}

void draw(){
        draw_entity(sphere);
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
