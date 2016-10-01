#ifndef INCLUDE_GAME_H
#define INCLUDE_GAME_H

#ifdef __ANDROID__
    #include "SDL.h"
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
#else
    #include <SDL2/SDL.h>
    #include <GL/glew.h>

    #define AV_LIB_IMPLEMENTATION
    #include "av.h"
    #undef AV_LIB_IMPLEMENTATION
#endif

#include "avmath.h"
#include "Texture.h"
#include "ShaderProgram.h"

//max digits to display for score
#define MAX_DIGITS 5 

enum Entity_Enum    { E_Player, E_Enemy, E_Max};
enum Shader_Enum    { Shader_Default, Shader_Max };
enum Shape_Enum     { Shape_Rect, Shape_Max };
enum Texture_Enum   { Texture_Default, Texture_PlayerGround, Texture_PlayerAir, 
                        Texture_PlayerDead, Texture_EnemyR, Texture_EnemyL, 
                        Texture_Spike,Texture_GSpike, Texture_Alarm,
                        Texture_Digit0, Texture_Digit1, Texture_Digit2,
                        Texture_Digit3, Texture_Digit4, Texture_Digit5,
                        Texture_Digit6, Texture_Digit7, Texture_Digit8,
                        Texture_Digit9, Texture_Score, Texture_HScore,
                        Texture_Wall, Texture_BackGround, 
                        Texture_FB, 
                        Texture_DeathScreen,
                        Texture_Max };
enum Control_Enum   { CM_Jump, CM_Back, CM_Right, CM_Left };
enum PState_Enum    { PS_InAir, PS_OnWall, PS_OnLeftWall, PS_OnRightWall };


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
    bool            jump; 
    bool            focused;
    vec2            vel;
    SDL_Joystick*   accel;
};

struct Renderer
{
    //not used on mobile
    const char* vert_shaders[Shader_Max];
    const char* frag_shaders[Shader_Max];
    uint64      vs_change_times[Shader_Max];
    uint64      fs_change_times[Shader_Max];

    Shader      shaders[Shader_Max];
    GLuint      attrib_in_pos;
    GLuint      attrib_in_uv;

    GLuint      color_buffer;
    GLuint      frame_buffer;

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

struct EnemySpawner
{
    CLK     clock;    
    float   spawn_time;
};


enum Wall_Enum {Wall_Left, Wall_Right, Wall_Floor, Wall_Max };

struct Alarm
{
    int     shader_enum;
    int     shape_enum;
    int     texture_enum;
    Rect    rect;
    vec4    color;
    float   anim_time;
    bool    fading;
    bool    valid;
};

struct GSpike
{
    int     shader_enum;
    int     shape_enum;
    int     texture_enum;
    Rect    rect;
    vec4    color;
    float   life_time;
    bool    valid;
    bool    retract;
};

#define MAX_ENTITIES 200
struct GameState
{
    bool            quit; //possibly encode other states (menu/death/etc..)
    bool            focused; //  pack everything into int
    bool            dead;

    int             score;
    int             high_score;

    float           game_time;
    int             win_height;
    int             win_width;
    mat3            projection;
    
    Entity          entities[MAX_ENTITIES];
    EnemySpawner    e_spawner;

    float           spawn_increase_time;
    float           spawn_increase_rate;

    Wall            walls[Wall_Max];

    Wall            spikes[2];
    Alarm           alarm;
    GSpike          gspike;


    SDL_Joystick*   accel;
    //make into an array of 2?
};

#ifdef __ANDROID__
extern "C"
int initGame(Renderer* renderer, GameState* state, SDL_Window* window);
extern "C"
void drawState(Renderer* renderer, GameState* state);
extern "C"
void processEvent(GameState* state, SDL_Event* event, SDL_Window* window);
extern "C"
void processTick(GameState* state, float dt);
#endif


#endif //INCLUDE_GAME_H
