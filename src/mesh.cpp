#include "mesh.h"

void loadMesh(Mesh* mesh, const MeshData* data)
{
    mesh->mode = data->mode;
    mesh->index_count = data->index_count;
    mesh->vertex_count = data->vertex_count;
    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);

    glGenBuffers(1, &mesh->vertex_buff);
    glGenBuffers(1, &mesh->index_buff);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buff);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertex_count*sizeof(Vertex3D),
            data->vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_buff);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->index_count*sizeof(uint16),
            data->indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);   //positions
    glEnableVertexAttribArray(1);   //normals
    glEnableVertexAttribArray(2);   //uvs

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), 
            (const void*)offsetof(Vertex3D, normal));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), 
            (const void*)offsetof(Vertex3D, uv));
}

void generatePlane(MeshData* data, vec3 a, vec3 b, vec3 c, vec3 d, float uv)
{
    data->vertex_count = 4;
    data->vertices = (Vertex3D*)malloc(sizeof(Vertex3D)*4);      
    vec3 normal = normalize(cross(b-a, d-a));

    data->vertices[0].normal = normal;
    data->vertices[0].pos = a;
    data->vertices[0].uv = {0, 0};

    data->vertices[1].normal = normal;
    data->vertices[1].pos = b;
    data->vertices[1].uv = {0, uv};

    data->vertices[2].normal = normal;
    data->vertices[2].pos = c;
    data->vertices[2].uv = {uv, uv};

    data->vertices[3].normal = normal;
    data->vertices[3].pos = d;
    data->vertices[3].uv = {uv, 0};

    data->index_count = 6;
    data->indices = (uint16*)malloc(sizeof(uint16*)*6);

    data->indices[0] = 0;
    data->indices[1] = 1;
    data->indices[2] = 2;
    data->indices[3] = 0;
    data->indices[4] = 2;
    data->indices[5] = 3;

    data->mode = GL_TRIANGLES;
}

void generateCube(MeshData* data, float sz)
{
    MeshData sides[6];

    float size = sz/2.f;
#if 0 // y up
    //front
    generatePlane(&sides[0], {-size, size, size},    {-size,-size, size},
                             {size, -size, size},    {size, size, size});
    //back
    generatePlane(&sides[1], {size, size, -size},    {size, -size, -size},
                             {-size, -size, -size,}, {-size, size, -size});
    //left
    generatePlane(&sides[2], {-size, size, -size},   {-size, -size, -size,}, 
                             {-size, -size, size},   {-size, size, size});
    //right
    generatePlane(&sides[3], {size, size, size},     {size, -size, size}, 
                             {size, -size, -size},   {size, size, -size});
    //top
    generatePlane(&sides[4], {-size, size, -size},   {-size, size, size}, 
                             {size, size, size},     {size, size, -size});
    //bottom
    generatePlane(&sides[5], {-size, -size, size},   {-size, -size, -size}, 
                             {size, -size, -size},   {size, -size, size});
#else // z up
    //front
    generatePlane(&sides[0], {-size, -size, size},    {-size,-size, -size},
                             {size, -size, -size},    {size, -size, size});
    //back
    generatePlane(&sides[1], {size, size, size},    {size, size, -size},
                             {-size, size, -size,}, {-size, size, size});
    //left
    generatePlane(&sides[2], {-size, size, size},   {-size, size, -size,}, 
                             {-size, -size, -size},   {-size, -size, size});
    //right
    generatePlane(&sides[3], {size, -size, size},     {size, -size, -size}, 
                             {size, size, -size},   {size, size, size});
    //top
    generatePlane(&sides[4], {-size, size, size},   {-size, -size, size}, 
                             {size, -size, size},     {size, size, size});
    //bottom
    generatePlane(&sides[5], {-size, -size, -size},   {-size, size, -size}, 
                             {size, size, -size},   {size, -size, -size});

#endif

    data->vertex_count = 4*6;
    data->vertices = (Vertex3D*)malloc(sizeof(Vertex3D)*4*6);
    for(int i = 0; i < 6; ++i){
        memcpy(&data->vertices[i*4], sides[i].vertices, sizeof(Vertex3D)*4);
    } 

    data->index_count = 6*6;
    data->indices = (uint16*)malloc(sizeof(uint16)*6*6);
    for(int i = 0, j = 0; i < 36; i += 6, j += 4){
        data->indices[i + 0] = 0 + j;
        data->indices[i + 1] = 1 + j;
        data->indices[i + 2] = 2 + j;
        data->indices[i + 3] = 0 + j;
        data->indices[i + 4] = 2 + j;
        data->indices[i + 5] = 3 + j;
    }
    data->mode = GL_TRIANGLES;
   
    for(int i = 0; i < 6; ++i){
        freeMeshData(&sides[i]);
    }
}

void generateSkybox(MeshData* data, float sz)
{
    MeshData sides[6];

    float size = sz/2.f;

    //front
    generatePlane(&sides[0], {-size, -size, size},    {-size,-size, -size},
                             {size, -size, -size},    {size, -size, size});
    //back
    generatePlane(&sides[1], {size, size, size},    {size, size, -size},
                             {-size, size, -size,}, {-size, size, size});
    //left
    generatePlane(&sides[2], {-size, size, size},   {-size, size, -size,}, 
                             {-size, -size, -size},   {-size, -size, size});
    //right
    generatePlane(&sides[3], {size, -size, size},     {size, -size, -size}, 
                             {size, size, -size},   {size, size, size});
    //top
    generatePlane(&sides[4], {-size, size, size},   {-size, -size, size}, 
                             {size, -size, size},     {size, size, size});
    //bottom
    generatePlane(&sides[5], {-size, -size, -size},   {-size, size, -size}, 
                             {size, size, -size},   {size, -size, -size});

    data->vertex_count = 4*6;
    data->vertices = (Vertex3D*)malloc(sizeof(Vertex3D)*4*6);
    for(int i = 0; i < 6; ++i){
        memcpy(&data->vertices[i*4], sides[i].vertices, sizeof(Vertex3D)*4);
    } 

    data->index_count = 6*6;
    data->indices = (uint16*)malloc(sizeof(uint16)*6*6);
    for(int i = 0, j = 0; i < 36; i += 6, j += 4){
        data->indices[i + 0] = 3 + j;
        data->indices[i + 1] = 2 + j;
        data->indices[i + 2] = 1 + j;
        data->indices[i + 3] = 3 + j;
        data->indices[i + 4] = 1 + j;
        data->indices[i + 5] = 0 + j;
    }
    data->mode = GL_TRIANGLES;
   
    for(int i = 0; i < 6; ++i){
        freeMeshData(&sides[i]);
    }

}

void generateFlatGrid(MeshData* data, float sizex, float sizez, int numx, int numz)
{
    int vertex_count = (numx + numz + 2)*2;
    data->vertices = (Vertex3D*)malloc(sizeof(Vertex3D)*vertex_count);
    int v_index = 0;

    for(int i = 0; i <= numz; ++i){
        data->vertices[v_index++].pos= {0 - sizex/2, 0, i*sizez/numz - sizez/2};
        data->vertices[v_index++].pos= {sizex - sizex/2, 0, i*sizez/numz - sizez/2};
    }
    for(int i = 0; i <= numz; ++i){
        data->vertices[v_index++].pos = {i*sizex/numx - sizex/2, 0, 0 - sizez/2};
        data->vertices[v_index++].pos = {i*sizex/numx - sizex/2, 0, sizez - sizez/2};
    }
    data->vertex_count = vertex_count;

    data->indices = (uint16*)malloc(sizeof(uint16)*vertex_count);
    data->index_count = vertex_count;
    for(int i = 0; i < vertex_count; ++i){
        data->indices[i] = i;
    }
    data->mode = GL_LINES;
}


void generateFlatTerrain(MeshData* data, float sizex, float sizey, int numx, int numy)
{
    int vertex_count = (numx + numy + 2)*2;
    data->vertices = (Vertex3D*)malloc(sizeof(Vertex3D)*vertex_count);
}

void freeMeshData(MeshData* data)
{
    free(data->vertices);
    free(data->indices);
}

void freeMesh(Mesh* mesh)
{
    glDeleteVertexArrays(1, &mesh->vao);
    glDeleteBuffers(1, &mesh->index_buff);
    glDeleteBuffers(1, &mesh->vertex_buff);
}

