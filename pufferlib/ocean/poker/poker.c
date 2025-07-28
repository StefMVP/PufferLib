#include "poker.h"
#include <time.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

int generate_dummy_action(Poker* env) {
    uint32_t call_amount = env->current_bet - env->players[0].current_bet;
    
    if (call_amount == 0) {
        if (rand() % 3 == 0) {
            env->actions[0] = ACTIONS.bet_pot;
        } else {
            env->actions[0] = ACTIONS.call;
        }
    } else {
        int choice = rand() % 4;
        if (choice == 0) {
            env->actions[0] = ACTIONS.fold;
        } else if (choice == 1) {
            env->actions[0] = ACTIONS.call;
        } else {
            env->actions[0] = ACTIONS.bet_pot;
        }
    }
    return 1;
}

#ifdef __EMSCRIPTEN__
typedef struct {
    Poker *env;
} WebRenderArgs;

void emscriptenStep(void *e) {
    WebRenderArgs *args = (WebRenderArgs *)e;
    Poker *env = args->env;

    if (generate_dummy_action(env)) {
        c_step(env);
    }
    c_render(env);
    return;
}

WebRenderArgs *web_args = NULL;
#endif

int main() {
    srand(time(NULL));

    Poker *env = calloc(1, sizeof(Poker));
    
    env->observations = (float*)calloc(POKER.obs_size, sizeof(float));
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
        if (generate_dummy_action(env)) {
            c_step(env);
        }
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