#include <Python.h>
#include "poker.h"

static PyObject* set_opponent_action(PyObject* self, PyObject* args);
static PyObject* vec_set_opponent_action(PyObject* self, PyObject* args);

#define Env Poker
#define MY_METHODS {"set_opponent_action", (PyCFunction)set_opponent_action, METH_VARARGS, "Set opponent action"}, {"vec_set_opponent_action", (PyCFunction)vec_set_opponent_action, METH_VARARGS, "Set opponent action for vectorized environments"}

#include "../env_binding.h"

static int my_init(Env *env, PyObject *args, PyObject *kwargs) {
    int width = unpack(kwargs, "width");
    int height = unpack(kwargs, "height");
    int starting_stack = unpack(kwargs, "starting_stack");
    int small_blind = unpack(kwargs, "small_blind");
    int big_blind = unpack(kwargs, "big_blind");
    int self_play_mode = unpack(kwargs, "self_play_mode");
    
    // Try to get generation_number, default to 1 if not found
    int generation_number = 1;
    PyObject *gen_obj = PyDict_GetItemString(kwargs, "generation_number");
    if (gen_obj && PyLong_Check(gen_obj)) {
        generation_number = PyLong_AsLong(gen_obj);
    }
    
    // Try to get intended_opponent_generation, default to 0 if not found
    int intended_opponent_generation = 0;
    PyObject *opp_gen_obj = PyDict_GetItemString(kwargs, "intended_opponent_generation");
    if (opp_gen_obj && PyLong_Check(opp_gen_obj)) {
        intended_opponent_generation = PyLong_AsLong(opp_gen_obj);
    }
    
    
    // Try to get max_episode_length, default to 500 if not found
    int max_episode_length = 500;
    PyObject *max_ep_obj = PyDict_GetItemString(kwargs, "max_episode_length");
    if (max_ep_obj && PyLong_Check(max_ep_obj)) {
        max_episode_length = PyLong_AsLong(max_ep_obj);
    }
    
    // Try to get max_hands_per_episode, default to 200 if not found
    int max_hands_per_episode = 200;
    PyObject *max_hands_obj = PyDict_GetItemString(kwargs, "max_hands_per_episode");
    if (max_hands_obj && PyLong_Check(max_hands_obj)) {
        max_hands_per_episode = PyLong_AsLong(max_hands_obj);
    }
    
    // Try to get render_mode, default to human mode for eval/interactive use
    int headless = 0;
    PyObject *render_obj = PyDict_GetItemString(kwargs, "render_mode");
    if (render_obj && PyUnicode_Check(render_obj)) {
        const char* render_str = PyUnicode_AsUTF8(render_obj);
        if (render_str && strcmp(render_str, "None") == 0) {
            headless = 1;
        }
        // Default is human mode (headless = 0) for visualization
    } else {
        // If no render_mode specified, default to human mode
        headless = 0;
    }
    
    env->width = (float)width;
    env->height = (float)height;
    env->config.starting_stack = starting_stack;
    env->config.small_blind = small_blind;
    env->config.big_blind = big_blind;
    env->self_play_mode = self_play_mode;
    env->current_generation = (float)generation_number;
    env->intended_opponent_generation = (float)intended_opponent_generation;
    env->max_episode_length = max_episode_length;
    env->max_hands_per_episode = max_hands_per_episode;
    
    env->headless = headless;
    
    allocate(env);
    return 0;
}

static int my_log(PyObject* dict, Log* log) {
    assign_to_dict(dict, "episode_return", log->episode_return);
    assign_to_dict(dict, "episode_length", log->episode_length);
    assign_to_dict(dict, "hands_played", log->hands_played);
    assign_to_dict(dict, "hands_won", log->hands_won);
    assign_to_dict(dict, "hero_folds", log->hero_folds);
    assign_to_dict(dict, "villain_folds", log->villain_folds);
    assign_to_dict(dict, "showdown_wins", log->showdown_wins);
    assign_to_dict(dict, "showdown_losses", log->showdown_losses);
    
    assign_to_dict(dict, "bb_per_100", log->bb_per_100);
    assign_to_dict(dict, "action_fold", log->action_fold);
    assign_to_dict(dict, "action_call", log->action_call);
    assign_to_dict(dict, "action_check", log->action_check);
    assign_to_dict(dict, "action_bet_pot", log->action_bet_pot);
    assign_to_dict(dict, "action_all_in", log->action_all_in);
    assign_to_dict(dict, "river_decisions", log->river_decisions);
    assign_to_dict(dict, "pot_size_won", log->pot_size_won);
    assign_to_dict(dict, "pot_size_lost", log->pot_size_lost);
    
    assign_to_dict(dict, "rainbow_boards", log->rainbow_boards);
    assign_to_dict(dict, "two_tone_boards", log->two_tone_boards);
    assign_to_dict(dict, "broadway_boards", log->broadway_boards);
    assign_to_dict(dict, "paired_boards", log->paired_boards);
    assign_to_dict(dict, "villain_aggression_freq", log->villain_aggression_freq);
    assign_to_dict(dict, "pot_randomization_factor", log->pot_randomization_factor);
    
    // Hero comprehensive stats
    assign_to_dict(dict, "hero_vpip", log->hero_vpip);
    assign_to_dict(dict, "hero_pfr", log->hero_pfr);
    assign_to_dict(dict, "hero_3bet", log->hero_3bet);
    assign_to_dict(dict, "hero_fold_to_3bet", log->hero_fold_to_3bet);
    assign_to_dict(dict, "hero_cbet_flop", log->hero_cbet_flop);
    assign_to_dict(dict, "hero_cbet_turn", log->hero_cbet_turn);
    assign_to_dict(dict, "hero_cbet_river", log->hero_cbet_river);
    assign_to_dict(dict, "hero_aggression_factor", log->hero_aggression_factor);
    assign_to_dict(dict, "hero_wtsd", log->hero_wtsd);
    assign_to_dict(dict, "hero_w_at_sd", log->hero_w_at_sd);
    
    // Villain comprehensive stats
    assign_to_dict(dict, "villain_vpip", log->villain_vpip);
    assign_to_dict(dict, "villain_pfr", log->villain_pfr);
    assign_to_dict(dict, "villain_3bet", log->villain_3bet);
    assign_to_dict(dict, "villain_fold_to_3bet", log->villain_fold_to_3bet);
    assign_to_dict(dict, "villain_cbet_flop", log->villain_cbet_flop);
    assign_to_dict(dict, "villain_cbet_turn", log->villain_cbet_turn);
    assign_to_dict(dict, "villain_cbet_river", log->villain_cbet_river);
    assign_to_dict(dict, "villain_aggression_factor", log->villain_aggression_factor);
    assign_to_dict(dict, "villain_wtsd", log->villain_wtsd);
    assign_to_dict(dict, "villain_w_at_sd", log->villain_w_at_sd);
    
    // Game flow stats
    assign_to_dict(dict, "preflop_all_in_rate", log->preflop_all_in_rate);
    assign_to_dict(dict, "avg_pot_size", log->avg_pot_size);
    assign_to_dict(dict, "showdown_rate", log->showdown_rate);
    assign_to_dict(dict, "limped_pots", log->limped_pots);
    assign_to_dict(dict, "three_bet_pots", log->three_bet_pots);
    
    assign_to_dict(dict, "pbs_updates", log->pbs_updates);
    assign_to_dict(dict, "opponent_range_entropy", log->opponent_range_entropy);
    assign_to_dict(dict, "belief_prediction_errors", log->belief_prediction_errors);
    
    assign_to_dict(dict, "generation_number", log->generation_number);
    assign_to_dict(dict, "opponent_generation", log->opponent_generation);
    assign_to_dict(dict, "n", log->n);
    return 0;
}

// FIXED: Single environment version
static PyObject* set_opponent_action(PyObject* self, PyObject* args) {
    Env* env = unpack_env(args);
    if (!env) {
        return NULL;
    }
    
    PyObject* action_obj = PyTuple_GetItem(args, 1);
    if (!action_obj || !PyLong_Check(action_obj)) {
        PyErr_SetString(PyExc_TypeError, "Second argument must be an integer");
        return NULL;
    }
    int action = PyLong_AsLong(action_obj);
    
    // CRITICAL: Validate action bounds to prevent C segfault
    if (action < 0 || action > 4) {
        char error_msg[200];
        snprintf(error_msg, sizeof(error_msg), 
                "Invalid poker action %d (must be 0-4: fold=0, call=1, check=2, bet_pot=3, all_in=4)", action);
        PyErr_SetString(PyExc_ValueError, error_msg);
        return NULL;
    }
    
    env->opponent_action_value = action;
    env->opponent_action_set = 1;
    
    Py_RETURN_NONE;
}

// NEW: Vectorized version for multi-environment
static PyObject* vec_set_opponent_action(PyObject* self, PyObject* args) {
    if (PyTuple_Size(args) != 3) {
        PyErr_SetString(PyExc_TypeError, "vec_set_opponent_action requires 3 arguments: vec_env, env_idx, action");
        return NULL;
    }
    
    VecEnv* vec = unpack_vecenv(args);
    if (!vec) {
        return NULL;
    }
    
    PyObject* env_idx_obj = PyTuple_GetItem(args, 1);
    if (!PyLong_Check(env_idx_obj)) {
        PyErr_SetString(PyExc_TypeError, "env_idx must be an integer");
        return NULL;
    }
    int env_idx = PyLong_AsLong(env_idx_obj);
    
    if (env_idx < 0 || env_idx >= vec->num_envs) {
        PyErr_SetString(PyExc_IndexError, "env_idx out of bounds");
        return NULL;
    }
    
    PyObject* action_obj = PyTuple_GetItem(args, 2);
    if (!PyLong_Check(action_obj)) {
        PyErr_SetString(PyExc_TypeError, "action must be an integer");
        return NULL;
    }
    int action = PyLong_AsLong(action_obj);
    
    // CRITICAL: Validate action bounds
    if (action < 0 || action > 4) {
        char error_msg[200];
        snprintf(error_msg, sizeof(error_msg), 
                "Invalid poker action %d (must be 0-4: fold=0, call=1, check=2, bet_pot=3, all_in=4)", action);
        PyErr_SetString(PyExc_ValueError, error_msg);
        return NULL;
    }
    
    // Set action for specific environment
    Env* env = vec->envs[env_idx];
    if (!env) {
        PyErr_SetString(PyExc_ValueError, "Invalid environment at index");
        return NULL;
    }
    
    env->opponent_action_value = action;
    env->opponent_action_set = 1;
    
    Py_RETURN_NONE;
}