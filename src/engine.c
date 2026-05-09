#include "engine.h"
#include "cglm/vec3.h"
#include "model.h"
#include "types.h"
#include "constants.h"
#include "renderer.h"

Color_RGBA red_color = {0.85f, 0.02f, 0.12f, 0.5f};
//Color_RGBA green_color = {0.20f, 1.0f, 0.69f, 0.5f};
Color_RGBA green_color = {0.20f, 1.0f, 0.69f, 1.0f};
Color_RGBA color_orange = {0.9f, 0.6f, 0.0f, 1.0f};
Color_RGBA color_star = {1.0f, 0.7f, 0.4f, 1.0f};
SDL_Window*   glWindow = NULL;
SDL_GLContext glContext = NULL;


GLuint VAO;
GLuint VBO;
GLuint EBO;
int last_frame_time = 0;
int lastTime = 0;
Model model;


void object_create(){

        // VAO / VBO / EBO ===========================================================
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        model_create_sphere(&model);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*model.vertices.size, model.vertices.array, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32)*model.faces.size, model.faces.array, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
}

void init() {
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

        if(!gladLoadGLLoader(SDL_GL_GetProcAddress)){
                puts("glad was not initialized");
                exit(1);
        }
        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


        // SHADERS =============================================================
        renderer_init();

        object_create();
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
                printf("Pos: %.2f, %.2f, %.2f\n", camera_pos[0], camera_pos[1], camera_pos[2]);
                printf("Fnt: %.2f, %.2f, %.2f\n", camera_front[0], camera_front[1], camera_front[2]);
                printf("Up:  %.2f, %.2f, %.2f\n\n", camera_up[0], camera_up[1], camera_up[2]);
        }**/
}

void update(int* step){

        int wait_time = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);
        if(wait_time > 0 && wait_time <= FRAME_TARGET_TIME)
                SDL_Delay(wait_time);

        float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;
        last_frame_time = SDL_GetTicks();

        *step = 0;
        float currentTime = SDL_GetTicks();
        if(currentTime - lastTime >= 500.0f){ // Updates every x seconds
                *step = 1;
                lastTime = SDL_GetTicks();
        }
}


void draw(const int step){
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_CULL_FACE);
        // NOT TRANSPARENT FIRST
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);

        // TRANSPARENT OBJECTS
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        draw_object(color_star, GL_TRIANGLES,VAO,model);

        // WIREFRAMES
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.0, -1.0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        draw_object(color_orange, GL_TRIANGLES, VAO, model);

        glDisable(GL_POLYGON_OFFSET_LINE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        SDL_GL_SwapWindow(glWindow);
}

void engine_quit(){
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(glWindow);
        SDL_Quit();
}
