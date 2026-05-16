#include "renderer/renderer.h"
#include "cglm/cam.h"
#include "cglm/quat.h"
#include "cglm/vec3.h"
#include "types.h"
#include "mesh.h"
#include "constants.h"
#include "renderer/shader.h"
#include "renderer/text.h"

#include <cglm/cglm.h>
#include <glad/glad.h>

GLuint shaderProgram;
GLuint textShader;
GLuint pointShader;

//vec3  camera_pos   = {149.6e6, 0.0f,  50000.0f};
vec3  camera_pos   = {149.6e6, 6840.0f, 0.2f};
vec3  gravity_up   = {0.0f, 1.0f,  0.0f};
versor qCamera;

float frustrumNear = 0.1;
float frustrumFar = 1e12;

// MODEL
typedef struct Model{ // Meshes, textures, materials, rig, shaders, uv mapping
        // Mesh
        GLuint VAO;
        u32    nFaces;
        // Material
        u32 materialID;
}Model;

typedef struct ModelArray{
        Model* array;
        u32     size;
}ModelArray;

ModelArray modelArray;

// MATERIAL
typedef struct Material{
        Color_RGB ambient;
        Color_RGB diffuse;
        Color_RGB specular;
        float     shininess;
        Color_RGB emission;
}Material;

typedef struct MaterialArray{
        Material* array;
        u32       size;
}MaterialArray;

MaterialArray materialArray;

// PROTOTYPES
u32 renderer_create_sphere();

// DEFINITIONS
u32 modelArray_push(GLuint VAO, u32 nFaces, u32 materialID){
        if(modelArray.size == 0){
                modelArray.array = (Model*) malloc(sizeof(Model));
        }else{
                modelArray.array = (Model*) realloc(modelArray.array, sizeof(Model)*(modelArray.size+1));
        }
        u32 index = modelArray.size;
        modelArray.array[index].VAO        = VAO;
        modelArray.array[index].nFaces     = nFaces;
        modelArray.array[index].materialID = materialID;
        modelArray.size++;
        return index;
}

Model modelArray_Get(u32 modelID){
        return modelArray.array[modelID];
}

u32 materialArray_push(Color_RGB ambient, Color_RGB diffuse, Color_RGB specular, float shininess, Color_RGB emission){
        if(materialArray.size == 0){
                materialArray.array = (Material*) malloc(sizeof(Material));
        }else{
                materialArray.array = (Material*) realloc(materialArray.array, sizeof(Material)*(materialArray.size+1));
        }
        u32 index = materialArray.size;
        materialArray.array[index].ambient   = ambient;
        materialArray.array[index].diffuse   = diffuse;
        materialArray.array[index].specular  = specular;
        materialArray.array[index].shininess = shininess;
        materialArray.array[index].emission  = emission;
        materialArray.size++;
        return index;
}

Material materialArray_Get(u32 materialID){
        if(materialID >= materialArray.size){
                fprintf(stderr, "[ERROR] materialID (%d) greater than materialArray size (%d)\n", materialID, materialArray.size);
                exit(0);
        }
        return materialArray.array[materialID];
}

void compileShaders(){
        shaderProgram = shCreateShaderProgram("shaders/simple_shader.vert", "shaders/simple_shader.frag");
        textShader    = shCreateShaderProgram(         "shaders/text.vert",          "shaders/text.frag");
        pointShader   = shCreateShaderProgram( "shaders/point_shader.vert",  "shaders/point_shader.frag");
}


//int vertexLightLocation;
//int lightPosLocation;
int transformLocation;
int viewLocation; 
int projLocation;   
int sizeMultiplier;
GLint frustrumFarLoc;
GLint nearResLoc;

int viewPos;
int ambientLoc;
int diffuseLoc;
int specularLoc;
int shininessLoc;
int emissionLoc;
int lightPos;
int lightAmbient;
int lightDiffuse;
int lightSpecular;

GLuint textVAO, textVBO;
int textColor;
int projection;

GLuint pointVAO, pointVBO;
GLint pointModelLoc;
GLint pointViewLoc;
GLint pointProjLoc;
GLint pointSizeLoc;
GLint pointColorLoc;

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

void points_setup(){
        glUseProgram(pointShader);

        glGenVertexArrays(1, &pointVAO);
        glGenBuffers(1, &pointVBO);

        glBindVertexArray(pointVAO);

        glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec3), (vec3){0.0f, 0.0f, 0.0f}, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
}

void setup_shaders(){
        compileShaders();
        glUseProgram(shaderProgram);
        // Vertex Shader
        transformLocation = glGetUniformLocation(shaderProgram, "model");
        viewLocation      = glGetUniformLocation(shaderProgram, "view");
        projLocation      = glGetUniformLocation(shaderProgram, "projection");
        sizeMultiplier    = glGetUniformLocation(shaderProgram, "sizeMultiplier");
        frustrumFarLoc    = glGetUniformLocation(shaderProgram, "frustrumFar");
        nearResLoc        = glGetUniformLocation(shaderProgram, "nearRes");

        // Fragment Shader
        viewPos       = glGetUniformLocation(shaderProgram, "viewPos");

        lightPos      = glGetUniformLocation(shaderProgram, "light.position");
        lightAmbient  = glGetUniformLocation(shaderProgram, "light.ambient");
        lightDiffuse  = glGetUniformLocation(shaderProgram, "light.diffuse");
        lightSpecular = glGetUniformLocation(shaderProgram, "light.specular");

        ambientLoc    = glGetUniformLocation(shaderProgram, "material.ambient");
        diffuseLoc    = glGetUniformLocation(shaderProgram, "material.diffuse");
        specularLoc   = glGetUniformLocation(shaderProgram, "material.specular");
        shininessLoc  = glGetUniformLocation(shaderProgram, "material.shininess");
        emissionLoc   = glGetUniformLocation(shaderProgram, "material.emission");

        glUniform1f(frustrumFarLoc, frustrumFar);
        glUniform1f(nearResLoc, 1);
        glUniform3f(lightPos, 0.0f, 0.0f, 0.0f);
        glUniform3f(lightAmbient, 0.2*color.star.R, 0.2*color.star.G, 0.2*color.star.B); // Light color
        glUniform3f(lightDiffuse, color.star.R, color.star.G, color.star.B); // Light color
        glUniform3f(lightSpecular, 1.0f, 1.0f, 1.0f); // Light color

        // POINT SHADER
        glUseProgram(pointShader);
        pointModelLoc = glGetUniformLocation(pointShader, "model");
        pointViewLoc  = glGetUniformLocation(pointShader, "view");
        pointProjLoc  = glGetUniformLocation(pointShader, "projection");
        pointSizeLoc  = glGetUniformLocation(pointShader, "size");
        pointColorLoc = glGetUniformLocation(pointShader, "pointColor");

        glUniform1f(pointSizeLoc, 1.0f);
        glUniform3f(pointColorLoc, color.red.R, color.red.G, color.red.B);

        points_setup();

        modelArray.array = NULL;
        modelArray.size  = 0;
}

vec3 light_position = {0.0f, 0.0f, 0.0f};
void renderer_update_light_position(vec3 position){
        glm_vec3_copy(position, light_position);
}

void renderer_init(){
        // GLAD
        if(!gladLoadGLLoader(SDL_GL_GetProcAddress)){
                fprintf(stderr, "[ERROR] Glad was not initialized\n");
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

        // Standard models (sphere, quad, square, etc)
        renderer_create_sphere();

        // Standard material
        materialArray_push(color.red, color.red, color.red, 32.0f, color.black);

        glm_quat_for((vec3){-1.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f}, qCamera);
}

void renderer_quit(){
        free(modelArray.array);
}

void renderer_draw(){
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_CULL_FACE);
        // NOT TRANSPARENT FIRST
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        // TRANSPARENT OBJECTS
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUseProgram(shaderProgram);

        /**
        // WIREFRAMES
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.0, -1.0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //draw_model(color.orange, *mesh);

        glDisable(GL_POLYGON_OFFSET_LINE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_DEPTH_TEST);
        **/
}

void renderer_draw_GUI(){
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUseProgram(textShader);
        //render_text("test", 0.5f, 0.5f, 1.0f, color.orange);
}

void renderer_draw_point_setup(){
        glUseProgram(pointShader);
        glEnable(GL_PROGRAM_POINT_SIZE);
}

void renderer_draw_point(vec3 position, Color_RGB color, float size){
        mat4 model;
        glm_mat4_identity(model);
        glm_translate(model, position);

        mat4 view;
        glm_mat4_identity(view);
        glm_quat_look(camera_pos, qCamera, view);

        mat4 proj;
        glm_mat4_identity(proj);
        glm_perspective(glm_rad(FOV), (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.1f, frustrumFar, proj);


        //glUniform3f(viewPos, camera_pos[0], camera_pos[1], camera_pos[2]);
        glUniformMatrix4fv(pointModelLoc, 1, GL_FALSE, (const float*)model);
        glUniformMatrix4fv(pointViewLoc, 1, GL_FALSE, (const float*)view);
        glUniformMatrix4fv(pointProjLoc, 1, GL_FALSE, (const float*)proj);
        glUniform3f(pointColorLoc, color.R, color.G, color.B);
        glUniform1f(pointSizeLoc, 20.0f);

        glBindVertexArray(pointVAO);
        glDrawArrays(GL_POINTS, 0, 1);
        glBindVertexArray(0);
}

// CAMERA 
void camera_move(vec3 direction, const float speed){

        float x = direction[0]; 
        float y = direction[1]; 
        float z = direction[2]; 


        vec3 camera_front;
        glm_quat_rotatev(qCamera, (vec3){0.0f, 0.0f, -1.0f}, camera_front);

        vec3 camera_right;
        glm_quat_rotatev(qCamera, (vec3){1.0f, 0.0f, 0.0f}, camera_right);

        vec3 aux;
        glm_vec3_crossn(gravity_up, camera_right, aux);
        glm_vec3_muladds(aux, x*speed, camera_pos); //pos += (front*spd)
                                                    // TODO: Need to update the camera UP
        glm_vec3_muladds(camera_right, y*speed, camera_pos);

        glm_vec3_muladds(gravity_up, z*speed, camera_pos);

        //printf("Movement direction: %.2f %.2f %.2f", x, y, z);
}

float yaw = -90;
float pitch = 0;
void camera_rotate(int dx, int dy, float sensitivity){ // TODO: Fix Loxodrome Spiraling and Collinear Vector Singularity
        float dYaw   = dx*sensitivity;
        float dPitch = dy*sensitivity;

        yaw   += dYaw;
        pitch += dPitch;

        if(pitch < -89.9){
                pitch  = -89.9;
        }
        if(pitch > 89.9){
                pitch  = 89.9;
        }

        versor qBase;
        glm_quat_from_vecs((vec3){0.0f, 1.0f, 0.0f}, gravity_up, qBase);

        versor qYaw, qPitch;
        glm_quatv(   qYaw,   glm_rad(-yaw), gravity_up);
        glm_quatv( qPitch, glm_rad(-pitch), (vec3){1.0f, 0.0f, 0.0f});

        glm_quat_mul( qYaw,  qBase,   qBase);
        glm_quat_mul(qBase, qPitch, qCamera);

        glm_quat_normalize(qCamera);

        vec3 camera_front;
        glm_quat_rotatev(qCamera, (vec3){0.0f, 0.0f, -1.0f}, camera_front);

        //printf("Camera up here is: (%.2f %.2f %.2f)\n",  local_up[0], local_up[1], local_up[2]);
        //printf("Planet up here is: (%.2f %.2f %.2f)\n", gravity_up[0], gravity_up[1], gravity_up[2]);
        //printf("dx: %.2f   dy: %.2f\n", dYaw, dPitch);
        //printf("yaw: %.2f   pitch: %.2f\n", yaw, pitch);
        //printf("Front: (%.2f %.2f %.2f)\n\n", camera_front[0], camera_front[1], camera_front[2]);
}

void camera_change_up(vec3 direction){
        glm_normalize(direction);
        glm_vec3_copy(direction, gravity_up);

        versor qBase;
        glm_quat_from_vecs((vec3){0.0f, 1.0f, 0.0f}, gravity_up, qBase);

        versor qYaw, qPitch;
        //glm_quatv(   qYaw,   glm_rad(-yaw), gravity_up);
        glm_quatv(   qYaw,   glm_rad(-yaw), (vec3){0.0f, 1.0f, 0.0f});
        glm_quatv( qPitch, glm_rad(-pitch), (vec3){1.0f, 0.0f, 0.0f});

        glm_quat_mul(qBase, qYaw,   qBase);
        //glm_quat_mul( qYaw,  qBase,   qBase);
        glm_quat_mul(qBase, qPitch, qCamera);

        glm_quat_normalize(qCamera);
        // DELETE LATER
        vec3 camera_front;
        glm_quat_rotatev(qCamera, (vec3){0.0f, 0.0f, -1.0f}, camera_front);
        //printf("yaw: %.2f   pitch: %.2f\n", yaw, pitch);
        //printf("Front: (%.2f %.2f %.2f)\n\n", camera_front[0], camera_front[1], camera_front[2]);
}

void camera_move_to_origin(){
        glm_vec3_zero(camera_pos); 
}

void camera_copy_position(vec3 dest){
        glm_vec3_copy(camera_pos, dest);
}

void camera_print_coords(){
        printf("Pos: %.2f, %.2f, %.2f\n", camera_pos[0], camera_pos[1], camera_pos[2]);
        //printf("Fnt: %.2f, %.2f, %.2f\n", camera_front[0], camera_front[1], camera_front[2]);
        //printf("Up:  %.2f, %.2f, %.2f\n\n", camera_up[0], camera_up[1], camera_up[2]);
}

void renderer_draw_model(const u32 modelID, vec3d position_double, vec3 scale){
        vec3 position;
        vec3d_to_vec3(position_double, position);
        Model model = modelArray_Get(modelID);
        Material material = materialArray_Get(model.materialID);

        //renderer_draw_point_setup();
        glUseProgram(pointShader);
        renderer_draw_point(position, color.star, scale[0]*2);


        // IF CLOSE DO THIS
        //renderer_draw(); // TODO remove from here
        glUseProgram(shaderProgram);


        // MODEL MATRIX
        mat4 mesh;
        glm_mat4_identity(mesh);
        glm_translate(mesh, position);
        float rotate_speed = -1.0f/10.0; // frequency
        //glm_rotate(mesh, rotate_speed*((float)SDL_GetTicks()/1000.0f)*GLM_PI*2, (vec3){0.0f, 1.0f, 0.0f});
        glm_scale(mesh, scale);


        // VIEW MATRIX
        mat4 view;
        glm_mat4_identity(view);
        glm_quat_look(camera_pos, qCamera, view);


        // PROJECTION MATRIX
        mat4 proj;
        glm_mat4_identity(proj);
        glm_perspective(glm_rad(FOV), (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, frustrumNear, frustrumFar, proj);


        glUniform3f(ambientLoc, material.ambient.R, material.ambient.G, material.ambient.B);
        glUniform3f(diffuseLoc, material.diffuse.R, material.diffuse.G, material.diffuse.B);
        glUniform3f(specularLoc, material.specular.R, material.specular.G, material.specular.B);
        glUniform1f(shininessLoc, material.shininess);
        glUniform3f(emissionLoc, material.emission.R, material.emission.G, material.emission.B);
        glUniform3f(lightPos, light_position[0], light_position[1], light_position[2]);

        glUniform3f(viewPos, camera_pos[0], camera_pos[1], camera_pos[2]);
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, (const float*)mesh);
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, (const float*)view);
        glUniformMatrix4fv(projLocation, 1, GL_FALSE, (const float*)proj);
        glUniform1f(sizeMultiplier, 1);

        glBindVertexArray(model.VAO);

        glDrawElements(GL_TRIANGLES, model.nFaces, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
}


u32 renderer_duplicate_model(u32 modelID){
        Model duplicate = modelArray_Get(modelID);
        u32 newID = modelArray_push(duplicate.VAO, duplicate.nFaces, duplicate.materialID);
        return newID;
}

void renderer_change_material(u32 modelID, u32 materialID){
        if(materialID >= materialArray.size){
                fprintf(stderr,"[ERROR] Material %d is not on materialArray", materialID);
                exit(1);
        }
        modelArray.array[modelID].materialID = materialID;
}

u32 renderer_create_material(Color_RGB ambient, Color_RGB diffuse, Color_RGB specular, float shininess, Color_RGB emission){
        return materialArray_push(ambient, diffuse, specular, shininess, emission);
}

GLuint renderer_create_VAO(const Mesh mesh){ // Create model from mesh
        GLuint VAO;
        GLuint VBO;
        GLuint EBO;
        // VAO / VBO / EBO ===========================================================
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        Vec3_Array VBO_data = {.array = NULL, .size = 0};
        for(int i = 0; i < mesh.vertices.size; i++){
                Vec3Array_Push(&VBO_data, mesh.vertices.array[i]);
                Vec3Array_Push(&VBO_data, mesh.normals.array[i]);
        }
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*mesh.vertices.size*2, VBO_data.array, GL_STATIC_DRAW);
        free(VBO_data.array);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32)*mesh.faces.size*3, mesh.faces.array, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        return VAO;
}

u32 renderer_create_model(Mesh mesh, u32 materialID){
        GLuint VAO = renderer_create_VAO(mesh);
        u32 modelID = modelArray_push(VAO, mesh.faces.size*3, materialID);
        return modelID;
}

u32 renderer_create_sphere(){
        Mesh sphere = {.faces = {0}, .vertices = {NULL}};
        mesh_create_sphere(&sphere,4);
        u32 modelID = renderer_create_model(sphere, 0);
        mesh_free(&sphere); 
        return modelID;
}

u32 renderer_get_sphere(){
        return 0;
}


// TODO optimize text rendering
void render_text(const char* text, float x, float y, const float scale, const Color_RGB color){
        x = ((x+1)/2)*SCREEN_WIDTH;
        y = ((y+1)/2)*SCREEN_HEIGHT;

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

                glBindTexture(GL_TEXTURE_2D, ch.TextureID); // << Expensive
                glBindBuffer(GL_ARRAY_BUFFER, textVBO);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                x += (ch.Advance >> 6) * scale;
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D,0);
}

void renderer_draw_quad(float x1, float x2, float y1, float y2){
}
