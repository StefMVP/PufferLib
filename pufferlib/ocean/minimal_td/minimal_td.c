// Standalone C demo for minimal_td environment
// Compile using: ./scripts/build_ocean.sh minimal_td [local|fast]
// Run with: ./minimal_td

#include "minimal_td.h"
#include <time.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void generate_dummy_action(MinimalTD* env) {
    env->actions[0] = rand() % 577;
}

#ifdef __EMSCRIPTEN__
typedef struct {
    MinimalTD *env;
} WebRenderArgs;

void emscriptenStep(void *e) {
    WebRenderArgs *args = (WebRenderArgs *)e;
    MinimalTD *env = args->env;

    generate_dummy_action(env);
    c_step(env);
    c_render(env);
    return;
}

WebRenderArgs *web_args = NULL;
#endif

int main() {
    srand(time(NULL));

    MinimalTD *env = calloc(1, sizeof(MinimalTD));
    
    env->observations = (float*)calloc(GRID_SIZE * 7, sizeof(float));
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

    env->config.tower_cost = 30;
    env->config.starting_gold = 150;
    env->config.enemy_base_hp = 1;
    env->config.min_towers_per_episode = 5;
    env->config.max_towers_per_episode = 10;
    
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