#ifndef TEXTURE_H
#define TEXTURE_H

#include "av.h"
#include <GL/glew.h>

enum CubeMap_Enum   
{
    CubeMap_Right, CubeMap_Left,
    CubeMap_Up, CubeMap_Down,
    CubeMap_Back, CubeMap_Front
};
//hopefuly a saner version
struct Image
{
    char*   data;    
    int     w;
    int     h;
    int     components; //maybe deal only with rgba?
};

int  loadImage(Image* image, const char* file_name);
void freeImage(Image* image);

void setTextureParams2D();
void freeTexture(GLuint* texture);
void loadTexture2D(GLuint* texture, Image* image);
int  loadTexture2D(GLuint* texture, const char* file_name);

int  loadTextureCubeMap(GLuint* texture, 
                        const char* right,
                        const char* left,
                        const char* up,
                        const char* down,
                        const char* back,
                        const char* front);

int loadTextureCubeMap(GLuint* texture, Image images[6]);
int loadTextureCubeMap(GLuint* texture, const char* files[6]);
int createWhiteTexture(GLuint* texture);
#endif
