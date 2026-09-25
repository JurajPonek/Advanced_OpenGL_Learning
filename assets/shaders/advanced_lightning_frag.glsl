#version 460 core

out vec4 frag_color;
in vec2 o_texture_coords;
in vec3 o_normal;
in vec4 frag_pos;
in vec4 frag_pos_light_space;
in mat3 TBN;
in vec3 view_dir_tbn;

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2DShadow tex3;
uniform samplerCubeArray tex4;

uniform float far_plane;
uniform bool use_normal_map;
uniform bool use_height_map;
uniform float height_scale;



layout(std140, binding = 0) uniform camera
{
    mat4 view;
    mat4 projection;
    vec3 camera_position;
};

struct PointLight
{
    vec3 point_pos;
    vec3 point_color;
    int shininess;
    float radius;
    float intensity;
    int shadow_map_index;
};

layout(std430, binding = 1) readonly buffer lights
{
    vec3 ambient;
    vec3 direction;
    vec3 direction_color;
    int num_of_points;
    PointLight points[];
};


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


float calculate_point_shadow(int index)
{
    vec3 light_pos = points[index].point_pos; 
    int shadow_map_idx = points[index].shadow_map_index; 

    vec3 sample_offset_directions[20] = vec3[]
        (
            vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
            vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
            vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
            vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
            vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
        ); 
    
    vec3 frag_to_light = frag_pos.xyz - light_pos;
    float view_distance = length(camera_position - frag_pos.xyz);
    float shadow = 0.0;
    int samples = 20;
    float bias = 0.005;
    float disk_radius = (1.0 + (view_distance / far_plane)) / 25.0;
    float current_depth = length(frag_to_light) / far_plane; // [0,1]
    if (current_depth > 1.0)
        return 0.0;
    for (int i = 0; i < samples; ++i)
    {
        float closest_depth = texture(tex4, vec4(frag_to_light + sample_offset_directions[i] * disk_radius, float(shadow_map_idx))).r; // [0,1]
        //texturu vzorkujeme pomocou 4D vektora vec4(smer.xyz, cislo_vrstvy)
        shadow += (current_depth - bias) > closest_depth ? 1.0 : 0.0;
    }
    return shadow/float(samples);

}

float calculate_directional_shadow(vec4 fragPosLightSpace)
{
    vec3 proj_coords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    vec3 frag_coord = proj_coords * 0.5 + 0.5;
    if (frag_coord.z > 1.0)
    {
        return 0.0;
    }
    float bias = max(0.05 * (1.0 - dot(normalize(o_normal), -direction)), 0.005); // -direction lebo chceme dopadajuci luc
    
    float currentDepth = frag_coord.z;
    vec2 texel_size = 1.0 / textureSize(tex3, 0);
    float shadow = 0.0;
    for (int x = -2; x <= 2; ++x)
    {
        for(int y = -2; y <= 2; ++y)
        {
            vec2 offset = vec2(x,y) * texel_size;
            shadow += texture(tex3, vec3(frag_coord.xy + offset, currentDepth - bias)); // pouzivame sampler2DShadow cize hardwerovo axcelerovane samplovanie 
            //funkcia texture() sama vykoná porovnanie a hardvérovo spriemeruje 4 susedné body zadarmo
            //funkcia texture() berie vec3 
            //.xy = UV súradnice v mape.
            //.z = Hĺbka, ktorú chceš otestovať (tvoja vzdialenosť mínus bias).
            //vracia float v rozsahu od 0.0 do 1.0 GPU to hardwerovo vyladilo
        }
    }
    shadow /= 25.0;
    return 1.0 - shadow;
}

float calculate_attenuation(float distance, float radius)
{
    float att = 1 / (distance * distance + 1.0f);
    
    float factor = distance / radius;
    float smoothCutoff = clamp(1.0 - factor * factor * factor * factor, 0.0, 1.0);
    smoothCutoff = smoothCutoff * smoothCutoff;

    return att * smoothCutoff;
}


vec3 calculate_ambient()
{
    return ambient;
}

vec3 calculate_direction(vec3 normal)
{
    vec3 ligth_dir = normalize(-direction);
    // vec3 normal = normalize(o_normal);
    float diff = max(dot(normal, ligth_dir), 0.0f);
    return direction_color * diff;
}

vec3 calculate_point(int index, vec3 normal)
{
    vec3 color = points[index].point_color;
    vec3 pos = points[index].point_pos;
    int shininess = points[index].shininess;
    float radius = points[index].radius;
    float intensity = points[index].intensity;

    float distance = length(pos - frag_pos.xyz);

    vec3 ligth_dir = normalize(pos - frag_pos.xyz);
    // vec3 normal = normalize(o_normal);
    vec3 view_dir = normalize(camera_position - frag_pos.xyz);
    vec3 halfway = normalize(view_dir + ligth_dir);
    float diff = max(dot(ligth_dir, normal), 0.0f);
    float spec = 0.0f;
    if (diff > 0.0f)
    {
        spec = pow(max(dot(halfway, normal), 0.0f), shininess); // vynasobvit spec mapou
    }
    float att = calculate_attenuation(distance, radius);
    return ((diff + spec) * att) * color * intensity;
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
    vec4 albedo = texture(tex0, tex_coords);
    vec3 ambient = calculate_ambient();
    vec3 direction = calculate_direction(normal);
    vec3 point = vec3(0.0);
    for (int i = 0; i < num_of_points; ++i)
    {
        float point_shadow = 0.0;
        if (points[i].shadow_map_index >= 0)
        {
            point_shadow += calculate_point_shadow(i);
        }
        point += (1.0 - point_shadow) * calculate_point(i, normal);
    }
    float dir_shadow = calculate_directional_shadow(frag_pos_light_space);
    frag_color = vec4((ambient + (1.0 - dir_shadow) * direction + point) * albedo.rgb, albedo.a);
}







