#include "fortress_survival.h"
#include <time.h>
#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void generate_dummy_action(FortressSurvival* env) {
    if (env->current_phase == PHASE_SETUP || env->current_phase == PHASE_WAVE_COMPLETE) {
        if (env->resources.lumber >= env->config.wall_build_cost && rand() % 20 == 0) {
            env->actions[0] = (rand() % 64) + 1;
        } else {
            env->actions[0] = 0;
        }
    } else {
        env->actions[0] = 0;
    }
}

#ifdef __EMSCRIPTEN__
typedef struct {
    FortressSurvival *env;
} WebRenderArgs;

void emscriptenStep(void *e) {
    WebRenderArgs *args = (WebRenderArgs *)e;
    FortressSurvival *env = args->env;

    generate_dummy_action(env);
    c_step(env);
    c_render(env);
    return;
}

WebRenderArgs *web_args = NULL;
#endif

int main() {
    srand(time(NULL));

    FortressSurvival *env = calloc(1, sizeof(FortressSurvival));
    
    env->observations = (float*)calloc(OBS_SIZE, sizeof(float));
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
    
    env->cell_size_render = 32.0f;
    
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