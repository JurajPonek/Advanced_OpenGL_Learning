#version 460 core

out vec4 frag_color;
in vec2 o_texture_coords;
in vec3 o_normal;
in vec4 frag_pos;
in vec4 frag_pos_light_space;
uniform sampler2D tex0;
uniform sampler2DShadow tex1;
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
};

layout(std430, binding = 1) readonly buffer lights
{
    vec3 ambient;
    vec3 direction;
    vec3 direction_color;
    int num_of_points;
    PointLight points[];
};


float calculate_shadow(vec4 fragPosLightSpace)
{
    vec3 proj_coords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    vec3 frag_coord = proj_coords * 0.5 + 0.5;
    if (frag_coord.z > 1.0)
    {
        return 0.0;
    }
    float bias = max(0.05 * (1.0 - dot(normalize(o_normal), -direction)), 0.005); // -direction lebo chceme dopadajuci luc
    
    float currentDepth = frag_coord.z;
    vec2 texel_size = 1.0 / textureSize(tex1, 0);
    float shadow = 0.0;
    for (int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            vec2 offset = vec2(x,y) * texel_size;
            shadow += texture(tex1, vec3(frag_coord.xy + offset, currentDepth - bias)); // pouzivame sampler2DShadow cize hardwerovo axcelerovane samplovanie 
            //funkcia texture() sama vykoná porovnanie a hardvérovo spriemeruje 4 susedné body zadarmo
            //funkcia texture() berie vec3 
            //.xy = UV súradnice v mape.
            //.z = Hĺbka, ktorú chceš otestovať (tvoja vzdialenosť mínus bias).
            //vracia float v rozsahu od 0.0 do 1.0 GPU to hardwerovo vyladilo
        }
    }
    shadow /= 9.0;
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

vec3 calculate_direction()
{
    vec3 ligth_dir = normalize(-direction);
    vec3 normal = normalize(o_normal);
    float diff = max(dot(normal, ligth_dir), 0.0f);
    return direction_color * diff;
}

vec3 calculate_point(int index)
{
    vec3 color = points[index].point_color;
    vec3 pos = points[index].point_pos;
    int shininess = points[index].shininess;
    float radius = points[index].radius;
    float intensity = points[index].intensity;

    float distance = length(pos - frag_pos.xyz);

    vec3 ligth_dir = normalize(pos - frag_pos.xyz);
    vec3 normal = normalize(o_normal);
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
    vec4 albedo = texture(tex0, o_texture_coords);
    vec3 ambient = calculate_ambient();
    vec3 direction = calculate_direction();
    vec3 point = vec3(0.0);
    for (int i = 0; i < num_of_points; ++i)
    {
        point += calculate_point(i);
    }
    float shadow = calculate_shadow(frag_pos_light_space);
    frag_color = vec4((ambient + (1.0 - shadow) * direction + point) * albedo.rgb, albedo.a);
}







