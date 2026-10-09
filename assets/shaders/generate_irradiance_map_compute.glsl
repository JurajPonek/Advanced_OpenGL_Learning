#version 460 core

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(binding = 0) uniform samplerCube environment_map;

layout(rgba16f, binding = 1) writeonly uniform imageCube irradiance_map;

const float PI = 3.14159265359;
const float mipLevel = 5.0;

vec3 get_direction(ivec3 id, vec2 size)
{
    vec2 uv = ((vec2(id.xy) + 0.5) / size) * 2.0 - 1.0;
    switch(id.z) {
        case 0: return normalize(vec3( 1.0, -uv.y, -uv.x)); // +X
        case 1: return normalize(vec3(-1.0, -uv.y,  uv.x)); // -X
        case 2: return normalize(vec3( uv.x,  1.0,  uv.y)); // +Y
        case 3: return normalize(vec3( uv.x, -1.0, -uv.y)); // -Y
        case 4: return normalize(vec3( uv.x, -uv.y,  1.0)); // +Z
        case 5: return normalize(vec3(-uv.x, -uv.y, -1.0)); // -Z
    }
    return vec3(0.0);
}

void main()
{
    ivec3 pixel_pos = ivec3(gl_GlobalInvocationID);
    ivec2 image_size = imageSize(irradiance_map);

    if (pixel_pos.x >= image_size.x || pixel_pos.y >= image_size.y)
        return;

    vec3 normal =  get_direction(pixel_pos, vec2(image_size));

    vec3 up = abs(normal.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(0.0, 0.0, 1.0);
    vec3 right = normalize(cross(up, normal));
    up = cross(normal, right);

    vec3 irradiance = vec3(0.0);

    float sampleDelta = 0.025; 
    float nrSamples = 0.0; 

    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // Sférické súradnice do lokálneho tangent space
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            
            // Tangent space do svetového priestoru podľa normály
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal; 

            irradiance += textureLod(environment_map, sampleVec, mipLevel).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }

    irradiance = PI * irradiance * (1.0 / nrSamples);

    imageStore(irradiance_map, pixel_pos, vec4(irradiance, 1.0));
}