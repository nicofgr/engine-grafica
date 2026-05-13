#include "engine.h"
#include "constants.h"
#include "types.h"

u32 sun;
u32 mercury;
u32 venus;

void init(){
        vec3d position = {0.0f, 0.0f, 0.0f};

        /**
        sun     = create_sphere(position, 695700.0f);
        mercury = create_sphere((vec3d){350000.0f, 0.0f, 0.0f}, 2439.7f);
        venus =   create_sphere((vec3d){360000.0f, 0.0f, 0.0f}, 6051.8f);
        **/
        sun     = create_sphere(position, 200.0f);
        mercury = create_sphere((vec3d){400.0f, 0.0f, 0.0f}, 20.7f);
        venus =   create_sphere((vec3d){600.0f, 0.0f, 0.0f}, 60.8f);

        u32 glowingMat = create_material(color.star, color.star, color.star, 32, (Color_RGB){50.0, 50.0, 50.0});
        change_material(sun, glowingMat);
}

void update(){
        //position_update(mercury, (vec3){5*cos(2*last_frame_time/1000.0), 0.0f, 5*sin(2*last_frame_time/1000.0)});
        //position_update(venus, (vec3){10*cos(last_frame_time/1000.0), 0.0f, 10*sin(last_frame_time/1000.0)});
}

void draw(){
        draw_object(sun);
        draw_object(mercury);
        draw_object(venus);
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
