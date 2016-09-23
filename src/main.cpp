#ifdef __ANDROID__
    #include "SDL.h"
    #define AV_LIB_IMPLEMENTATION
    #include "av.h"
#undef AV_LIB_IMPLEMENTATION

#else
#include <SDL2/SDL.h>
#endif



#include "Game.h"

#ifdef _WIN32
static const char*  game_lib = "./game.dll";
static const char*  game_lib_copy = "./game_cp.dll";
#else
static const char*  game_lib = "./game.so";
static const char*  game_lib_copy = "./game_cp.so";
#endif

static uint64       game_lib_change_time = 0;
static void*        game_lib_handle = NULL;

#ifndef __ANDROID__
int                 (*initGame)(Renderer*, GameState*, SDL_Window*);
void                (*processEvent)(GameState*, SDL_Event*, SDL_Window*);
void                (*processTick)(GameState*, float);
void                (*drawState)(Renderer*, GameState*);
#endif

static const char*  default_title = "Sickest game 4Head";
static int          default_height = 900;
static int          default_width = default_height*9/16;
static int          default_tick = 60;

static int          height = default_height;
static int          width = default_width;
static int          tick_rate = 60;


static GameState    game_state;
static Renderer     renderer;

static SDL_Window*   window;
static SDL_GLContext context;

#ifdef __ANDROID__
void loadGame()
{

}
#else
void loadGame()
{
    uint64  new_change_time = fileChangeTime(game_lib);
    if(new_change_time > game_lib_change_time){

        if(game_lib_handle){
            int free_status = freeLibrary(game_lib_handle);
            for(int i = 0; i < 1000; ++i){
                if(free_status == 0){
                    game_lib_handle = NULL;
                    break; 
                }
                //printf("failed to free lib %s, trying again\n", dlerror()); 
            }
        }
        printf("test wait for close lib handle\n");
        SDL_Delay(150);
        printf("woke up\n");

        int status;
        printf("trying to copy libs\n");
        for(int i = 0; i < 1000; ++i){
            status = copyFile(game_lib, game_lib_copy);
            if(status == 0) break;
            printf("failed to copy libs %s %s\n", game_lib, game_lib_copy);
            SDL_Delay(1);
        }
        assert(status == 0);
        printf("wiating for whatever the fuck\n");
        //SDL_Delay(1500);
        printf("woke up\n");
             
        printf("loading lib %s\n", game_lib_copy);
        game_lib_handle = loadLibrary(game_lib_copy);
        if(game_lib_handle){
            printf("getting function pointers\n");
            *(void**)(&initGame) = getSymAddress(game_lib_handle,
                                        "initGame"); 
            *(void**)(&processEvent) = getSymAddress(game_lib_handle,
                                        "processEvent");
            *(void**)(&processTick) = getSymAddress(game_lib_handle,
                                        "processTick");
            *(void**)(&drawState) = getSymAddress(game_lib_handle, 
                                        "drawState");
            assert(processEvent && initGame && processTick && drawState);
             
        }else{
            //printf("%s\nfailed to laod lib %s\n", dlerror(), game_lib_copy);
            assert(game_lib_handle);
        }
        game_lib_change_time = new_change_time;
    }
}
#endif

void myGlMessageCallback(GLenum source,
            GLenum type,
            GLuint id,
            GLenum severity,
            GLsizei length,
            const GLchar *message,
            void *userParam)
{
    printf("%s\n", message);
}

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

//#ifdef __ANDROID__ 
#if 1
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif
    

    

    int arg_index = checkArg(argc, argv, "-h");
    if(arg_index != -1 && arg_index < argc-1){
        height = strToInt(argv[arg_index+1]);
        width = height*16/9;
    }
    arg_index = checkArg(argc, argv, "-w");
    if(arg_index != -1 && arg_index < argc-1){
        width = strToInt(argv[arg_index+1]);
    }
    arg_index = checkArg(argc, argv, "-tick");
    if(arg_index != -1 && arg_index < argc-1){
        tick_rate = strToInt(argv[arg_index+1]);
    }

    window = SDL_CreateWindow(default_title, SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED, width, height,
                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    context = SDL_GL_CreateContext(window);

#ifndef __ANDROID__
    glewExperimental = GL_TRUE;
    GLenum status = glewInit();

    if(status != GLEW_OK){
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Init error",
                (const char*)glewGetErrorString(status), 0);
        return -1;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback((GLDEBUGPROC)myGlMessageCallback, NULL);
    printf("\tUsing glew %s\n", glewGetString(GLEW_VERSION));
#endif

    printf("\tVendor: %s\n", glGetString (GL_VENDOR));
    printf("\tRenderer: %s\n", glGetString (GL_RENDERER));
    printf("\tVersion: %s\n", glGetString (GL_VERSION));
    printf("\tGLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    SDL_Log("DISABLING VSYNC LUL\n");
    int st = SDL_GL_SetSwapInterval(0);
    if(st == -1){
        printf("failed to disable vsync, setting tick_rate to default: %d\n", 60);
        tick_rate = 60;
    }
    
    loadGame();
    initGame(&renderer, &game_state, window);
    
    CLK loop_clock;
    CLK perf_clock;

    float dt = 1.f/tick_rate;
    float accumulator_time = 0.f;
    float perf_time = 0.f;

    int frames = 0;
    
    loop_clock.start();
    perf_clock.start();
    SDL_Log("WTFFSFASDS\n");
    SDL_Log("STARTING TEST\n");
    while(!game_state.quit){
        accumulator_time = loop_clock.time();
        loadGame();
        if(accumulator_time > dt){
            perf_time += loop_clock.restart();

            SDL_Event event;
            while(SDL_PollEvent(&event))
                processEvent(&game_state, &event, window);

            processTick(&game_state, dt);
            drawState(&renderer, &game_state);
            SDL_GL_SwapWindow(window);

            accumulator_time -= dt;
            ++frames;
        }else{
#ifndef __ANDROID__
            SDL_Delay((int)((dt-accumulator_time)*1000.f));
#endif
        }
        if(perf_clock.time() > 1.f){
            float avg_time = perf_time/frames; 
            printf("fps: %d   avg time: %f\n", frames, avg_time);
            SDL_Log("fps: %d   avg time: %f\n", frames, avg_time);
            perf_time = 0;
            frames = 0;
            perf_clock.restart();
        }
    }
    SDL_Quit();

    return 0;
}
