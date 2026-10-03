#version 460 core
in vec2 o_texture_coords;
out vec4 FragColor;

layout(binding = 0) uniform sampler2D tex0;

void main()
{
    float val = texture(tex0, o_texture_coords).r;
    FragColor = vec4(val, val, val, 1.0); 
}