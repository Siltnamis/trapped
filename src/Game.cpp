#include "Game.h"

static const int frame_buff_w = 720;
static const int frame_buff_h = 1280;

static const vec2 default_player_size   = {80, 100};
static const vec2 default_player_pos    = {frame_buff_w/2 - 80/2, 0};
static const vec2 default_rwall_size    = {20, 1080};
static const vec2 default_rwall_pos     = {0, 0};
static const vec2 default_lwall_size    = {20, 1080};
static const vec2 default_lwall_pos     = {frame_buff_w-20, 0};

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


static void resetGameStateDefaults(GameState* state)
{
        state->dead= false;
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

        //left wall init
        state->left_wall.shader_enum = Shader_Default;
        state->left_wall.shape_enum = Shape_Rect;
        //state->left_wall.texture_enum = Texture_Wall;
        state->left_wall.texture_enum = Texture_Default;
        state->left_wall.rect.position = default_rwall_pos;
        state->left_wall.rect.size = default_rwall_size;
        //state->left_wall.color = {1, 1, 1, 1};
        state->left_wall.color = {0.9f, 0.04f, 0.51f, 1};

        //right wall init
        state->right_wall.shader_enum = Shader_Default;
        state->right_wall.shape_enum = Shape_Rect;
        //state->right_wall.texture_enum = Texture_Wall;
        state->right_wall.texture_enum = Texture_Default;
        state->right_wall.rect.position = default_lwall_pos;
        state->right_wall.rect.size = default_lwall_size;
        //state->right_wall.color = {1, 1, 1, 1};
        state->right_wall.color = {0.9f, 0.04f, 0.51f, 1};

        state->spikes[0].shader_enum = Shader_Default;
        state->spikes[0].shape_enum = Shape_Rect;
        state->spikes[0].texture_enum = Texture_Spike;
        state->spikes[0].color = state->left_wall.color;
        state->spikes[0].rect.position = {0, state->left_wall.rect.h};
        state->spikes[0].rect.size = {state->left_wall.rect.w, 
            frame_buff_h-state->left_wall.rect.h};

        state->spikes[1].shader_enum = Shader_Default;
        state->spikes[1].shape_enum = Shape_Rect;
        state->spikes[1].texture_enum = Texture_Spike;
        state->spikes[1].color = state->right_wall.color;
        state->spikes[1].rect.position = {state->right_wall.rect.position.x,
            state->right_wall.rect.h};
        state->spikes[1].rect.size = {state->right_wall.rect.w, 
            frame_buff_h-state->right_wall.rect.h};

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

        //get rectangle vertices/uvs to gpu
#if 0
        Vertex v[6] =  {    { {0.0003f, 0.9997f}, {0.0003f, 0.9997f} },
                            { {0.0003f, 0.0003f}, {0.0003f, 0.0003f} },
                            { {0.9997f, 0.0003f}, {0.9997f, 0.0003f} },

                            { {0.0003f, 0.9997f}, {0.0003f, 0.9997f} },
                            { {0.9997f, 0.0003f}, {0.9997f, 0.0003f} },
                            { {0.9997f, 0.9997f}, {0.9997f, 0.9997f} }
                        };
#else
        Vertex v[6] =  {    { {0.f, 1.f}, {0.f, 1.f} },
                            { {0.f, 0.f}, {0.f, 0.f} },
                            { {1.f, 0.f}, {1.f, 0.f} },

                            { {0.f, 1.f}, {0.f, 1.f} },
                            { {1.f, 0.f}, {1.f, 0.f} },
                            { {1.f, 1.f}, {1.f, 1.f} }
                        };

#endif

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
        loadTexture2D(&renderer->textures[Texture_Wall], "textures/rmntdw.png");
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

        loadTexture2D(&renderer->textures[Texture_DeathScreen], "textures/dscreen.png");
        
    }

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

    return 0;
}

static void
resetStateAfterDeath(GameState* state)
{
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
    //else if(shader_enum == Shader_Skybox){
        //glUseProgram(renderer->shaders[shader_enum].program);
        //renderer->shaders[shader_enum].addUniforms(2, "view_mat", "proj_mat");
    //}
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

extern "C"
void drawState(Renderer* renderer, GameState* state)
{
    //checkShaderReload(renderer);
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
    }//
    

    {//draw walls
        model_mat.z.xy = state->left_wall.rect.position; 
        model_mat.m00 = state->left_wall.rect.w;
        model_mat.m11 = state->left_wall.rect.h;

        m = state->projection*model_mat;

        //BUFFER stuff
        glBindBuffer(GL_ARRAY_BUFFER, renderer->vert_buffers[Shape_Rect]);

        glEnableVertexAttribArray(attrib_pos);
        glVertexAttribPointer(attrib_pos, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

        glEnableVertexAttribArray(attrib_uv);
        glVertexAttribPointer(attrib_uv, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                (const void*)offsetof(Vertex, uv));

        //texture stuff
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderer->textures[state->left_wall.texture_enum]);

        //shader stuff
        Shader* shader = &renderer->shaders[state->left_wall.shader_enum];
        glUseProgram(shader->program);
        glUniformMatrix3fv(shader->getUniform("mvp_mat"), 1, GL_FALSE, m.e);
        glUniform4fv(shader->getUniform("color"), 1, &state->left_wall.color.e[0]);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        model_mat.z.xy = state->right_wall.rect.position;
        model_mat.m00 = state->right_wall.rect.w;
        model_mat.m11 = state->right_wall.rect.h;
        m = state->projection*model_mat;


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderer->textures[state->right_wall.texture_enum]);

        shader = &renderer->shaders[state->right_wall.shader_enum];
        glUseProgram(shader->program);
        glUniformMatrix3fv(shader->getUniform("mvp_mat"), 1, GL_FALSE, m.e);
        glUniform4fv(
        shader->getUniform("color"), 1, &state->right_wall.color.e[0]);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        for(int i = 0; i < 2; ++i){
            if(i == 1){
                model_mat.z.xy = {state->spikes[i].rect.x+state->spikes[i].rect.w,
                                    state->spikes[i].rect.y};
                model_mat.m00 = -state->spikes[i].rect.w-10;
            }
            else{
                model_mat.z.xy = state->spikes[i].rect.position;
                model_mat.m00 = state->spikes[i].rect.w+10;
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
    }

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

    {//draw fucking score

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

        //move this to processTick
        //if(state->score > state->high_score) state->high_score = state->score;

    }//draw fucking score

    if(state->dead)
    {//draw death screen
        
        vec2 size = {500, 500}; 
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

    //left wall detection
    if(pos.x < state->left_wall.rect.x + state->left_wall.rect.w){
        pos.x = state->left_wall.rect.x + state->left_wall.rect.w; 
        vel.x = 0.f;
        if( pStateCheck(player->p_state, PS_InAir) ){
            vel.y = -50;
            pStateSet(&player->p_state, PS_OnWall);
            player->texture_enum = Texture_PlayerGround;
        }
    }
    //right wall
    if(pos.x + size.x > state->right_wall.rect.x){
        pos.x = state->right_wall.rect.x - size.x;
        vel.x = 0.f; 
        if( pStateCheck(player->p_state, PS_InAir) ){
            vel.y = -50;
            pStateSet(&player->p_state, PS_OnWall);
            player->texture_enum = Texture_PlayerGround;
        }
    }
    //spikes
    for(int i = 0; i < 2; ++i){
        if(rectToRectCollision(&player->rect, &state->spikes[i].rect)){
            player->texture_enum = Texture_PlayerDead;
            state->dead = true;
        }
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
#if 0
            if(player->rect.position.x < ent->rect.position.x + ent->rect.size.x &&
                player->rect.position.x + player->rect.size.x > ent->rect.position.x &&
                player->rect.position.y < ent->rect.position.y + ent->rect.size.y &&
                player->rect.position.y + player->rect.size.y > ent->rect.position.y)
#else
            if(rectToRectCollision(&player->rect, &ent->rect))
#endif
            {
                //printf("COLLISION WITH PLAYER KEK\n");
                //player->color = {1, 0, 1, 1};
                player->texture_enum = Texture_PlayerDead;
                state->dead = true;
                state->e_spawner.spawn_time = 1.f;
                //SDL_Delay(100);
            }
            
            if( rectToRectCollision(&ent->rect, &state->right_wall.rect))
            {
                ent->rect.x = state->right_wall.rect.x - ent->rect.w;
                ent->velocity.x = -ent->velocity.x;
                ent->texture_enum = Texture_EnemyL;
            }
            if( rectToRectCollision(&ent->rect, &state->left_wall.rect)){
                 //ent->velocity = -ent->velocity*0.1f;
                ent->rect.x = state->left_wall.rect.x + state->left_wall.rect.w;
                ent->velocity.x = -ent->velocity.x;
                ent->texture_enum = Texture_EnemyR;
                 //ent->valid = false;
            }
            if( rectToRectCollision(&ent->rect, &state->spikes[0].rect))
            {
                ent->rect.x = state->spikes[0].rect.x + state->spikes[0].rect.w;
                ent->velocity.x = -ent->velocity.x;
                ent->texture_enum = Texture_EnemyR;

            }
            if( rectToRectCollision(&ent->rect, &state->spikes[1].rect)){
                ent->rect.x = state->spikes[1].rect.x - ent->rect.w;
                ent->velocity.x = -ent->velocity.x;
                ent->texture_enum = Texture_EnemyL;
            }

#if 0       //not needed
            //ent collision  with walls
            if(ent->rect.x < 0.f){
                ent->velocity.x = -ent->velocity.x;
            }else if(ent->rect.x + ent->rect.w > frame_buff_w){
                ent->velocity.x = -ent->velocity.x;
            }
#endif
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

extern "C"
void processTick(GameState* state, float dt)
{
    if(state->score > state->high_score) state->high_score = state->score;
    if(state->dead == false){
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
    }
}
