#version 330 core
layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec3 aNormal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float sizeMultiplier;
void main(){
        gl_Position = projection*view*model*vec4(aPos.x, aPos.y, aPos.z, 1.0);
        gl_PointSize = min((50*sizeMultiplier)/gl_Position.z, 400.0f);
}

