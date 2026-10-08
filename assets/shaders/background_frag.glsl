#version 460 core


uniform samplerCube tex0;
layout (location = 0) in vec3 v_RayDir;
out vec4 frag_color;


void main()
{
    vec3 dir = normalize(v_RayDir);
    frag_color = vec4(texture(tex0, dir).rgb, 1.0);
}

