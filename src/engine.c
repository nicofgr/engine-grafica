#include "engine.h"
#include "cglm/vec3.h"
#include "model.h"
#include "types.h"
#include "constants.h"
#include "renderer.h"

SDL_Window*   glWindow = NULL;
SDL_GLContext glContext = NULL;

int last_frame_time = 0;
int lastTime = 0;

typedef struct Entity{
        u32  modelID;
        vec3 position;
        vec3 scale;
        Color_RGBA color;
        float luminosity;
}Entity;

typedef struct EntityArray{
        Entity* array;
        u32 size;
}EntityArray;

EntityArray entityArray;

u32 EntityArray_Push(const u32 modelID, const vec3 position, const vec3 scale, const Color_RGBA color, const float luminosity){
        if(entityArray.size == 0){
                entityArray.array = (Entity*) malloc(sizeof(Entity));
        }else{
                entityArray.array = (Entity*) realloc(entityArray.array, sizeof(Entity)*(entityArray.size+1));
        }

        u32 index = entityArray.size;
        entityArray.array[index].modelID = 0;
        glm_vec3_copy((float*)position, entityArray.array[index].position);
        glm_vec3_copy((float*)scale, entityArray.array[index].scale);
        entityArray.array[index].color = color;
        entityArray.array[index].luminosity = luminosity;

        entityArray.size++;
        return index;
}

Entity EntityArray_Get(const u32 entityID){
        if(entityID >= entityArray.size){
                fprintf(stderr, "[ERROR] entityID: %d out of bounds", entityID);
                exit(0);
        }
        return entityArray.array[entityID];
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

u32 create_sphere(vec3 position,  float radius, Color_RGBA color, float luminosity){

        u32 modelID = renderer_create_sphere();
        vec3 scale = (vec3){radius, radius, radius};
        u32 entityID = EntityArray_Push(modelID, position, scale, color, luminosity);

        return entityID;
}

void draw_entity(u32 entityID){
        Entity entity = EntityArray_Get(entityID);
        renderer_draw_object(entity.color, entity.modelID, entity.position, entity.scale, entity.luminosity);
}

void move_entity(u32 entityID, vec3 displacement){
        glm_vec3_add(displacement, entityArray.array[entityID].position, entityArray.array[entityID].position);
}

void position_update(u32 entityID, vec3 newPos){
        glm_vec3_copy(newPos, entityArray.array[entityID].position);
}

void engine_init(Engine engine) {
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
        camera_move(direction);

        int x = 0, y = 0;
        if(SDL_GetRelativeMouseMode() == SDL_TRUE)
                SDL_GetRelativeMouseState(&x, &y);

        camera_rotate(x, y);

        /**
        if(0){  // for debugging
                printf("%d, %d\n", x, y);
                camera_print_coords();
        }**/
}

float delta_time = 0;
void engine_update(Engine engine){

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
        renderer_draw_GUI();
        SDL_GL_SwapWindow(glWindow);
}

void engine_quit(){
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(glWindow);
        SDL_Quit();
        free(entityArray.array);
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

