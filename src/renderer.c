#include "renderer.h"
#include "types.h"
#include "model.h"
#include "constants.h"

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

int rotate = TRUE;
float rotate_speed = -1.0f/10.0; // frequency
                                 //
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
void renderer_init(){
        compileShaders();
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


void draw_object(const Color_RGBA color, GLenum mode, GLuint VAO, Model modelo){
        mat4 model;
        glm_mat4_identity(model);
        //glm_scale(model, (vec4){0.1f, 0.1f, 0.1f});
        glm_scale(model, (vec3){2.0f, 2.0f, 2.0f});
        glm_translate(model, translation);
        if(rotate == TRUE){
                glm_rotate(model, rotate_speed*((float)SDL_GetTicks()/1000.0f)*GLM_PI*2, (vec3){0.0f, 1.0f, 0.0f});
                //glm_rotate(model, rotate_speed*((float)SDL_GetTicks()/1000.0f)*GLM_PI*2, (vec3){0.2f, 0.0f, 0.4f});
        }

        mat4 view;
        glm_mat4_identity(view);
        glm_translate(view, translation);

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


        int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
        int transformLocation   = glGetUniformLocation(shaderProgram, "model");
        int viewLocation        = glGetUniformLocation(shaderProgram, "view");
        int projLocation        = glGetUniformLocation(shaderProgram, "projection");
        int sizeMultiplier      = glGetUniformLocation(shaderProgram, "sizeMultiplier");
        glUseProgram(shaderProgram);
        glUniform4f(vertexColorLocation, color.R, color.G, color.B, color.A);
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, (const float*)model);
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, (const float*)view);
        glUniformMatrix4fv(projLocation, 1, GL_FALSE, (const float*)proj);
        glUniform1f(sizeMultiplier, 1);

        glBindVertexArray(VAO);

        glDrawElements(mode, modelo.faces.size, GL_UNSIGNED_INT, 0);
}
