#include "client.h"
#include "../shared/constants.h"
#include "../shared/types.h"
#include "network.h"
#include "time.h"

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

// STRUCTS
typedef struct Particle{
        float lastx;
        float lasty;
        float x;
        float y;
        float pBestx;
        float pBesty;
        float gBestx;
        float gBesty;
        float velx;
        float vely;
}Particle;

// PROTOTYPES
int PSO_init(Particle** pArray);
int PSO_update(Particle* pArray);
int PSO_draw(Particle* pArray);

// GLOBALS
Particle* pArray;

void init(){
        /**
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
        mars    = create_sphere((vec3d){   228.0e6, 0.0f, 0.0f},   3389.5f);
        jupiter = create_sphere((vec3d){   778.0e6, 0.0f, 0.0f},  69886.0f);
        saturn  = create_sphere((vec3d){    1.43e9, 0.0f, 0.0f},  58232.0f);
        uranus  = create_sphere((vec3d){    2.87e9, 0.0f, 0.0f},  25362.8f);
        neptune = create_sphere((vec3d){     4.5e9, 0.0f, 0.0f},  24622.0f);
        pluto   = create_sphere((vec3d){     5.9e9, 0.0f, 0.0f},   2376.6f);

        u32 glowingMat = create_material(color.star, color.star, color.star, 32, (Color_RGB){50.0, 50.0, 50.0});
        change_material(sun, glowingMat);
        **/
        PSO_init(&pArray);
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
        PSO_update(pArray);
}

void draw(){
        /**
        draw_object(sun);
        draw_object(mercury);
        draw_object(venus);
        draw_object(earth);
        //draw_patch_sphere(patchSphere);
        draw_object(mars);
        draw_object(jupiter);
        draw_object(saturn);
        draw_object(uranus);
        draw_object(neptune);
        draw_object(pluto);
        **/
        PSO_draw(pArray);
}

Engine engine = {
                 .init = &init,
                 .update = &update,
                 .draw = &draw
                };

int main(int argc, char** argv) {
        srand(time(NULL));
        engine_run(engine);

        free(pArray);
        return 0;
}


// AI STUFF
int coords[][2] = {{2,3}, 
                   {8,7}, 
                   {6,2}};

float funcao_objetivo(float x, float y){
        int nRows = sizeof(coords) / sizeof(coords[0]);
        float dist = 0;

        for(int i = 0; i < nRows; i++){
                dist += sqrt(pow( x - coords[i][0] ,2) + pow( y - coords[i][1] ,2));
        }

        return dist;
}


int PSO_init(Particle** pArray){
        int nParticles = 20; // Dont change 
        int bup = 10;
        int blo = 0;

        *pArray = (Particle*) malloc( nParticles * sizeof( Particle ));

        for(int i = 0; i < nParticles; i++){
                (*pArray)[i].x = rand() % bup;
                (*pArray)[i].y = rand() % bup;
                (*pArray)[i].pBestx = (*pArray)[i].x;
                (*pArray)[i].pBesty = (*pArray)[i].y;
                (*pArray)[i].lastx = (*pArray)[i].x;
                (*pArray)[i].lasty = (*pArray)[i].y;
        }

        // FIND AND UPDATE SWARM BEST GLOBAL POSITION
        float gBestx = (*pArray)[0].pBestx;
        float gBesty = (*pArray)[0].pBesty;
        float gBest = funcao_objetivo((*pArray)[0].pBestx, (*pArray)[0].pBesty);

        for(int i = 0; i < nParticles; i++){
                float currDist = funcao_objetivo((*pArray)[i].pBestx, (*pArray)[i].pBesty);
                if(currDist < gBest){
                        gBest = currDist;
                        gBestx = (*pArray)[i].pBestx;
                        gBesty = (*pArray)[i].pBesty;
                }
        }

        for(int i = 0; i < nParticles; i++){
                (*pArray)[i].gBestx = gBestx;
                (*pArray)[i].gBesty = gBesty;
        }
        // ===============================================
        // INITIALIZE VELOCITIES 
        for(int i = 0; i < nParticles; i++){
                (*pArray)[i].velx = (rand() % (2*bup)) - bup;
                (*pArray)[i].vely = (rand() % (2*bup)) - bup;
        }
        // ==============================================
        for(int i = 0; i < nParticles; i++){
                printf("Particle [%d]: \n", i);
                printf("Position:  (%.2f %.2f) \n", (*pArray)[i].x, (*pArray)[i].y);
                printf("Pers Best: (%.2f %.2f) \n", (*pArray)[i].pBestx, (*pArray)[i].pBesty);
                printf("Glob Best: (%.2f %.2f) \n", (*pArray)[i].gBestx, (*pArray)[i].gBesty);
                printf("Velocity:  (%.2f %.2f) \n", (*pArray)[i].velx, (*pArray)[i].vely);
                printf("Score:     %.2f\n", funcao_objetivo((*pArray)[i].x, (*pArray)[i].y));
                puts("");
        }

        return 0;
}

int PSO_update(Particle* pArray){
        float w    = 0.3f; // Inertia Weight (smaller than 1)
        float phiP = 3.0f; // Cognitive coefficient
        float phiG = 1.0f; // Social coefficient
        int nParticles = 20; // Dont change

        for(int i = 0; i < nParticles; i++){
                double rp = rand() / (double)RAND_MAX ;
                double rg = rand() / (double)RAND_MAX ;
                // Update the particles velocity
                pArray[i].velx = w*pArray[i].velx + phiP * rp * (pArray[i].pBestx - pArray[i].x) + phiG * rg * (pArray[i].gBestx - pArray[i].x);
                pArray[i].vely = w*pArray[i].vely + phiP * rp * (pArray[i].pBesty - pArray[i].y) + phiG * rg * (pArray[i].gBesty - pArray[i].y);
                // Update the particles position
                pArray[i].lastx = pArray[i].x;
                pArray[i].lasty = pArray[i].y;
                pArray[i].x += pArray[i].velx;
                pArray[i].y += pArray[i].vely;

                if(funcao_objetivo(pArray[i].x, pArray[i].y) < funcao_objetivo(pArray[i].pBestx, pArray[i].pBesty)){
                        pArray[i].pBestx = pArray[i].x;
                        pArray[i].pBesty = pArray[i].y;
                        if(funcao_objetivo(pArray[i].pBestx, pArray[i].pBesty) < funcao_objetivo(pArray[i].gBestx, pArray[i].gBesty)){
                                for(int j = 0; j < nParticles; j++){
                                        pArray[j].gBestx = pArray[i].pBestx;
                                        pArray[j].gBesty = pArray[i].pBesty;
                                }
                        }
                }       
        }
        printf("%f\n", funcao_objetivo(pArray[0].gBestx, pArray[0].gBesty));
        return 0;
}

int PSO_draw(Particle* pArray){
        int nParticles = 20;
        for(int i = 0; i < nParticles; i++){
                float inter = get_interpolator();
                float x = pArray[i].lastx + inter*(pArray[i].x - pArray[i].lastx);
                float y = pArray[i].lasty + inter*(pArray[i].y - pArray[i].lasty);
                engine_draw_circle(3 + i, x, y);
        }
        engine_draw_circle(3 + nParticles, pArray[0].gBestx, pArray[0].gBesty);

        return 0;
}
