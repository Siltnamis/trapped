#include "Game.h"

static const int frame_buff_w = 720;
static const int frame_buff_h = 1280;

//most likely not needed
static const vec2 default_player_size   = {80, 100};
static const vec2 default_player_pos    = {frame_buff_w/2 - 80/2, 20};

static const char vertex_shader_src[] = {
    "#version 100\n"
    "precision mediump float;\n"

    "attribute vec2 in_pos;\n"
    "attribute vec2 in_uv;\n"

    "varying vec2 t_coord;\n"

    "uniform mat3 mvp_mat;\n"

    "void main()\n"
    "{\n"
        "t_coord = in_uv;\n"
        "gl_Position = vec4((mvp_mat * vec3(in_pos, 1)), 1);\n"
        "//gl_Position = vec4(0, 0, in_pos.x, 1);\n"
    "}\n"
};

static const char fragment_shader_src[] = {
    "#version 100\n"
    "precision mediump float;\n"

    "varying vec2 t_coord;\n"

    "uniform sampler2D   texture;\n"
    "uniform vec4        color;\n"

    "void main()\n"
    "{\n"
        "vec4 t_color = texture2D(texture, vec2(t_coord.x, t_coord.y));\n"
        "//gl_FragColor = vec4(1, 1, 1, color.r);\n"
        "gl_FragColor = t_color*color;\n"
    "}\n"
};

static void
loadShader(Renderer* renderer, const char* vs, const char* fs, int shader_enum);

static void closeGame()
{
    //TODO: FREE SHADERS AND OTHER RESOURCES???????????????
    
}

static void readHighScore(GameState* state)
{
    SDL_RWops* io = SDL_RWFromFile("highscore.data", "rb");
    if( io != NULL ){
        int score; 
        if( SDL_RWread(io, (void*)&score, sizeof(int), 1) > 0 ){
            printf("read some shit : %d\n", score);
            state->high_score = score;
        }
        SDL_RWclose(io);
    }
}

static void saveHighScore(GameState* state)
{
    SDL_RWops* io = SDL_RWFromFile("highscore.data", "wb"); 
    if( io != NULL ){
        if( SDL_RWwrite(io, (void*)&state->high_score, sizeof(int), 1) != 1){
            printf("could not write score\n");
        } else {
            printf("wrote fucking score\n");
        }
        SDL_RWclose(io);
    }
}


static void resetGameStateDefaults(GameState* state)
{
        state->dead= false;
        state->new_hscore = false;
        state->game_time = 0.f;
        state->projection = mat3_ortho(0, frame_buff_w, 0, frame_buff_h);


        Entity* player = &state->entities[E_Player];
        player->type = E_Player;
        player->valid = true;
        player->shader_enum = Shader_Default;
        player->shape_enum = Shape_Rect;
        player->texture_enum = Texture_PlayerGround;
        player->rect.size = default_player_size;
        player->rect.position = default_player_pos;
        //player->color = {0.7, 0.1, 0.8, 1};
        player->color = {1, 1, 1, 1};
        player->p_state = 0;
        
        state->e_spawner.spawn_time = 1.f;
        state->e_spawner.clock.restart();
        state->spawn_increase_time = 0.f;
        state->spawn_increase_rate = 1.f;

        //init walls
        float wall_width = 20.f;
        vec4 wall_color = {0.9f, 0.04f, 0.51f, 1.f};
        state->walls[Wall_Left].shader_enum     = Shader_Default;
        state->walls[Wall_Left].shape_enum      = Shape_Rect;
        state->walls[Wall_Left].texture_enum    = Texture_Default;
        state->walls[Wall_Left].rect.position   = {0, 0};
        state->walls[Wall_Left].rect.size       = {wall_width, frame_buff_h};
        state->walls[Wall_Left].color           = wall_color;

        state->walls[Wall_Right].shader_enum    = Shader_Default;
        state->walls[Wall_Right].shape_enum     = Shape_Rect;
        state->walls[Wall_Right].texture_enum   = Texture_Default;
        state->walls[Wall_Right].rect.position  = {frame_buff_w - wall_width, 0};
        state->walls[Wall_Right].rect.size      = {wall_width, frame_buff_h};
        state->walls[Wall_Right].color          = wall_color;

        state->walls[Wall_Floor].shader_enum    = Shader_Default;
        state->walls[Wall_Floor].shape_enum     = Shape_Rect;
        state->walls[Wall_Floor].texture_enum   = Texture_Default;
        state->walls[Wall_Floor].rect.position  = {0, 0};
        state->walls[Wall_Floor].rect.size      = {frame_buff_w, wall_width};
        state->walls[Wall_Floor].color          = wall_color;


        state->spikes[0].shader_enum = Shader_Default;
        state->spikes[0].shape_enum = Shape_Rect;
        state->spikes[0].texture_enum = Texture_Spike;
        state->spikes[0].color = wall_color;
        state->spikes[0].rect.size = {wall_width, 200};
        state->spikes[0].rect.position = {0, frame_buff_h-200};

        state->spikes[1].shader_enum    = Shader_Default;
        state->spikes[1].shape_enum     = Shape_Rect;
        state->spikes[1].texture_enum   = Texture_Spike;
        state->spikes[1].color          = wall_color;
        state->spikes[1].rect.size      = {wall_width, 200};
        state->spikes[1].rect.position  = {frame_buff_w-wall_width, frame_buff_h-200};

        state->alarm.shader_enum    = Shader_Default;
        state->alarm.shape_enum     = Shape_Rect;
        state->alarm.texture_enum   = Texture_Alarm;
        state->alarm.anim_time      = 3.f;

        state->alarm.color          = {1, 1, 1, 1};

        state->alarm.rect.size      = {10, 40};
        float xpos = (rand()%(frame_buff_w - 40) + 20);
        state->alarm.rect.position  = {xpos, 50};

        state->gspike.shader_enum   = Shader_Default;
        state->gspike.shape_enum    = Shape_Rect;
        state->gspike.texture_enum  = Texture_GSpike;
        state->gspike.color         = {1, 1, 1, 1};
        state->gspike.rect.size     = {140, 40};
        state->gspike.life_time     = 5.f;
        state->gspike.valid         = false;

        state->hscore_anim.shader_enum = Shader_Default;
        state->hscore_anim.shape_enum = Shape_Rect;
        state->hscore_anim.texture_enum = Texture_HScoreGZ;
        state->hscore_anim.rect.size = { 400, 60 };
        state->hscore_anim.rect.x = (frame_buff_w-state->hscore_anim.rect.w)/2;
        state->hscore_anim.rect.y = 1000;
        state->hscore_anim.color = {1, 1, 1, 0};

        state->hscore_anim.fading = false;
        
        readHighScore(state);
};

extern "C"
int initGame(Renderer* renderer, GameState* state, SDL_Window* window)
{
    //init renderer
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(1, 1, 1, 1);

        *renderer = {};
        //setup shader shit
        renderer->vert_shaders[Shader_Default] = "shaders/unlit.vert"; 
        renderer->frag_shaders[Shader_Default] = "shaders/unlit.frag"; 
        renderer->vs_change_times[Shader_Default] = 0;
        renderer->fs_change_times[Shader_Default] = 0;
        loadShader(renderer, vertex_shader_src, fragment_shader_src, Shader_Default);

        renderer->vert_shaders[Shader_Text] = "shaders/text.vert"; 
        renderer->frag_shaders[Shader_Text] = "shaders/text.frag"; 
        renderer->vs_change_times[Shader_Text] = 0;
        renderer->fs_change_times[Shader_Text] = 0;
        //loadShader(renderer, vertex_shader_src, fragment_shader_src, Shader_Default);

        //get rectangle vertices/uvs to gpu
        Vertex v[6] =  {    { {0.f, 1.f}, {0.f, 1.f} },
                            { {0.f, 0.f}, {0.f, 0.f} },
                            { {1.f, 0.f}, {1.f, 0.f} },

                            { {0.f, 1.f}, {0.f, 1.f} },
                            { {1.f, 0.f}, {1.f, 0.f} },
                            { {1.f, 1.f}, {1.f, 1.f} }
                        };


        GLuint vert_buffer;  
        glGenBuffers(1, &vert_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vert_buffer);
        glBufferData(GL_ARRAY_BUFFER, 6*sizeof(Vertex), &v, GL_STATIC_DRAW);        

        renderer->vert_buffers[Shape_Rect] = vert_buffer;

        //setup frame buffer texture shit
        glGenFramebuffers(1, &renderer->frame_buffer);
        glBindFramebuffer(GL_FRAMEBUFFER, renderer->frame_buffer);

        GLuint* fb_tex = &renderer->textures[Texture_FB];
        glGenTextures(1, fb_tex);
        glBindTexture(GL_TEXTURE_2D, *fb_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
                frame_buff_w, frame_buff_h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                GL_TEXTURE_2D, *fb_tex, 0);
        
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
            printf("frame buff is incomplete \n");
            return -1;
        }

        loadTexture2D(&renderer->textures[Texture_PlayerGround], 
                "textures/player_ground.png");
        loadTexture2D(&renderer->textures[Texture_PlayerAir], 
                "textures/player_air.png");
        loadTexture2D(&renderer->textures[Texture_PlayerDead], 
                "textures/player_dead.png");
        loadTexture2D(&renderer->textures[Texture_EnemyL], "textures/enemyl.png");
        loadTexture2D(&renderer->textures[Texture_EnemyR], "textures/enemyr.png");
        loadTexture2D(&renderer->textures[Texture_Spike], "textures/spike.png");
        loadTexture2D(&renderer->textures[Texture_GSpike], "textures/ggspike.png");
        //loadTexture2D(&renderer->textures[Texture_Wall], "textures/rmntdw.png");
        loadTexture2D(&renderer->textures[Texture_Alarm], "textures/alarm.png");
        loadTexture2D(&renderer->textures[Texture_BackGround], "textures/bg.png");
        createWhiteTexture(&renderer->textures[Texture_Default]);
        
        loadTexture2D(&renderer->textures[Texture_Digit0], "textures/digit0.png");
        loadTexture2D(&renderer->textures[Texture_Digit1], "textures/digit1.png");
        loadTexture2D(&renderer->textures[Texture_Digit2], "textures/digit2.png");
        loadTexture2D(&renderer->textures[Texture_Digit3], "textures/digit3.png");
        loadTexture2D(&renderer->textures[Texture_Digit4], "textures/digit4.png");
        loadTexture2D(&renderer->textures[Texture_Digit5], "textures/digit5.png");
        loadTexture2D(&renderer->textures[Texture_Digit6], "textures/digit6.png");
        loadTexture2D(&renderer->textures[Texture_Digit7], "textures/digit7.png");
        loadTexture2D(&renderer->textures[Texture_Digit8], "textures/digit8.png");
        loadTexture2D(&renderer->textures[Texture_Digit9], "textures/digit9.png");

        loadTexture2D(&renderer->textures[Texture_Score], "textures/score.png");
        loadTexture2D(&renderer->textures[Texture_HScore], "textures/hscore.png");
        loadTexture2D(&renderer->textures[Texture_HScoreGZ], "textures/hscoregz.png");

        loadTexture2D(&renderer->textures[Texture_DeathScreen], "textures/dscreen.png");
        loadTexture2D(&renderer->textures[Texture_Name], "textures/name.png");
        loadTexture2D(&renderer->textures[Texture_Paused], "textures/paused.png");
        
    }

#ifdef __ANDROID__
    //admib bullshit
    {

    }
#endif

    //init GameState
    {
        *state = {};

        state->quit = false;
        state->focused = false;
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        state->win_width = w;
        state->win_height = h;

        resetGameStateDefaults(state); 
        state->paused = true;

        for(int i = 0; i < SDL_NumJoysticks(); ++i){
            state->accel = SDL_JoystickOpen(i);
            if( SDL_JoystickNumAxes(state->accel) != 0 ){
                break;
            }else{
                SDL_JoystickClose(state->accel);
            }
        }
        SDL_Log("There are %d joysticks available\n", SDL_NumJoysticks());
        SDL_Log("joystick number of axis = %d\n", SDL_JoystickNumAxes(state->accel));
        SDL_Log("joystick number of hats = %d\n", SDL_JoystickNumHats(state->accel));
        SDL_Log("joystick number of balls = %d\n", SDL_JoystickNumBalls(state->accel));
        SDL_Log("joystick number of buttons = %d\n",SDL_JoystickNumButtons(state->accel));
        
    }


    //init some AD BULLSHIt LUL


    return 0;
}

static void
resetStateAfterDeath(GameState* state)
{
    saveHighScore(state);

    resetGameStateDefaults(state); 
    state->score = {};
    for(int i = 1; i < MAX_ENTITIES; ++i){
        state->entities[i].valid = false;
    }

}

static void
setupShader(Renderer* renderer, int shader_enum)
{
    
    if(shader_enum == Shader_Default){
        GLuint program = renderer->shaders[shader_enum].program;
        glUseProgram(program);
        renderer->shaders[shader_enum].addUniforms(2, "mvp_mat", "color");
        renderer->attrib_in_pos = glGetAttribLocation(program, "in_pos");
        renderer->attrib_in_uv = glGetAttribLocation(program, "in_uv");
        printf("%d loc\n", glGetAttribLocation(program, "in_pos"));
    }
    else if(shader_enum == Shader_Text){
        glUseProgram(renderer->shaders[shader_enum].program);
        renderer->shaders[shader_enum].addUniforms(1, "mvp_mat");
        printf("inited text shader kek\n");

    }
    //glUseProgram(0); ??
     
}

static void
loadShader(Renderer* renderer, const char* vs, const char* fs, int shader_enum)
{
    if(renderer->shaders[shader_enum].program){
        renderer->shaders[shader_enum].deleteShaderProgram();
    }
    renderer->shaders[shader_enum].loadFromString(GL_VERTEX_SHADER, vs);
    renderer->shaders[shader_enum].loadFromString(GL_FRAGMENT_SHADER, fs);

    renderer->shaders[shader_enum].createAndLinkProgram();
    setupShader(renderer, shader_enum);
}

static void 
checkShaderReload(Renderer* renderer)
{
    uint64 new_vs_change_time;
    uint64 new_fs_change_time;
    for(int i = 0; i < Shader_Max; ++i){
        new_vs_change_time = fileChangeTime(renderer->vert_shaders[i]); 
        new_fs_change_time = fileChangeTime(renderer->frag_shaders[i]); 

        if( new_vs_change_time > renderer->vs_change_times[i] ||
            new_fs_change_time > renderer->fs_change_times[i]
          )
        {
            if(renderer->shaders[i].program){
                renderer->shaders[i].deleteShaderProgram(); 
            }
            renderer->shaders[i].loadFromFile(GL_VERTEX_SHADER, 
                    renderer->vert_shaders[i]);
            renderer->shaders[i].loadFromFile(GL_FRAGMENT_SHADER, 
                    renderer->frag_shaders[i]);
            
            renderer->shaders[i].createAndLinkProgram();
            setupShader(renderer, i);

            renderer->vs_change_times[i] = new_vs_change_time;
            renderer->fs_change_times[i] = new_fs_change_time;
        } 
    }
}

static void printmat(mat2 m){
    printf("%f\t%f\n%f\t%f\n\n", m.m00, m.m10, m.m01, m.m11);
}
static void printmat(mat3 m){
    printf("%f\t%f\t%f\n", m.m00, m.m10, m.m20);
    printf("%f\t%f\t%f\n", m.m01, m.m11, m.m21);
    printf("%f\t%f\t%f\n", m.m02, m.m12, m.m22);

    printf("\n");
}
#if 0
static void drawText(Renderer* renderer)
{
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
    Text text;

    Font font;

    TextBuffer tbuff;

    fontLoad(&font, "fcrystal.ttf", 32);
    textBufferCreate(&tbuff, 1024);

    char tstr[128] = {};
    text.pos = {10, 32};
    text.font = &font;
    text.color = {0.7, 0.8, 0.9, 1};
    text.str = tstr;
    text.size = 24;

    sprintf(text.str, "Frame time: .3fms");


    textBufferBegin(&tbuff);
    textBufferPush(&tbuff, &text);
    textBufferEnd(&tbuff);

    int height = 900;
    int width = height*9/16;
    mat4 ortho = mat4_ortho(0, width, height, 0, 1, -1);
    
    glBindVertexArray(tbuff.vao);
    Shader* shader = &renderer->shaders[Shader_Text];
    glUseProgram(shader->program);
    glUniformMatrix4fv(shader->getUniform("mvp_mat"), 1, GL_FALSE, ortho.e);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, text.font->texture);
    glDrawArrays(GL_TRIANGLES, 0, tbuff.vertex_count);
    

    fontFree(&font);
    textBufferDestroy(&tbuff);
}
#endif

extern "C"
void drawState(Renderer* renderer, GameState* state)
{
    glEnable(GL_BLEND);
    //glDisable(GL_BLEND);

    checkShaderReload(renderer);
    glClear(GL_COLOR_BUFFER_BIT);

    GLuint attrib_pos = renderer->attrib_in_pos;
    GLuint attrib_uv = renderer->attrib_in_uv;

    mat3 model_mat = mat3_identity();
    mat3 m;

    glBindFramebuffer(GL_FRAMEBUFFER, renderer->frame_buffer);
    glViewport(0, 0, frame_buff_w, frame_buff_h);
    glClear(GL_COLOR_BUFFER_BIT);

    {//draw background
        model_mat.z.xy = {0, 0};
        //model_mat.m00 = state->win_width;
        //model_mat.m11 = state->win_height;
        model_mat.m00 = frame_buff_w;
        model_mat.m11 = frame_buff_h;
        m = state->projection*model_mat;

        glBindBuffer(GL_ARRAY_BUFFER, renderer->vert_buffers[Shape_Rect]);
        glEnableVertexAttribArray(attrib_pos);
        glVertexAttribPointer(attrib_pos, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

        glEnableVertexAttribArray(attrib_uv);
        glVertexAttribPointer(attrib_uv, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                (const void*)offsetof(Vertex, uv));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderer->textures[Texture_BackGround]);

        Shader* shader = &renderer->shaders[Shader_Default];
        glUseProgram(shader->program);
        glUniformMatrix3fv(shader->getUniform("mvp_mat"), 1, GL_FALSE, m.e);
        vec4 col = {1, 1, 1, 1};
        glUniform4fv(shader->getUniform("color"), 1, &col.e[0]);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }//draw background
    

    {//draw Entities 
        glBindBuffer(GL_ARRAY_BUFFER, renderer->vert_buffers[Shape_Rect]);

        glEnableVertexAttribArray(attrib_pos);
        glVertexAttribPointer(attrib_pos, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

        glEnableVertexAttribArray(attrib_uv);
        glVertexAttribPointer(attrib_uv, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                (const void*)offsetof(Vertex, uv));

        Entity* ent;
        for(int i = 0; i < MAX_ENTITIES; ++i){
            ent = &state->entities[i];
            if(ent->valid){
                model_mat.z.xy = {ent->rect.position.x, ent->rect.position.y};
                model_mat.m00 = ent->rect.size.x;
                model_mat.m11 = ent->rect.size.y;
                m = state->projection*model_mat; 

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, renderer->textures[ent->texture_enum]);

                glUseProgram(renderer->shaders[ent->shader_enum].program);
                glUniformMatrix3fv(
                    renderer->shaders[ent->shader_enum].getUniform("mvp_mat"),
                    1, GL_FALSE, m.e);
                glUniform4fv(
                    renderer->shaders[ent->shader_enum].getUniform("color"),
                    1, &ent->color.e[0]);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
        }

    }//draw entities

    if(state->gspike.valid){
        Shader* shader;
        model_mat.z.xy = state->gspike.rect.position;
        model_mat.m00 = state->gspike.rect.w;
        model_mat.m11 = state->gspike.rect.h;
        m = state->projection*model_mat;

        shader = &renderer->shaders[state->gspike.shader_enum];
        glUseProgram(shader->program);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,renderer->textures[state->gspike.texture_enum]);
        glUniformMatrix3fv(shader->getUniform("mvp_mat"), 1, GL_FALSE, m.e);
        glUniform4fv(
            shader->getUniform("color"), 1, &state->gspike.color.e[0]);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    {//draw walls
        Shader* shader;
        for(int i = 0; i < Wall_Max; ++i){
            model_mat.z.xy = state->walls[i].rect.position; 
            model_mat.m00 = state->walls[i].rect.w;
            model_mat.m11 = state->walls[i].rect.h;
            m = state->projection*model_mat;

            shader = &renderer->shaders[state->walls[i].shader_enum];
            glUseProgram(shader->program);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D,renderer->textures[state->walls[i].texture_enum]);
            glUniformMatrix3fv(shader->getUniform("mvp_mat"), 1, GL_FALSE, m.e);
            glUniform4fv(
                shader->getUniform("color"), 1, &state->walls[i].color.e[0]);

            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        for(int i = 0; i < 2; ++i){
            if(i == 1){
                model_mat.z.xy = {state->spikes[i].rect.x+state->spikes[i].rect.w,
                                    state->spikes[i].rect.y};
                model_mat.m00 = -state->spikes[i].rect.w-15;
            }
            else{
                model_mat.z.xy = state->spikes[i].rect.position;
                model_mat.m00 = state->spikes[i].rect.w+15;
            }
            model_mat.m11 = state->spikes[i].rect.h;
            m = state->projection*model_mat;   

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D,renderer->textures[state->spikes[i].texture_enum]);

            shader = &renderer->shaders[state->spikes[i].shader_enum];
            glUseProgram(shader->program);
            glUniformMatrix3fv(shader->getUniform("mvp_mat"), 1, GL_FALSE, m.e);
            glUniform4fv(
                shader->getUniform("color"), 1, &state->spikes[i].color.e[0]);

            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

    }//draw walls
        
    {//draw alarm
    if(state->alarm.valid){
        model_mat.z.xy = state->alarm.rect.position;  
        model_mat.m00 = state->alarm.rect.w;
        model_mat.m11 = state->alarm.rect.h;
        m = state->projection*model_mat;

        Shader* shader = &renderer->shaders[state->alarm.shader_enum];
        glUseProgram(shader->program);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,renderer->textures[state->alarm.texture_enum]);
        glUniformMatrix3fv(shader->getUniform("mvp_mat"), 1, GL_FALSE, m.e);
        glUniform4fv(
            shader->getUniform("color"), 1, &state->alarm.color.e[0]);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    }//draw alarm


    {//draw fucking score

    //if(!state->paused){
        glBindBuffer(GL_ARRAY_BUFFER, renderer->vert_buffers[Shape_Rect]);

        glEnableVertexAttribArray(attrib_pos);
        glVertexAttribPointer(attrib_pos, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

        glEnableVertexAttribArray(attrib_uv);
        glVertexAttribPointer(attrib_uv, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                (const void*)offsetof(Vertex, uv));

        int hscore = state->high_score;
        //hscore = state->score;
        //score = 121151;
        //printf("score: %d\n", score);
        vec2 size = {15, 30};
        for(int i = 1; i <= MAX_DIGITS; ++i){
            int digit = hscore%10; 
            //printf("digit %d\n", digit);
            //model_mat.z.xy = {ent->rect.position.x, ent->rect.position.y};
            model_mat.m00 = size.x;
            if(digit == 1){
                model_mat.m00 = 10;
            }
            model_mat.m11 = size.y;
            model_mat.z.xy = {frame_buff_w - (size.x + 2)*i - 45, 
                frame_buff_h - size.y - 10};

            m = state->projection*model_mat;

            vec4 color = {1, 1, 1, 1};
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, renderer->textures[Texture_Digit0+digit]);

            glUseProgram(renderer->shaders[Shader_Default].program);
            glUniformMatrix3fv(
                renderer->shaders[Shader_Default].getUniform("mvp_mat"),
                1, GL_FALSE, m.e);
            glUniform4fv(
                renderer->shaders[Shader_Default].getUniform("color"),
                1, &color.e[0]);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            hscore /= 10;
        }
    //}

        //draw highscore text
        vec2 hsize = {180, 30};
        vec2 hpos = {(frame_buff_w - (size.x + 2)*MAX_DIGITS - 55 - hsize.x), 
            frame_buff_h - hsize.y - 13};
        model_mat.m00 = hsize.x;
        model_mat.m11 = hsize.y;
        model_mat.z.xy = hpos;

        m = state->projection*model_mat;

        vec4 color = {1, 1, 1, 1};
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderer->textures[Texture_HScore]);

        glUseProgram(renderer->shaders[Shader_Default].program);
        glUniformMatrix3fv(
            renderer->shaders[Shader_Default].getUniform("mvp_mat"),
            1, GL_FALSE, m.e);
        glUniform4fv(
            renderer->shaders[Shader_Default].getUniform("color"),
            1, &color.e[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);


    if(state->paused == false){
        //draw score text
        vec2 stsize = {140, 45};
        vec2 stpos = {(frame_buff_w - stsize.x)/2, frame_buff_h - 100 - stsize.y};

        model_mat.m00 = stsize.x;
        model_mat.m11 = stsize.y;
        model_mat.z.xy = stpos; 

        m = state->projection*model_mat;

        color = {1, 1, 1, 1};
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderer->textures[Texture_Score]);

        glUseProgram(renderer->shaders[Shader_Default].program);
        glUniformMatrix3fv(
            renderer->shaders[Shader_Default].getUniform("mvp_mat"),
            1, GL_FALSE, m.e);
        glUniform4fv(
            renderer->shaders[Shader_Default].getUniform("color"),
            1, &color.e[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //draw score digits
        
        int score = state->score; 
        int digits = 1;
        for(int i = 0; i < MAX_DIGITS; ++i){
            score /= 10;
            if(score > 0){
                ++digits;
            }
        }
        
        score = state->score;
        vec2 sdsize = {20, 40};
        for(int i = 0; i < digits; ++i){
            int digit = score%10;
            score /= 10;

            model_mat.m00 = sdsize.x;
            if(digit == 1){
                model_mat.m00 = sdsize.x*0.75;
            }
            model_mat.m11 = sdsize.y;

            model_mat.z.xy = {frame_buff_w/2 - (sdsize.x + 4)*i + 
                (sdsize.x + 4)*((digits/2.f) - 1), 
                stpos.y - sdsize.y - 10};


            m = state->projection*model_mat;

            color = {1, 1, 1, 1};
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, renderer->textures[Texture_Digit0+digit]);

            glUseProgram(renderer->shaders[Shader_Default].program);
            glUniformMatrix3fv(
                renderer->shaders[Shader_Default].getUniform("mvp_mat"),
                1, GL_FALSE, m.e);
            glUniform4fv(
                renderer->shaders[Shader_Default].getUniform("color"),
                1, &color.e[0]);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }
    }//draw fucking score

    if(state->paused){
        vec2 size = {500, 100};
        model_mat.m00 = size.x;
        model_mat.m11 = size.y;
        model_mat.z.xy = {(frame_buff_w-size.x)/2, 1000};
        m = state->projection*model_mat;

        vec4 color = {1, 1, 1, 1};
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderer->textures[Texture_Name]);

        glUseProgram(renderer->shaders[Shader_Default].program);
        glUniformMatrix3fv(
                renderer->shaders[Shader_Default].getUniform("mvp_mat"),
                1, GL_FALSE, m.e);
        glUniform4fv(
                renderer->shaders[Shader_Default].getUniform("color"),
                1, &color.e[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

    }
    


    {//draw hscore_anim

        model_mat.z.xy = state->hscore_anim.rect.position;
        model_mat.m00 = state->hscore_anim.rect.w;
        model_mat.m11 = state->hscore_anim.rect.h;
        m = state->projection*model_mat;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderer->textures[state->hscore_anim.texture_enum]);

        glUseProgram(renderer->shaders[Shader_Default].program);
        glUniformMatrix3fv(
            renderer->shaders[Shader_Default].getUniform("mvp_mat"),
            1, GL_FALSE, m.e);
        glUniform4fv(
            renderer->shaders[Shader_Default].getUniform("color"),
            1, &state->hscore_anim.color.e[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);


    }//draw hscore_anim

    if(state->dead)
    {//draw death screen
        
        vec2 size = {500, 400}; 
        model_mat.m00 = size.x;
        model_mat.m11 = size.y;
        model_mat.z.xy = {(frame_buff_w-size.x)/2, (frame_buff_h-size.y)/2+50};

        m = state->projection*model_mat;

        vec4 color = {1, 1, 1, 1};
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderer->textures[Texture_DeathScreen]);

        glUseProgram(renderer->shaders[Shader_Default].program);
        glUniformMatrix3fv(
            renderer->shaders[Shader_Default].getUniform("mvp_mat"),
            1, GL_FALSE, m.e);
        glUniform4fv(
            renderer->shaders[Shader_Default].getUniform("color"),
            1, &color.e[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }//

    
    if(state->paused)
    {//draw paused
        vec2 size = {500, 50};
        model_mat.m00 = size.x;
        model_mat.m11 = size.y;
        model_mat.z.xy = {(frame_buff_w-size.x)/2, 800};
        vec4 color = {1, 1, 1, 1};

        m = state->projection*model_mat;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderer->textures[Texture_Paused]);

        glUseProgram(renderer->shaders[Shader_Default].program);
        glUniformMatrix3fv(
            renderer->shaders[Shader_Default].getUniform("mvp_mat"),
            1, GL_FALSE, m.e);
        glUniform4fv(
            renderer->shaders[Shader_Default].getUniform("color"),
            1, &color.e[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    {//draw framebuffer texture to screen

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, state->win_width, state->win_height);
        glClear(GL_COLOR_BUFFER_BIT);

        model_mat.z.xy = {0, 0};
        model_mat.m00 = state->win_width;
        model_mat.m11 = state->win_height;
        mat3 proj = mat3_ortho(0, state->win_width, 0, state->win_height);
        m = proj*model_mat;

        glBindBuffer(GL_ARRAY_BUFFER, renderer->vert_buffers[Shape_Rect]);
        glEnableVertexAttribArray(attrib_pos);
        glVertexAttribPointer(attrib_pos, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

        glEnableVertexAttribArray(attrib_uv);
        glVertexAttribPointer(attrib_uv, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                (const void*)offsetof(Vertex, uv));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderer->textures[Texture_FB]);

        Shader* shader = &renderer->shaders[Shader_Default];
        glUseProgram(shader->program);
        glUniformMatrix3fv(shader->getUniform("mvp_mat"), 1, GL_FALSE, m.e);
        vec4 col = {1, 1, 1, 1};
        glUniform4fv(shader->getUniform("color"), 1, &col.e[0]);

        glDrawArrays(GL_TRIANGLES, 0, 6);

    }
    //drawText(renderer);

}

static void pStateSet(int* p_state, int state_enum)
{
    *p_state |= 1 << state_enum;
}
static void pStateClear(int* p_state, int state_enum)
{
    *p_state &= ~(1 << state_enum);
}
static bool pStateCheck(int p_state, int state_enum)
{
    return p_state & (1 << state_enum);
}

//TODO: TRY NOT TO MODIFY LOGIC STATE
//TODO: SEPERATE INPUT AND LOGIC
extern "C"
void processEvent(GameState* state, SDL_Event* event, SDL_Window* window)
{
    Entity* player = &state->entities[E_Player];
    switch (event->type)
    {
        case SDL_QUIT:
            state->quit = true;
        break;

        case SDL_KEYDOWN:
        {
            if( event->key.keysym.sym == SDLK_r ){
                state->e_spawner.spawn_time = 1.f;
                state->high_score = 0.f;
            }
        }break;

        case SDL_WINDOWEVENT:
        {
            if(event->window.event == SDL_WINDOWEVENT_RESIZED){
                state->win_width = event->window.data1; 
                state->win_height = event->window.data2; 
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glViewport(0, 0, event->window.data1, event->window.data2);

                printf("%dx%d\n", state->win_width, state->win_height);
            }
        }break;

        case SDL_JOYBUTTONDOWN:
        {
            goto act_press;
        }break;

        case SDL_MOUSEBUTTONDOWN:
        {
            act_press:
            //event->button.button = SDL_BUTTON_LEFT;

            if(state->paused == false){
                if(state->dead == false){
                    if(event->button.button == SDL_BUTTON_LEFT){
                        if( !pStateCheck(player->p_state, PS_InAir) ){
                            player->velocity.y = 550;
                            pStateSet(&player->p_state, PS_InAir);
                            player->texture_enum = Texture_PlayerAir;
                        }
                        if( pStateCheck(player->p_state, PS_OnWall) ){

                            //if(player->rect.position.x > state->win_width/2.f){
                            if(player->rect.position.x > frame_buff_w/2.f){
                                player->velocity.x = -350;
                                player->velocity.y = 450;
                            }else{
                                player->velocity.x = 350;
                                player->velocity.y = 450;
                            }
                            pStateClear(&player->p_state, PS_OnWall);
                            player->texture_enum = Texture_PlayerAir;
                        }
                    }else if(event->button.button == SDL_BUTTON_RIGHT){
                        if(SDL_GetRelativeMouseMode() == SDL_FALSE){
                            SDL_SetRelativeMouseMode(SDL_TRUE); 
                            state->focused = true;
                        }else {
                            SDL_SetRelativeMouseMode(SDL_FALSE); 
                            state->focused = false;
                        }
                    }
                } else {
                    state->dead = false;
                    resetStateAfterDeath(state);
                }
          }else{
                state->paused = false;
          }

        }break;

        case SDL_MOUSEBUTTONUP:
        {
            if(event->button.button == SDL_BUTTON_LEFT){
                //state->controls.thrust -hhhhhhhhh= false;
            }
        }break;

        case SDL_MOUSEMOTION:
        {
            if(state->dead == false){
                float ox = 0;
                float oy = 0;
                if(state->focused){
                    if( !pStateCheck(player->p_state, PS_OnWall) ){
                        ox = 0.75*event->motion.xrel;
                    }
                }
                if(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)){


                }
                player->velocity.x += ox;
            }
        }break;


    }
}

static bool rectToRectCollision(Rect* A, Rect* B)
{
    return (A->x < B->x + B->w &&
            A->x + A->w > B->x &&
            A->y < B->y + B->h &&
            A->y + A->h > B->y);
}

static void resolveCollisions(GameState* state)
{
    //int  p_state = state->entities[E_Player].p_state;
    vec2 pos = state->entities[E_Player].rect.position;
    vec2 size = state->entities[E_Player].rect.size;
    vec2 vel = state->entities[E_Player].velocity;
    Entity* player = &state->entities[E_Player];
     
    if( pStateCheck(player->p_state, PS_InAir) ){
        if(pos.y < 0.f){ //hit ground
            pos.y = 0.f;
            vel.y = 0.f;
            pStateClear(&player->p_state, PS_InAir);
            pStateClear(&player->p_state, PS_OnWall);
            player->texture_enum = Texture_PlayerGround;
        }
    }

    
    //WALL DETECTION
    for(int i = 0; i < Wall_Max; ++i){
        if(rectToRectCollision(&player->rect, &state->walls[i].rect)){
            if(i == Wall_Left){
                pos.x = state->walls[i].rect.x + state->walls[i].rect.w;
                vel.x = 0;
                if( pStateCheck(player->p_state, PS_InAir) ){
                    vel.y = -50;
                    pStateSet(&player->p_state, PS_OnWall);
                    player->texture_enum = Texture_PlayerGround;
                }
            }else if(i == Wall_Right){
                pos.x = state->walls[i].rect.x - size.x;
                vel.x = 0;
                if( pStateCheck(player->p_state, PS_InAir) ){
                    vel.y = -50;
                    pStateSet(&player->p_state, PS_OnWall);
                    player->texture_enum = Texture_PlayerGround;
                }
            }else {
                pos.y = state->walls[i].rect.h;    
                if( pStateCheck(player->p_state, PS_InAir) ){
                    pStateClear(&player->p_state, PS_OnWall);
                    pStateClear(&player->p_state, PS_InAir);
                    player->texture_enum = Texture_PlayerGround;
                }
            }
        }
    } 

    //spikes
    for(int i = 0; i < 2; ++i){
        if(rectToRectCollision(&player->rect, &state->spikes[i].rect)){
            player->texture_enum = Texture_PlayerDead;
            state->dead = true;
        }
    }

    
    if(state->gspike.valid && rectToRectCollision(&player->rect, &state->gspike.rect)){
        player->texture_enum = Texture_PlayerDead;
        state->dead = true;
    }
    
    player->color = {1, 1, 1, 1};
    for(int i = 1; i < MAX_ENTITIES; ++i){
        Entity* ent = &state->entities[i];
        if(ent->valid){
            if(ent->rect.position.y < -ent->rect.size.y/2.f){
                ent->valid = false;
                state->score += 1;
                break;
            }


            //player-enemy collision
            if(rectToRectCollision(&player->rect, &ent->rect))
            {
                //printf("COLLISION WITH PLAYER KEK\n");
                //player->color = {1, 0, 1, 1};
                player->texture_enum = Texture_PlayerDead;
                state->dead = true;
                state->e_spawner.spawn_time = 1.f;
                //SDL_Delay(100);
            }
            

            //enemy-wall collision
            if(rectToRectCollision(&ent->rect, &state->walls[Wall_Left].rect))
            {
                ent->rect.x = state->walls[Wall_Left].rect.x + ent->rect.w;
                ent->velocity.x = -ent->velocity.x;
                ent->texture_enum = Texture_EnemyR;
            }
            if(rectToRectCollision(&ent->rect, &state->walls[Wall_Right].rect))
            {
                ent->rect.x = state->walls[Wall_Right].rect.x - ent->rect.w;
                ent->velocity.x = -ent->velocity.x;
                ent->texture_enum = Texture_EnemyL;
            }
        }
    }
    //printf("SCORE LEL %d\n", state->score.score);
    state->entities[E_Player].rect.position = pos;
    state->entities[E_Player].velocity = vel;
}

static void spawnEnemy(EnemySpawner* spawner, GameState* state)
{
    if(spawner->clock.time() > spawner->spawn_time){
        for(int i = 1; i < MAX_ENTITIES; ++i){
            if(!state->entities[i].valid){
                Entity* ent = &state->entities[i];
                *ent = {};
                ent->type = E_Enemy;
                ent->valid = true;
                ent->shader_enum = Shader_Default;
                ent->shape_enum = Shape_Rect;
                //ent->texture_enum = Texture_Enemy;

                //ent->position = {((float)(rand()%1000)/1000.f)*(frame_buff_w-30.f),
                //                    (float)frame_buff_h};
                ent->rect.position = {((float)(rand()%1000)/1000.f)*(frame_buff_w),
                                    (float)frame_buff_h-10};
                ent->rect.size = {40, 120};


                float rx = ((float)(rand()%1000)/1000.f);
                float ry = ((float)(rand()%1000)/1000.f);
                ent->color = {rx, ry, 0.2f, 1};
                //ent->color = {1, 1, 1, 1};
                int sign = rand()%2;
                if(sign == 0){
                    rx = -rx;
                    ent->texture_enum = Texture_EnemyL;
                }else 
                    ent->texture_enum = Texture_EnemyR;

                ent->velocity = {rx*300, 0};

                //ent->rect = {ent->position.x, ent->position.y, 30, 30};
                break;
            }
        }    
        spawner->clock.restart();
    }
}


static void spawnAlarm(GameState* state, float dt)
{
    static float wait_time = 5.f;
    Alarm* alarm = &state->alarm;
    if(alarm->valid){
        if(alarm->anim_time <= 0){
            float xpos = (rand()%(frame_buff_w - 40) + 20);
            alarm->rect.x = xpos;
            alarm->anim_time = 3.f;
        }
        if(alarm->anim_time > 0){
            if(alarm->fading){
                alarm->color.a -= 5.f*dt;
                if(alarm->color.a < 0){
                    alarm->fading = false;
                }
            }else {
                alarm->color.a += 3.f*dt;
                if(alarm->color.a > 1){
                    alarm->fading = true;
                }
            }

            alarm->anim_time -= dt;
            if(alarm->anim_time <= 0){
                //spawn spike;
                state->gspike.valid = true;
                state->gspike.rect.x = alarm->rect.x - state->gspike.rect.w/2;
                state->gspike.rect.y = -state->gspike.rect.h;
                state->gspike.life_time = 5.f;
                state->gspike.retract = false;
                alarm->valid = false;
                alarm->color.a = 0.f;
            }
        }
    }else{
        if(wait_time > 0){
            wait_time -= dt;
        }else{
            alarm->valid = true;
            wait_time = 5.f;
        }

    }

}

static void spawnSpike(GameState* state, float dt)
{
    GSpike* spike = &state->gspike;    
    
    if(spike->valid){
        if(spike->rect.y < 0 && spike->retract == false){
            spike->rect.y += 75.f*dt;
        }else{
            if(spike->life_time > 0){
                spike->life_time -= dt;
            }else{
                spike->rect.y -= 75.f*dt;
                spike->retract = true;
            }
        }
    }
}

static void tickHighScoreAnim(GameState* state, float dt)
{
    if(state->new_hscore == true){
        if(state->hscore_anim.fading){
            if(state->hscore_anim.color.a > 0){
                state->hscore_anim.color.a -= 0.3f*dt;
                //if(state->hscore_anim.color.a <= 0)
                //    state->hscore_anim.fading = false;
            }
        }
        else{
            if(state->hscore_anim.color.a < 1)
                state->hscore_anim.color.a += 0.9f*dt;  
            if(state->hscore_anim.color.a >= 1)
                    state->hscore_anim.fading = true;
        }
    }
}

extern "C"
void processTick(GameState* state, float dt)
{
    if(state->score > state->high_score) {
        state->high_score = state->score;
        state->new_hscore = true;
    }
    if(state->dead == false && state->paused == false){
        Entity* player = &state->entities[E_Player];
        if( !pStateCheck(player->p_state, PS_OnWall) && state->accel != NULL){
            int16 xaxis = SDL_JoystickGetAxis(state->accel, 0);
            if(xaxis < -32768/4){
                xaxis = -32768/4;
            }
            if(xaxis > 32767/4){
                xaxis = 32767/4;
            }
            player->velocity.x += xaxis*0.0065f; 
        }

        player->rect.position.x += player->velocity.x*dt; 
        if( pStateCheck(player->p_state, PS_InAir) )
            player->velocity.x *= 0.92f;
        else
            player->velocity.x *= 0.90f;
        
        if( pStateCheck(player->p_state, PS_InAir) ){

            player->rect.position.y += player->velocity.y*dt;

            if( pStateCheck(player->p_state, PS_OnWall) ){
                player->velocity.y -= 50*dt;
            }else{
                player->velocity.y -= 950*dt;
            }
        } 
        for(int i = 1; i < MAX_ENTITIES; ++i){
            Entity* ent = &state->entities[i];
            if(ent->valid){
                ent->rect.position.x += ent->velocity.x*dt;
                ent->rect.position.y += ent->velocity.y*dt;
                ent->velocity.y -= 950*dt;
            }
        }
        resolveCollisions(state);
        spawnEnemy(&state->e_spawner, state);
        state->game_time += dt;
        if(state->game_time - state->spawn_increase_time > state->spawn_increase_rate){
            state->e_spawner.spawn_time *= 0.99;
            state->spawn_increase_time = state->game_time;
        }
        spawnSpike(state, dt);
        spawnAlarm(state, dt);
        tickHighScoreAnim(state, dt);
    }
}
