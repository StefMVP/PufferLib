// Standalone C demo for rift environment
// Compile using: ./scripts/build_ocean.sh rift [local|fast]
// Run with: ./rift

#include "rift.h"
#include <stdio.h>
#include <time.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void generate_dummy_action(Rift* env) {
    // Check if SHIFT is held for manual control
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        // Manual control - map keyboard to actions with diagonal support
        bool w = IsKeyDown(KEY_W);
        bool s = IsKeyDown(KEY_S);
        bool a = IsKeyDown(KEY_A);
        bool d = IsKeyDown(KEY_D);
        
        // Handle 4-directional movement (no diagonals)
        if (w) {
            env->actions[0] = ACTION_MOVE_UP;
        } else if (s) {
            env->actions[0] = ACTION_MOVE_DOWN;
        } else if (a) {
            env->actions[0] = ACTION_MOVE_LEFT;
        } else if (d) {
            env->actions[0] = ACTION_MOVE_RIGHT;
        } else if (IsKeyDown(KEY_SPACE)) {
            env->actions[0] = ACTION_WHIRLWIND;
        } else if (IsKeyDown(KEY_Q)) {
            env->actions[0] = ACTION_USE_HEALTH_POTION;
        } else if (IsKeyDown(KEY_E)) {
            env->actions[0] = ACTION_USE_MANA_POTION;
        } else if (IsKeyDown(KEY_F)) {
            env->actions[0] = ACTION_INTERACT;
        } else {
            env->actions[0] = ACTION_NOOP;
        }
    } else {
        // AI control - random actions
        env->actions[0] = rand() % 13;  // 0-12 for our action space
    }
}

#ifdef __EMSCRIPTEN__
typedef struct {
    Rift *env;
} WebRenderArgs;

void emscriptenStep(void *e) {
    WebRenderArgs *args = (WebRenderArgs *)e;
    Rift *env = args->env;

    generate_dummy_action(env);
    c_step(env);
    c_render(env);
    return;
}

WebRenderArgs *web_args = NULL;
#endif

int main() {
    srand(time(NULL));

    Rift *env = calloc(1, sizeof(Rift));
    
    // Allocate observation space - using larger size to handle both rift and town
    int max_obs_size = (MAP_SIZE + 30); // Matches python observation space
    env->observations = (float*)calloc(max_obs_size, sizeof(float));
    env->actions = (float*)calloc(1, sizeof(float));
    env->rewards = (float*)calloc(1, sizeof(float));
    env->terminals = (unsigned char*)calloc(1, sizeof(unsigned char));

    if (!env->observations || !env->actions || !env->rewards || !env->terminals) {
        fprintf(stderr, "ERROR: Failed to allocate memory for demo buffers.\n");
        free(env->observations);
        free(env->actions);
        free(env->rewards);
        free(env->terminals);
        free(env);
        return 1;
    }

    // Initialize configuration
    env->config = DEFAULT_CONFIG;
    
    init(env);
    allocate(env);
    c_reset(env);

#ifdef __EMSCRIPTEN__
    WebRenderArgs *args = calloc(1, sizeof(WebRenderArgs));
    args->env = env;
    web_args = args;

    emscripten_set_main_loop_arg(emscriptenStep, args, 0, true);
#else
    c_render(env);

    while (!WindowShouldClose()) {
        generate_dummy_action(env);
        c_step(env);
        c_render(env);
    }

    c_close(env);
    free(env->observations);
    free(env->actions);
    free(env->rewards);
    free(env->terminals);
    free(env);
#endif

    return 0;
}