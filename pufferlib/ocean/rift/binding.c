#include "rift.h"

#define Env Rift
#include "../env_binding.h"

static int my_init(Env *env, PyObject *args, PyObject *kwargs) {
    allocate(env);
    return 0;
}

static int my_log(PyObject* dict, Log* log) {
    assign_to_dict(dict, "episode_return", log->episode_return);
    assign_to_dict(dict, "episode_length", log->episode_length);
    assign_to_dict(dict, "monsters_killed", log->monsters_killed);
    assign_to_dict(dict, "boss_kills", log->boss_kills);
    assign_to_dict(dict, "gold_earned", log->gold_earned);
    assign_to_dict(dict, "damage_dealt", log->damage_dealt);
    assign_to_dict(dict, "damage_taken", log->damage_taken);
    assign_to_dict(dict, "rift_completions", log->rift_completions);
    assign_to_dict(dict, "vendor_transactions", log->vendor_transactions);
    assign_to_dict(dict, "blizzards_cast", log->blizzards_cast);
    assign_to_dict(dict, "deaths", log->deaths);
    assign_to_dict(dict, "completion_rewards", log->completion_rewards);
    assign_to_dict(dict, "monster_kill_rewards", log->monster_kill_rewards);
    assign_to_dict(dict, "death_penalties", log->death_penalties);
    assign_to_dict(dict, "hero_level", log->hero_level);
    assign_to_dict(dict, "total_experience", log->total_experience);
    assign_to_dict(dict, "shop_purchases", log->shop_purchases);
    assign_to_dict(dict, "town_time_efficiency", log->town_time_efficiency);
    assign_to_dict(dict, "current_gold", log->current_gold);
    assign_to_dict(dict, "total_strength", log->total_strength);
    assign_to_dict(dict, "total_dexterity", log->total_dexterity);
    assign_to_dict(dict, "total_intelligence", log->total_intelligence);
    assign_to_dict(dict, "total_vitality", log->total_vitality);
    return 0;
}