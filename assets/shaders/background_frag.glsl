#version 460 core


uniform samplerCube tex0;
layout (location = 0) in vec3 v_RayDir;
out vec4 frag_color;
uniform float exposure;

void main()
{
    vec3 dir = normalize(v_RayDir);
    frag_color = vec4(textureLod(tex0, dir, exposure).rgb, 1.0);
}

