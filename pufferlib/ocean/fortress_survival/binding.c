#include <Python.h>
#include "fortress_survival.h"

#define Env FortressSurvival
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
    assign_to_dict(dict, "buildings_built", log->buildings_built);
    assign_to_dict(dict, "waves_completed", log->waves_completed);
    assign_to_dict(dict, "lumber_produced", log->lumber_produced);
    assign_to_dict(dict, "gold_earned", log->gold_earned);
    assign_to_dict(dict, "damage_dealt", log->damage_dealt);
    assign_to_dict(dict, "damage_taken", log->damage_taken);
    assign_to_dict(dict, "walls_destroyed", log->walls_destroyed);
    assign_to_dict(dict, "towers_built", log->towers_built);
    assign_to_dict(dict, "mills_built", log->mills_built);
    assign_to_dict(dict, "heroes_leveled", log->heroes_leveled);
    assign_to_dict(dict, "survival_time", log->survival_time);
    assign_to_dict(dict, "wave_completion_rewards", log->wave_completion_rewards);
    assign_to_dict(dict, "enemy_kill_rewards", log->enemy_kill_rewards);
    assign_to_dict(dict, "death_penalties", log->death_penalties);
    assign_to_dict(dict, "goal_death_penalties", log->goal_death_penalties);
    assign_to_dict(dict, "goal_deaths", log->goal_deaths);
    assign_to_dict(dict, "current_wave", log->current_wave);
    assign_to_dict(dict, "current_lumber", log->current_lumber);
    assign_to_dict(dict, "current_gold", log->current_gold);
    assign_to_dict(dict, "goal_health_ratio", log->goal_health_ratio);
    assign_to_dict(dict, "games_lost", log->games_lost);
    return 0;
}