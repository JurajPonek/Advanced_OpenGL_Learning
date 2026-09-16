#version 460 core
layout(location = 0) in vec3 i_position;
layout(location = 1) in vec3 i_normal;
layout(location = 2) in vec2 i_texture_coords;

uniform mat4 model;
out vec2 o_texture_coords;
out vec3 o_normal;
out vec4 frag_pos;
layout(std140, binding = 0) uniform camera
{
    mat4 view;
    mat4 projection;
    vec3 camera_position;
};
void main()
{
    gl_Position = projection * view * model * vec4(i_position, 1.0);
    o_texture_coords = i_texture_coords;
    o_normal = normalize(transpose(inverse(mat3(model))) * i_normal) ; 
    // TOTO TREBA POSIELAT CEZ UNIFROM A VYPOCITAT NA CPU
    frag_pos = model * vec4(i_position, 1);
}