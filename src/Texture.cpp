#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
//
#ifdef __ANDROID__
#include "SDL.h"
int loadImage(Image* image, const char* file_name)
{
    SDL_RWops* file = SDL_RWFromFile(file_name, "rb"); 
    if(file != NULL){
        int64 size = SDL_RWsize(file);
        if(size != -1){
            char* buffer = (char*)malloc(size+1);
            if(buffer != NULL){
                int64 read_size = SDL_RWread(file, buffer, 1, size);
                if(read_size == size){

                    image->data =(char*)stbi_load_from_memory((const unsigned char*)buffer,
                           size, &image->w, &image->h, &image->components, 4); 
                    if(image->data != NULL)
                        return 0;
                    else
                        return -1;

                }else{
                    free(buffer);
                    SDL_RWclose(file);
                    return -1;
                }
            }else{
                SDL_RWclose(file);
                return -1;
            }
        }else{
            SDL_RWclose(file);
            return -1;
        }
    }else{
        return -1;
    }
}
#else
int loadImage(Image* image, const char* file_name)
{
    image->data = (char*)stbi_load(file_name, &image->w, &image->h, &image->components, 4);
    if(image->data == NULL){
        return -1; 
    }
    return 0;
}
#endif

void freeImage(Image* image)
{
    stbi_image_free(image->data);
}

void setTextureParams2D()
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //Set anisotropic filtering
    //TODO::check if supported?
    //float amount;
    //glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &amount);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fmin(4.f,amount));  

 	//set mipmap base and max level
    //defualts are good enaugh probably
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 2);
    //generate mipmaps
	//glGenerateMipmap(GL_TEXTURE_2D); 
}

void freeTexture(GLuint* texture)
{
    //glBindTexture(0);
    glDeleteTextures(1, texture);
}

void loadTexture2D(GLuint* texture, Image* image)
{
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data);
    setTextureParams2D();
}

int loadTexture2D(GLuint* texture, const char* file_name)
{
    Image img;
    int status = loadImage(&img, file_name); 
    if(status != 0)
        return -1;
    loadTexture2D(texture, &img);
    freeImage(&img);
    return 0;
}

int loadTextureCubeMap(GLuint* texture, Image images[6])
{
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, *texture);

    for(int i = 0; i < 6; ++i){
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, 
                images[i].w, images[i].h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                images[i].data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    return 0;
}

int loadTextureCubeMap(GLuint* texture, const char* files[6])
{
#if 0
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, *texture);
    for(int i = 0; i < 6; ++i){
        Image img;
        int status = loadImage(&img, files[i]);
        if(status != 0)
            return -1;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA,
                img.w, img.w, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.data);
        freeImage(&img);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    return 0;
#else
    Image images[6];
    int status;
    for(int i = 0; i < 6; ++i){
        status = loadImage(&images[i], files[i]);
        if(status != 0){
            for(int j = 0; j < i; ++j)
                freeImage(&images[j]);
            return -1;
        }
    }
    return loadTextureCubeMap(texture, images);
#endif
}

int loadTextureCubeMap(GLuint* texture, 
        const char* right,
        const char* left,
        const char* up,
        const char* down,
        const char* back,
        const char* front)
{
#if 1 
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, *texture);
    const char* files[6] = {right, left, up, down, back, front};
    for(int i = 0; i < 6; ++i){
        Image img;
        int status = loadImage(&img, files[i]);
        if(status != 0)
            return -1;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, 
            img.w, img.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.data);
        freeImage(&img);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif
    return 0;
}


int createWhiteTexture(GLuint* texture)
{
    glGenTextures(1, texture);
    char pixel[4] = {(char)255, (char)255, (char)255, (char)255};
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    return 0;
}
