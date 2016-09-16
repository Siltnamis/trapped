#ifndef SHADER_H
#define SHADER_H

#ifdef __ANDROID__
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#include <GL/glew.h>
#endif

#include "av.h"
#include "string.h"

#define MAX_UNIFORMS 10

struct Shader
{
    GLuint  program = 0;
    GLuint  vertex = 0;
    GLuint  fragment = 0;
    GLuint  geometry = 0;
    int8    uniform_count = 0;

    GLint   uniforms[MAX_UNIFORMS];
    char*   uniform_names[MAX_UNIFORMS]; 

    GLint getUniform(const char* name)
    {
        for(int i = 0; i < uniform_count; ++i){
            if(uniform_names[i]){
                if( strcmp(uniform_names[i], name) == 0 ){
                    return uniforms[i];
                }
            }
        }
        return -1;
    }
        
    void addUniforms(int num, ...) 
    {
        uniform_count = num;
        va_list va;
        va_start(va, num);
        for(int i = 0; i < num; ++i){
            char* name = va_arg(va, char*);
            i32 len = strlen(name);
            uniform_names[i] = (char*)calloc(len+1, sizeof(char));
            strcpy(uniform_names[i], name);
            uniforms[i] = glGetUniformLocation(program, name);  
            if(uniforms[i] == -1){
                fprintf(stderr, "Program:%d\tno uniform variable named %s\n",
                       program, name);
            }
        } 
        va_end(va); 
    }

    void clearUniforms()
    {
        for(int i = 0; i < uniform_count; ++i){
            uniforms[i] = 0;
            if(uniform_names[i]){
                free(uniform_names[i]);
            }
            uniform_names[i] = nullptr;
        }
        uniform_count = 0;
    }

    bool loadFromFile(GLenum type, const char* file_name)
    {   
        bool status = false;
        FILE* source = fopen(file_name, "r"); 
        if(source){
            fseek(source, 0, SEEK_END); 
            u32 size = ftell(source);
            fseek(source, 0, SEEK_SET);

            char* source_str = (char*)calloc(size+1, sizeof(char));
            status = ( fread(source_str, sizeof(char), size, source) == size );
            loadFromString(type, source_str); 

            free(source_str);
            fclose(source);
        } else {
            fprintf(stderr, "Failed to open file: %s\n", file_name);
        }
        return status;
    }

    void loadFromString(GLenum type, const char* source)   
    {
        char vertex_name[]  = "vertex";
        char fragment_name[]= "fragment";
        char geometry_name[]= "geometry";
        char* shader_name;
        GLuint shader = glCreateShader(type);

        if(type == GL_VERTEX_SHADER){
            vertex = shader;
            shader_name = vertex_name;
        } else if(type == GL_FRAGMENT_SHADER){
            fragment = shader;
            shader_name = fragment_name;
        } 
#ifdef NO_GEOMETRY_SHADER
        else if(type == GL_GEOMETRY_SHADER){
            geometry = shader;
            shader_name = geometry_name;
        }
#endif

        glShaderSource(shader, 1,&source, nullptr);
        GLint status;
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if(status == GL_FALSE){
            GLint log_length;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
            char* info_log = (char*)calloc(log_length+1, sizeof(char));
            glGetShaderInfoLog(shader, log_length, nullptr, info_log);
            fprintf(stderr, "Compile failure in %s shader:\n%s\n", shader_name, info_log);
            free(info_log);
        }
    }

    void createAndLinkProgram()
    {
        program = glCreateProgram();
        if(vertex)   glAttachShader(program, vertex);    
        if(geometry) glAttachShader(program, geometry);    
        if(fragment) glAttachShader(program, fragment);    

        GLint status;
        glLinkProgram(program); 
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if(status == GL_FALSE){
            GLint log_length;  
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
            char* info_log = (char*)calloc(log_length+1, sizeof(char));
            glGetProgramInfoLog(program, log_length, nullptr, info_log);
            fprintf(stderr, "Linker failure: %s\n", info_log);
            free(info_log);
        }
        if(vertex){
            glDetachShader(program, vertex);    
            glDeleteShader(vertex);
            vertex = 0;
        }
        if(fragment){
            glDetachShader(program, fragment);    
            glDeleteShader(fragment);
            fragment = 0;
        }
        if(geometry){
            glDetachShader(program, geometry); 
            glDeleteShader(geometry);
            geometry = 0;
        }
    } 

    void deleteShaderProgram()
    {
        glDeleteProgram(program);
        program = 0;
        clearUniforms();
    }

    void use()
    {
        glUseProgram(program);
    }
    void unuse()
    {
        glUseProgram(0);
    }
};
#endif
