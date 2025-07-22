#ifndef RIFT_RENDER_H
#define RIFT_RENDER_H

#include "render_core.h"
#include "render_rift.h"
#include "render_town.h"

void c_render(Rift* env);
void render_rift(Rift* env);
void render_town(Rift* env);
void render_character_tab(Rift* env);
void render_shop_tab(Rift* env);

void c_render(Rift* env) {
    if (!env->client) {
        env->client = make_client(env);
    }
    
    if (IsKeyDown(KEY_ESCAPE)) exit(0);
    
    Vector2 player_screen = grid_to_screen(env->player.x, env->player.y, env->client->cell_size);
    env->client->camera.target = player_screen;
    
    BeginDrawing();
    ClearBackground((Color){CLEAR_BG_R, CLEAR_BG_G, CLEAR_BG_B, 255});
    
    if (env->current_phase == PHASE_RIFT) {
        render_rift(env);
    } else {
        render_town(env);
    }
    
    render_ui(env, env->client->width, env->client->height);
    
    EndDrawing();
}

#endif