#version 100
precision mediump float;

varying vec2 t_coord;

uniform sampler2D   texture;
uniform vec4        color;

void main()
{
    vec4 t_color = texture2D(texture, vec2(t_coord.x, t_coord.y));
    //gl_FragColor = vec4(1, 1, 1, color.r);
    gl_FragColor = t_color*color;
}
