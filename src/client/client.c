#include "client.h"
#include "cglm/box.h"
#include "cglm/quat.h"
#include "cglm/vec3.h"
#include "../shared/types.h"
#include "../shared/constants.h"
#include "../shared/network.h"
#include "renderer/renderer.h"
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <stdio.h>

SDL_Window*   glWindow  = NULL;
SDL_GLContext glContext = NULL;

int last_frame_time   = 0;
int last_physics_time = 0;
vec3d original_origin;
//float speed = 300000/100.0; // speed of light/10
//float speed = (SPEED_OF_C/1000.0)/64;         // speed of light
float speed = 10.0;
vec3d camera_global_pos;

typedef struct Object{   // Model, Position, Rotation, Scale
        u32    modelID;
        vec3d  position; // Relative to whatever origin ( NOT GLOBAL POSITION AND NOT RELATIVE TO CAMERA )
        versor rotation;
        vec3   scale;
}Object;

typedef struct ObjectArray{
        Object* array;
        u32     size;
}ObjectArray;

ObjectArray objectArray;

u32 ObjectArray_Push(const u32 modelID, vec3d position, const vec3 scale){
        if(objectArray.size == 0){
                objectArray.array = (Object*) malloc(sizeof(Object));
        }else{
                objectArray.array = (Object*) realloc(objectArray.array, sizeof(Object)*(objectArray.size+1));
        }

        u32 index = objectArray.size;
        objectArray.array[index].modelID = modelID;
        vec3d_copy(position, objectArray.array[index].position);
        versor rotation;
        glm_quat_identity(rotation);
        glm_quat_copy(rotation, objectArray.array[index].rotation);
        glm_vec3_copy((float*)scale,    objectArray.array[index].scale);

        objectArray.size++;
        return index;
}

Object* ObjectArray_Get(const u32 objectID){
        if(objectID >= objectArray.size){
                fprintf(stderr, "[ERROR][ObjectArray_Get] objectID: %d out of bounds", objectID);
                exit(0);
        }
        return &objectArray.array[objectID];
}

void video_init(){
        if(SDL_Init(SDL_INIT_EVERYTHING) != 0){
                printf("SDL2 could not initialize video subsystem\n");
                exit(1);
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

        glWindow = SDL_CreateWindow("OpenGL 3.3", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
        if(glWindow == NULL){
                printf("Error creating window\n");
                exit(1);
        }

        glContext = SDL_GL_CreateContext(glWindow);
        if(glContext == NULL){
                printf("Error creating GL context\n");
                exit(1);
        }
}



u32 create_material(Color_RGB ambient, Color_RGB diffuse, Color_RGB specular, float shininess, Color_RGB emission){
        return renderer_create_material(ambient, diffuse, specular, shininess, emission);
}

u32 check_duplicate_model(u32 objectID){
        Object* object = ObjectArray_Get(objectID);
        for(int i = 0; i < objectArray.size; i++){
                if(i == objectID)
                        continue;
                if(objectArray.array[i].modelID == object->modelID)
                        return TRUE;
        }
        return FALSE;
}

void object_update_model(u32 objectID, u32 modelID){
        objectArray.array[objectID].modelID = modelID;
}


void rotate_object(u32 objectID, versor rotation){
        versor orig;
        glm_quat_copy(objectArray.array[objectID].rotation, orig);
        glm_quat_mul(rotation, orig, objectArray.array[objectID].rotation);
}

void rotate_object_local(u32 objectID, versor rotation){
        versor orig;
        glm_quat_copy(objectArray.array[objectID].rotation, orig);
        glm_quat_mul(orig, rotation, objectArray.array[objectID].rotation);
}

void change_material(u32 objectID, u32 materialID){
        Object* object = ObjectArray_Get(objectID);
        if(check_duplicate_model(objectID) == TRUE){
                // create new model with new material
                u32 newModelID = renderer_duplicate_model(object->modelID);
                object_update_model(objectID, newModelID);
        }
        // edit model with new material
        renderer_change_material(object->modelID, materialID);
}

u32 create_sphere(vec3d position,  float radius){
        u32  modelID  = renderer_get_sphere();
        vec3 scale    = (vec3){radius, radius, radius};
        u32  objectID = ObjectArray_Push(modelID, position, scale);

        return objectID;
}

void move_object_local(u32 objectID, vec3d delta_pos){
        versor rotation;
        glm_quat_copy(objectArray.array[objectID].rotation, rotation);
        vec3 delta_pos_f;
        vec3d_to_vec3(delta_pos, delta_pos_f);
        glm_quat_rotatev(rotation, delta_pos_f, delta_pos_f);

        vec3_to_vec3d(delta_pos_f, delta_pos);
        vec3d_add(objectArray.array[objectID].position, delta_pos, objectArray.array[objectID].position);
}

#define MAXLOD 0
u32* create_patchtree(vec3d position, float scale, u32 maxLOD){
        maxLOD = MAXLOD;
        u32 size = (pow(4,maxLOD+1)-1)/3;
        printf("[PatchTree_init]\n");
        printf("Generating quadtree of size %d\n", size);
        u32* patchTree = (u32*)malloc(sizeof(u32)*size);

        // Root patch
        u32  modelID  = renderer_get_triangle();
        vec3 scale3   = (vec3){scale, scale, scale};
        u32  objectID = ObjectArray_Push(modelID, position, scale3);
        patchTree[0] = objectID;

        // Other patches
        for(int i = 1; i < size*0.25; i++){
                u32 parentID = i-1;
                u32 parentObjectID = patchTree[parentID];

                versor parent_rotation;
                glm_quat_copy(objectArray.array[parentObjectID].rotation, parent_rotation);

                float scale  = objectArray.array[parentObjectID].scale[0]*0.5;
                vec3 scale3   = (vec3){scale, scale, scale};

                vec3d position;
                vec3d_copy(objectArray.array[parentObjectID].position, position);

                u32  objectID1 = ObjectArray_Push(modelID, position, scale3);
                u32  objectID2 = ObjectArray_Push(modelID, position, scale3);
                u32  objectID3 = ObjectArray_Push(modelID, position, scale3);
                u32  objectID4 = ObjectArray_Push(modelID, position, scale3);

                rotate_object(objectID1, parent_rotation);
                rotate_object(objectID2, parent_rotation);
                rotate_object(objectID3, parent_rotation);
                rotate_object(objectID4, parent_rotation);

                move_object_local(objectID1, (vec3d){                0.0f,      scale, 0.0f}); // TOP
                move_object_local(objectID3, (vec3d){ scale*(0.5*sqrt(3)), -scale*0.5, 0.0f}); // LEFT
                move_object_local(objectID4, (vec3d){-scale*(0.5*sqrt(3)), -scale*0.5, 0.0f}); // RIGHT
                                                                                   //
                versor upsidedown;
                glm_quat(upsidedown, glm_rad(180), 0.0, 0.0, 1.0);
                rotate_object_local(objectID2, upsidedown);

                u32 blockstartID = (i-1)*4;
                patchTree[blockstartID + 1] = objectID1;
                patchTree[blockstartID + 2] = objectID2;
                patchTree[blockstartID + 3] = objectID3;
                patchTree[blockstartID + 4] = objectID4;
        }
        printf("patchtree created!\n");
        fflush(stdout);
        return patchTree;
}


float distance_to_object(u32 objectID){
        vec3 camera_pos;
        camera_copy_position(camera_pos);

        vec3 object_position;
        vec3d object_position_d;
        vec3d_copy(objectArray.array[objectID].position, object_position_d);
        vec3d_to_vec3(object_position_d, object_position);

        vec3 distance_vec;
        glm_vec3_sub(object_position, camera_pos, distance_vec);
        //printf("Obj pos: (%.2f %.2f %.2f)\n", object_position[0], object_position[1], object_position[2]);

        //printf(" Dis Vec: (%.2f %.2f %.2f)\n", distance_vec[0], distance_vec[1], distance_vec[2]);
        double distance = (distance_vec[0] * distance_vec[0]) + (distance_vec[1] * distance_vec[1]) + (distance_vec[2] * distance_vec[2]);
        distance = sqrt(distance);
        return distance;
}

void patch_subdivide(u32* patchTree, u32 patchID, u32 lod){
        if(lod == MAXLOD){ // Cant subdivide more
                draw_object(patchTree[patchID]);
                return;
        }
        for(int i = 1; i <= 4; i++){  // 1 to 4 inclusive
                u32 childID = 4*patchID + i;
                u32 childObjectID = patchTree[childID];
                float distance = distance_to_object(childObjectID);
                if(distance < (5000-lod*1000)){
                        patch_subdivide(patchTree, childID, lod+1);
                        continue;
                }
                draw_object(patchTree[childID]);
        }
}

void draw_patchtree(u32* patchTree){
        if(patchTree == NULL){
                fprintf(stderr,"[ERROR][draw_patchtree] The pointer has value NULL\n");
                exit(0);
        }
        float distance = distance_to_object(patchTree[0]);
        for(int i = 1; i < MAXLOD; i++){ // Tree level
                for(int j = 0; j < pow(4,i); j++){
                        
                }
        }
        if(distance < 5000){
                patch_subdivide(patchTree, 0, 0);
                return;
        }
        draw_object(patchTree[0]);
}

void draw_patch_sphere(u32** patchSphere){
        for(int i = 0; i < 20; i++){
                if(patchSphere[i] == NULL) continue;
                draw_patchtree(patchSphere[i]);
        }
}

u32** create_patch_sphere(){ // TODO Use the icosahedron mesh data to simplify this function

        u32** patchSphere = (u32**) malloc(sizeof(u32*) * 20);
        for(int i = 0; i < 20; i++){
                patchSphere[i] = NULL;
        }
        for(int i = 0; i < 20; i++){
                //patchSphere[i] = create_patchtree((vec3d){1.0f * i, 0.0f, GOLDEN_RATIO}, 1.0, 1);
        }

        // POSITIONS
        float den = 1.0/(3.0 * sqrt(GOLDEN_RATIO+2));
        float cst1 = GOLDEN_RATIO*den;       // 0.53934... 0.28355...
        float cst2 = (GOLDEN_RATIO+1)*den;   // 0.87268... 0.45879...
        float cst3 = (1+2*GOLDEN_RATIO)*den; // 1.41202... 0.74234...
        float scale = 0.5;
        patchSphere[0]  = create_patchtree((vec3d){ cst1,  0.0f,  cst3}, scale, 1);
        patchSphere[1]  = create_patchtree((vec3d){ cst2,  cst2,  cst2}, scale, 1);
        patchSphere[2]  = create_patchtree((vec3d){ 0.0f,  cst3,  cst1}, scale, 1);
        patchSphere[3]  = create_patchtree((vec3d){-cst2,  cst2,  cst2}, scale, 1);
        patchSphere[4]  = create_patchtree((vec3d){-cst1,  0.0f,  cst3}, scale, 1);
        patchSphere[5]  = create_patchtree((vec3d){ cst1,  0.0f, -cst3}, scale, 1);
        patchSphere[6]  = create_patchtree((vec3d){ cst2, -cst2, -cst2}, scale, 1);
        patchSphere[7]  = create_patchtree((vec3d){ 0.0f, -cst3, -cst1}, scale, 1);
        patchSphere[8]  = create_patchtree((vec3d){-cst2, -cst2, -cst2}, scale, 1);
        patchSphere[9]  = create_patchtree((vec3d){-cst1,  0.0f, -cst3}, scale, 1);
        patchSphere[10] = create_patchtree((vec3d){ cst2,  cst2, -cst2}, scale, 1);
        patchSphere[11] = create_patchtree((vec3d){ cst3,  cst1,  0.0f}, scale, 1);
        patchSphere[12] = create_patchtree((vec3d){ cst3, -cst1,  0.0f}, scale, 1);
        patchSphere[13] = create_patchtree((vec3d){ cst2, -cst2,  cst2}, scale, 1);
        patchSphere[14] = create_patchtree((vec3d){ 0.0f, -cst3,  cst1}, scale, 1);
        patchSphere[15] = create_patchtree((vec3d){-cst2, -cst2,  cst2}, scale, 1);
        patchSphere[16] = create_patchtree((vec3d){-cst3, -cst1,  0.0f}, scale, 1);
        patchSphere[17] = create_patchtree((vec3d){-cst3,  cst1,  0.0f}, scale, 1);
        patchSphere[18] = create_patchtree((vec3d){-cst2,  cst2, -cst2}, scale, 1);
        patchSphere[19] = create_patchtree((vec3d){ 0.0f,  cst3, -cst1}, scale, 1);

        // ROTATIONS
        for(int i = 0; i < 20; i++){ // WARNING at the current state this only works at origin and LOD 0
                u32 objectID = patchSphere[i][0];
                vec3d position;
                vec3d_copy(objectArray.array[patchSphere[i][0]].position, position);
                vec3 normal;
                vec3d_to_vec3(position, normal);
                versor rotation;
                glm_quat_from_vecs((vec3){0.0f, 0.0f, 1.0f},normal,rotation);
                rotate_object_local(objectID, rotation);
                glm_quatv(rotation, glm_rad(30), (vec3){0.0f, 0.0f, 1.0f});
                rotate_object_local(objectID, rotation);
        }
        return patchSphere;
}

void draw_object(u32 objectID){
        Object* object = ObjectArray_Get(objectID);
        renderer_draw_model(object->modelID, object->position, object->rotation, object->scale);
}

void move_object(u32 objectID, vec3d displacement){
        vec3d_add(displacement, objectArray.array[objectID].position, objectArray.array[objectID].position);
}


void position_update(u32 objectID, vec3d newPos){
        vec3d_copy(newPos, objectArray.array[objectID].position);
}

void print_position(u32 objectID){
        printf("(%.2f %.2f %.2f)\n", objectArray.array[objectID].position[0], objectArray.array[objectID].position[1], objectArray.array[objectID].position[2]);
}

void camera_position_get(vec3 dest){
        camera_copy_position(dest);
}

void engine_camera_position_set(vec3d position){
        renderer_set_camera_pos(position);
}

void object_position_copy(u32 objectID, vec3d dest){
        vec3d_copy(objectArray.array[objectID].position, dest);
}

void engine_init(Engine engine) {
        net_init();
        NetConn connection = net_connect(8080);

        vec3d_zero(original_origin);
        video_init();
        renderer_init();
        engine.init();
}

int mouseDeltaX;
int mouseDeltaY;
vec3 movementDirection;

void input(int * quit){
        SDL_Event e;
        const Uint8* states = SDL_GetKeyboardState(NULL);
        while(SDL_PollEvent(&e)){
                switch(e.type){
                        case SDL_QUIT:
                                *quit = TRUE;
                                break;
                        case SDL_KEYDOWN:
                                if(e.key.keysym.sym == SDLK_ESCAPE)
                                        *quit = TRUE;
                                if(e.key.keysym.sym == SDLK_TAB){
                                        if(SDL_GetRelativeMouseMode() == SDL_TRUE){
                                                SDL_SetRelativeMouseMode(SDL_FALSE);
                                        }else{
                                                SDL_SetRelativeMouseMode(SDL_TRUE);
                                                SDL_GetRelativeMouseState(NULL, NULL);
                                        }
                                }
                                if(e.key.repeat == 0){
                                        if(e.key.keysym.sym == SDLK_PERIOD){
                                                speed *= 2;
                                        }
                                        else if(e.key.keysym.sym == SDLK_COMMA){
                                                speed /= 2;
                                        }
                                }
                                break;  
                }
        }

        // MOUSE
        int dx = 0, dy = 0;
        if(SDL_GetRelativeMouseMode() == SDL_TRUE)
                SDL_GetRelativeMouseState(&dx, &dy);
        mouseDeltaX = dx;
        mouseDeltaY = dy;

        // MOVEMENT
        vec3 direction = {0.0f, 0.0f, 0.0f};
        if(states[SDL_SCANCODE_W]){
                direction[0] += 1.0f; 
        }
        if(states[SDL_SCANCODE_S]){
                direction[0] -= 1.0f; 
        }
        if(states[SDL_SCANCODE_A]){
                direction[1] -= 1.0f; 
        }
        if(states[SDL_SCANCODE_D]){
                direction[1] += 1.0f; 
        }
        if(states[SDL_SCANCODE_SPACE]){
                direction[2] += 1.0f; 
        }
        if(states[SDL_SCANCODE_LSHIFT]){
                direction[2] -= 1.0f; 
        }
        glm_normalize(direction);
        glm_vec3_copy(direction, movementDirection);
}

void move_origin(vec3d newOrigin){
        vec3d_add(original_origin, newOrigin, original_origin);
        for(int i = 0; i < objectArray.size; i++){
                vec3d_sub(objectArray.array[i].position, newOrigin, objectArray.array[i].position);
        }
        vec3 newLightPos;
        vec3d_to_vec3(objectArray.array[0].position, newLightPos);
        renderer_update_light_position(newLightPos);

        camera_move_to_origin();
}

float delta_time = 1.0/TARGET_PPS;
float counter = 0;
void engine_fixed_update(Engine engine){
        // MOVE ORIGIN
        vec3 camPosf;
        camera_copy_position(camPosf);
        float mag_squared = glm_vec3_norm2(camPosf);
        if(mag_squared >= pow(10000,2)){ // if dist > 10,000km
                vec3d camPos;
                vec3_to_vec3d(camPosf, camPos);
                move_origin(camPos);
        }

        // MOVEMENT AND ROTATION
        camera_rotate(mouseDeltaX, mouseDeltaY, 0.2);
        camera_move(movementDirection, speed*delta_time);


        vec3 cameraPos;
        camera_copy_position(cameraPos);
        vec3d camera_pos_double;
        vec3_to_vec3d(cameraPos, camera_pos_double);
        vec3d_add(original_origin, camera_pos_double, camera_global_pos);
        renderer_set_camera_world_pos(camera_global_pos);

        engine.update();

        //update_patches();

        counter += delta_time;
}

void engine_draw(Engine engine){
        u32 new_frame_time = SDL_GetTicks();  // milliseconds
        float delta_frame_time = (new_frame_time - last_frame_time)/1000.0f; // seconds
        last_frame_time = new_frame_time;

        renderer_draw();
        engine.draw();

        // GUI & TEXT
        vec3 cameraPos;
        camera_copy_position(cameraPos);

        float gpu_time = renderer_gpu_time();

        renderer_draw_GUI();
        char buffer[128];
        snprintf(buffer, 128, "Time:  %.2f sec", last_frame_time/1000.0f);
        render_text(buffer, -0.95f, 0.9f, 0.35f, color.orange);
        snprintf(buffer, 128, "CPU time: %.2f ms", delta_frame_time*1000.0);
        render_text(buffer, -0.95f, 0.83f, 0.35f, color.orange);
        snprintf(buffer, 128, "GPU time: %.2f ms", gpu_time);
        render_text(buffer, -0.95f, 0.76f, 0.35f, color.orange);

        if(speed*1000 > (float)SPEED_OF_C/10.0f){
                snprintf(buffer, 128, "Speed: %.2f c", (speed*1000)/(float)SPEED_OF_C); // unit/s
                render_text(buffer, -0.95f, 0.69f, 0.35f, color.orange);
        }else{
                snprintf(buffer, 128, "Speed: %.2f km/s", speed); // unit/s
                render_text(buffer, -0.95f, 0.69f, 0.35f, color.orange);
        }

        snprintf(buffer, 128, "Orig Pos:  %.2e %.2e %.2e km", camera_global_pos[0], camera_global_pos[1], camera_global_pos[2]);
        render_text(buffer, -0.95f, 0.62f, 0.35f, color.orange);
        snprintf(buffer, 128, "Local Pos: %.2f %.2f %.2f km", cameraPos[0], cameraPos[1], cameraPos[2]);
        render_text(buffer, -0.95f, 0.55f, 0.35f, color.orange);


        snprintf(buffer, 128, "PPS: %.f", 1/delta_time);
        render_text(buffer, 0.7f, 0.9f, 0.35f, color.orange);
        snprintf(buffer, 128, "FPS: %.f", 1/delta_frame_time);
        render_text(buffer, 0.7f, 0.83f, 0.35f, color.orange);
        snprintf(buffer, 128, "ctr: %.2f", counter);
        render_text(buffer, 0.7f, 0.76f, 0.35f, color.orange);

        SDL_GL_SwapWindow(glWindow);

        renderer_draw_finish();
}

void draw_point(vec3 position, Color_RGB color, float size){
        renderer_draw_point(position, color, size);
}

void distant_objects_point(){
        for(int i = 0; i < objectArray.size; i++){
                renderer_draw_point(objectArray.array[i].position, color.red, 1.0f);

        }
}

void engine_quit(){
        net_shutdown();
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(glWindow);
        SDL_Quit();
        free(objectArray.array);
        renderer_quit();
}

void engine_run(Engine engine){
        int quit = FALSE;

        engine_init(engine);

        int lastTime = SDL_GetTicks();
        int accumulator = 0;

        while(quit == FALSE){
                net_update();
                int newTime = SDL_GetTicks();
                int frameTime = newTime - lastTime;
                if(frameTime > 250)
                        frameTime = 250;
                lastTime = newTime;

                accumulator += frameTime;
                
                while(accumulator >= PHYSICS_TARGET_TIME){
                        input(&quit);   // TODO: Move input out of fixed time step but get the movement here;
                        engine_fixed_update(engine);
                        accumulator -= PHYSICS_TARGET_TIME;
                }
                engine_draw(engine);
        }
        engine_quit();
}

