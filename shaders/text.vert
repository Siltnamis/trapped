#version 330 core

layout(location = 0)    in vec2 vPosition;
//layout(location = 1)  in vec3 vNormal;
layout(location = 1)    in vec2 vTCoord;
layout(location = 2)    in vec4 in_color;

out     vec2    t_coord;
out     vec4    out_color;

uniform mat4    mvp_mat;
uniform vec2    size = vec2(1, 1);

void main()
{
    t_coord = vTCoord;
    out_color = in_color;
    gl_Position = mvp_mat * vec4(vPosition*size, 0, 1);
}
