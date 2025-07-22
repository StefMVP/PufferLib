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
    if (IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT)) {
        env->human_mode = !env->human_mode;
        printf("SHIFT TOGGLE: Human mode %s\n", env->human_mode ? "ON" : "OFF");
    }
    
    if (env->human_mode) {
        bool w = IsKeyDown(KEY_W);
        bool s = IsKeyDown(KEY_S);
        bool a = IsKeyDown(KEY_A);
        bool d = IsKeyDown(KEY_D);
        bool space = IsKeyDown(KEY_SPACE);
        
        if (space) {
            env->actions[0] = ACTION_BLIZZARD;
        }
        else if (w && a) {
            env->actions[0] = ACTION_MOVE_UP_LEFT;
        } else if (w && d) {
            env->actions[0] = ACTION_MOVE_UP_RIGHT;
        } else if (s && a) {
            env->actions[0] = ACTION_MOVE_DOWN_LEFT;
        } else if (s && d) {
            env->actions[0] = ACTION_MOVE_DOWN_RIGHT;
        } else if (w) {
            env->actions[0] = ACTION_MOVE_UP;
        } else if (s) {
            env->actions[0] = ACTION_MOVE_DOWN;
        } else if (a) {
            env->actions[0] = ACTION_MOVE_LEFT;
        } else if (d) {
            env->actions[0] = ACTION_MOVE_RIGHT;
        } else if (space) {
            env->actions[0] = ACTION_BLIZZARD;
        } else if (IsKeyDown(KEY_Q)) {
            env->actions[0] = ACTION_USE_HEALTH_POTION;
        } else if (IsKeyDown(KEY_E)) {
            env->actions[0] = ACTION_USE_MANA_POTION;
        } else if (IsKeyDown(KEY_F)) {
            env->actions[0] = ACTION_INTERACT;
        } else if (IsKeyDown(KEY_Z)) {
            env->actions[0] = ACTION_EXIT_TOWN;
        } else {
            env->actions[0] = ACTION_NOOP;
        }
    } else {
        env->actions[0] = rand() % 22;
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
    
    uint16_t max_obs_size = OBS_SIZE;
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