#version 460 core
layout (location = 0) out vec3 g_normal;
layout (location = 1) out vec4 g_albedo;


in vec2 o_texture_coords;
in vec3 o_normal;
in vec3 view_dir_tbn;
in mat3 TBN;

uniform bool use_normal_map;
uniform bool use_height_map;
uniform float height_scale;

uniform sampler2D tex0; //albedo
uniform sampler2D tex1; //normal
uniform sampler2D tex2; //height


vec2 calculate_parallax_mapping(vec3 view_dir, vec2 texture_coords)
{
    view_dir = normalize(view_dir);
    const float min_layers = 8.0;
    const float max_layers = 32.0;
    float num_of_layers = mix(max_layers, min_layers, max(dot(vec3(0.0, 0.0, 1.0), view_dir), 0.0));
    float step_size = 1.0 / num_of_layers;
    float current_layer_depth = 0.0;
    vec2 p = view_dir.xy / view_dir.z * height_scale;
    vec2 delta = p / num_of_layers;
    vec2 current_tex_coords = texture_coords;
    float current_depth_map_value = texture(tex2, current_tex_coords).r;

    while(current_layer_depth < current_depth_map_value)
    {
        current_tex_coords -= delta;
        current_depth_map_value = texture(tex2, current_tex_coords).r;
        current_layer_depth += step_size;
    }
    vec2 prev_tex_coords = current_tex_coords + delta;
    float after_depth = current_depth_map_value - current_layer_depth;
    float before_depth = texture(tex2, prev_tex_coords).r - current_layer_depth + step_size;
    float weight = after_depth / (after_depth - before_depth);
    vec2 final_texture_coords = prev_tex_coords * weight + current_tex_coords * (1.0 - weight);
    
    return final_texture_coords;
}


void main()
{
    vec2 tex_coords = vec2(0.0);
    if (use_height_map && height_scale > 0.00001)
    {
        tex_coords = calculate_parallax_mapping(view_dir_tbn, o_texture_coords);
        if (tex_coords.x > 1.0 || tex_coords.y > 1.0 || tex_coords.x < 0.0 || tex_coords.y < 0.0)
        {
            discard;
        }
    }
    else
    {
        tex_coords = o_texture_coords;
    }

    vec3 normal = vec3(0.0);
    if (use_normal_map)
    {
        normal = texture(tex1, tex_coords).rgb * 2.0 - 1.0;
        normal = normalize(TBN * normal);
    }
    else
    {
        normal = normalize(o_normal);
    }

    g_normal = normal;
    g_albedo = vec4(texture(tex0, tex_coords).rgb, 1.0f);
}