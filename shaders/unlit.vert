#version 100
precision mediump float;

attribute vec2 in_pos;
attribute vec2 in_uv;

varying vec2 t_coord;

uniform mat3 mvp_mat;	

void main()
{
    t_coord = in_uv;
    gl_Position = vec4((mvp_mat * vec3(in_pos, 1)), 1);
    //gl_Position = vec4(0, 0, in_pos.x, 1);
}
