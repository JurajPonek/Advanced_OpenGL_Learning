#version 460 core

layout (location = 0) in vec3 i_position;
layout (location = 1) in vec3 i_normal;
layout (location = 2) in vec3 i_tangent;
layout (location = 3) in vec2 i_texture_coords;

out vec2 o_texture_coords;


void main()
{
    vec2 positions[3] = vec2[](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );
    vec2 pos = positions[gl_VertexID];
    gl_Position = vec4(pos, 0.0, 1.0);
    o_texture_coords = pos * 0.5 + 0.5;
};

