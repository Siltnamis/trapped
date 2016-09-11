#ifndef INCLUDE_GAME_H
#define INCLUDE_GAME_H

#include <SDL2/SDL.h>
#include <GL/glew.h>

#define AV_LIB_IMPLEMENTATION
#include "av.h"
#undef AV_LIB_IMPLEMENTATION

#include "avmath.h"
#include "Texture.h"
#include "ShaderProgram.h"

enum Entity_Enum    { E_Player, E_Enemy, E_Max};
enum Shader_Enum    { Shader_Default, Shader_Max };
//enum Model_Enum     { Model_Cube, Model_Grid, Model_Max };
enum Shape_Enum     { Shape_Rect, Shape_Max };
enum Texture_Enum   { Texture_Default, Texture_Player, Texture_Enemy, 
                        Texture_Wall, Texture_Max };
enum Control_Enum   { CM_Jump, CM_Back, CM_Right, CM_Left};
enum PState_Enum    { PS_InAir, PS_OnWall, PS_OnLeftWall, PS_OnRightWall};


struct Vertex
{
    vec2 pos;
    vec2 uv;
};



struct CLK
{
    uint64 counter;
    uint64 frequency;

    CLK()
    {
        frequency =  SDL_GetPerformanceFrequency();
    }
    inline void start()
    {
        counter = SDL_GetPerformanceCounter();
    }
    inline float time()
    {
        return (float)(SDL_GetPerformanceCounter() - counter)/frequency;
    }
    inline float restart()
    {
        float t = time();
        start();
        return t;
    }
};

//probalby not needed.
//remove?
struct Controls
{
    bool    jump; 
    bool    focused;
    vec2    accel;
};

struct Renderer
{
    const char* vert_shaders[Shader_Max];
    const char* frag_shaders[Shader_Max];
    
    uint64      vs_change_times[Shader_Max];
    uint64      fs_change_times[Shader_Max];

    Shader      shaders[Shader_Max];
    GLuint      attrib_in_pos;
    GLuint      attrib_in_uv;

    GLuint      textures[Texture_Max];
    GLuint      vert_buffers[Shape_Max];
};

union Rect
{
    struct
    {
        vec2    position;
        vec2    size;
    };
    struct
    {
        float x, y, w, h;
    };
};

struct Entity
{
    int     type;
    bool    valid;
    int     shader_enum;
    int     shape_enum;
    int     texture_enum;
    vec2    position;
    vec2    velocity;
    vec4    color;
    //vec2    origin; not needed prob
    union
    {
        //player shit
        struct
        {
            Rect    rect;
            int     p_state;
        };
        //enemy shit
        struct
        {
        };
    };
};

struct Wall
{
    int     shader_enum;
    int     shape_enum; //is this reduntant?
    int     texture_enum;
    Rect    rect;
    vec4    color;
};

//TODO remove
/*
struct Player
{
    int         shader_enum;
    int         shape_enum;
    int         texture_enum;
    Rectangle   rect;
    vec2        velocity;
    bool        focused = false;
};
*/

struct EnemySpawner
{
    CLK     clock;    
    float   spawn_time;
};

#define MAX_ENTITIES 200
struct GameState
{
    bool            quit;
    Controls        controls;
    int             win_height;
    int             win_width;
    mat3            projection;

    int             score;
    
    Entity          entities[MAX_ENTITIES];
    EnemySpawner    e_spawner;

    Wall            left_wall;
    Wall            right_wall;


};

#endif //INCLUDE_GAME_H
