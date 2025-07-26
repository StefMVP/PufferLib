#include "mazing_contest.h"

#define Env MazingContest
#include "../env_binding.h"

static int my_init(Env* env, PyObject* args, PyObject* kwargs) {
    // Start with default config - inline to avoid linking issues
    env->config = (GameConfig){
        .gold = 50,
        .lumber = 1,
        .phase = PHASE_BUILD,
        .phase_timer = 0,
        .round_number = 1,
        .runner_time = 0.0f,
        .best_time = 0.0f,
        .maze_completed = 0,
        .build_time_limit = 600,
        .max_moves = 3000000,
        .max_rounds = 1,
        .min_gold = 10,
        .max_gold = 30,
        .min_lumber = 2,
        .max_lumber = 4,
        .num_thunderclap_towers = 5,
        .thunderclap_slowdown_factor = 0.3f,
        .thunderclap_slowdown_duration = 60,
        .thunderclap_range = 50.0f,
        .thunderclap_cooldown = 60,
        .runner_base_speed = 6.0f,
        .path_length_reward_multiplier = 1.4f,
        // tower_utilization_reward removed
        .wall_touch_base_reward = 0.1f,
        .thunderclap_activation_reward = 0.0f,
        .thunderclap_slowdown_reward = 0.06f,
        .thunderclap_unused_penalty = 3.0f,
        .runner_time_reward_multiplier = 0.0f,
        .grid_cell_size = 32.0f,
        .grid_cell_center_offset = 16.0f,
        .gold_normalization = 50.0f,
        .lumber_normalization = 5.0f,
        .phase_normalization = 2.0f,
        .build_time_normalization = 600.0f,
        .runner_time_normalization = 30.0f,
        .fps = 60.0f,
        .stuck_teleport_ticks = 120,
        .tower_touch_cooldown_ticks = 60,
        .cell_size_render = 50.0f,
        .min_random_walls = 8,
        .max_random_walls = 20,
        .min_random_thunderclaps = 0,
        .max_random_thunderclaps = 0,
        .goal_enabled = 1
    };
    
    // Override config from Python BEFORE calling full init
    if (kwargs) {
        PyObject* build_time_limit = PyDict_GetItemString(kwargs, "build_time_limit");
        if (build_time_limit && PyLong_Check(build_time_limit)) {
            env->config.build_time_limit = PyLong_AsLong(build_time_limit);
        }
        
        // max_rounds parameter removed - training framework handles episode termination
        
        PyObject* max_moves = PyDict_GetItemString(kwargs, "max_moves");
        if (max_moves && PyLong_Check(max_moves)) {
            env->config.max_moves = PyLong_AsLong(max_moves);
        }
        
        PyObject* max_rounds = PyDict_GetItemString(kwargs, "max_rounds");
        if (max_rounds && PyLong_Check(max_rounds)) {
            env->config.max_rounds = PyLong_AsLong(max_rounds);
        }
        
        PyObject* min_gold = PyDict_GetItemString(kwargs, "min_gold");
        if (min_gold && PyLong_Check(min_gold)) {
            env->config.min_gold = PyLong_AsLong(min_gold);
        }
        
        PyObject* max_gold = PyDict_GetItemString(kwargs, "max_gold");
        if (max_gold && PyLong_Check(max_gold)) {
            env->config.max_gold = PyLong_AsLong(max_gold);
        }
        
        PyObject* min_lumber = PyDict_GetItemString(kwargs, "min_lumber");
        if (min_lumber && PyLong_Check(min_lumber)) {
            env->config.min_lumber = PyLong_AsLong(min_lumber);
        }
        
        PyObject* max_lumber = PyDict_GetItemString(kwargs, "max_lumber");
        if (max_lumber && PyLong_Check(max_lumber)) {
            env->config.max_lumber = PyLong_AsLong(max_lumber);
        }
        
        PyObject* goal_enabled = PyDict_GetItemString(kwargs, "goal_enabled");
        if (goal_enabled && PyLong_Check(goal_enabled)) {
            env->config.goal_enabled = PyLong_AsLong(goal_enabled);
        }
    }
    
    // Call main init
    allocate(env);
    return 0;
}

static int my_log(PyObject* dict, Log* log) {
    assign_to_dict(dict, "episode_return", log->episode_return);
    assign_to_dict(dict, "episode_length", log->episode_length);
    assign_to_dict(dict, "towers_built", log->towers_built);
    assign_to_dict(dict, "total_gold_spent", log->total_gold_spent);
    assign_to_dict(dict, "best_time", log->best_time);
    assign_to_dict(dict, "rounds_completed", log->rounds_completed);
    assign_to_dict(dict, "average_path_length", log->average_path_length);
    assign_to_dict(dict, "walls_built", log->walls_built);
    assign_to_dict(dict, "path_length_rewards", log->path_length_rewards);
    // towers_touched, towers_untouched, tower_utilization_rewards removed
    assign_to_dict(dict, "thunderclap_towers_built", log->thunderclap_towers_built);
    assign_to_dict(dict, "thunderclap_slowdown_time", log->thunderclap_slowdown_time);
    assign_to_dict(dict, "thunderclap_rewards", log->thunderclap_rewards);
    assign_to_dict(dict, "wall_touches", log->wall_touches);
    assign_to_dict(dict, "wall_touch_rewards", log->wall_touch_rewards);
    assign_to_dict(dict, "wall_touch_1", log->wall_touch_1);
    assign_to_dict(dict, "wall_touch_2", log->wall_touch_2);
    assign_to_dict(dict, "wall_touch_3", log->wall_touch_3);
    assign_to_dict(dict, "wall_touch_4", log->wall_touch_4);
    assign_to_dict(dict, "wall_touch_5", log->wall_touch_5);
    assign_to_dict(dict, "wall_touch_6", log->wall_touch_6);
    assign_to_dict(dict, "goal_visits", log->goal_visits);
    assign_to_dict(dict, "efficiency_rewards", log->efficiency_rewards);
    assign_to_dict(dict, "efficiency_percentage", log->efficiency_percentage);
    return 0;
}