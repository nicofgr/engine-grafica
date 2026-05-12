#include "renderer/shader.h"

#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* _getFileContent(const char* path){
        FILE* file;
        file = fopen(path,"r");
        if(file == NULL){
                fprintf(stderr, "File \"%s\" failed to open\n", path);
                exit(0);
        }
        fseek(file, 0, SEEK_END); // seek to end of file
        int size = ftell(file); // get current file pointer
        fseek(file, 0, SEEK_SET); // seek back to beginning of file
        char* fileContent = (char*)calloc(size+1,sizeof(char));
        char data[50];
        while(fgets(data, 50, file) != NULL){
                strcat(fileContent, data);
        }
        fclose(file);
        return fileContent;
}

unsigned int shCreateShaderProgram(const char* vertexShaderPath, const char* fragmentShaderPath){
        char* dataptr = _getFileContent(vertexShaderPath);
        const char* vertexSource = dataptr;

        unsigned int vertexShader;
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexSource, NULL);
        glCompileShader(vertexShader);
    
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if(!success){
                glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
                fprintf(stderr, "ERROR::SHADER::VERTEX::COMPILATION_FAILED %s\n",infoLog);
                exit(0);
        }
        free(dataptr);

        dataptr = _getFileContent(fragmentShaderPath);
        const char* fragmentSource = dataptr;

        unsigned int fragmentShader;
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if(!success){
                glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
                fprintf(stderr, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED %s\n",infoLog);
                exit(0);
        }
        free(dataptr);

        unsigned int shaderProgram;
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
    
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if(!success) {
                glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
                fprintf(stderr, "ERROR::SHADER::PROGRAM::COMPILATION_FAILED %s\n",infoLog);
                exit(0);
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return shaderProgram;
}

