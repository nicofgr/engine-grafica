#include "renderer/renderer.h"
#include "cglm/cam.h"
#include "types.h"
#include "model.h"
#include "constants.h"
#include "renderer/shader.h"
#include "renderer/text.h"

#include <cglm/cglm.h>
#include <glad/glad.h>

GLuint shaderProgram;
GLuint textShader;

vec3 camera_pos   = {0.0f, 0.0f,  7.0f};
vec3 camera_front = {0.0f, 0.0f, -1.0f};
vec3 camera_up    = {0.0f, 1.0f,  0.0f};

float yaw = -90.0f;
float pitch = 0.0f;

typedef struct Object{
        GLuint VAO;
        u32    nFaces;
}Object;

typedef struct ObjectArray{
        Object* array;
        u32     size;
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
        shaderProgram = shCreateShaderProgram("shaders/simple_shader.vert", "shaders/simple_shader.frag");
        textShader    = shCreateShaderProgram("shaders/text.vert", "shaders/text.frag");
}


//int vertexLightLocation;
//int lightPosLocation;
int transformLocation;
int viewPos;
int viewLocation; 
int projLocation;   
int sizeMultiplier;

int luminosityLoc;
int ambientLoc;
int diffuseLoc;
int specularLoc;
int shininessLoc;

int lightPos;
int lightAmbient;
int lightDiffuse;
int lightSpecular;

GLuint textVAO, textVBO;
int textColor;
int projection;

void setup_text(){
        glUseProgram(textShader);
        text_init();
        glUniform1i(glGetUniformLocation(textShader, "text"), 0);
        glGenVertexArrays(1, &textVAO);
        glGenBuffers(1, &textVBO);
        glBindVertexArray(textVAO);
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*6*4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        textColor = glGetUniformLocation(textShader, "textColor");

        mat4 ortho;
        glm_ortho(0.0f, SCREEN_WIDTH, 0.0f, SCREEN_HEIGHT, -1.0f, 1.0f, ortho);
        projection = glGetUniformLocation(textShader, "projection");
        glUniformMatrix4fv(projection, 1, GL_FALSE, (const float*) ortho );
}

void setup_shaders(){
        compileShaders();
        glUseProgram(shaderProgram);
        lightPos      = glGetUniformLocation(shaderProgram, "light.position");
        lightAmbient  = glGetUniformLocation(shaderProgram, "light.ambient");
        lightDiffuse  = glGetUniformLocation(shaderProgram, "light.diffuse");
        lightSpecular = glGetUniformLocation(shaderProgram, "light.specular");

        transformLocation   = glGetUniformLocation(shaderProgram, "model");
        viewLocation        = glGetUniformLocation(shaderProgram, "view");
        projLocation        = glGetUniformLocation(shaderProgram, "projection");
        sizeMultiplier      = glGetUniformLocation(shaderProgram, "sizeMultiplier");
        viewPos             = glGetUniformLocation(shaderProgram, "viewPos");

        luminosityLoc       = glGetUniformLocation(shaderProgram, "luminosity");
        ambientLoc          = glGetUniformLocation(shaderProgram, "material.ambient");
        diffuseLoc          = glGetUniformLocation(shaderProgram, "material.diffuse");
        specularLoc         = glGetUniformLocation(shaderProgram, "material.specular");
        shininessLoc        = glGetUniformLocation(shaderProgram, "material.shininess");

        glUniform3f(lightPos, 0.0f, 0.0f, 0.0f);
        glUniform3f(lightAmbient, 0.2*color.star.R, 0.2*color.star.G, 0.2*color.star.B); // Light color
        glUniform3f(lightDiffuse, color.star.R, color.star.G, color.star.B); // Light color
        glUniform3f(lightSpecular, 1.0f, 1.0f, 1.0f); // Light color

        objectArray.array = NULL;
        objectArray.size  = 0;

}

void renderer_init(){
        // GLAD
        if(!gladLoadGLLoader(SDL_GL_GetProcAddress)){
                puts("glad was not initialized");
                exit(1);
        }

        // OpenGL
        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // SHADER
        setup_shaders();

        // FreeType
        setup_text();
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
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_DEPTH_TEST);
        **/
}

void renderer_draw_GUI(){
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        render_text("test", 0.0f, 0.0f, 1.0f, color.orange);
        render_text("test", 0.5f, 0.5f, 1.0f, color.orange);
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

void renderer_draw_object(const Color_RGBA color, u32 ID, vec3 position, vec3 scale, float luminosity){
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
        glUseProgram(shaderProgram);
        glUniform3f(ambientLoc, color.R, color.G, color.B);
        glUniform3f(diffuseLoc, color.R, color.G, color.B);
        glUniform3f(specularLoc, color.R, color.G, color.B);
        glUniform1f(shininessLoc, 32);
        glUniform1f(luminosityLoc, luminosity);
        glUniform3f(viewPos, camera_pos[0], camera_pos[1], camera_pos[2]);
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, (const float*)model);
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, (const float*)view);
        glUniformMatrix4fv(projLocation, 1, GL_FALSE, (const float*)proj);
        glUniform1f(sizeMultiplier, 1);

        glBindVertexArray(object.VAO);

        glDrawElements(GL_TRIANGLES, object.nFaces, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
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

        Vec3_Array VBO_data = {.array = NULL, .size = 0};
        for(int i = 0; i < model.vertices.size; i++){
                Vec3Array_Push(&VBO_data, model.vertices.array[i]);
                Vec3Array_Push(&VBO_data, model.normals.array[i]);
        }
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*model.vertices.size*2, VBO_data.array, GL_STATIC_DRAW);
        free(VBO_data.array);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32)*model.faces.size*3, model.faces.array, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        return VAO;
}

u32 renderer_create_sphere(){
        Model sphere = {.faces = {0}, .vertices = {NULL}};

        model_create_sphere(&sphere);

        GLuint VAO = renderer_create_VAO(sphere);

        u32 ID = objectArray_push(VAO, sphere.faces.size*3);

        model_free(&sphere); 
        return ID;
}


void camera_print_coords(){
        printf("Pos: %.2f, %.2f, %.2f\n", camera_pos[0], camera_pos[1], camera_pos[2]);
        printf("Fnt: %.2f, %.2f, %.2f\n", camera_front[0], camera_front[1], camera_front[2]);
        printf("Up:  %.2f, %.2f, %.2f\n\n", camera_up[0], camera_up[1], camera_up[2]);
}

void render_text(const char* text, float x, float y, const float scale, const Color_RGBA color){
        x = x*SCREEN_WIDTH;
        y = y*SCREEN_HEIGHT;
        glUseProgram(textShader);

        glUniform3f(textColor, color.R, color.G, color.B);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(textVAO);

        for(const char* c = text; *c; c++){
                Character ch = CharMap_Get(*c);

                float xpos = x + ch.Bearing[0] * scale;
                float ypos = y - (ch.Size[1] - ch.Bearing[1]) * scale;

                float w = ch.Size[0] * scale;
                float h = ch.Size[1] * scale;

                float vertices[6][4] = {
                        { xpos,     ypos + h, 0.0f, 0.0f },
                        { xpos,     ypos,     0.0f, 1.0f },
                        { xpos + w, ypos,     1.0f, 1.0f },

                        { xpos,     ypos + h, 0.0f, 0.0f },
                        { xpos + w, ypos,     1.0f, 1.0f },
                        { xpos + w, ypos + h, 1.0f, 0.0f },
                };

                glBindTexture(GL_TEXTURE_2D, ch.TextureID);
                glBindBuffer(GL_ARRAY_BUFFER, textVBO);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                x += (ch.Advance >> 6) * scale;
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D,0);
}
