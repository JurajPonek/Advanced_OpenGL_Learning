#version 460 core

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0) uniform sampler2D tex0;

layout(rgba8, binding = 1) uniform image2D img_output;



void main()
{
    ivec2 tex_coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 img_size = imageSize(img_output);
    vec2 texel_size = vec2(1.0 / img_size); 
    if (tex_coords.x >= img_size.x || tex_coords.y >= img_size.y)
        return;
  vec4 center = texelFetch(tex0, tex_coords, 0);
    vec4 right  = texelFetch(tex0, tex_coords + ivec2(1, 0), 0);
    vec4 up     = texelFetch(tex0, tex_coords + ivec2(0, 1), 0);

    vec4 result = (center + right + up) / 3.0;
    

    imageStore(img_output, tex_coords, result);
}
