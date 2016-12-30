#ifndef MESH_H
#define MESH_H

#include "av/av.h"
#include "av/avmath.h"
#include "GL/glew.h"

struct Vertex2D
{
    vec2    pos;
    vec2    uv;
    vec4    color;
};

struct Vertex3D
{
    vec3    pos;
    vec3    normal; 
    vec2    uv;
    //vec4  color
};

struct MeshData
{
    GLenum      mode;
    Vertex3D*   vertices;
    uint16*     indices;
    //vec3*       pos;
    //vec3*       normals;   
    //vec2*       uvs;    //needed for mesh?
    int32       vertex_count;
    int32       index_count;
};

struct Mesh
{
    GLenum  mode;
    GLuint  vao;
    GLuint  vertex_buff;
    GLuint  index_buff;
    int32   index_count;
    int32   vertex_count;
};

void loadMesh(Mesh* mesh, const MeshData* data);
void freeMesh(Mesh* mesh);
void generateFlatGrid(MeshData* data, float sizex, float sizez, int numx, int numz);
void generateFlatTerrain(MeshData* data, float sizex, float sizez, int numx, int numz);
void generatePlane(MeshData* data, vec3 a, vec3 b, vec3 c, vec3 d, float uv = 1.f);
void generateCube(MeshData* data, float sz);
void generateSkybox(MeshData* data, float sz);
void freeMeshData(MeshData* data);



#endif
