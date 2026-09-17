#version 460 core

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec3 i_normal;
layout(location = 2) in vec2 i_texture_coords;

uniform mat4 model;
out vec3 o_normal;

layout(std140, binding=0) uniform camera
{
    mat4 view;
    mat4 projection;
    vec3 camera_position;
};


void main()
{
    gl_Position = view * model * vec4(i_position, 1.0f);
    mat3 normal_matrix = mat3(transpose(inverse(view * model)));
    o_normal = normalize(vec3(vec4(normal_matrix * i_normal, 0.0)));
}




