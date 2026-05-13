#version 330 core

layout (location = 0) in vec3 aPos;

out vec3 FragPos;

uniform mat4  model;
uniform mat4  view;
uniform mat4  projection;
uniform float size;

//uniform float frustrumFar;
//uniform float nearRes; // Larger values = better res at close distances
// Res = log(C*Far + 1) / ((2^n - 1) * C/(C*x+1))

void main(){
        FragPos = vec3(model * vec4(aPos, 1.0));

        vec4 pos = projection * view * model * vec4(aPos, 1.0);
        //pos.z = (2*log(nearRes*pos.w + 1) / log(nearRes*frustrumFar + 1) - 1) * pos.w;
        gl_Position = pos;
        gl_PointSize = size;
        //gl_Position = projection * view * model * vec4(aPos, 1.0);
}

