#include "poker.h"

#define Env Poker
#include "../env_binding.h"

static int my_init(Env *env, PyObject *args, PyObject *kwargs) {
    int width = unpack(kwargs, "width");
    int height = unpack(kwargs, "height");
    int starting_stack = unpack(kwargs, "starting_stack");
    int small_blind = unpack(kwargs, "small_blind");
    int big_blind = unpack(kwargs, "big_blind");
    
    env->width = (float)width;
    env->height = (float)height;
    env->config.starting_stack = starting_stack;
    env->config.small_blind = small_blind;
    env->config.big_blind = big_blind;
    
    allocate(env);
    return 0;
}

static int my_log(PyObject* dict, Log* log) {
    assign_to_dict(dict, "episode_return", log->episode_return);
    assign_to_dict(dict, "episode_length", log->episode_length);
    assign_to_dict(dict, "hands_played", log->hands_played);
    assign_to_dict(dict, "showdowns_reached", log->showdowns_reached);
    assign_to_dict(dict, "hands_won", log->hands_won);
    assign_to_dict(dict, "big_blinds_won", log->big_blinds_won);
    return 0;
}