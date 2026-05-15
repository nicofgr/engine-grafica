#include "engine.h"
#include "cglm/vec3.h"
#include "types.h"
#include "constants.h"
#include "renderer/renderer.h"
#include <SDL2/SDL_video.h>

SDL_Window*   glWindow = NULL;
SDL_GLContext glContext = NULL;

int last_frame_time = 0;
int lastTime = 0;
vec3d original_origin;
float speed = 300000/100.0; // speed of light/10

typedef struct Object{  // Model, Position, Rotation, Scale
        u32   modelID;
        vec3d position;
        vec3  scale;
}Object;

typedef struct ObjectArray{
        Object* array;
        u32 size;
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
        glm_vec3_copy((float*)scale,    objectArray.array[index].scale);

        objectArray.size++;
        return index;
}

Object* ObjectArray_Get(const u32 objectID){
        if(objectID >= objectArray.size){
                fprintf(stderr, "[ERROR] objectID: %d out of bounds", objectID);
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

void draw_object(u32 objectID){
        Object* object = ObjectArray_Get(objectID);
        renderer_draw_model(object->modelID, object->position, object->scale);
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

void object_position_copy(u32 objectID, vec3d dest){
        vec3d_copy(objectArray.array[objectID].position, dest);
}

void engine_init(Engine engine) {
        vec3d_zero(original_origin);
        video_init();
        renderer_init();
        engine.init();
}

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
                                                speed *= 10;
                                        }
                                        else if(e.key.keysym.sym == SDLK_COMMA){
                                                speed /= 10;
                                        }
                                }
                                break;  
                }
        }

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
        camera_move(direction, speed * delta_time);

        int dx = 0, dy = 0;
        if(SDL_GetRelativeMouseMode() == SDL_TRUE)
                SDL_GetRelativeMouseState(&dx, &dy);

        camera_rotate(dx, dy, 0.2);
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

float delta_time = 0;
void engine_update(Engine engine){
        vec3 camPosf;
        camera_copy_position(camPosf);
        float mag_squared = glm_vec3_norm2(camPosf);
        if(mag_squared >= pow(10000,2)){ // if dist > 10000km
                vec3d camPos;
                vec3_to_vec3d(camPosf, camPos);
                move_origin(camPos);
        }

        int wait_time = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);
        if(wait_time > 0 && wait_time <= FRAME_TARGET_TIME)
                SDL_Delay(wait_time);

        delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;
        last_frame_time = SDL_GetTicks();

        engine.update();
        /**
        *step = 0;
        float currentTime = SDL_GetTicks();
        if(currentTime - lastTime >= 500.0f){ // Updates every x seconds
                *step = 1;
                lastTime = SDL_GetTicks();
        }
        **/
}


void engine_draw(Engine engine){
        renderer_draw();
        engine.draw();

        // GUI & TEXT
        vec3 cameraPos;
        camera_copy_position(cameraPos);
        vec3 origOriginf;
        vec3 distToOrigin;
        vec3d_to_vec3(original_origin, origOriginf);
        glm_vec3_add(origOriginf, cameraPos, distToOrigin);
        renderer_draw_GUI();
        char buffer[128];
        snprintf(buffer, 128, "Time:  %.2f sec", last_frame_time/1000.0f);
        render_text(buffer, -0.95f, 0.9f, 0.4f, color.orange);
        snprintf(buffer, 128, "dt:    %.2f ms", delta_time*1000.0f);
        render_text(buffer, -0.95f, 0.8f, 0.4f, color.orange);
        snprintf(buffer, 128, "Speed: %.2f km/s", speed); // unit/s
        render_text(buffer, -0.95f, 0.7f, 0.4f, color.orange);
        snprintf(buffer, 128, "Pos: %.2e %.2e %.2e km", distToOrigin[0], distToOrigin[1], distToOrigin[2]);
        //snprintf(buffer, 128, "Pos: %.2f %.2f %.2f km", cameraPos[0], cameraPos[1], cameraPos[2]);
        render_text(buffer, -0.95f, 0.6f, 0.4f, color.orange);
        snprintf(buffer, 128, "FPS: %.f", 1/delta_time);
        render_text(buffer, 0.7f, 0.9f, 0.4f, color.orange);

        SDL_GL_SwapWindow(glWindow);
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
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(glWindow);
        SDL_Quit();
        free(objectArray.array);
        renderer_quit();
}

void engine_run(Engine engine){
        int quit = FALSE;
        //int counter = 0;

        engine_init(engine);

        while(quit == FALSE){
                input(&quit);
                engine_update(engine);
                engine_draw(engine);
                //quit = TRUE;
        }
        engine_quit();
}

