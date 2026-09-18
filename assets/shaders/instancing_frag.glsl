#version 460 core
out vec4 frag_color;
in vec2 o_texture_coords;
in vec3 o_normal;
uniform sampler2D tex0;


void main()
{
    frag_color = texture(tex0, o_texture_coords);
}