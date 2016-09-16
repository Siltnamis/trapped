#include <stdio.h>
#include "SDL.h"

static const char*  default_title = "Sickest game 4Head";
static int          default_height = 900;
static int          default_width = default_height*9/16;
static int          default_tick = 60;

static int          height = default_height;
static int          width = default_width;
static int          tick_rate = 60;

static SDL_Window*   window;
static SDL_GLContext context;


int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

#if 1
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
#endif
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    window = SDL_CreateWindow(default_title, SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED, width, height,
                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    context = SDL_GL_CreateContext(window);

    SDL_Delay(2000);


    SDL_Quit();
    return 0;
};
