//https://developer.nvidia.com/gpugems/gpugems2/part-ii-shading-lighting-and-shadows/chapter-16-accurate-atmospheric-scattering
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
uniform float f_coef;
uniform float frustrum_far;

float max_float = 1e38;

// Returns [distance to near] and [distance through]
//https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection
vec2 raySphere(vec3 sphere_center, float sphere_radius, vec3 ray_origin, vec3 ray_direction){
        vec3 dist_to_center = ray_origin - sphere_center;
        float a = dot(ray_direction, ray_direction);
        float b = 2.0 * dot(ray_direction, dist_to_center);
        float c = dot(dist_to_center, dist_to_center) - (sphere_radius * sphere_radius);
        float delta = b * b - 4 * a * c;

        vec3  sphere_center_vec  = sphere_center - ray_origin;
        float sphere_center_dist = length(sphere_center_vec);
        float angle = acos(dot(normalize(ray_direction), normalize(sphere_center_vec)));
        float closest_distance = sphere_center_dist*sin(angle);

        if(delta > 0){ // Passing through
                float dist_to_middle = sphere_center_dist*cos(angle);
                float middle_to_near = sqrt((sphere_radius*sphere_radius) - (closest_distance * closest_distance));
                float sqrt_delta = sqrt(delta);
                float denum = 1/(2 * a);
                float distance_to_sphere_near = (-b - sqrt_delta) * denum;
                float distance_to_sphere_far  = (-b + sqrt_delta) * denum;

                if(distance_to_sphere_far >= 0){
                        return vec2(distance_to_sphere_near, distance_to_sphere_far - distance_to_sphere_near);
                }
        }
        return vec2(max_float, 0); // Not hitting
}

float linear_depth(float depth){
        float z_ndc = depth * 2.0 - 1.0;
        float linear_depth = pow(2.0, (z_ndc + 1.0)/f_coef) - 1.0;
        return linear_depth;
}

float sphere_radius = 3000;

vec2 resolution = vec2(800,600);
float u_tan_half_fov = tan((70.0*0.5f)*(3.1415926/180.0)); // TODO: Move this to cpu
void main(){
        vec4 original_color = texture(screenTexture, TexCoords);
        vec2 uv = TexCoords * 2.0 - 1.0;
        uv.x *= (resolution.x/resolution.y);
        uv   *= u_tan_half_fov;

        vec3 ray_origin    = camera_position;
        vec3 view_vector   = (camera_front + uv.x * camera_right + uv.y * camera_up);
        vec3 ray_direction = normalize(view_vector);

        vec2 hit_info = raySphere(planet_center, sphere_radius, ray_origin, ray_direction);
        float distance_to_atmosphere = hit_info.x;
        float depth = linear_depth(texture(u_depth_texture, TexCoords).r) * length(view_vector) ;
        float distance_through_atmosphere = min(hit_info.y, depth - distance_to_atmosphere);

        if(distance_through_atmosphere > 0){
                FragColor = vec4( vec3(distance_through_atmosphere/(sphere_radius*2.0)) , 1.0) + original_color;
                return;
        }
        FragColor = original_color;

        //FragColor = vec4(uv, 0, 1);
        
        //FragColor = vec4(vec3(1 - tx),1.0); // Invert colors
        //float average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b; // B&W
        //FragColor = vec4(average, average, average, 1.0);
}
