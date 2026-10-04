#version 460 core

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec3 i_normal;
layout(location = 2) in vec3 i_tangent;
layout(location = 3) in vec2 i_texture_coords;

out vec2 o_texture_coords;
out vec3 o_normal;
out mat3 TBN;
out vec3 view_dir_tbn;

uniform mat4 model;

layout(std140, binding = 0) uniform camera
{
    mat4 view;
    mat4 projection;
    vec3 camera_position;
};

void main()
{
    gl_Position = projection * view * model * vec4(i_position, 1.0f);
    o_texture_coords = i_texture_coords;
    o_normal = normalize(transpose(inverse(mat3(model))) * i_normal); // TOTO TREBA POSIELAT CEZ UNIFROM A VYPOCITAT NA CPU
    vec3 T = normalize(vec3(model * vec4(i_tangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(i_normal, 0.0)));
    T = normalize(T - dot(N, T) * N);
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);
    vec4 frag_pos = model * vec4(i_position, 1.0f);
    view_dir_tbn = transpose(TBN) * normalize(camera_position - frag_pos.xyz);
}






