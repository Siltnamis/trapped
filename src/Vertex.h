#ifndef VERTEX_H
#define VERTEX_H

#include "av.h"
#include "avmath.h"

struct Vertex{
    Vertex(){};
    Vertex(const vec3 &pos, const vec3 &normal, const vec2 &tcoord){
        this->position = pos;
        this->normal = normal;
        this->textureCoord = tcoord;
    }
    vec3 position;
    vec3 normal;
    vec2 textureCoord;
};
#endif
