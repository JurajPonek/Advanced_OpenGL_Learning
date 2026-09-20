#version 460 core

layout (location = 0) in vec3 i_position;
layout (location = 1) in vec3 i_normal;
layout (location = 2) in vec2 i_texture_coords;
layout(std140, binding = 0) uniform camera
{
    mat4 view;
    mat4 projection;
    vec3 camera_position;
};
uniform mat4 model;
uniform mat4 light_space_matrix;
out vec3 o_normal;
out vec2 o_texture_coords;
out vec4 frag_pos;
out vec4 frag_pos_light_space;

void main()
{
    gl_Position = projection * view * model * vec4(i_position, 1.0f);
    o_texture_coords = i_texture_coords;
    o_normal = transpose(inverse(mat3(model))) * i_normal;
    frag_pos = model * vec4(i_position, 1.0f);
    frag_pos_light_space = light_space_matrix * frag_pos;
};
