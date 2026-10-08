#version 460 core

layout (location = 0) out vec3 v_RayDir;
uniform mat4 u_InvViewProj;


void main()
{
    vec2 pos = vec2(
        (gl_VertexID == 1) ?  3.0 : -1.0,
               (gl_VertexID == 2) ?  3.0 : -1.0
    );

    gl_Position = vec4(pos, 1.0, 1.0);

    vec4 unprojected = u_InvViewProj * vec4(pos, 1.0, 1.0);
    v_RayDir = unprojected.xyz / unprojected.w;
}
