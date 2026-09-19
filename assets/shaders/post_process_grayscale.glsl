#version 460 core

layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 fragColor;

layout (binding = 0) uniform sampler2D tex0;

void main()
{
    fragColor = texture(tex0, inUV);
    float average = 0.2126 * fragColor.r + 0.7152 * fragColor.g + 0.0722 * fragColor.b;
    fragColor = vec4(average, average, average, 1.0);
}