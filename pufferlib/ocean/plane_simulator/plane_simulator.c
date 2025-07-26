#include "plane_simulator.h"
#include <time.h>
#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

void generate_dummy_action(PlaneSimulator* env) {
    env->actions[0] = (float)rand() / RAND_MAX;
    env->actions[1] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f;
    env->actions[2] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f;
    env->actions[3] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f;
}

#ifdef __EMSCRIPTEN__
typedef struct {
    PlaneSimulator *env;
} WebRenderArgs;

void emscriptenStep(void *e) {
    WebRenderArgs *args = (WebRenderArgs *)e;
    PlaneSimulator *env = args->env;

    generate_dummy_action(env);
    c_step(env);
    c_render(env);
    return;
}

WebRenderArgs *web_args = NULL;
#endif

int main() {
    srand(time(NULL));

    PlaneSimulator *env = calloc(1, sizeof(PlaneSimulator));
    
    env->observations = (float*)calloc(OBS_SIZE, sizeof(float));
    env->actions = (float*)calloc(ACTION_SIZE, sizeof(float));
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
    
    env->mode = MODE_TAKEOFF;
    memset(&env->log, 0, sizeof(Log));
    c_reset(env);

#ifdef __EMSCRIPTEN__
    WebRenderArgs *args = calloc(1, sizeof(WebRenderArgs));
    args->env = env;
    web_args = args;

    emscripten_set_main_loop_arg(emscriptenStep, args, 0, true);
#else
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