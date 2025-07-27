#include "poker.h"

#define Env Poker
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
    
    env->width = (float)width;
    env->height = (float)height;
    env->config.starting_stack = starting_stack;
    env->config.small_blind = small_blind;
    env->config.big_blind = big_blind;
    env->self_play_mode = self_play_mode;
    env->current_generation = (float)generation_number;
    
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
    assign_to_dict(dict, "vpip_hands", log->vpip_hands);
    assign_to_dict(dict, "pfr_hands", log->pfr_hands);
    assign_to_dict(dict, "three_bet_hands", log->three_bet_hands);
    assign_to_dict(dict, "three_bet_opportunities", log->three_bet_opportunities);
    assign_to_dict(dict, "aggression_bets_raises", log->aggression_bets_raises);
    assign_to_dict(dict, "aggression_calls", log->aggression_calls);
    assign_to_dict(dict, "cbet_hands", log->cbet_hands);
    assign_to_dict(dict, "cbet_opportunities", log->cbet_opportunities);
    assign_to_dict(dict, "fold_to_cbet_hands", log->fold_to_cbet_hands);
    assign_to_dict(dict, "fold_to_cbet_opportunities", log->fold_to_cbet_opportunities);
    assign_to_dict(dict, "wtsd_hands", log->wtsd_hands);
    assign_to_dict(dict, "flops_seen", log->flops_seen);
    assign_to_dict(dict, "ats_hands", log->ats_hands);
    assign_to_dict(dict, "ats_opportunities", log->ats_opportunities);
    assign_to_dict(dict, "generation_number", log->generation_number);
    return 0;
}