#version 330 core
// z = (2*log(C*w + 1) / log(C*Far + 1) - 1) * w 
// https://outerra.blogspot.com/2009/08/logarithmic-z-buffer.html

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float sizeMultiplier;

uniform float frustrumFar;
uniform float nearRes; // Larger values = better res at close distances
// Res = log(C*Far + 1) / ((2^n - 1) * C/(C*x+1))

void main(){
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal; // TODO move this to the cpu

        vec4 pos = projection * view * model * vec4(aPos, 1.0);
        pos.z = (2*log(nearRes*pos.w + 1) / log(nearRes*frustrumFar + 1) - 1) * pos.w;
        gl_Position = pos;
        //gl_Position = projection * view * model * vec4(aPos, 1.0);
        //gl_PointSize = min((50*sizeMultiplier)/gl_Position.z, 400.0f);
}

