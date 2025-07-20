#include "minimal_td.h"

#define Env MinimalTD
#include "../env_binding.h"

static int my_init(Env *env, PyObject *args, PyObject *kwargs) {
    int width = unpack(kwargs, "width");
    int height = unpack(kwargs, "height");
    int cell_size = unpack(kwargs, "cell_size");
    env->cell_size_render = (float)cell_size;
    
    allocate(env);
    return 0;
}

static int my_log(PyObject* dict, Log* log) {
    assign_to_dict(dict, "episode_return", log->episode_return);
    assign_to_dict(dict, "episode_length", log->episode_length);
    assign_to_dict(dict, "enemies_killed", log->enemies_killed);
    assign_to_dict(dict, "enemies_escaped", log->enemies_escaped);
    assign_to_dict(dict, "towers_placed", log->towers_placed);
    assign_to_dict(dict, "win_rate", log->win_rate);
    assign_to_dict(dict, "lives_remaining", log->lives_remaining);
    assign_to_dict(dict, "normal_towers", log->normal_towers);
    assign_to_dict(dict, "splash_towers", log->splash_towers);
    assign_to_dict(dict, "sniper_towers", log->sniper_towers);
    assign_to_dict(dict, "gold_efficiency", log->gold_efficiency);
    return 0;
}