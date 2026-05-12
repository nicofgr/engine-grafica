#include "engine.h"
#include "constants.h"

u32 sun;
u32 planet1;
u32 planet2;

void init(){
        vec3 position = {0.0f, 0.0f, 0.0f};

        sun = create_sphere(position, 2.0f, color.star, 2.0f);
        planet1 = create_sphere((vec3){5.0f, 0.0f, 0.0f}, 0.2f, color.red, 0.0f);
        planet2 = create_sphere((vec3){10.0f, 0.0f, 0.0f}, 0.5f, color.red, 0.0f);
}

void update(){
        //position_update(planet1, (vec3){5*cos(2*last_frame_time/1000.0), 0.0f, 5*sin(2*last_frame_time/1000.0)});
        //position_update(planet2, (vec3){10*cos(last_frame_time/1000.0), 0.0f, 10*sin(last_frame_time/1000.0)});
}

void draw(){
        draw_object(sun);
        draw_object(planet1);
        draw_object(planet2);
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
