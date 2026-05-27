#include "client.h"
#include "../shared/constants.h"
#include "../shared/types.h"
#include "network.h"

u32 sun;
u32 mercury;
u32 venus;
u32 earth;
u32 mars;
u32 jupiter;
u32 saturn;
u32 uranus;
u32 neptune;
u32 pluto;

u32** patchSphere = NULL;

void init(){
        engine_camera_position_set((vec3d){149.6e6, 6371.9f, 0.0f});
        engine_camera_position_set((vec3d){0.0f, 0.0f, 0.0f});
        //sun     = create_sphere((vec3d){0.0f, 0.0f, 0.0f},       695700.0f);
        sun     = create_sphere((vec3d){0.0f, 0.0f, 0.0f},       1.0f);
        mercury = create_sphere((vec3d){    57.9e6, 0.0f, 0.0f},   2439.7f);
        venus   = create_sphere((vec3d){   108.0e6, 0.0f, 0.0f},   6051.8f);
        earth   = create_sphere((vec3d){   149.6e6, 0.0f, 0.0f},   6371.8f);
        //patchSphere = create_patch_sphere((vec3d){0.0f, 0.0f, 0.0f}, 1.0f);
        patchSphere = create_patch_sphere();
        //versor rotation;
        //glm_quat(rotation, glm_rad(80), 0.0, 1.0, 0.0);
        //rotate_patch(patch, rotation);
        /**
        mars    = create_sphere((vec3d){   228.0e6, 0.0f, 0.0f},   3389.5f);
        jupiter = create_sphere((vec3d){   778.0e6, 0.0f, 0.0f},  69886.0f);
        saturn  = create_sphere((vec3d){    1.43e9, 0.0f, 0.0f},  58232.0f);
        uranus  = create_sphere((vec3d){    2.87e9, 0.0f, 0.0f},  25362.8f);
        neptune = create_sphere((vec3d){     4.5e9, 0.0f, 0.0f},  24622.0f);
        pluto   = create_sphere((vec3d){     5.9e9, 0.0f, 0.0f},   2376.6f);
        **/

        u32 glowingMat = create_material(color.star, color.star, color.star, 32, (Color_RGB){50.0, 50.0, 50.0});
        change_material(sun, glowingMat);
}

void update(){
        /**
        //position_update(mercury, (vec3){5*cos(2*last_frame_time/1000.0), 0.0f, 5*sin(2*last_frame_time/1000.0)});
        //position_update(venus, (vec3){10*cos(last_frame_time/1000.0), 0.0f, 10*sin(last_frame_time/1000.0)});
        vec3 up;
        vec3 camera_pos;
        vec3 earth_pos;
        vec3d earth_pos_d;
        object_position_copy(3, earth_pos_d);
        vec3d_to_vec3(earth_pos_d, earth_pos);

        camera_position_get(camera_pos);
        glm_vec3_sub(camera_pos, earth_pos, up);
        glm_normalize(up);
        //camera_change_up(up);
        **/
}

void draw(){
        draw_object(sun);
        draw_object(mercury);
        draw_object(venus);
        draw_object(earth);
        //draw_patch_sphere(patchSphere);
        /**
        draw_object(mars);
        draw_object(jupiter);
        draw_object(saturn);
        draw_object(uranus);
        draw_object(neptune);
        draw_object(pluto);
        **/
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
