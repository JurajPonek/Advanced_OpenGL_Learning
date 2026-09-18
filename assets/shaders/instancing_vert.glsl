#version 460 core

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec3 i_normal;
layout(location = 2) in vec2 i_texture_coords;

out vec2 o_texture_coords;
out vec3 o_normal;
layout(std140, binding = 0) uniform camera
{
    mat4 view;
    mat4 projection;
    vec3 camera_position;
};


layout(std430, binding = 1) readonly buffer transforms
{
    mat4 model_matrices[];
};
void main()
{
    gl_Position = projection * view * model_matrices[gl_InstanceID] * vec4(i_position, 1.0);
    o_texture_coords = i_texture_coords;
}