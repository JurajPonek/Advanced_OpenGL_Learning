#version 460 core
layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 fragColor;

layout (binding = 0) uniform sampler2D tex0;
uniform float gamma;

void main()
{
    vec3 linear_color = vec3(texture(tex0, inUV).rgb);
    vec3 corrected_color = pow(linear_color, vec3(1.0/gamma));
    fragColor = vec4(corrected_color, 1.0);
}