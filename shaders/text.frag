#version 330 core

in vec2 t_coord;
in vec4 out_color;

uniform sampler2D textureMap;

void main()
{
    vec3 scolor = out_color.xyz;
    float val = texture2D(textureMap, vec2(t_coord.x, t_coord.y)).r;
    gl_FragColor = vec4(scolor, val);
    //gl_FragColor = out_color * vec4(1.0, 1.0, 1.0, val);
    //gl_FragColor = vec4(val, val, val, val);
    //gl_FragColor = vec4(1, 1, 1, 1.0);
    //gl_FragColor = texture2D(textureMap, vec2(t_coord.x, t_coord.y));
}
