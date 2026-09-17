#version 460 core

layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

in vec3 o_normal[];
layout(std140, binding = 0) uniform camera
{
    mat4 view;
    mat4 projection;
    vec3 camera_position;
};

const float MAGNITUDE = 0.4f;

void generate_line(int index)
{
    gl_Position = projection * gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = projection * (gl_in[index].gl_Position  + vec4(o_normal[index], 0.0f) * MAGNITUDE);
    EmitVertex();
    EndPrimitive();
}

void main()
{
    for (int i = 0; i < 4; ++i)
    {
        generate_line(i);
    }
}
