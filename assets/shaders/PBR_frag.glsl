#version 460 core

out vec4 frag_color;
in vec2 o_texture_coords;

uniform sampler2D tex0; //albedo_ao
uniform sampler2D tex1; //normal
uniform sampler2D tex2; //roughness_metallic
uniform sampler2DShadow tex3; //shadow map
uniform samplerCubeArray tex4; //omnidirectional shadow map
uniform sampler2D tex5; //ssao
uniform sampler2D tex6; //depth

uniform float far_plane;
uniform mat4 light_space_matrix;
uniform bool ssao;
uniform mat4 inv_view_proj;




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

const float PI = 3.14159265359;


vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 calculatePBR(vec3 N, vec3 V, vec3 L, vec3 radiance, vec3 albedo, float roughness, float metallic)
{
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    if (NdotL <= 0.0) return vec3(0.0);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    
    float NDF = DistributionGGX(N, H, roughness);        
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);  

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001;
    vec3 specular     = numerator / denominator;  

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

    return (kD * albedo / PI + specular) * radiance * NdotL;
}

float calculate_point_shadow(int index, vec4 frag_pos)
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

float calculate_directional_shadow(vec4 fragPosLightSpace, vec3 normal)
{
    vec3 proj_coords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    vec3 frag_coord = proj_coords * 0.5 + 0.5;
    if (frag_coord.z > 1.0 || 
        frag_coord.x < 0.0 || frag_coord.x > 1.0 || 
        frag_coord.y < 0.0 || frag_coord.y > 1.0)
    {
        return 0.0; 
    }
    float bias = max(0.05 * (1.0 - dot(normalize(normal), -direction)), 0.005); // -direction lebo chceme dopadajuci luc
    
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


void main()
{
    float depth = texture(tex6, o_texture_coords).r;
    if (depth >= 1.0) 
    {
        discard;
    }
    float ssao_factor = 1.0;
    if (ssao)
    {
        ssao_factor = texture(tex5, o_texture_coords).r;
    }
    vec4 sreen_space_coords = vec4(o_texture_coords * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 frag_pos = inv_view_proj * sreen_space_coords;
    frag_pos /= frag_pos.w;
    vec3 normal = normalize(texture(tex1, o_texture_coords).rgb);
    vec4 albedo_ao = texture(tex0, o_texture_coords);
    vec4 roughness_metallic = texture(tex2, o_texture_coords);
    float roughness = max(roughness_metallic.r, 0.001);
    float metallic = roughness_metallic.g;
    vec3 albedo = albedo_ao.rgb;
    float ao = albedo_ao.a;
    vec3 V = normalize(camera_position - frag_pos.xyz);

    //ambient
    vec3 total_light = ambient * albedo * ssao_factor * ao;
    //directional
    vec4 frag_pos_light_space = light_space_matrix * frag_pos;
    float dir_shadow = calculate_directional_shadow(frag_pos_light_space, normal);
    vec3 L_dir = normalize(-direction);
    vec3 dir_radiance = direction_color;
    total_light += (1.0 - dir_shadow) * calculatePBR(normal, V, L_dir, dir_radiance, albedo, roughness, metallic);
    //point
    for (int i = 0; i < num_of_points; ++i)
    {
        float dist = length(points[i].point_pos - frag_pos.xyz);
        if (dist > points[i].radius) continue;

        float point_shadow = 0.0;
        if (points[i].shadow_map_index >= 0)
        {
            point_shadow = calculate_point_shadow(i, frag_pos);
        }

        vec3 L_point = normalize(points[i].point_pos - frag_pos.xyz);
        float att = calculate_attenuation(dist, points[i].radius);
        vec3 radiance = points[i].point_color * att * points[i].intensity;

        total_light += (1.0 - point_shadow) * calculatePBR(normal, V, L_point, radiance, albedo, roughness, metallic);
    }
    frag_color = vec4(total_light, 1.0);
}







