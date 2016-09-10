#include "Game.h"

extern "C"
int initGame(Renderer* renderer, GameState* state, SDL_Window* window)
{
    //init renderer
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        //setup shader shit
        renderer->vert_shaders[Shader_Default] = "shaders/unlit.vert"; 
        renderer->frag_shaders[Shader_Default] = "shaders/unlit.frag"; 
        renderer->vs_change_times[Shader_Default] = 0;
        renderer->fs_change_times[Shader_Default] = 0;

        //get rectangle vertices/uvs to gpu
        
#if 0
        Vertex v[6] =  {    { {-1.f,  1.f}, {0.f, 1.f} },
                            { {-1.f, -1.f}, {0.f, 0.f} },
                            { { 1.f, -1.f}, {1.f, 0.f} },

                            { {-1.f,  1.f}, {0.f, 1.f} },
                            { { 1.f, -1.f}, {1.f, 0.f} },
                            { { 1.f,  1.f}, {1.f, 1.f} }
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

        loadTexture2D(&renderer->textures[Texture_Player], "./textures/rsanic.png");
        loadTexture2D(&renderer->textures[Texture_Enemy], "./textures/rdorito.png");
        createWhiteTexture(&renderer->textures[Texture_Default]);

        //glViewport(0, 0, 100, 100);
    }

    //init GameState
    {
        *state = {};
        state->quit = false;
        state->controls = {};

        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        state->projection = mat3_ortho(0, w, 0, h);

        Entity* player = &state->entities[E_Player];
        player->shader_enum = Shader_Default;
        player->shape_enum = Shape_Rect;
        player->rect.size = {80, 80};
        player->rect.position = {(w-80.f)/2.f, 0};
        player->color = {0.7, 0.1, 0.8, 1};
        player->color = {1, 1, 1, 1};
        player->valid = true;
        player->p_state = 0;

#if 0
        state->player.shader_enum = Shader_Default;
        state->player.shape_enum = Shape_Rect;
        state->player.rect.size = {50, 50};
        state->player.rect.position = {(w-50.f)/2.f, 0};
#endif
        state->win_width = w;
        state->win_height = h;

        state->e_spawner.spawn_time = 1.f;
        state->e_spawner.clock.restart();
    }

    return 0;
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
checkShaderReload(Renderer* renderer)
{
    uint64 new_vs_change_time;
    uint64 new_fs_change_time;
    for(int i = 0; i < Shader_Max; ++i){
        new_vs_change_time = fileChangeTime(renderer->vert_shaders[i]); 
        new_fs_change_time = fileChangeTime(renderer->frag_shaders[i]); 
        //printf("new change time kappa %lu %lu\n", new_vs_change_time, new_fs_change_time);
        //printf("%s\n%s\n\n", renderer->vert_shaders[i], renderer->frag_shaders[i]);
        //SDL_Delay(100);
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
    checkShaderReload(renderer);
    glClearColor(0.2, 0.2, 0.3, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderer->textures[Texture_Enemy]);

    {//draw rect
        glBindBuffer(GL_ARRAY_BUFFER, renderer->vert_buffers[Shape_Rect]);

        GLuint attrib_pos = renderer->attrib_in_pos;
        glEnableVertexAttribArray(attrib_pos);
        glVertexAttribPointer(attrib_pos, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

        GLuint attrib_uv = renderer->attrib_in_uv;
        glEnableVertexAttribArray(attrib_uv);
        glVertexAttribPointer(attrib_uv, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                (const void*)offsetof(Vertex, uv));

        mat3 model_mat = mat3_identity();
        Entity* ent;
        for(int i = 1; i < MAX_ENTITIES; ++i){
            ent = &state->entities[i];
            if(ent->valid){
                model_mat.z.xy = {ent->rect.position.x, ent->rect.position.y};
                model_mat.m00 = ent->rect.size.x;
                model_mat.m11 = ent->rect.size.y;
                mat3 m = state->projection*model_mat; 

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
    //printf("location %d\n", glGetUniformLocation(renderer->shaders[ent->shader_enum].program, "texture"));
    }

    {//draw player
        mat3 model_mat = mat3_identity();
        Entity* player = &state->entities[E_Player];
        model_mat.z.xy = {player->rect.position.x, player->rect.position.y};
        //model_mat.z.xy = {10, 10};
        model_mat.m00 = player->rect.size.x;
        //model_mat.m00 = 100;
        model_mat.m11 = player->rect.size.y;
        //model_mat.m11 = 2;
        mat3 m = state->projection*model_mat;
        //printmat(m);
        //printmat(model_mat);
        //printmat(state->projection);
        //SDL_Delay(2000);
        //m = state->projection;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderer->textures[Texture_Player]);

        glBindBuffer(GL_ARRAY_BUFFER, renderer->vert_buffers[player->shape_enum]);

        GLuint attrib_pos = renderer->attrib_in_pos;
        glEnableVertexAttribArray(attrib_pos);
        glVertexAttribPointer(attrib_pos, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

        GLuint attrib_uv = renderer->attrib_in_uv;
        glEnableVertexAttribArray(attrib_uv);
        glVertexAttribPointer(attrib_uv, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                (const void*)offsetof(Vertex, uv));

        glUseProgram(renderer->shaders[player->shader_enum].program);
        glUniformMatrix3fv(
                renderer->shaders[player->shader_enum].getUniform("mvp_mat"),
                1, GL_FALSE, m.e);
        glUniform4fv(
                renderer->shaders[player->shader_enum].getUniform("color"),
                1, &player->color.e[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    //printf("location %d\n", glGetUniformLocation(renderer->shaders[player->shader_enum].program, "texture"));
    }

    //SDL_Delay(20);
    //glDrawArrays(GL_POINTS, 0, 6);
    //glDrawElements(GL_TRIANGLES, renderer->test.index_count, GL_UNSIGNED_INT, 0);
}


extern "C"
void processEvent(GameState* state, SDL_Event* event, SDL_Window* window)
{
    Entity* player = &state->entities[E_Player];
    switch (event->type)
    {
        case SDL_QUIT:
            state->quit = true;
        break;

        case SDL_WINDOWEVENT:
        {
            if(event->window.event == SDL_WINDOWEVENT_RESIZED){
                state->win_width = event->window.data1; 
                state->win_height = event->window.data2; 
                state->projection = mat3_ortho(0, state->win_width, 0, state->win_height);
                glViewport(0, 0, state->win_width, state->win_height);

                printf("%dx%d\n", state->win_width, state->win_height);
            }
        }break;

        case SDL_MOUSEBUTTONDOWN:
        {
            if(event->button.button == SDL_BUTTON_LEFT){
                if(!(player->p_state & (1 << PS_InAir))){
                    player->velocity.y = 550;
                    //state->controls.jump = true;
                    player->p_state |= 1 << PS_InAir;
                }
                if( player->p_state & (1 << PS_OnWall)){
                    if(player->rect.position.x > state->win_width/2.f){
                        player->velocity.x = -300;
                    }else{
                        player->velocity.x = 300;
                    }
                    player->p_state &= ~(1 << PS_OnWall);
                }
            }else if(event->button.button == SDL_BUTTON_RIGHT){
                if(SDL_GetRelativeMouseMode() == SDL_FALSE){
                    SDL_SetRelativeMouseMode(SDL_TRUE); 
                    state->controls.focused = true;
                }else {
                    SDL_SetRelativeMouseMode(SDL_FALSE); 
                    state->controls.focused = false;
                }
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
            float ox = 0;
            float oy = 0;
            if(state->controls.focused){
                if(!(player->p_state & (1 << PS_OnWall))){
                    ox = 0.75*event->motion.xrel;
                }
            }
            if(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)){

#if 0
                if(!state->controls.jump){
                    oy = -event->motion.yrel;
                    state->controls.jump = true;
                    state->player.velocity.y += oy;
                }
#endif
            }
            //state->controls.accel.x = ox;
            //state->controls.accel.y += oy;
            player->velocity.x += ox;
            //printf("%f y\n", player->velocity.y);
                /*
                player_state->camera.yaw += radians(ox);
                player_state->camera.pitch += radians(oy);
                if(player_state->camera.pitch > 1.57f)
                    player_state->camera.pitch = 1.57f;
                if(player_state->camera.pitch < -1.57f)
                    player_state->camera.pitch = -1.57f;
                player_state->camera.update();
                */
        }break;


    }
}

static void resolveCollisions(GameState* state)
{
    int  p_state = state->entities[E_Player].p_state;
    vec2 pos = state->entities[E_Player].rect.position;
    vec2 size = state->entities[E_Player].rect.size;
    vec2 vel = state->entities[E_Player].velocity;
    Entity* player = &state->entities[E_Player];
     
    if(p_state & (1 << PS_InAir)){
        if(pos.y < 0.f){ //hit ground
            pos.y = 0.f;
            vel.y = 0.f;
            p_state &= ~(1 << PS_InAir);
            p_state &= ~(1 << PS_OnWall);
        }
    }
    if(pos.x < 0.f){
        pos.x = 0.f; 
        vel.x = 0.f;
        if(p_state & (1 << PS_InAir)){
            vel.y = -50;
            p_state |= 1 << PS_OnWall;
        }
        
    }
    if(pos.x + size.x > state->win_width){
        pos.x = state->win_width - size.x;
        vel.x = 0.f; 
        if(p_state & 1 << PS_InAir){
            vel.y = -50;
            p_state |= 1 << PS_OnWall;
        }
    }
    
    player->color = {1, 1, 1, 1};
    for(int i = 1; i < MAX_ENTITIES; ++i){
        Entity* ent = &state->entities[i];
        if(ent->valid){
            if(ent->rect.position.y < 0.f){
                ent->valid = false;
                break;
            }

            //player-enemy collision
            if(player->rect.position.x < ent->rect.position.x + ent->rect.size.x &&
                player->rect.position.x + player->rect.size.x > ent->rect.position.x &&
                player->rect.position.y < ent->rect.position.y + ent->rect.size.y &&
                player->rect.position.y + player->rect.size.y > ent->rect.position.y)
            {
                printf("COLLISION WITH PLAYER KEK\n");
                player->color = {1, 0, 1, 1};
                //SDL_Delay(100);
            }

            //ent collision  with walls
            if(ent->rect.x < 0.f){
                ent->velocity.x = -ent->velocity.x;
            }else if(ent->rect.x + ent->rect.w > state->win_width){
                ent->velocity.x = -ent->velocity.x;
            }
        }
    }

    state->entities[E_Player].rect.position = pos;
    state->entities[E_Player].velocity = vel;
    state->entities[E_Player].p_state = p_state;
}

static void spawnEnemy(EnemySpawner* spawner, GameState* state)
{
    spawner->spawn_time = 0.4f;
    if(spawner->clock.time() > spawner->spawn_time){
        for(int i = 1; i < MAX_ENTITIES; ++i){
            if(!state->entities[i].valid){
                Entity* ent = &state->entities[i];
                *ent = {};
                ent->type = E_Enemy;
                ent->valid = true;
                ent->shader_enum = Shader_Default;
                ent->shape_enum = Shape_Rect;

                ent->position = {((float)(rand()%1000)/1000.f)*(state->win_width-30.f),
                                    (float)state->win_height};

                //ent->position = {(state->win_width-30)/2.f, state->win_height-30.f};

                float rx = ((float)(rand()%1000)/1000.f);
                float ry = ((float)(rand()%1000)/1000.f);
                ent->color = {rx, ry, 0.5, 1};
                ent->color = {1, 1, 1, 1};
                int sign = rand()%2;
                if(sign == 0)
                //if(ent->position.x > (state->win_width-30.f)/2.f)
                    rx = -rx;

                ent->velocity = {rx*300, -ry*100};

                ent->rect = {ent->position.x, ent->position.y, 30, 30};
                break;
            }
        }    
        spawner->clock.restart();
    }
}

extern "C"
void processTick(GameState* state, float dt)
{
    Entity* player = &state->entities[E_Player];
    player->rect.position.x += player->velocity.x*dt; 
    player->velocity.x *= 0.95f;
    //bool sgn = player->velocity.x > 0.f;
    //player->velocity.x -= (-player->velocity.x/player->velocity.x)*100*dt;
    //bool sgn2 = player->velocity.x > 0.f;
    //if(sgn != sgn2){
    //    player->velocity.x = 0.f;
    //}
    if(player->p_state & (1 << PS_InAir)){
        player->rect.position.y += player->velocity.y*dt;
        if(!(player->p_state & (1 << PS_OnWall))){
            player->velocity.y -= 1000*dt;
        }
    } 
    for(int i = 1; i < MAX_ENTITIES; ++i){
        Entity* ent = &state->entities[i];
        if(ent->valid){
            ent->rect.position.x += ent->velocity.x*dt;
            ent->rect.position.y += ent->velocity.y*dt;
            ent->velocity.y -= 1000*dt;
        }
    }
    resolveCollisions(state);
    spawnEnemy(&state->e_spawner, state);

    printf("player %d\n", (int)player->p_state);
}

