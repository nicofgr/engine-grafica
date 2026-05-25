#version 330 core

in vec3 FragPos;

out vec4 FragColor;

uniform vec3 pointColor;
 
void main(){
        vec2 coord = gl_PointCoord - vec2(0.5);
        float dist = length(coord);

        float alpha = 1.0 - smoothstep(0.1, 0.5, dist);
        FragColor = vec4(pointColor.xyz*alpha, alpha);
        //FragColor = vec4(pointColor, 1.0);
}

