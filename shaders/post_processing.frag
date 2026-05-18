#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main(){
        vec4 tx = texture(screenTexture, TexCoords);
        FragColor = tx;
        //FragColor = vec4(vec3(1 - tx),1.0); // Invert colors
        //float average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b; // B&W
        //FragColor = vec4(average, average, average, 1.0);
}
