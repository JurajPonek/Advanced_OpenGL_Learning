#version 460 core
out vec4 frag_color;
in vec2 o_texture_coords;
in vec3 o_normal;
in vec4 frag_pos;
uniform sampler2D tex0;
uniform sampler2D tex1;

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

vec4 calculate_ambient()
{
    return vec4(ambient, 1.0f);
}

vec4 calculate_direction()
{
    vec3 ligth_dir = normalize(-direction);
    float diff =  max(dot(o_normal, ligth_dir), 0.0f);
    vec4 dir_color = vec4(direction_color * diff, 1.0f);
    return dir_color;
}

vec4 calculate_point(int index)
{
    vec3 attenuation = points[index].attenuation;
    vec3 color = points[index].point_color;
    vec3 pos = points[index].point_pos;

    float distance = length(pos - frag_pos.xyz);
    float att = 1.0 / (attenuation.x + (attenuation.y * distance) + (attenuation.z * (distance * distance)));
    vec3 ligth_dir = normalize(pos - frag_pos.xyz);
    float diff = max(dot(ligth_dir, o_normal), 0.0f);
    vec3 reflected_dir = reflect(-ligth_dir, o_normal);
    vec3 camera_dir = normalize(camera_position - frag_pos.xyz);
    float spec = pow(max(dot(reflected_dir, camera_dir), 0.0f), 32) * (texture(tex1, o_texture_coords).r);
    return vec4(((diff + spec) * att)  * color, 1.0f);
}


void main()
{
    vec4 albedo = texture(tex0, o_texture_coords);
    vec4 color = calculate_ambient();
    color += calculate_direction();
    for (int i = 0; i < num_of_points; i++)
    {
        color += calculate_point(i);
    }
    frag_color = color * albedo;
}