#version 460 core
layout(location = 0) in vec3 i_position;
layout(location = 1) in vec3 i_normal;
layout(location = 2) in vec2 i_texture_coords;

uniform mat4 model;



void main()
{
    gl_Position = model * vec4(i_position, 1.0);
}