#version 460 core

out vec4 frag_color;
in vec3 o_texture_coords;
in vec3 o_normal;
in vec4 frag_pos;
uniform samplerCube tex0;

layout(std140, binding = 0) uniform camera
{
    mat4 view;
    mat4 projection;
    vec3 camera_position;

};

void main()
{
    frag_color = texture(tex0, o_texture_coords);
}