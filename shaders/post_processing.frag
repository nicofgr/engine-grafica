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

uniform vec3 u_light_position;

float max_float = 1e38;

// Returns [distance to near] and [distance through]
//https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection
vec2 raySphere(vec3 sphere_center, float sphere_radius, vec3 ray_origin, vec3 ray_direction){
        vec2 result;
        vec3 dist_to_center = ray_origin - sphere_center;
        float bh = dot(ray_direction, dist_to_center);
        float c  = dot(dist_to_center, dist_to_center) - (sphere_radius * sphere_radius);
        float delta = bh * bh - c;

        // Passing through
        float sqrt_delta = sqrt( max(delta,0.0) );
        float distance_to_sphere_near = (-bh - sqrt_delta);
        float distance_to_sphere_far  = (-bh + sqrt_delta);

        /** // TODO: Find out why this isnt working
        float hit_mask = step(0.0, delta);
        float far_mask = step(0.0, distance_to_sphere_far);
        float valid_hit = hit_mask * far_mask; 

        vec2 hit_result  = vec2(distance_to_sphere_near, distance_to_sphere_far - distance_to_sphere_near);
        vec2 miss_result = vec2(max_float, 0.0);

        return mix(miss_result, hit_result, valid_hit);
        **/
        if(distance_to_sphere_far >= 0){
                return vec2(distance_to_sphere_near, distance_to_sphere_far - distance_to_sphere_near);
        }
        return vec2(max_float, 0); // Not hitting
}

float linear_depth(float depth){
        float z_ndc = depth * 2.0 - 1.0;
        float linear_depth = pow(2.0, (z_ndc + 1.0)/f_coef) - 1.0;
        return linear_depth;
}

float phase_function(float cos_theta){
        return 0.75 * (1 + (cos_theta * cos_theta));
}

float sphere_radius = 2700;
float inner_radius  = 2349.7; // This will be a uniform

float optical_length(vec3 ray_origin, vec3 ray_direction, int n_samples, float dist_near, float dist_through_atmosphere, vec3 sphere_center){
        vec3 sampler_start = ray_origin + ray_direction*dist_near;
        float result = 0.0;
        float ds = dist_through_atmosphere/n_samples;
        vec3 sampler = sampler_start;
        for(int i = 0; i < n_samples; i++){
                float height = (length(sampler-sphere_center) - inner_radius) / (sphere_radius - inner_radius); // 0 to 1;
                result += exp(-height*4) * ds;
                sampler = sampler + ray_direction*ds;
        }
        return result;
}


vec2 resolution = vec2(800,600);
float u_tan_half_fov = tan((70.0*0.5f)*(3.1415926535/180.0)); // TODO: Move this to cpu
void main(){
        vec4 original_color = texture(screenTexture, TexCoords);
        vec2 uv = TexCoords * 2.0 - 1.0;
        uv.x *= (resolution.x/resolution.y);
        uv   *= u_tan_half_fov;

        vec3 ray_origin    = camera_position;
        vec3 view_vector   = (camera_front + uv.x * camera_right + uv.y * camera_up);
        vec3 ray_direction = normalize(view_vector);

        vec2  hit_info = raySphere(planet_center, sphere_radius, ray_origin, ray_direction);
        float distance_to_atmosphere = hit_info.x;
        float depth = linear_depth(texture(u_depth_texture, TexCoords).r) * length(view_vector) ;  // Depth info without atmosphere
        float distance_through_atmosphere = min(hit_info.y, depth - distance_to_atmosphere);       // Pass through or hit ground
        vec4  atmosphere = vec4( vec3(distance_through_atmosphere/(sphere_radius*2.0)), 1.0);      // Only the atmosphere effect


        vec3 light_direction = normalize(u_light_position - planet_center);
        float strength = phase_function(dot(light_direction, ray_direction));

        vec4 scattering_constant = vec4(0.0058, 0.00135, 0.0331, 1.0);

        //FragColor = atmosphere * strength*scattering_constant;
        //original_color = vec4(vec3( linear_depth(texture(u_depth_texture, TexCoords).r))/frustrum_far  ,1.0); // depth


        if(distance_through_atmosphere > 0){
                float optical_thickness = optical_length(ray_origin, ray_direction, 5, distance_to_atmosphere, distance_through_atmosphere, planet_center);
                FragColor = atmosphere*strength*scattering_constant*optical_thickness + original_color;
                return;
        }
        FragColor = original_color;

        //FragColor = vec4(uv, 0, 1);
        
        //FragColor = vec4(vec3(1 - tx),1.0); // Invert colors
        //float average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b; // B&W
        //FragColor = vec4(average, average, average, 1.0);
}
