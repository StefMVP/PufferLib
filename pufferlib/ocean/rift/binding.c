#include "rift.h"

#define Env Rift
#include "../env_binding.h"

static int my_init(Env *env, PyObject *args, PyObject *kwargs) {
    int width = unpack(kwargs, "width");
    int height = unpack(kwargs, "height");
    int cell_size = unpack(kwargs, "cell_size");
    
    allocate(env);
    return 0;
}

static int my_log(PyObject* dict, Log* log) {
    assign_to_dict(dict, "episode_return", log->episode_return);
    assign_to_dict(dict, "episode_length", log->episode_length);
    assign_to_dict(dict, "monsters_killed", log->monsters_killed);
    assign_to_dict(dict, "boss_kills", log->boss_kills);
    assign_to_dict(dict, "items_collected", log->items_collected);
    assign_to_dict(dict, "gold_earned", log->gold_earned);
    assign_to_dict(dict, "damage_dealt", log->damage_dealt);
    assign_to_dict(dict, "damage_taken", log->damage_taken);
    assign_to_dict(dict, "rift_completions", log->rift_completions);
    assign_to_dict(dict, "vendor_transactions", log->vendor_transactions);
    assign_to_dict(dict, "whirlwinds_used", log->whirlwinds_used);
    return 0;
}