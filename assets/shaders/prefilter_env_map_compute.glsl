#version 460 core
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(binding = 0, rgba16f) writeonly uniform image2DArray out_cubemap;
layout(binding = 0) uniform samplerCube env_map;

uniform float roughness;
uniform float env_resolution;   
uniform int  sample_count;

const float PI = 3.14159265359;

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

vec2 Hammersley(uint i, uint N) { return vec2(float(i) / float(N), RadicalInverse_VdC(i)); }

float DistributionGGX(float NdotH, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a * a;
    float d  = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness * roughness;
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    vec3 H = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
    return normalize(tangent * H.x + bitangent * H.y + N * H.z);
}

vec3 FaceToDirection(int face, vec2 uv)
{
    switch (face)
    {
        case 0: return normalize(vec3( 1.0, -uv.y, -uv.x)); // +X
        case 1: return normalize(vec3(-1.0, -uv.y,  uv.x)); // -X
        case 2: return normalize(vec3( uv.x,  1.0,  uv.y)); // +Y
        case 3: return normalize(vec3( uv.x, -1.0, -uv.y)); // -Y
        case 4: return normalize(vec3( uv.x, -uv.y,  1.0)); // +Z
        default: return normalize(vec3(-uv.x, -uv.y, -1.0)); // -Z
    }
}

void main()
{
    ivec3 coords = ivec3(gl_GlobalInvocationID);
    ivec2 size = imageSize(out_cubemap).xy;
    if (coords.x >= size.x || coords.y >= size.y || coords.z >= 6)
        return;

    vec2 uv = (vec2(coords.xy) + 0.5) / vec2(size) * 2.0 - 1.0;
    vec3 N = FaceToDirection(int(coords.z), uv);
    vec3 R = N;
    vec3 V = N;

    if (roughness == 0.0) { imageStore(out_cubemap, coords, textureLod(env_map, N, 0.0)); return; }

    float saTexel = 4.0 * PI / (6.0 * env_resolution * env_resolution);

    vec3 color = vec3(0.0);
    float total_weight = 0.0;

    for (int i = 0; i < sample_count; ++i)
    {
        vec2 Xi = Hammersley(i, sample_count);
        vec3 H  = ImportanceSampleGGX(Xi, N, roughness);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0)
        {
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float D   = DistributionGGX(NdotH, roughness);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;

            float saSample = 1.0 / (float(sample_count) * pdf + 0.0001);
            float mip = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

            color += textureLod(env_map, L, mip).rgb * NdotL;
            total_weight += NdotL;
        }
    }
    color /= max(total_weight, 0.0001);
    imageStore(out_cubemap, coords, vec4(color, 1.0));
}