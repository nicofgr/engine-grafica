#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D u_depth_texture;

uniform vec3 camera_position; 
uniform vec3 camera_front;
uniform vec3 camera_right;
uniform vec3 camera_up;

uniform vec3 planet_center;

float max_float = 1e38;

vec2 raySphere(vec3 sphere_center, float sphere_radius, vec3 ray_origin, vec3 ray_direction){
        vec3 offset = ray_origin - sphere_center;
        float a = 1.0;
        float b = 2.0 * dot(offset, ray_direction);
        float c = dot(offset, offset) - pow(sphere_radius,2);
        float d = pow(b,2) - 4 * a * c;

        if(d > 0){
                float s = sqrt(d);
                float distance_to_sphere_near = max(0, (-b - s)/(2.0 * a));
                float distance_to_sphere_far = (- b + s)/(2.0 * a);

                if(distance_to_sphere_far >= 0){
                        return vec2(distance_to_sphere_near, distance_to_sphere_far - distance_to_sphere_near);
                }
        }
        return vec2(max_float, 0);
}

float sphere_radius = 2439.7;

vec2 resolution = vec2(800,600);
float u_tan_half_fov = tan((70.0*0.5f)*(3.1415926/180.0)); // TODO: Move this to cpu
void main(){
        //vec2 uv = (TexCoords * 2.0 - resolution.xy) / resolution.y;
        vec2 uv = TexCoords * 2.0 - 1.0;
        uv.x *= (resolution.x/resolution.y);
        uv   *= u_tan_half_fov;

        vec3 ray_origin    = camera_position;
        //vec3 ray_direction = normalize(vec3(camera_front.xy+uv.xy, 1.0));
        vec3 ray_direction = normalize(camera_front + uv.x * camera_right + uv.y * camera_up);

        vec2 hit_info = raySphere(planet_center, sphere_radius, ray_origin, ray_direction);
        float distance_to_atmosphere      = hit_info.x;
        float distance_through_atmosphere = hit_info.y;

        FragColor = vec4( vec3(distance_through_atmosphere/(sphere_radius*2.0)) , 1.0);


        //FragColor = vec4(uv, 0, 1);
        
        //vec4 tx = texture(screenTexture, TexCoords);
        //FragColor = tx;
        //FragColor = vec4(vec3(1 - tx),1.0); // Invert colors
        //float average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b; // B&W
        //FragColor = vec4(average, average, average, 1.0);
}
