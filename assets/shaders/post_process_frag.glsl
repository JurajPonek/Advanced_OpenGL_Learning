#version 460 core
layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 fragColor;

layout (binding = 0) uniform sampler2D tex0;

void main()
{
    vec3 sceneColor = vec3(texture(tex0, inUV).rgb);
    fragColor = vec4(sceneColor, 1.0);
}