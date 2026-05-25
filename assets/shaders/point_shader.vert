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
        vec4 viewPos = view * model * vec4(aPos, 1.0);
        
        float distance = length(viewPos.xyz);

        float nearRes = 1;
        float frustrumFar = 1e12;

        FragPos = vec3(model * vec4(aPos, 1.0));

        vec4 pos = projection * viewPos;
        pos.z = (2*log(nearRes*pos.w + 1) / log(nearRes*frustrumFar + 1) - 1) * pos.w;
        gl_Position = pos;
        gl_PointSize = clamp(size / distance, 2.0, 300.0);
}

