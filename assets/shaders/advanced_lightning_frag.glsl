#version 460 core

out vec4 frag_color;
in vec2 o_texture_coords;
in vec3 o_normal;
in vec4 frag_pos;
uniform sampler2D tex0;
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
    vec3 attenuation;
};

layout(std430, binding = 1) readonly buffer lights
{
    vec3 ambient;
    vec3 direction;
    vec3 direction_color;
    int num_of_points;
    PointLight points[];
};


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
    vec3 attenuation = points[index].attenuation;
    vec3 color = points[index].point_color;
    vec3 pos = points[index].point_pos;

    float distance = length(pos - frag_pos.xyz);
    float att = 1.0 / (attenuation.x + (attenuation.y * distance) + (attenuation.z * (distance * distance)));

    vec3 ligth_dir = normalize(pos - frag_pos.xyz);
    vec3 normal = normalize(o_normal);
    vec3 view_dir = normalize(camera_position - frag_pos.xyz);
    vec3 halfway = normalize(view_dir + ligth_dir);
    float diff = max(dot(ligth_dir, normal), 0.0f);
    if (diff > 0.0f)
    {
        float spec = pow(max(dot(halfway, normal), 0.0f), 64);
    }
    else 
    {
        float spec = 0.0f;
    }
    float spec = pow(max(dot(halfway, normal), 0.0f), 64); // vynasobvit spec mapou
    return ((diff + spec) * att) * color;
}

void main()
{
    vec4 albedo = texture(tex0, o_texture_coords);
    vec3 color = calculate_ambient() + calculate_direction();
    for (int i = 0; i < num_of_points; ++i)
    {
        color += calculate_point(i);
    }
    frag_color = vec4(color * albedo.rgb, albedo.a);
}







