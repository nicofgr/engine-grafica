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

uniform vec3  u_planet_center;
uniform float f_coef;
uniform float frustrum_far;

uniform vec3  u_light_position;

#define M_PI 3.1415926535897932384626433832795
float max_float = 1e38;
float sphere_radius = 6400.0;
float inner_radius  = 6371.8; // This will be a uniform
vec2  resolution = vec2(800,600);
float u_tan_half_fov = tan((70.0*0.5f)*(M_PI/180.0)); // TODO: Move this to cpu
vec3 scattering_coef = vec3(0.058, 0.135, 0.331);

float linear_depth(float depth){
        float  z_ndc = depth * 2.0 - 1.0;
        float  linear_depth = pow(2.0, (z_ndc + 1.0)/f_coef) - 1.0;
        return linear_depth;
}

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
        float distance_to_sphere_near = max((-bh - sqrt_delta), 0.0);
        float distance_to_sphere_far  = (-bh + sqrt_delta);

        if(distance_to_sphere_far >= 0){
                return vec2(distance_to_sphere_near, distance_to_sphere_far - distance_to_sphere_near);
        }
        return vec2(max_float, 0); // Not hitting
}

float phase_function(float cos_theta){
        return 0.75 * (1 + (cos_theta * cos_theta));
}

float density_at_point(vec3 point){
        float height  = (length(point - u_planet_center) - inner_radius) / (sphere_radius - inner_radius); // Varies from 0 to 1
        float density = exp(-height*16) * (1-height);
        return density;
}

vec3 out_scattering(vec3 ray_origin, vec3 ray_direction, int n_samples, float dist_near, float dist_through_atmosphere){
        vec3  sampler_start = ray_origin + ray_direction*dist_near;
        float result = 0.0;
        float ds = dist_through_atmosphere/n_samples;
        vec3  sampler = sampler_start;
        sampler += ray_direction*(ds*0.5);
        for(int i = 0; i < n_samples; i++){
                result  += (density_at_point(sampler) * ds);
                sampler += ray_direction*ds;
        }
        return result * scattering_coef;
}

vec3 in_scattering(vec3 ray_origin, vec3 ray_direction, int n_samples, float dist_near, float dist_through_atmosphere){
        vec3  sampler_start = ray_origin + ray_direction*dist_near;
        vec3 result;
        float ds = dist_through_atmosphere/n_samples;
        vec3  sampler = sampler_start;
        sampler += ray_direction*(ds*0.5);
        for(int i = 0; i < n_samples; i++){
                vec3  sample_to_sun_dir = normalize(u_light_position - sampler);
                vec2  sample_to_sun_ray = raySphere(u_planet_center, sphere_radius, sampler, sample_to_sun_dir); // to the edge of atmos
                vec3 sample_to_sun = out_scattering(sampler, sample_to_sun_dir, 5, 0.0, sample_to_sun_ray.y);

                float dist = length(sampler - sampler_start);
                vec3 sample_to_camera = out_scattering(sampler, -ray_direction, 5, 0.0, dist); 

                result  += (density_at_point(sampler) * exp(-( sample_to_sun + sample_to_camera ))) * ds;
                sampler += ray_direction*ds;
        }
        return result * scattering_coef;
}


void main(){
        vec4 original_color = texture(screenTexture, TexCoords);
        vec2 uv = TexCoords * 2.0 - 1.0;
        uv.x *= (resolution.x/resolution.y);
        uv   *= u_tan_half_fov;

        vec3 ray_origin    = camera_position;
        vec3 view_vector   = (camera_front + uv.x * camera_right + uv.y * camera_up);
        vec3 ray_direction = normalize(view_vector);

        vec2  hit_info = raySphere(u_planet_center, sphere_radius, ray_origin, ray_direction);
        float distance_to_atmosphere = hit_info.x;
        float depth = linear_depth(texture(u_depth_texture, TexCoords).r) * length(view_vector);  // Depth info without atmosphere
        float distance_through_atmosphere = min(hit_info.y, depth - distance_to_atmosphere);       // Pass through or hit ground
        vec4  atmosphere = vec4( vec3(distance_through_atmosphere/(sphere_radius*2.0)), 1.0);      // Only the atmosphere effect

        if(distance_through_atmosphere > 0){
                vec3  light_direction = normalize(u_light_position - u_planet_center);
                float phase = phase_function(dot(light_direction, ray_direction));


                vec3 in_scatteringg = in_scattering(ray_origin, ray_direction, 50, distance_to_atmosphere, distance_through_atmosphere);
                atmosphere = vec4(( in_scatteringg * phase ),1.0);
                FragColor = atmosphere + original_color;
                return;
        }
        FragColor = original_color;

        //FragColor = vec4(uv, 0, 1);
        //original_color = vec4(vec3( linear_depth(texture(u_depth_texture, TexCoords).r))/frustrum_far  ,1.0); // depth
        //FragColor = vec4(vec3(1 - tx),1.0); // Invert colors
        //float average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b; // B&W
        //FragColor = vec4(average, average, average, 1.0);
}
