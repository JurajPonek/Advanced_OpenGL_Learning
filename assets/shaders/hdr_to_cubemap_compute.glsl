#version 460 core

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0) uniform sampler2D hdr_map;

layout(rgba16f, binding = 1) writeonly uniform imageCube cube_map;

const vec2 invAtan = vec2(0.159154943, 0.318309886);

vec3 get_direction(ivec3 id, vec2 size)
{
    vec2 uv = ((vec2(id.xy) + 0.5) / vec2(size)) * 2.0 - 1.0;
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
    ivec2 image_size = imageSize(cube_map);
    if (pixel_pos.x >= image_size.x || pixel_pos.y >= image_size.y)
        return;
    vec3 dir = get_direction(pixel_pos, vec2(image_size.xy));
    vec2 uv = vec2(atan(dir.z, dir.x), asin(dir.y)) * invAtan + 0.5;

    vec4 color = textureLod(hdr_map, uv, 0.0);
    imageStore(cube_map, pixel_pos, color);

}

