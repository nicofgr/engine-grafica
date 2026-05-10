#include "renderer.h"
#include "types.h"
#include "model.h"
#include "constants.h"

#include <cglm/cglm.h>
#include <glad/glad.h>

const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"uniform float sizeMultiplier;\n"
"void main()\n"
"{\n"
"   gl_Position = projection*view*model*vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"   gl_PointSize = min((50*sizeMultiplier)/gl_Position.z, 400.0f);\n"
"}\0";

const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec4 ourColor;\n"
"void main()\n"
"{\n"
"   FragColor = ourColor;\n"
"}\0";

GLuint vertexShader;
GLuint fragmentShader;
GLuint shaderProgram;


vec3 camera_pos   = {0.0f, 0.0f,  7.0f};
vec3 camera_front = {0.0f, 0.0f, -1.0f};
vec3 camera_up    = {0.0f, 1.0f,  0.0f};

float yaw = -90.0f;
float pitch = 0.0f;

typedef struct Object{
        GLuint VAO;
        u32 nFaces;
}Object;

typedef struct ObjectArray{
        Object* array;
        u32 size;
}ObjectArray;

ObjectArray objectArray;

u32 objectArray_push(GLuint VAO, u32 nFaces){
        if(objectArray.size == 0){
                objectArray.array = (Object*) malloc(sizeof(Object));
        }else{
                objectArray.array = (Object*) realloc(objectArray.array, sizeof(Object)*(objectArray.size+1));
        }
        u32 index = objectArray.size;
        objectArray.array[index].VAO = VAO;
        objectArray.array[index].nFaces = nFaces;
        objectArray.size++;
        return index;
}
void compileShaders(){
        int success;
        char infoLog[512];
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if(!success){
                glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
                printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n %s\n", infoLog);
                printf("%d\n", glGetError());
                exit(1);
        }

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if(!success){
                glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
                printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n %s\n", infoLog);
                exit(1);
        }

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if(!success) {
                glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
                printf("ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n %s\n", infoLog);
                exit(1);
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
}


int vertexColorLocation;
int transformLocation;
int viewLocation; 
int projLocation;   
int sizeMultiplier;

void renderer_init(){
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

        compileShaders();
        glUseProgram(shaderProgram);
        vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
        transformLocation   = glGetUniformLocation(shaderProgram, "model");
        viewLocation        = glGetUniformLocation(shaderProgram, "view");
        projLocation        = glGetUniformLocation(shaderProgram, "projection");
        sizeMultiplier      = glGetUniformLocation(shaderProgram, "sizeMultiplier");

        objectArray.array = NULL;
        objectArray.size  = 0;
}

void renderer_quit(){
        free(objectArray.array);
}

void renderer_draw(){
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_CULL_FACE);
        // NOT TRANSPARENT FIRST
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);

        // TRANSPARENT OBJECTS
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        /**
        // WIREFRAMES
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.0, -1.0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //draw_object(color.orange, *model);

        glDisable(GL_POLYGON_OFFSET_LINE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        **/
}

void camera_move(vec3 direction){
        const float camera_speed = 0.1f;

        float x = direction[0]; 
        float y = direction[1]; 
        float z = direction[2]; 

        vec3 aux;
        glm_vec3_copy(camera_front, aux);
        aux[1] = 0.0f;
        glm_vec3_muladds(aux, x*camera_speed, camera_pos); //pos += (front*spd)

        glm_vec3_crossn(camera_front, camera_up, aux);
        glm_vec3_muladds(aux, y*camera_speed, camera_pos);

        glm_vec3_muladds(camera_up, z*camera_speed, camera_pos);
}

void camera_rotate(float x, float y){
        const float sensitivity = 0.25;
        yaw += x*sensitivity;
        pitch -= y*sensitivity;
        if(pitch > 89.9f)
                pitch = 89.9f;
        if(pitch < -89.9f)
                pitch = -89.9f;
}

vec3 translation = {0.0f, 0.0f, 0.0f};
vec3 scale = {1.0f, 1.0f, 1.0f};


Object objectArray_Get(u32 ID){
        return objectArray.array[ID];
}

void renderer_draw_object(const Color_RGBA color, u32 ID, vec3 position, vec3 scale){
        Object object = objectArray_Get(ID);

        mat4 model;
        glm_mat4_identity(model);
        glm_translate(model, position);
        float rotate_speed = -1.0f/10.0; // frequency
        glm_rotate(model, rotate_speed*((float)SDL_GetTicks()/1000.0f)*GLM_PI*2, (vec3){0.0f, 1.0f, 0.0f});
        glm_scale(model, scale);

        mat4 view;
        glm_mat4_identity(view);

        vec3 target_dir;
        vec3 direction;
        direction[0] = cos(glm_rad(yaw)) * cos(glm_rad(pitch));
        direction[1] = sin(glm_rad(pitch));
        direction[2] = sin(glm_rad(yaw)) * cos(glm_rad(pitch));
        glm_normalize_to(direction, camera_front);
        glm_vec3_add(camera_pos, camera_front, target_dir);
        glm_lookat(camera_pos, target_dir, camera_up, view);

        mat4 proj;
        glm_mat4_identity(proj);
        glm_perspective(glm_rad(FOV), (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.1f, 100.0f, proj);


        // MOVE GET UNIFORM LOCATION TO INIT
        //glUseProgram(shaderProgram);
        glUniform4f(vertexColorLocation, color.R, color.G, color.B, color.A);
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, (const float*)model);
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, (const float*)view);
        glUniformMatrix4fv(projLocation, 1, GL_FALSE, (const float*)proj);
        glUniform1f(sizeMultiplier, 1);

        glBindVertexArray(object.VAO);

        glDrawElements(GL_TRIANGLES, object.nFaces, GL_UNSIGNED_INT, 0);
}


GLuint renderer_create_VAO(const Model model){ // Create object from model
        GLuint VAO;
        GLuint VBO;
        GLuint EBO;
        // VAO / VBO / EBO ===========================================================
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*model.vertices.size, model.vertices.array, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32)*model.faces.size, model.faces.array, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        return VAO;
}

u32 renderer_create_sphere(){
        Model sphere = {.faces = 0, .vertices = NULL};

        model_create_sphere(&sphere);

        GLuint VAO = renderer_create_VAO(sphere);

        u32 ID = objectArray_push(VAO, sphere.faces.size);

        model_free(&sphere); 
        return ID;
}


void camera_print_coords(){
        printf("Pos: %.2f, %.2f, %.2f\n", camera_pos[0], camera_pos[1], camera_pos[2]);
        printf("Fnt: %.2f, %.2f, %.2f\n", camera_front[0], camera_front[1], camera_front[2]);
        printf("Up:  %.2f, %.2f, %.2f\n\n", camera_up[0], camera_up[1], camera_up[2]);
}

