#version 460 core
layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 fragColor;

layout (binding = 0) uniform sampler2D tex0;
uniform float gamma;
uniform bool enable_hdr;

vec3 reinhard(vec3 hdrColor) 
{
    return hdrColor / (hdrColor + vec3(1.0));
}

vec3 aces(vec3 x) 
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{
    vec3 hdr_color = texture(tex0, inUV).rgb;
    if(enable_hdr)
    {
        hdr_color = aces(hdr_color);
    }
    vec3 corrected_color = pow(hdr_color, vec3(1.0/gamma));
    fragColor = vec4(corrected_color, 1.0);
}