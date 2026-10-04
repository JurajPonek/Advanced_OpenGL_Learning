#version 460 core

in vec2 o_texture_coords;

out vec4 frag_color;

uniform sampler2D tex0; // normals
uniform sampler2D tex1; // depth buffer
uniform sampler2D tex2; // ssao_noise

layout(std140, binding = 0) uniform camera
{
    mat4 view;
    mat4 projection;
    vec3 camera_position;
};
uniform mat4 inverse_proj;
uniform vec3 samples[64];
uniform float power;

const int kernel_size = 64;
const float radius = 0.5;    
const float bias = 0.025;  

const vec2 noise_scale = vec2(1920.0 / 4.0, 1080.0 / 4.0);

vec3 get_view_pos(vec2 uv, mat4 inv_proj)
{
    float depth = texture(tex1, uv).r;
    vec4 sreen_space_coords = vec4(uv.xy * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 view_space_pos = inv_proj * sreen_space_coords;
    view_space_pos /= view_space_pos.w;
    return view_space_pos.xyz;
}
float calculate_ssao_occlusion(mat3 TBN, vec3 frag_pos)
{
    float occlusion = 0.0;
    for(int i = 0; i < kernel_size; ++i)
    {
        vec3 sample_pos = TBN * samples[i]; 
        sample_pos = frag_pos + sample_pos * radius; 

        vec4 offset = vec4(sample_pos, 1.0);
        offset = projection * offset;          // 1. View -> Clip Space
        offset.xyz /= offset.w;               // 2. Clip -> NDC Space (-1 až 1)
        offset.xyz = offset.xyz * 0.5 + 0.5;   // 3. NDC -> UV Space (0 až 1)

        float sample_depth = get_view_pos(offset.xy, inverse_proj).z;

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(frag_pos.z - sample_depth));

        occlusion += (sample_depth >= sample_pos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / float(kernel_size));
    return occlusion;
}



void main()
{
    float depth = texture(tex1, o_texture_coords).r;
    if (depth >= 1.0) {
        frag_color = vec4(1.0);
        return;
    }
    vec3 frag_pos_view_space = get_view_pos(o_texture_coords, inverse_proj);
    vec3 world_normal = texture(tex0, o_texture_coords).rgb;
    vec3 view_normal = normalize(mat3(view) * world_normal);
    vec3 random_vec = normalize(texture(tex2, o_texture_coords * noise_scale).xyz);
    vec3 tangent = normalize(random_vec - view_normal * dot(random_vec, view_normal));
    vec3 bitangent = cross(view_normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, view_normal);
    float ao = pow(calculate_ssao_occlusion(TBN, frag_pos_view_space), power);
    frag_color = vec4(vec3(ao), 1.0); 


}
