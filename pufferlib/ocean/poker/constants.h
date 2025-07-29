#ifndef POKER_CONSTANTS_H
#define POKER_CONSTANTS_H

#include <stdint.h>

typedef struct {
    int fold, call, check, bet_pot, all_in;
} ActionTypes;

static const ActionTypes ACTIONS = {
    .fold = 0, .call = 1, .check = 2, .bet_pot = 3, .all_in = 4
};

typedef struct {
    uint32_t num_players;
    uint32_t max_episode_length;
    uint32_t action_count;
    uint32_t obs_size;
    uint32_t starting_stack;
    uint32_t small_blind;
    uint32_t big_blind;
} PokerConfig;

static const PokerConfig POKER = {
    .num_players = 2,
    .max_episode_length = 1000,
    .action_count = 5,  // fold, call, check, bet_pot, all_in
    .obs_size = 30,     // Streamlined strategic features: hand strength, board texture, opponent modeling
    .starting_stack = 200,
    .small_blind = 1,
    .big_blind = 2
};

typedef struct {
    int preflop, flop, turn, river, showdown;
} GamePhase;

static const GamePhase PHASE = {
    .preflop = 0, .flop = 1, .turn = 2, .river = 3, .showdown = 4
};

typedef struct {
    int clubs, diamonds, hearts, spades;
} Suits;

static const Suits SUIT = {
    .clubs = 0, .diamonds = 1, .hearts = 2, .spades = 3
};

typedef struct {
    int two, three, four, five, six, seven, eight, nine, ten, jack, queen, king, ace;
} Ranks;

static const Ranks RANK = {
    .two = 0, .three = 1, .four = 2, .five = 3, .six = 4, .seven = 5,
    .eight = 6, .nine = 7, .ten = 8, .jack = 9, .queen = 10, .king = 11, .ace = 12
};

typedef struct {
    uint32_t simulation_count;
    float reward_weight;
} MonteCarloConfig;

static const MonteCarloConfig MC = {
    .simulation_count = 50,
    .reward_weight = 0.15f
};

#endif