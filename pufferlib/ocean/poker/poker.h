#ifndef POKER_H
#define POKER_H

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "raylib.h"
#include "constants.h"
#include "render.h"

typedef struct GameConfig {
    uint32_t starting_stack;
    uint32_t small_blind;
    uint32_t big_blind;
    float showdown_bonus;
    float fold_penalty;
} GameConfig;

static const GameConfig DEFAULT_CONFIG = {
    .starting_stack = 200,
    .small_blind = 1,
    .big_blind = 2,
    .showdown_bonus = 0.1f,
    .fold_penalty = 0.0f
};

typedef struct Card {
    uint8_t rank;
    uint8_t suit;
} Card;

typedef struct Player {
    Card hole_cards[2];
    uint32_t stack;
    uint32_t starting_stack;
    uint32_t current_bet;
    uint8_t folded;
    uint8_t all_in;
    uint8_t position;
} Player;

typedef struct Log {
    float episode_return;
    float episode_length;
    float hands_played;
    float hands_won;
    float hero_folds;
    float villain_folds;
    float showdown_wins;
    float showdown_losses;
    
    float bb_per_100;
    float action_fold;
    float action_call;
    float action_check;
    float action_bet_pot;
    float action_all_in;
    float river_decisions;
    float pot_size_won;
    float pot_size_lost;
    
    float rainbow_boards;
    float two_tone_boards;
    float broadway_boards;
    float paired_boards;
    float villain_aggression_freq;
    float pot_randomization_factor;
    
    // Hero comprehensive stats
    float hero_vpip;
    float hero_pfr;
    float hero_3bet;
    float hero_fold_to_3bet;
    float hero_cbet_flop;
    float hero_cbet_turn;
    float hero_cbet_river;
    float hero_aggression_factor;
    float hero_wtsd;
    float hero_w_at_sd;
    
    // Villain comprehensive stats
    float villain_vpip;
    float villain_pfr;
    float villain_3bet;
    float villain_fold_to_3bet;
    float villain_cbet_flop;
    float villain_cbet_turn;
    float villain_cbet_river;
    float villain_aggression_factor;
    float villain_wtsd;
    float villain_w_at_sd;
    
    // Game flow stats
    float preflop_all_in_rate;
    float avg_pot_size;
    float showdown_rate;
    float limped_pots;
    float three_bet_pots;
    
    float pbs_updates;
    float pbs_accuracy_sum;
    float opponent_range_entropy;
    float belief_prediction_errors;
    
    float generation_number;
    float opponent_generation;
    float n;
    
    float mc_reward_total;
    float mc_reward_fold;
    float mc_reward_call;
    float mc_reward_check;
    float mc_reward_bet_pot;
    float mc_reward_all_in;
    float mc_simulation_count;
} Log;

typedef struct ActionHistory {
    char actions[20][50];
    int count;
} ActionHistory;

typedef struct Poker {
    Client* client;
    Log log;
    float* observations;
    float* actions;
    float* rewards;
    unsigned char* terminals;
    
    Card deck[52];
    Card community_cards[5];
    Player players[2];
    
    uint8_t deck_position;
    uint8_t community_count;
    uint8_t current_player;
    uint8_t button;
    uint8_t phase;
    uint32_t pot;
    uint32_t current_bet;
    uint8_t betting_round_over;
    uint8_t human_mode;
    
    uint32_t rng_state;
    
    // Betting action tracking for proper round logic
    uint8_t last_action[2];        // Last action by each player (0=fold, 1=call/check, 2=bet)
    uint8_t actions_this_round;    // Number of actions in current betting round
    uint8_t consecutive_checks;    // Count of consecutive checks
    
    // Opponent action control
    int opponent_action_set;
    int opponent_action_value;
    uint8_t self_play_mode;
    float exploration_epsilon;  // Exploration rate for opponent diversity
    
    uint8_t hero_raised_preflop;
    uint8_t villain_raised_preflop;
    uint8_t saw_flop;
    
    uint8_t hero_bet_preflop;
    uint8_t villain_bet_preflop;
    uint8_t preflop_raises_count;
    
    float current_generation;
    float intended_opponent_generation;
    
    ActionHistory hand_history;
    
    GameConfig config;
    uint32_t tick;
    uint16_t episode_length;
    uint32_t max_episode_length;
    uint32_t max_hands_per_episode;
    uint16_t hand_number;
    float episode_return;
    
    uint16_t episode_showdowns;
    uint16_t episode_hands_won;
    
    uint16_t episode_hero_folds;
    uint16_t episode_villain_folds;
    uint16_t episode_showdown_wins;
    uint16_t episode_showdown_losses;
    
    uint16_t episode_action_fold;
    uint16_t episode_action_call; 
    uint16_t episode_action_check;
    uint16_t episode_action_bet_pot;
    uint16_t episode_action_all_in;
    uint16_t episode_river_decisions;
    uint16_t episode_pot_size_won;
    uint16_t episode_pot_size_lost;
    
    uint16_t episode_rainbow_boards;
    uint16_t episode_two_tone_boards;
    uint16_t episode_broadway_boards;
    uint16_t episode_paired_boards;
    uint16_t villain_aggressive_actions;
    uint16_t villain_total_actions;
    
    float episode_mc_reward_total;
    float episode_mc_reward_fold;
    float episode_mc_reward_call;
    float episode_mc_reward_check;
    float episode_mc_reward_bet_pot;
    float episode_mc_reward_all_in;
    uint32_t episode_mc_simulation_count;
    
    // Hero stats tracking
    uint16_t hero_hands_vpip;
    uint16_t hero_hands_pfr;
    uint8_t hero_vpip_this_hand;
    uint8_t hero_pfr_this_hand;
    uint16_t hero_opportunities_3bet;
    uint16_t hero_actual_3bet;
    uint16_t hero_opportunities_fold_to_3bet;
    uint16_t hero_actual_fold_to_3bet;
    uint16_t hero_opportunities_cbet_flop;
    uint16_t hero_actual_cbet_flop;
    uint16_t hero_opportunities_cbet_turn;
    uint16_t hero_actual_cbet_turn;
    uint16_t hero_opportunities_cbet_river;
    uint16_t hero_actual_cbet_river;
    uint16_t hero_aggressive_postflop_actions;
    uint16_t hero_total_postflop_actions;
    uint16_t hero_went_to_showdown;
    uint16_t hero_won_at_showdown;
    
    // Villain stats tracking
    uint16_t villain_hands_vpip;
    uint16_t villain_hands_pfr;
    uint8_t villain_vpip_this_hand;
    uint8_t villain_pfr_this_hand;
    uint16_t villain_opportunities_3bet;
    uint16_t villain_actual_3bet;
    uint16_t villain_opportunities_fold_to_3bet;
    uint16_t villain_actual_fold_to_3bet;
    uint16_t villain_opportunities_cbet_flop;
    uint16_t villain_actual_cbet_flop;
    uint16_t villain_opportunities_cbet_turn;
    uint16_t villain_actual_cbet_turn;
    uint16_t villain_opportunities_cbet_river;
    uint16_t villain_actual_cbet_river;
    uint16_t villain_aggressive_postflop_actions;
    uint16_t villain_total_postflop_actions;
    uint16_t villain_went_to_showdown;
    uint16_t villain_won_at_showdown;
    
    // Game flow tracking
    uint16_t preflop_all_ins;
    uint16_t total_pot_size;
    uint16_t limped_hands;
    uint16_t three_bet_hands;
    
    float cached_opponent_strength;  // Cached PBS values for performance
    float cached_range_width;
    float belief_update_count;
    
    // Opponent tracking of hero's tendencies
    uint16_t hero_total_cbets;
    uint16_t hero_cbet_opportunities;
    uint16_t hero_total_bets;
    uint16_t hero_total_decisions;
    float total_belief_entropy;
    float belief_accuracy_checks;
    
    // Cache hand evaluations to avoid repeated computation
    uint32_t cached_hero_hand_rank;
    uint32_t cached_villain_hand_rank;
    uint8_t hand_cache_valid;
    
    // Cache board texture analysis to avoid repeated computation
    uint8_t board_pairs;
    uint8_t max_suit_count;
    uint8_t broadway_cards;
    uint8_t ace_high_board;
    uint8_t board_cache_valid;
    
    uint32_t base_starting_stack;
    float pot_randomization_factor;
    
    float width;
    float height;
    int headless;
} Poker;

static inline uint8_t card_to_index(Card card) {
    return card.rank * 4 + card.suit;
}

static inline Card index_to_card(uint8_t index) {
    Card card;
    card.rank = index / 4;
    card.suit = index % 4;
    return card;
}

void execute_opponent_action(Poker* env, int action);

static inline void compute_board_texture(Poker* env) {
    if (env->board_cache_valid) return;
    
    uint32_t ranks[13] = {0};
    uint32_t suits[4] = {0};
    int broadway_count = 0;
    
    // Process only the community cards that exist
    for (int i = 0; i < env->community_count; i++) {
        Card card = env->community_cards[i];
        ranks[card.rank]++;
        suits[card.suit]++;
        if (card.rank >= 8) broadway_count++;  // T, J, Q, K, A
    }
    
    int pairs = 0;
    for (int i = 0; i < 13; i++) {
        if (ranks[i] >= 2) pairs++;
    }
    
    int max_suit = suits[0];
    if (suits[1] > max_suit) max_suit = suits[1];
    if (suits[2] > max_suit) max_suit = suits[2];
    if (suits[3] > max_suit) max_suit = suits[3];
    
    env->board_pairs = pairs;
    env->max_suit_count = max_suit;
    env->broadway_cards = broadway_count;
    env->ace_high_board = (ranks[12] > 0) ? 1 : 0;
    env->board_cache_valid = 1;
}

void init_deck(Poker* env) {
    for (int i = 0; i < 52; i++) {
        env->deck[i].rank = i / 4;
        env->deck[i].suit = i % 4;
    }
}

static inline uint32_t fast_rand(Poker* env) {
    env->rng_state ^= env->rng_state << 13;
    env->rng_state ^= env->rng_state >> 17;
    env->rng_state ^= env->rng_state << 5;
    return env->rng_state;
}

static inline uint32_t fast_hand_eval(Card hole_cards[2], Card community_cards[5]) {
    uint32_t rank_counts[13] = {0};
    uint32_t suit_counts[4] = {0};
    
    // Count ranks and suits from all 7 cards
    for (int i = 0; i < 2; i++) {
        rank_counts[hole_cards[i].rank]++;
        suit_counts[hole_cards[i].suit]++;
    }
    for (int i = 0; i < 5; i++) {
        rank_counts[community_cards[i].rank]++;
        suit_counts[community_cards[i].suit]++;
    }
    
    // Check for flush (5+ of same suit)
    int is_flush = 0;
    for (int i = 0; i < 4; i++) {
        if (suit_counts[i] >= 5) {
            is_flush = 1;
            break;
        }
    }
    
    // Count pairs, trips, quads
    int pairs = 0, trips = 0, quads = 0;
    for (int i = 0; i < 13; i++) {
        if (rank_counts[i] == 2) pairs++;
        else if (rank_counts[i] == 3) trips++;
        else if (rank_counts[i] >= 4) quads++;
    }
    
    // Quick hand ranking (approximate)
    if (quads > 0) return 7;              // Four of a kind
    if (trips > 0 && pairs > 0) return 6; // Full house
    if (is_flush) return 5;               // Flush
    if (trips > 0) return 3;              // Three of a kind
    if (pairs >= 2) return 2;             // Two pair
    if (pairs == 1) return 1;             // One pair
    return 0;                             // High card
}

void shuffle_deck(Poker* env) {
    for (int i = 51; i > 0; i--) {
        int j = fast_rand(env) % (i + 1);
        Card temp = env->deck[i];
        env->deck[i] = env->deck[j];
        env->deck[j] = temp;
    }
    env->deck_position = 0;
}

Card deal_card(Poker* env) {
    return env->deck[env->deck_position++];
}

uint32_t hand_rank(Card cards[7], int num_cards);

uint32_t evaluate_hand(Card hole_cards[2], Card community_cards[5], int hole_count, int community_count) {
    Card all_cards[7];
    for (int i = 0; i < hole_count; i++) {
        all_cards[i] = hole_cards[i];
    }
    for (int i = 0; i < community_count; i++) {
        all_cards[hole_count + i] = community_cards[i];
    }
    return hand_rank(all_cards, hole_count + community_count);
}

uint32_t hand_rank(Card cards[7], int num_cards) {
    uint32_t ranks[13] = {0};
    uint32_t suits[4] = {0};
    
    for (int i = 0; i < num_cards; i++) {
        ranks[cards[i].rank]++;
        suits[cards[i].suit]++;
    }
    
    uint8_t pairs = 0;
    uint8_t three_kind = 0;
    uint8_t four_kind = 0;
    uint8_t straight = 0;
    uint8_t flush = 0;
    
    for (int i = 0; i < 13; i++) {
        if (ranks[i] == 2) pairs++;
        if (ranks[i] == 3) three_kind = 1;
        if (ranks[i] == 4) four_kind = 1;
    }
    
    for (int i = 0; i < 4; i++) {
        if (suits[i] >= 5) flush = 1;
    }
    
    for (int i = 0; i <= 8; i++) {
        if (ranks[i] && ranks[i+1] && ranks[i+2] && ranks[i+3] && ranks[i+4]) {
            straight = 1;
            break;
        }
    }
    
    if (ranks[12] && ranks[0] && ranks[1] && ranks[2] && ranks[3]) straight = 1;
    
    if (straight && flush) return 8;
    if (four_kind) return 7;
    if (three_kind && pairs) return 6;
    if (flush) return 5;
    if (straight) return 4;
    if (three_kind) return 3;
    if (pairs == 2) return 2;
    if (pairs == 1) return 1;
    return 0;
}

uint8_t compare_hands(Poker* env) {
    // Use cached hand evaluations if available
    if (!env->hand_cache_valid) {
        env->cached_hero_hand_rank = fast_hand_eval(env->players[0].hole_cards, env->community_cards);
        env->cached_villain_hand_rank = fast_hand_eval(env->players[1].hole_cards, env->community_cards);
        env->hand_cache_valid = 1;
    }
    
    uint32_t rank1 = env->cached_hero_hand_rank;
    uint32_t rank2 = env->cached_villain_hand_rank;
    
    if (rank1 > rank2) return 0;
    if (rank2 > rank1) return 1;
    return 2;
}

void init(Poker* env) {
    env->tick = 0;
    env->config = DEFAULT_CONFIG;
    env->width = 1000.0f;
    env->height = 750.0f;
    env->human_mode = 0;
    env->self_play_mode = 0;
    env->headless = 0;  // Default to human/render mode for evaluation
    env->exploration_epsilon = 0.15f;  // 15% exploration for diversity
    env->opponent_action_set = 0;
    env->opponent_action_value = -1;
}

void allocate(Poker* env) {
    env->tick = 0;
    env->episode_return = 0.0f;
    env->hand_number = 0;
    env->client = NULL;
    env->base_starting_stack = 200;
    env->pot_randomization_factor = 1.0f;
    env->villain_aggressive_actions = 0;
    env->villain_total_actions = 0;
    env->rng_state = (uint32_t)time(NULL) + (uintptr_t)env;
    memset(&env->log, 0, sizeof(Log));
    init_deck(env);
    init(env);
}

void c_close(Poker* env) {
    
}

void free_allocated(Poker* env) {
    free(env->actions);
    free(env->observations);
    free(env->terminals);
    free(env->rewards);
    c_close(env);
}

void update_opponent_beliefs(Poker* env) {
    if (env->villain_total_actions > 0) {
        float aggression_factor = (float)env->villain_aggressive_actions / (float)env->villain_total_actions;
        
        env->cached_opponent_strength = 0.35f + (aggression_factor * 0.35f);
        env->cached_range_width = 0.9f - (aggression_factor * 0.4f);
        
        env->belief_update_count++;
        env->total_belief_entropy += env->cached_range_width;
    } else {
        env->cached_opponent_strength = 0.5f;
        env->cached_range_width = 0.8f;
    }
}

float get_opponent_hand_strength_expectation(Poker* env) {
    return env->cached_opponent_strength;
}

float get_opponent_range_width(Poker* env) {
    return env->cached_range_width;
}

void compute_observations(Poker* env) {
    int obs_idx = 0;
    float max_stack = env->base_starting_stack;
    
    // Update opponent beliefs first (simplified for performance)
    update_opponent_beliefs(env);
    
    // Strategic observations (30 features total - with PBS)
    
    // === HAND STRENGTH ANALYSIS (5 features) === OPTIMIZED WITH CACHING
    uint32_t hand_rank = 0;
    if (env->community_count >= 3) {
        if (!env->hand_cache_valid) {
            env->cached_hero_hand_rank = fast_hand_eval(env->players[0].hole_cards, env->community_cards);
            env->cached_villain_hand_rank = fast_hand_eval(env->players[1].hole_cards, env->community_cards);
            env->hand_cache_valid = 1;
        }
        hand_rank = env->cached_hero_hand_rank;
    } else {
        // Preflop: use basic hole card strength
        Card c1 = env->players[0].hole_cards[0];
        Card c2 = env->players[0].hole_cards[1];
        if (c1.rank == c2.rank) hand_rank = 1 + c1.rank / 4;  // Pocket pairs
        else if (c1.suit == c2.suit) hand_rank = 1;  // Suited
        else hand_rank = 0;  // Offsuit
    }
    env->observations[obs_idx++] = hand_rank / 8.0f;  // Normalized hand rank (0-1)
    
    // Hand categories for strategic decisions
    env->observations[obs_idx++] = (hand_rank >= 6) ? 1.0f : 0.0f;  // Monster hands (full house+)
    env->observations[obs_idx++] = (hand_rank >= 4 && hand_rank <= 5) ? 1.0f : 0.0f;  // Strong hands (straight/flush)
    env->observations[obs_idx++] = (hand_rank >= 2 && hand_rank <= 3) ? 1.0f : 0.0f;  // Medium hands (pair/two pair/trips)
    env->observations[obs_idx++] = (hand_rank <= 1) ? 1.0f : 0.0f;  // Weak hands (high card/pair)
    
    // === PUBLIC BELIEF STATE FEATURES (6 features) ===
    float opponent_expected_strength = get_opponent_hand_strength_expectation(env);
    float opponent_range_width = get_opponent_range_width(env);
    float my_strength = hand_rank / 8.0f;
    
    env->observations[obs_idx++] = opponent_expected_strength;  // Expected opponent hand strength
    env->observations[obs_idx++] = opponent_range_width;       // How wide opponent's range is
    env->observations[obs_idx++] = my_strength - opponent_expected_strength;  // Relative hand strength
    env->observations[obs_idx++] = (my_strength > opponent_expected_strength) ? 1.0f : 0.0f;  // Ahead in equity
    env->observations[obs_idx++] = (opponent_range_width < 0.3f) ? 1.0f : 0.0f;  // Opponent has narrow range
    env->observations[obs_idx++] = (opponent_expected_strength > 0.6f) ? 1.0f : 0.0f;  // Opponent likely strong
    
    // === BOARD TEXTURE ANALYSIS (6 features) === OPTIMIZED WITH CACHING
    if (env->community_count >= 3) {
        compute_board_texture(env);
        env->observations[obs_idx++] = (env->board_pairs > 0) ? 1.0f : 0.0f;  // Paired board
        env->observations[obs_idx++] = (env->max_suit_count <= 2) ? 1.0f : 0.0f;  // Rainbow
        env->observations[obs_idx++] = (env->max_suit_count == 3) ? 1.0f : 0.0f;  // Two-tone
        env->observations[obs_idx++] = (env->max_suit_count >= 4) ? 1.0f : 0.0f;  // Flush possible
        env->observations[obs_idx++] = (env->broadway_cards >= 3) ? 1.0f : 0.0f;  // Broadway heavy
        env->observations[obs_idx++] = (env->ace_high_board) ? 1.0f : 0.0f;  // Ace high board
    } else {
        // Preflop: no board texture features
        env->observations[obs_idx++] = 0.0f;  // No pairs
        env->observations[obs_idx++] = 0.0f;  // No rainbow
        env->observations[obs_idx++] = 0.0f;  // No two-tone
        env->observations[obs_idx++] = 0.0f;  // No flush draws
        env->observations[obs_idx++] = 0.0f;  // No broadway
        env->observations[obs_idx++] = 0.0f;  // No ace
    }
    
    // === POT AND BETTING DYNAMICS (5 features) ===
    env->observations[obs_idx++] = env->pot / (float)(2 * max_stack);
    
    uint32_t call_amount = env->current_bet - env->players[0].current_bet;
    env->observations[obs_idx++] = call_amount / (float)max_stack;
    
    float pot_odds = (call_amount > 0) ? call_amount / (float)(env->pot + call_amount) : 0.0f;
    env->observations[obs_idx++] = pot_odds;
    
    float bet_to_pot = (env->pot > 0) ? env->current_bet / (float)env->pot : 0.0f;
    env->observations[obs_idx++] = (bet_to_pot > 2.0f) ? 2.0f : bet_to_pot;
    
    float hero_investment = max_stack - env->players[0].stack;
    env->observations[obs_idx++] = hero_investment / (float)max_stack;
    
    // === STACK DYNAMICS (4 features) ===
    env->observations[obs_idx++] = env->players[0].stack / (float)max_stack;
    env->observations[obs_idx++] = env->players[1].stack / (float)max_stack;
    
    uint32_t effective_stack = (env->players[0].stack < env->players[1].stack) ? env->players[0].stack : env->players[1].stack;
    env->observations[obs_idx++] = effective_stack / (float)max_stack;
    
    float stack_ratio = (env->players[1].stack > 0) ? (float)env->players[0].stack / (float)env->players[1].stack : 1.0f;
    env->observations[obs_idx++] = (stack_ratio > 2.0f) ? 2.0f : stack_ratio;
    
    // === GAME STATE AND POSITION (5 features) ===
    env->observations[obs_idx++] = (env->players[0].position == 0) ? 1.0f : 0.0f;  // Hero on button
    env->observations[obs_idx++] = (env->current_player == 0) ? 1.0f : 0.0f;      // Hero to act
    env->observations[obs_idx++] = (call_amount == 0) ? 1.0f : 0.0f;              // Can check
    env->observations[obs_idx++] = (env->current_bet > env->config.big_blind * 3) ? 1.0f : 0.0f;  // Facing big bet
    env->observations[obs_idx++] = env->phase / 4.0f;  // Game phase (0=preflop, 0.25=flop, 0.5=turn, 0.75=river, 1.0=showdown)
}

static void add_log(Poker* env) {
    env->log.episode_return += env->episode_return;
    env->log.episode_length += env->episode_length;
    env->log.hands_played += env->hand_number;
    env->log.hands_won += env->episode_hands_won;
    env->log.hero_folds += env->episode_hero_folds;
    env->log.villain_folds += env->episode_villain_folds;
    env->log.showdown_wins += env->episode_showdown_wins;
    env->log.showdown_losses += env->episode_showdown_losses;
    
    env->log.action_fold += env->episode_action_fold;
    env->log.action_call += env->episode_action_call;
    env->log.action_check += env->episode_action_check;
    env->log.action_bet_pot += env->episode_action_bet_pot;
    env->log.action_all_in += env->episode_action_all_in;
    env->log.river_decisions += env->episode_river_decisions;
    env->log.pot_size_won += env->episode_pot_size_won;
    env->log.pot_size_lost += env->episode_pot_size_lost;
    
    env->log.rainbow_boards += env->episode_rainbow_boards;
    env->log.two_tone_boards += env->episode_two_tone_boards;
    env->log.broadway_boards += env->episode_broadway_boards;
    env->log.paired_boards += env->episode_paired_boards;
    env->log.villain_aggression_freq = (env->villain_total_actions > 0) ? 
        (float)env->villain_aggressive_actions / (float)env->villain_total_actions : 0.5f;
    env->log.pot_randomization_factor = env->pot_randomization_factor;
    
    env->log.mc_reward_total += env->episode_mc_reward_total;
    env->log.mc_reward_fold += env->episode_mc_reward_fold;
    env->log.mc_reward_call += env->episode_mc_reward_call;
    env->log.mc_reward_check += env->episode_mc_reward_check;
    env->log.mc_reward_bet_pot += env->episode_mc_reward_bet_pot;
    env->log.mc_reward_all_in += env->episode_mc_reward_all_in;
    env->log.mc_simulation_count += env->episode_mc_simulation_count;
    
    // Calculate and log comprehensive stats
    float hands_played_float = (float)env->hand_number;
    
    // Hero stats
    env->log.hero_vpip = (hands_played_float > 0) ? (env->hero_hands_vpip / hands_played_float) * 100.0f : 0.0f;
    env->log.hero_pfr = (hands_played_float > 0) ? (env->hero_hands_pfr / hands_played_float) * 100.0f : 0.0f;
    env->log.hero_3bet = (env->hero_opportunities_3bet > 0) ? (env->hero_actual_3bet / (float)env->hero_opportunities_3bet) * 100.0f : 0.0f;
    env->log.hero_fold_to_3bet = (env->hero_opportunities_fold_to_3bet > 0) ? (env->hero_actual_fold_to_3bet / (float)env->hero_opportunities_fold_to_3bet) * 100.0f : 0.0f;
    env->log.hero_cbet_flop = (env->hero_opportunities_cbet_flop > 0) ? (env->hero_actual_cbet_flop / (float)env->hero_opportunities_cbet_flop) * 100.0f : 0.0f;
    env->log.hero_cbet_turn = (env->hero_opportunities_cbet_turn > 0) ? (env->hero_actual_cbet_turn / (float)env->hero_opportunities_cbet_turn) * 100.0f : 0.0f;
    env->log.hero_cbet_river = (env->hero_opportunities_cbet_river > 0) ? (env->hero_actual_cbet_river / (float)env->hero_opportunities_cbet_river) * 100.0f : 0.0f;
    env->log.hero_aggression_factor = (env->hero_total_postflop_actions > 0) ? env->hero_aggressive_postflop_actions / (float)(env->hero_total_postflop_actions - env->hero_aggressive_postflop_actions + 1) : 0.0f;
    env->log.hero_wtsd = (hands_played_float > 0) ? (env->hero_went_to_showdown / hands_played_float) * 100.0f : 0.0f;
    env->log.hero_w_at_sd = (env->hero_went_to_showdown > 0) ? (env->hero_won_at_showdown / (float)env->hero_went_to_showdown) * 100.0f : 0.0f;
    
    // Villain stats
    env->log.villain_vpip = (hands_played_float > 0) ? (env->villain_hands_vpip / hands_played_float) * 100.0f : 0.0f;
    env->log.villain_pfr = (hands_played_float > 0) ? (env->villain_hands_pfr / hands_played_float) * 100.0f : 0.0f;
    env->log.villain_3bet = (env->villain_opportunities_3bet > 0) ? (env->villain_actual_3bet / (float)env->villain_opportunities_3bet) * 100.0f : 0.0f;
    env->log.villain_fold_to_3bet = (env->villain_opportunities_fold_to_3bet > 0) ? (env->villain_actual_fold_to_3bet / (float)env->villain_opportunities_fold_to_3bet) * 100.0f : 0.0f;
    env->log.villain_cbet_flop = (env->villain_opportunities_cbet_flop > 0) ? (env->villain_actual_cbet_flop / (float)env->villain_opportunities_cbet_flop) * 100.0f : 0.0f;
    env->log.villain_cbet_turn = (env->villain_opportunities_cbet_turn > 0) ? (env->villain_actual_cbet_turn / (float)env->villain_opportunities_cbet_turn) * 100.0f : 0.0f;
    env->log.villain_cbet_river = (env->villain_opportunities_cbet_river > 0) ? (env->villain_actual_cbet_river / (float)env->villain_opportunities_cbet_river) * 100.0f : 0.0f;
    env->log.villain_aggression_factor = (env->villain_total_postflop_actions > 0) ? env->villain_aggressive_postflop_actions / (float)(env->villain_total_postflop_actions - env->villain_aggressive_postflop_actions + 1) : 0.0f;
    env->log.villain_wtsd = (hands_played_float > 0) ? (env->villain_went_to_showdown / hands_played_float) * 100.0f : 0.0f;
    env->log.villain_w_at_sd = (env->villain_went_to_showdown > 0) ? (env->villain_won_at_showdown / (float)env->villain_went_to_showdown) * 100.0f : 0.0f;
    
    // Game flow stats
    env->log.preflop_all_in_rate = (hands_played_float > 0) ? (env->preflop_all_ins / hands_played_float) * 100.0f : 0.0f;
    env->log.avg_pot_size = (hands_played_float > 0) ? env->total_pot_size / hands_played_float : 0.0f;
    env->log.showdown_rate = (hands_played_float > 0) ? (env->episode_showdowns / hands_played_float) * 100.0f : 0.0f;
    env->log.limped_pots = (hands_played_float > 0) ? (env->limped_hands / hands_played_float) * 100.0f : 0.0f;
    env->log.three_bet_pots = (hands_played_float > 0) ? (env->three_bet_hands / hands_played_float) * 100.0f : 0.0f;
    
    // PBS-related statistics
    env->log.pbs_updates = env->belief_update_count;
    env->log.pbs_accuracy_sum = env->belief_accuracy_checks;
    env->log.opponent_range_entropy = (env->belief_update_count > 0) ? 
        env->total_belief_entropy / env->belief_update_count : 0.0f;
    env->log.belief_prediction_errors = env->belief_accuracy_checks;  // For now, same as accuracy checks
    
    if (env->log.hands_played > 0) {
        env->log.bb_per_100 = (env->log.episode_return / (float)env->config.big_blind) * 100.0f / env->log.hands_played;
    }
    
    env->log.generation_number = env->current_generation;
    env->log.opponent_generation = env->intended_opponent_generation;
    env->log.n += 1;
}

void add_action_to_history(Poker* env, const char* action) {
    if (!env->headless && env->hand_history.count < 20) {
        strcpy(env->hand_history.actions[env->hand_history.count], action);
        env->hand_history.count++;
    }
}

uint32_t get_randomized_stack(Poker* env) {
    uint32_t base_stack = env->base_starting_stack;
    float randomization = 0.5f + (fast_rand(env) % 100) / 100.0f;
    uint32_t randomized_stack = (uint32_t)(base_stack * randomization);
    
    uint32_t min_stack = env->config.big_blind * 50;
    uint32_t max_stack = env->config.big_blind * 300;
    if (randomized_stack < min_stack) randomized_stack = min_stack;
    if (randomized_stack > max_stack) randomized_stack = max_stack;
    
    return randomized_stack;
}

float get_pot_randomization_factor(Poker* env) {
    return 0.8f + (fast_rand(env) % 70) / 100.0f;
}

void start_new_hand(Poker* env) {
    env->hand_number++;
    
    // Check if episode should end after completing this many hands
    if (env->hand_number >= env->max_hands_per_episode) {
        env->terminals[0] = 1;
        add_log(env);
        c_reset(env);
        return;
    }
    env->pot = 0;
    env->current_bet = 0;
    env->community_count = 0;
    env->phase = PHASE.preflop;  // Start at preflop for full poker
    env->betting_round_over = 0;
    
    // Invalidate caches since cards will change
    env->hand_cache_valid = 0;
    env->board_cache_valid = 0;
    
    env->hand_history.count = 0;
    
    env->hero_raised_preflop = 0;
    env->villain_raised_preflop = 0;
    env->saw_flop = 0;
    
    env->hero_bet_preflop = 0;
    env->villain_bet_preflop = 0;
    env->preflop_raises_count = 0;
    
    // Reset per-hand tracking flags
    env->hero_vpip_this_hand = 0;
    env->hero_pfr_this_hand = 0;
    env->villain_vpip_this_hand = 0;
    env->villain_pfr_this_hand = 0;
    
    // Randomize stack sizes for training variation
    uint32_t randomized_stack = get_randomized_stack(env);
    env->pot_randomization_factor = get_pot_randomization_factor(env);
    
    for (int i = 0; i < 2; i++) {
        env->players[i].current_bet = 0;
        env->players[i].folded = 0;
        env->players[i].all_in = 0;
        env->players[i].stack = randomized_stack;
        env->players[i].starting_stack = randomized_stack;
    }
    
    env->button = (env->button + 1) % 2;
    env->players[0].position = env->button;
    env->players[1].position = 1 - env->button;
    
    // Reset belief tracking for new hand
    env->cached_opponent_strength = 0.5f;  // Neutral prior
    env->cached_range_width = 0.8f;        // Wide range prior
    env->belief_update_count = 0;
    
    // Reset betting round tracking for new hand
    env->actions_this_round = 0;
    env->consecutive_checks = 0;
    env->last_action[0] = 0;
    env->last_action[1] = 0;
    
    
    shuffle_deck(env);
    
    for (int i = 0; i < 2; i++) {
        env->players[0].hole_cards[i] = deal_card(env);
        env->players[1].hole_cards[i] = deal_card(env);
    }
    
    // No community cards dealt at preflop
    env->community_count = 0;
    add_action_to_history(env, "--- PREFLOP ---");
    
    // Board texture analysis only applies when community cards exist
    if (env->community_count >= 3) {
        compute_board_texture(env);
        
        if (env->max_suit_count <= 2) env->episode_rainbow_boards++;
        if (env->max_suit_count == 3) env->episode_two_tone_boards++;
        if (env->broadway_cards >= 3) env->episode_broadway_boards++;
        if (env->board_pairs > 0) env->episode_paired_boards++;
    }
    
    // Apply pot randomization to blinds
    uint32_t small_blind_amt = (uint32_t)(env->config.small_blind * env->pot_randomization_factor);
    uint32_t big_blind_amt = (uint32_t)(env->config.big_blind * env->pot_randomization_factor);
    
    if (env->button == 0) {
        env->players[0].current_bet = small_blind_amt;
        env->players[1].current_bet = big_blind_amt;
        if (!env->headless) {
            char sb_msg[50], bb_msg[50];
            snprintf(sb_msg, sizeof(sb_msg), "Hero SB $%d", small_blind_amt);
            snprintf(bb_msg, sizeof(bb_msg), "Villain BB $%d", big_blind_amt);
            add_action_to_history(env, sb_msg);
            add_action_to_history(env, bb_msg);
        }
    } else {
        env->players[1].current_bet = small_blind_amt;
        env->players[0].current_bet = big_blind_amt;
        if (!env->headless) {
            char sb_msg[50], bb_msg[50];
            snprintf(sb_msg, sizeof(sb_msg), "Villain SB $%d", small_blind_amt);
            snprintf(bb_msg, sizeof(bb_msg), "Hero BB $%d", big_blind_amt);
            add_action_to_history(env, sb_msg);
            add_action_to_history(env, bb_msg);
        }
    }
    
    env->pot = small_blind_amt + big_blind_amt;
    env->current_bet = big_blind_amt;
    env->current_player = env->button;  // First to act preflop is button (SB)
    
    for (int i = 0; i < 2; i++) {
        env->players[i].stack -= env->players[i].current_bet;
        if (env->players[i].stack == 0) {
            env->players[i].all_in = 1;
        }
    }
    
    compute_observations(env);
}

void advance_phase(Poker* env) {
    env->phase++;
    env->current_bet = 0;
    env->betting_round_over = 0;
    
    // Reset betting round tracking
    env->actions_this_round = 0;
    env->consecutive_checks = 0;
    env->last_action[0] = 0;
    env->last_action[1] = 0;
    
    for (int i = 0; i < 2; i++) {
        env->players[i].current_bet = 0;
    }
    
    // CRITICAL: Set who acts first on each street
    // Preflop: SB (button) acts first
    // Postflop: BB (1 - button) acts first 
    env->current_player = 1 - env->button;
    
    if (env->phase == PHASE.flop) {
        for (int i = 0; i < 3; i++) {
            env->community_cards[i] = deal_card(env);
        }
        env->community_count = 3;
        add_action_to_history(env, "--- FLOP ---");
        env->saw_flop = 1;
        
        // Invalidate cache and track board texture when flop is dealt
        env->board_cache_valid = 0;
        compute_board_texture(env);
        if (env->max_suit_count <= 2) env->episode_rainbow_boards++;
        if (env->max_suit_count == 3) env->episode_two_tone_boards++;
        if (env->broadway_cards >= 3) env->episode_broadway_boards++;
        if (env->board_pairs > 0) env->episode_paired_boards++;
    } else if (env->phase == PHASE.turn) {
        env->community_cards[3] = deal_card(env);
        env->community_count = 4;
        add_action_to_history(env, "--- TURN ---");
        env->board_cache_valid = 0;  // Invalidate cache for new card
    } else if (env->phase == PHASE.river) {
        env->community_cards[4] = deal_card(env);
        env->community_count = 5;
        add_action_to_history(env, "--- RIVER ---");
        env->board_cache_valid = 0;  // Invalidate cache for new card
    } else if (env->phase == PHASE.showdown) {
        uint8_t winner = compare_hands(env);
        float hero_investment = (float)env->players[0].starting_stack - (float)env->players[0].stack;
        float net_profit = 0.0f;
        
        // Track showdown stats
        env->hero_went_to_showdown++;
        env->villain_went_to_showdown++;
        env->total_pot_size += env->pot;
        
        if (winner == 0) {
            env->players[0].stack += env->pot;
            net_profit = env->pot - hero_investment;
            
            env->rewards[0] += net_profit;
            env->episode_return += net_profit;
            env->episode_hands_won += 1;
            env->episode_showdown_wins += 1;
            env->episode_pot_size_won += env->pot;
            env->hero_won_at_showdown++;
            
            add_action_to_history(env, "Hero wins pot");
        } else if (winner == 1) {
            env->players[1].stack += env->pot;
            net_profit = -hero_investment;
            
            env->rewards[0] += net_profit;
            env->episode_return += net_profit;
            env->episode_showdown_losses += 1;
            env->episode_pot_size_lost += env->pot;
            env->villain_won_at_showdown++;
            
            add_action_to_history(env, "Villain wins pot");
        } else {
            uint32_t split_pot = env->pot / 2;
            env->players[0].stack += split_pot;
            env->players[1].stack += split_pot;
            add_action_to_history(env, "Split pot");
        }
        
        env->episode_showdowns += 1;
        
        if (env->saw_flop) {
        }
        
        start_new_hand(env);
        return;
    }
    
    // First to act post-flop is always left of button (big blind)
    env->current_player = 1 - env->button;
}

void track_comprehensive_stats(Poker* env, int player, int action, int is_preflop, int is_cbet_spot) {
    // Track VPIP (Voluntarily Put $ In Pot) - once per hand
    if (is_preflop && (action == ACTIONS.call || action == ACTIONS.bet_pot || action == ACTIONS.all_in)) {
        if (player == 0 && !env->hero_vpip_this_hand) {
            env->hero_hands_vpip++;
            env->hero_vpip_this_hand = 1;
        } else if (player == 1 && !env->villain_vpip_this_hand) {
            env->villain_hands_vpip++;
            env->villain_vpip_this_hand = 1;
        }
    }
    
    // Track PFR (Preflop Raise) - once per hand
    if (is_preflop && (action == ACTIONS.bet_pot || action == ACTIONS.all_in)) {
        if (player == 0 && !env->hero_pfr_this_hand) {
            env->hero_hands_pfr++;
            env->hero_pfr_this_hand = 1;
        } else if (player == 1 && !env->villain_pfr_this_hand) {
            env->villain_hands_pfr++;
            env->villain_pfr_this_hand = 1;
        }
    }
    
    // Track continuation betting
    if (is_cbet_spot) {
        if (player == 0) {
            if (env->phase == PHASE.flop) {
                env->hero_opportunities_cbet_flop++;
                if (action == ACTIONS.bet_pot || action == ACTIONS.all_in) {
                    env->hero_actual_cbet_flop++;
                }
            } else if (env->phase == PHASE.turn) {
                env->hero_opportunities_cbet_turn++;
                if (action == ACTIONS.bet_pot || action == ACTIONS.all_in) {
                    env->hero_actual_cbet_turn++;
                }
            } else if (env->phase == PHASE.river) {
                env->hero_opportunities_cbet_river++;
                if (action == ACTIONS.bet_pot || action == ACTIONS.all_in) {
                    env->hero_actual_cbet_river++;
                }
            }
        } else {
            if (env->phase == PHASE.flop) {
                env->villain_opportunities_cbet_flop++;
                if (action == ACTIONS.bet_pot || action == ACTIONS.all_in) {
                    env->villain_actual_cbet_flop++;
                }
            } else if (env->phase == PHASE.turn) {
                env->villain_opportunities_cbet_turn++;
                if (action == ACTIONS.bet_pot || action == ACTIONS.all_in) {
                    env->villain_actual_cbet_turn++;
                }
            } else if (env->phase == PHASE.river) {
                env->villain_opportunities_cbet_river++;
                if (action == ACTIONS.bet_pot || action == ACTIONS.all_in) {
                    env->villain_actual_cbet_river++;
                }
            }
        }
    }
    
    // Track postflop aggression
    if (!is_preflop) {
        if (player == 0) {
            env->hero_total_postflop_actions++;
            if (action == ACTIONS.bet_pot || action == ACTIONS.all_in) {
                env->hero_aggressive_postflop_actions++;
            }
        } else {
            env->villain_total_postflop_actions++;
            if (action == ACTIONS.bet_pot || action == ACTIONS.all_in) {
                env->villain_aggressive_postflop_actions++;
            }
        }
    }
}

void track_betting_action(Poker* env, int player, int action_type, int is_check) {
    // Track action types: 0=fold, 1=call/check, 2=bet
    env->last_action[player] = action_type;
    env->actions_this_round++;
    
    if (is_check) {
        env->consecutive_checks++;
    } else {
        env->consecutive_checks = 0;  // Reset if not a check
    }
}

uint8_t is_betting_round_over(Poker* env) {
    // Round over if someone folded
    if (env->players[0].folded || env->players[1].folded) return 1;
    
    // Round over only if BOTH players have acted AND bets are equal
    if (env->actions_this_round >= 2 && 
        env->players[0].current_bet == env->players[1].current_bet) return 1;
    
    // Round continues - need more actions or bets aren't equal
    return 0;
}



void opponent_action(Poker* env) {
    if (env->opponent_action_set) {
        // Use Python opponent model action
        int action = env->opponent_action_value;
        env->opponent_action_set = 0;  // Reset flag
        execute_opponent_action(env, action);
        
    } else {
        // Generation 1 self-play: Opponent action will be set by Python using same model
        // If we reach here, it means Python didn't set an opponent action
        // This should only happen in very early training before model is available
        
        uint32_t call_amount = env->current_bet - env->players[1].current_bet;
        
        // Improved policy for generation 1 self-play bootstrap
        // This should be replaced by true neural network inference ASAP
        
        // Calculate hand strength
        if (!env->hand_cache_valid) {
            env->cached_hero_hand_rank = fast_hand_eval(env->players[0].hole_cards, env->community_cards);
            env->cached_villain_hand_rank = fast_hand_eval(env->players[1].hole_cards, env->community_cards);
            env->hand_cache_valid = 1;
        }
        
        float hand_strength = env->cached_villain_hand_rank / 8.0f;
        uint32_t random_val = fast_rand(env) % 100;
        int action;
        
        // More aggressive policy to match neural network behavior
        if (call_amount == 0) {
            // No bet to call - bet with good hands or bluff occasionally
            if (hand_strength > 0.5f || random_val < 25) {
                action = ACTIONS.bet_pot;
            } else {
                action = ACTIONS.check;
            }
        } else {
            // Facing a bet - be more aggressive like neural network
            if (hand_strength > 0.6f) {
                action = ACTIONS.call;  // Call with strong hands
            } else if (hand_strength > 0.2f && random_val < 40) {
                action = ACTIONS.call;  // Sometimes call with medium hands  
            } else {
                action = ACTIONS.fold;  // Fold weak hands
            }
        }
        
        execute_opponent_action(env, action);
    }
}

void execute_opponent_action(Poker* env, int action) {
    uint32_t call_amount = env->current_bet - env->players[1].current_bet;
    
    // CRITICAL: Validate action bounds to prevent segmentation fault
    if (action < 0 || action > 4) {
        printf("🚨 INVALID ACTION: %d (must be 0-4). Using fold.\n", action);
        action = ACTIONS.fold;  // Default to fold for invalid actions
    }
    
    // Track opponent action for modeling
    env->villain_total_actions++;
    if (action == ACTIONS.bet_pot || action == ACTIONS.all_in) {
        env->villain_aggressive_actions++;
    }
    
    // Track comprehensive stats for villain
    int is_preflop = (env->phase == PHASE.preflop);
    int is_cbet_spot = (env->villain_raised_preflop && env->phase >= PHASE.flop);
    track_comprehensive_stats(env, 1, action, is_preflop, is_cbet_spot);
    
    // Track villain 3-bet opportunities and actions
    if (env->phase == PHASE.preflop) {
        if (action == ACTIONS.bet_pot || action == ACTIONS.all_in) {
            if (!env->villain_bet_preflop) {
                env->villain_bet_preflop = 1;
                env->preflop_raises_count++;
            } else {
                env->preflop_raises_count++;
            }
        }
        
        // Villain 3-bet opportunity: hero has raised and villain faces decision to re-raise
        if (env->preflop_raises_count >= 1 && env->hero_bet_preflop && call_amount > env->config.big_blind) {
            env->villain_opportunities_3bet++;
            
            if (action == ACTIONS.bet_pot || action == ACTIONS.all_in) {
                env->villain_actual_3bet++;
            }
        }
    }
    
    if (action == ACTIONS.fold) {
        env->players[1].folded = 1;
        add_action_to_history(env, "Villain folds");
        track_betting_action(env, 1, 0, 0);  // Player 1, fold action, not a check
    } else if (action == ACTIONS.call) {
        if (call_amount == 0) {
            add_action_to_history(env, "Villain checks");
            track_betting_action(env, 1, 1, 1);  // Player 1, call/check action, is a check
        } else if (call_amount <= env->players[1].stack) {
            env->players[1].current_bet += call_amount;
            env->players[1].stack -= call_amount;
            env->pot += call_amount;
            if (!env->headless) {
                char action_str[50];
                snprintf(action_str, sizeof(action_str), "Villain calls $%d", call_amount);
                add_action_to_history(env, action_str);
            }
            track_betting_action(env, 1, 1, 0);  // Player 1, call/check action, not a check
        } else {
            // All-in call
            uint32_t all_in_amount = env->players[1].stack;
            env->players[1].current_bet += all_in_amount;
            env->players[1].stack = 0;
            env->players[1].all_in = 1;
            env->pot += all_in_amount;
            if (!env->headless) {
                char action_str[50];
                snprintf(action_str, sizeof(action_str), "Villain calls all-in $%d", all_in_amount);
                add_action_to_history(env, action_str);
            }
            track_betting_action(env, 1, 1, 0);  // Player 1, call/check action, not a check
        }
    } else if (action == ACTIONS.check) {
        // Check action (only valid when no bet to call)
        if (call_amount == 0) {
            add_action_to_history(env, "Villain checks");
            track_betting_action(env, 1, 1, 1);  // Player 1, call/check action, is a check
        } else {
            // Invalid check when facing bet - treat as fold
            env->players[1].folded = 1;
            add_action_to_history(env, "Villain folds (invalid check)");
            track_betting_action(env, 1, 0, 0);  // Player 1, fold action, not a check
        }
    } else {
        // Betting actions
        uint32_t bet_amount = 0;
        const char* action_name = "";
        
        if (action == ACTIONS.bet_pot) {
            bet_amount = call_amount + env->pot;
            action_name = "pot";
        } else if (action == ACTIONS.all_in) {
            bet_amount = env->players[1].stack;
            action_name = "all-in";
            env->players[1].all_in = 1;
        }
        
        if (bet_amount > env->players[1].stack) {
            bet_amount = env->players[1].stack;
            env->players[1].all_in = 1;
            action_name = "all-in";
        }
        
        if (bet_amount > 0) {
            env->players[1].current_bet += bet_amount;
            env->players[1].stack -= bet_amount;
            env->pot += bet_amount;
            env->current_bet = env->players[1].current_bet;
            
            if (!env->headless) {
                char action_str[50];
                snprintf(action_str, sizeof(action_str), "Villain bets %s $%d", action_name, bet_amount);
                add_action_to_history(env, action_str);
            }
            track_betting_action(env, 1, 2, 0);  // Player 1, bet action, not a check
            
            if (env->phase == PHASE.preflop) {
                env->villain_raised_preflop = 1;
            }
        }
    }
}

void c_reset(Poker* env) {
    env->tick = 0;
    env->episode_length = 0;
    env->episode_return = 0.0f;
    env->hand_number = 0;
    env->button = 0;
    
    env->episode_showdowns = 0;
    env->episode_hands_won = 0;
    env->episode_hero_folds = 0;
    env->episode_villain_folds = 0;
    env->episode_showdown_wins = 0;
    env->episode_showdown_losses = 0;
    
    env->episode_action_fold = 0;
    env->episode_action_call = 0;
    env->episode_action_check = 0;
    env->episode_action_bet_pot = 0;
    env->episode_action_all_in = 0;
    env->episode_river_decisions = 0;
    env->episode_pot_size_won = 0;
    env->episode_pot_size_lost = 0;
    
    env->episode_rainbow_boards = 0;
    env->episode_two_tone_boards = 0;
    env->episode_broadway_boards = 0;
    env->episode_paired_boards = 0;
    env->villain_aggressive_actions = 0;
    env->villain_total_actions = 0;
    
    env->episode_mc_reward_total = 0.0f;
    env->episode_mc_reward_fold = 0.0f;
    env->episode_mc_reward_call = 0.0f;
    env->episode_mc_reward_check = 0.0f;
    env->episode_mc_reward_bet_pot = 0.0f;
    env->episode_mc_reward_all_in = 0.0f;
    env->episode_mc_simulation_count = 0;
    
    // Reset comprehensive stat tracking
    env->hero_hands_vpip = 0;
    env->hero_hands_pfr = 0;
    env->hero_vpip_this_hand = 0;
    env->hero_pfr_this_hand = 0;
    env->hero_bet_preflop = 0;
    env->hero_opportunities_3bet = 0;
    env->hero_actual_3bet = 0;
    env->hero_opportunities_fold_to_3bet = 0;
    env->hero_actual_fold_to_3bet = 0;
    env->hero_opportunities_cbet_flop = 0;
    env->hero_actual_cbet_flop = 0;
    env->hero_opportunities_cbet_turn = 0;
    env->hero_actual_cbet_turn = 0;
    env->hero_opportunities_cbet_river = 0;
    env->hero_actual_cbet_river = 0;
    env->hero_aggressive_postflop_actions = 0;
    env->hero_total_postflop_actions = 0;
    env->hero_went_to_showdown = 0;
    env->hero_won_at_showdown = 0;
    
    env->villain_hands_vpip = 0;
    env->villain_hands_pfr = 0;
    env->villain_vpip_this_hand = 0;
    env->villain_pfr_this_hand = 0;
    env->villain_bet_preflop = 0;
    env->preflop_raises_count = 0;
    env->villain_opportunities_3bet = 0;
    env->villain_actual_3bet = 0;
    env->villain_opportunities_fold_to_3bet = 0;
    env->villain_actual_fold_to_3bet = 0;
    env->villain_opportunities_cbet_flop = 0;
    env->villain_actual_cbet_flop = 0;
    env->villain_opportunities_cbet_turn = 0;
    env->villain_actual_cbet_turn = 0;
    env->villain_opportunities_cbet_river = 0;
    env->villain_actual_cbet_river = 0;
    env->villain_aggressive_postflop_actions = 0;
    env->villain_total_postflop_actions = 0;
    env->villain_went_to_showdown = 0;
    env->villain_won_at_showdown = 0;
    
    env->preflop_all_ins = 0;
    env->total_pot_size = 0;
    env->limped_hands = 0;
    env->three_bet_hands = 0;
    
    // Reset opponent modeling of hero
    env->hero_total_cbets = 0;
    env->hero_cbet_opportunities = 0;
    env->hero_total_bets = 0;
    env->hero_total_decisions = 0;
    
    // Reset PBS state
    env->cached_opponent_strength = 0.5f;  // Neutral prior
    env->cached_range_width = 0.8f;        // Wide range prior
    env->belief_update_count = 0;
    env->total_belief_entropy = 0.0f;
    env->belief_accuracy_checks = 0.0f;
    
    // Reset betting round tracking
    env->actions_this_round = 0;
    env->consecutive_checks = 0;
    env->last_action[0] = 0;
    env->last_action[1] = 0;
    
    // Reset caches
    env->hand_cache_valid = 0;
    env->board_cache_valid = 0;
    
    for (int i = 0; i < 2; i++) {
        env->players[i].stack = env->config.starting_stack;
        env->players[i].current_bet = 0;
        env->players[i].folded = 0;
        env->players[i].all_in = 0;
    }
    
    start_new_hand(env);
    compute_observations(env);
}

void deal_community_card(Poker* env) {
    if (env->phase == PHASE.flop) {
        for (int i = 0; i < 3; i++) {
            env->community_cards[i] = deal_card(env);
        }
        env->community_count = 3;
        env->board_cache_valid = 0;
        compute_board_texture(env);
    } else if (env->phase == PHASE.turn) {
        env->community_cards[3] = deal_card(env);
        env->community_count = 4;
        env->board_cache_valid = 0;
    } else if (env->phase == PHASE.river) {
        env->community_cards[4] = deal_card(env);
        env->community_count = 5;
        env->board_cache_valid = 0;
    }
}

float monte_carlo_action_value(Poker* env, int action) {
    if (env->phase >= PHASE.showdown) return 0.0f;
    
    // SIMPLIFIED HAND STRENGTH EVALUATION - No stack copying to prevent segfault
    uint32_t hero_hand_rank = fast_hand_eval(env->players[0].hole_cards, env->community_cards);
    uint32_t villain_hand_rank = fast_hand_eval(env->players[1].hole_cards, env->community_cards);
    
    float hero_strength = (float)hero_hand_rank / 8.0f;
    float villain_strength = (float)villain_hand_rank / 8.0f;
    
    // Estimate win probability based on hand strength comparison
    float win_probability = 0.5f;
    if (hero_strength > villain_strength + 0.1f) {
        win_probability = 0.75f;
    } else if (hero_strength < villain_strength - 0.1f) {
        win_probability = 0.25f;
    }
    
    // Calculate expected value for each action
    float expected_value = 0.0f;
    uint32_t call_amount = env->current_bet - env->players[0].current_bet;
    float pot_size = (float)env->pot;
    
    if (action == ACTIONS.fold) {
        expected_value = 0.0f;
    } else if (action == ACTIONS.call && call_amount > 0) {
        expected_value = (win_probability * pot_size) - ((1.0f - win_probability) * (float)call_amount);
    } else if (action == ACTIONS.check) {
        expected_value = win_probability * pot_size * 0.5f;
    } else if (action == ACTIONS.bet_pot) {
        expected_value = (win_probability * pot_size * 1.8f) - ((1.0f - win_probability) * pot_size);
    } else if (action == ACTIONS.all_in) {
        float all_in_amount = (float)env->players[0].stack;
        expected_value = (win_probability * (pot_size + all_in_amount)) - ((1.0f - win_probability) * all_in_amount);
    }
    
    return expected_value / 10.0f; // Scale to reasonable reward range
}

void c_step(Poker* env) {
    env->terminals[0] = 0;
    env->rewards[0] = 0.0f;
    
    // Don't increment episode_length yet - do it only when actual actions are taken
    
    if (!env->headless && (IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT))) {
        env->human_mode = !env->human_mode;
    }
    
    if (env->current_player != 0) {
        // Always process opponent action, whether self-play or random
        opponent_action(env);
        env->current_player = 0;
        
        // Increment episode length after opponent action
        env->episode_length++;
    } else {
        if (env->human_mode && !env->headless) {
            int human_action = -1;
            if (IsKeyPressed(KEY_ZERO) || IsKeyPressed(KEY_KP_0)) human_action = ACTIONS.fold;
            else if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1)) human_action = ACTIONS.call;
            else if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2)) human_action = ACTIONS.check;
            else if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3)) human_action = ACTIONS.bet_pot;
            else if (IsKeyPressed(KEY_FOUR) || IsKeyPressed(KEY_KP_4)) human_action = ACTIONS.all_in;
            
            if (human_action == -1) {
                compute_observations(env);
                return;
            }
            env->actions[0] = human_action;
        }
        
        int action = (int)env->actions[0];
        
        // Calculate Monte Carlo reward for this action
        float mc_reward = monte_carlo_action_value(env, action) * MC.reward_weight;
        env->rewards[0] += mc_reward;
        env->episode_return += mc_reward;
        
        // Track MC reward stats
        env->episode_mc_reward_total += mc_reward;
        env->episode_mc_simulation_count += 1; // Track each MC call
        if (action == ACTIONS.fold) env->episode_mc_reward_fold += mc_reward;
        else if (action == ACTIONS.call) env->episode_mc_reward_call += mc_reward;
        else if (action == ACTIONS.check) env->episode_mc_reward_check += mc_reward;
        else if (action == ACTIONS.bet_pot) env->episode_mc_reward_bet_pot += mc_reward;
        else if (action == ACTIONS.all_in) env->episode_mc_reward_all_in += mc_reward;
        
        // Only increment episode length when actual actions are taken
        env->episode_length++;
        
        if (action >= 0 && action < POKER.action_count) {
            uint32_t call_amount = env->current_bet - env->players[0].current_bet;
            env->episode_river_decisions += 1;
            
            // Track 3-bet opportunities and actions
            if (env->phase == PHASE.preflop) {
                // Track betting sequence for proper 3-bet detection
                if (action == ACTIONS.bet_pot || action == ACTIONS.all_in) {
                    if (!env->hero_bet_preflop) {
                        env->hero_bet_preflop = 1;
                        env->preflop_raises_count++;
                    } else {
                        // Hero is raising again - potential 3-bet or 4-bet
                        env->preflop_raises_count++;
                    }
                }
                
                // 3-bet opportunity: villain has raised and hero faces a decision to re-raise
                // This happens when there have been raises and hero faces a bet > big blind
                if (env->preflop_raises_count >= 1 && env->villain_bet_preflop && call_amount > env->config.big_blind) {
                    env->hero_opportunities_3bet++;
                    
                    if (action == ACTIONS.bet_pot || action == ACTIONS.all_in) {
                        env->hero_actual_3bet++;
                    }
                }
            }
            
            // Track comprehensive stats for hero
            int is_preflop = (env->phase == PHASE.preflop);
            int is_cbet_spot = (env->hero_raised_preflop && env->phase >= PHASE.flop);
            track_comprehensive_stats(env, 0, action, is_preflop, is_cbet_spot);
            
            // Track hero's aggression for opponent modeling
            env->hero_total_decisions++;
            if (action == ACTIONS.bet_pot || action == ACTIONS.all_in) {
                env->hero_total_bets++;
            }
            if (is_cbet_spot) {
                env->hero_cbet_opportunities++;
                if (action == ACTIONS.bet_pot || action == ACTIONS.all_in) {
                    env->hero_total_cbets++;
                }
            }
            
            if (action == ACTIONS.fold) {
                env->players[0].folded = 1;
                env->episode_hero_folds += 1;
                env->episode_action_fold += 1;
                add_action_to_history(env, "Hero folds");
                track_betting_action(env, 0, 0, 0);  // Player 0, fold action, not a check
                
                
                env->players[1].stack += env->pot;
                float hero_investment = (float)env->players[0].starting_stack - (float)env->players[0].stack;
                float net_profit = -hero_investment;
                env->rewards[0] += net_profit;
                env->episode_return += net_profit;
                env->episode_pot_size_lost += env->pot;
                
                start_new_hand(env);
                return;
                
            } else if (action == ACTIONS.call) {
                if (call_amount <= env->players[0].stack) {
                    env->players[0].current_bet += call_amount;
                    env->players[0].stack -= call_amount;
                    env->pot += call_amount;
                    env->episode_action_call += 1;
                    if (call_amount == 0) {
                        add_action_to_history(env, "Hero checks");
                        track_betting_action(env, 0, 1, 1);  // Player 0, call/check action, is a check
                    } else {
                        if (!env->headless) {
                            char action_str[50];
                            snprintf(action_str, sizeof(action_str), "Hero calls $%d", call_amount);
                            add_action_to_history(env, action_str);
                        }
                        track_betting_action(env, 0, 1, 0);  // Player 0, call/check action, not a check
                    }
                    if (env->players[0].stack == 0) {
                        env->players[0].all_in = 1;
                    }
                } else {
                    env->players[0].folded = 1;
                    add_action_to_history(env, "Hero folds");
                }
                
            } else if (action == ACTIONS.check) {
                // Check - only valid when no bet to call
                if (call_amount == 0) {
                    env->episode_action_check += 1;
                    add_action_to_history(env, "Hero checks");
                    track_betting_action(env, 0, 1, 1);  // Player 0, call/check action, is a check
                } else {
                    // Invalid check when facing bet - treat as fold
                    env->players[0].folded = 1;
                    env->episode_action_fold += 1;
                    add_action_to_history(env, "Hero folds (invalid check)");
                    track_betting_action(env, 0, 0, 0);  // Player 0, fold action, not a check
                    env->players[1].stack += env->pot;
                    float hero_investment = (float)env->players[0].starting_stack - (float)env->players[0].stack;
                    float net_profit = -hero_investment;
                    env->rewards[0] += net_profit;
                    env->episode_return += net_profit;
                    env->episode_pot_size_lost += env->pot;
                    start_new_hand(env);
                    return;
                }
                
            } else {
                // Handle betting actions
                uint32_t bet_amount = 0;
                
                if (action == ACTIONS.bet_pot) {
                    bet_amount = call_amount + env->pot;
                    env->episode_action_bet_pot += 1;
                } else if (action == ACTIONS.all_in) {
                    bet_amount = env->players[0].stack;
                    env->episode_action_all_in += 1;
                    env->players[0].all_in = 1;
                }
                
                if (bet_amount > env->players[0].stack) {
                    bet_amount = env->players[0].stack;
                    env->players[0].all_in = 1;
                }
                
                if (bet_amount >= call_amount) {
                    env->players[0].current_bet += bet_amount;
                    env->players[0].stack -= bet_amount;
                    env->pot += bet_amount;
                    env->current_bet = env->players[0].current_bet;
                    
                    if (!env->headless) {
                        char action_str[50];
                        if (action == ACTIONS.all_in) {
                            snprintf(action_str, sizeof(action_str), "Hero all-in $%d", bet_amount);
                        } else {
                            snprintf(action_str, sizeof(action_str), "Hero bets $%d", bet_amount);
                        }
                        add_action_to_history(env, action_str);
                    }
                    track_betting_action(env, 0, 2, 0);  // Player 0, bet action, not a check
                    
                    // Track preflop raising for c-bet opportunities
                    if (env->phase == PHASE.preflop) {
                        env->hero_raised_preflop = 1;
                    }
                    
                    if (env->players[0].stack == 0) {
                        env->players[0].all_in = 1;
                    }
                } else {
                    env->players[0].folded = 1;
                    add_action_to_history(env, "Hero folds");
                }
            }
        }
        
        env->current_player = 1;
    }
    
    if (is_betting_round_over(env)) {
        if (env->players[0].folded) {
            env->players[1].stack += env->pot;
            float hero_investment = (float)env->players[0].starting_stack - (float)env->players[0].stack;
            float net_profit = -hero_investment;
            env->rewards[0] += net_profit;
            env->episode_return += net_profit;
            env->episode_pot_size_lost += env->pot;
            
            start_new_hand(env);
            return;
        } else if (env->players[1].folded) {
            env->players[0].stack += env->pot;
            float hero_investment = (float)env->players[0].starting_stack - (float)env->players[0].stack;
            float net_profit = env->pot - hero_investment;
            env->rewards[0] += net_profit;
            env->episode_return += net_profit;
            env->episode_hands_won += 1;
            env->episode_villain_folds += 1;
            env->episode_pot_size_won += env->pot;
            
            start_new_hand(env);
            return;
        } else {
            // Check if any player is all-in - if so, run out remaining cards
            if (env->players[0].all_in || env->players[1].all_in) {
                // Players are all-in, advance through all remaining streets to showdown
                if (env->phase == PHASE.preflop) {
                    advance_phase(env); // Go to flop
                    advance_phase(env); // Go to turn  
                    advance_phase(env); // Go to river
                    advance_phase(env); // Go to showdown
                } else if (env->phase == PHASE.flop) {
                    advance_phase(env); // Go to turn
                    advance_phase(env); // Go to river
                    advance_phase(env); // Go to showdown
                } else if (env->phase == PHASE.turn) {
                    advance_phase(env); // Go to river
                    advance_phase(env); // Go to showdown
                } else if (env->phase == PHASE.river) {
                    advance_phase(env); // Go to showdown
                }
            } else {
                // Normal betting: advance to next street only
                advance_phase(env);
            }
        }
    }
    
    env->tick++;
    compute_observations(env);
}

static Client* make_client(float width, float height) {
    Client* client = (Client*)calloc(1, sizeof(Client));
    client->width = width;
    client->height = height;
    client->card_width = 60.0f;
    client->card_height = 84.0f;
    client->font_size = 24;
    
    InitWindow((int)width, (int)height, "PufferLib Poker");
    SetTargetFPS(60);
    return client;
}

void c_close_client(Client* client) {
    CloseWindow();
    free(client);
}

static const char* rank_to_string(uint8_t rank) {
    static const char* ranks[] = {"2", "3", "4", "5", "6", "7", "8", "9", "T", "J", "Q", "K", "A"};
    return ranks[rank];
}

static const char* suit_to_string(uint8_t suit) {
    static const char* suits[] = {"C", "D", "H", "S"};
    return suits[suit];
}

static Color suit_color(uint8_t suit) {
    if (suit == SUIT.hearts || suit == SUIT.diamonds) {
        return RED;
    }
    return BLACK;
}

static void draw_card(Card card, float x, float y, float width, float height) {
    DrawRectangle((int)x, (int)y, (int)width, (int)height, WHITE);
    DrawRectangleLines((int)x, (int)y, (int)width, (int)height, BLACK);
    
    Color color = suit_color(card.suit);
    const char* rank_str = rank_to_string(card.rank);
    const char* suit_str = suit_to_string(card.suit);
    
    DrawText(rank_str, (int)(x + 5), (int)(y + 5), 16, color);
    DrawText(suit_str, (int)(x + 5), (int)(y + 25), 16, color);
    DrawText(rank_str, (int)(x + width - 15), (int)(y + height - 25), 16, color);
    DrawText(suit_str, (int)(x + width - 15), (int)(y + height - 45), 16, color);
}

static void draw_card_back(float x, float y, float width, float height) {
    DrawRectangle((int)x, (int)y, (int)width, (int)height, BLUE);
    DrawRectangleLines((int)x, (int)y, (int)width, (int)height, BLACK);
    DrawText("?", (int)(x + width/2 - 10), (int)(y + height/2 - 10), 20, WHITE);
}

void c_render(Poker* env) {
    if (env->headless) {
        return;
    }
    
    if (env->client == NULL) {
        env->client = make_client(env->width, env->height);
    }
    
    Client* client = env->client;
    
    if (IsKeyDown(KEY_ESCAPE)) {
        exit(0);
    }
    
    BeginDrawing();
    
    Color table_green = (Color){35, 100, 35, 255};
    Color felt_dark = (Color){25, 70, 25, 255};
    Color gold = (Color){255, 215, 0, 255};
    Color silver = (Color){192, 192, 192, 255};
    Color chip_red = (Color){220, 20, 20, 255};
    Color chip_blue = (Color){20, 20, 220, 255};
    
    ClearBackground(felt_dark);
    
    float center_x = client->width / 2;
    float center_y = client->height / 2;
    
    DrawRectangle(50, 80, client->width - 100, client->height - 160, table_green);
    DrawRectangleLines(50, 80, client->width - 100, client->height - 160, gold);
    
    DrawEllipse(center_x, center_y - 20, 180, 120, felt_dark);
    DrawEllipseLines(center_x, center_y - 20, 180, 120, gold);
    
    DrawText("Poker - HU NL", 10, 10, 28, gold);
    
    if (env->human_mode) {
        DrawRectangle(client->width - 100, 5, 90, 30, (Color){0, 150, 0, 200});
        DrawRectangleLines(client->width - 100, 5, 90, 30, WHITE);
        DrawText("HUMAN", client->width - 90, 12, 16, WHITE);
    } else {
        DrawRectangle(client->width - 100, 5, 90, 30, (Color){150, 0, 0, 200});
        DrawRectangleLines(client->width - 100, 5, 90, 30, WHITE);
        DrawText("AI MODE", client->width - 90, 12, 16, WHITE);
    }
    
    DrawText("(SHIFT to toggle)", client->width - 140, 40, 10, GRAY);
    
    DrawText(TextFormat("Hand #%d", env->hand_number), 10, 45, 18, WHITE);
    
    // Calculate and display running BB/100
    float running_bb_100 = 0.0f;
    if (env->hand_number > 0) {
        running_bb_100 = (env->episode_return / (float)env->config.big_blind) * 100.0f / (float)env->hand_number;
    }
    DrawText(TextFormat("BB/100: %.1f", running_bb_100), 10, 70, 16, running_bb_100 >= 0 ? GREEN : RED);
    
    DrawText(TextFormat("POT: $%d", env->pot), center_x - 50, center_y - 120, 20, gold);
    
    const char* phase_names[] = {"Pre-flop", "Flop", "Turn", "River", "Showdown"};
    DrawText(phase_names[env->phase], center_x - 40, center_y + 100, 18, WHITE);
    
    float hero_cards_x = center_x - 30;
    float hero_cards_y = client->height - 100;
    
    DrawText(TextFormat("$%d", env->players[0].stack), hero_cards_x + 120, hero_cards_y + 10, 14, WHITE);
    DrawText(TextFormat("Bet: $%d", env->players[0].current_bet), hero_cards_x + 120, hero_cards_y + 30, 12, GRAY);
    
    if (env->players[0].folded) {
        DrawText("FOLDED", hero_cards_x + 120, hero_cards_y + 50, 12, RED);
    } else if (env->current_player == 0 && env->human_mode) {
        DrawText("YOUR TURN", hero_cards_x + 120, hero_cards_y + 50, 12, YELLOW);
    }
    
    float villain_cards_x = center_x - 30;
    float villain_cards_y = 80;
    
    DrawText(TextFormat("$%d", env->players[1].stack), villain_cards_x + 120, villain_cards_y + 10, 14, WHITE);
    DrawText(TextFormat("Bet: $%d", env->players[1].current_bet), villain_cards_x + 120, villain_cards_y + 30, 12, GRAY);
    
    if (env->players[1].folded) {
        DrawText("FOLDED", villain_cards_x + 120, villain_cards_y + 50, 12, RED);
    } else if (env->current_player == 1) {
        DrawText("THINKING...", villain_cards_x + 120, villain_cards_y + 50, 12, YELLOW);
    }
    
    float card_width = 50;
    float card_height = 70;
    float card_spacing = 8;
    
    for (int i = 0; i < 2; i++) {
        Card card = env->players[0].hole_cards[i];
        float x = hero_cards_x + i * (card_width + card_spacing);
        draw_card(card, x, hero_cards_y, card_width, card_height);
    }
    
    for (int i = 0; i < 2; i++) {
        float x = villain_cards_x + i * (card_width + card_spacing);
        if (env->phase == PHASE.showdown && !env->players[1].folded) {
            Card card = env->players[1].hole_cards[i];
            draw_card(card, x, villain_cards_y, card_width, card_height);
        } else {
            draw_card_back(x, villain_cards_y, card_width, card_height);
        }
    }
    
    if (env->community_count > 0) {
        float board_start_x = center_x - (env->community_count * (card_width + card_spacing)) / 2;
        float board_y = center_y - 35;
        
        DrawRectangle(board_start_x - 10, board_y - 10, 
                     env->community_count * (card_width + card_spacing) + 10, 
                     card_height + 20, (Color){20, 20, 20, 150});
        
        for (int i = 0; i < env->community_count; i++) {
            Card card = env->community_cards[i];
            float x = board_start_x + i * (card_width + card_spacing);
            draw_card(card, x, board_y, card_width, card_height);
        }
    }
    
    if (env->human_mode) {
        float action_panel_x = 15;
        float action_panel_y = center_y + 80;
        float action_width = 250;
        float action_height = 200;
        
        DrawRectangle(action_panel_x, action_panel_y, action_width, action_height, (Color){30, 30, 30, 220});
        DrawRectangleLines(action_panel_x, action_panel_y, action_width, action_height, gold);
        
        if (env->current_player == 0 && !env->players[0].folded) {
            DrawText("YOUR TURN", action_panel_x + 15, action_panel_y + 15, 18, YELLOW);
            
            const char* actions[] = {
                "0: Fold", "1: Call", "2: Check", "3: Bet Pot", "4: All-in"
            };
            
            for (int i = 0; i < 5; i++) {
                Color action_color = WHITE;
                if (i == 0) action_color = (Color){255, 100, 100, 255};
                else if (i == 1) action_color = (Color){100, 255, 100, 255};
                else if (i >= 2) action_color = (Color){255, 200, 100, 255};
                
                DrawText(actions[i], action_panel_x + 15, action_panel_y + 45 + i * 20, 14, action_color);
            }
            
            uint32_t call_amount = env->current_bet - env->players[0].current_bet;
            if (call_amount > 0) {
                DrawText(TextFormat("To call: $%d", call_amount), action_panel_x + 15, action_panel_y + 180, 12, YELLOW);
            }
        } else {
            DrawText("WAITING...", action_panel_x + 15, action_panel_y + 15, 18, GRAY);
            DrawText("Opponent's turn", action_panel_x + 15, action_panel_y + 45, 14, GRAY);
        }
    }
    
    float history_panel_x = client->width - 280;
    float history_panel_y = center_y + 80;
    float history_width = 260;
    float history_height = 320;
    
    DrawRectangle(history_panel_x, history_panel_y, history_width, history_height, (Color){25, 25, 25, 220});
    DrawRectangleLines(history_panel_x, history_panel_y, history_width, history_height, silver);
    
    DrawText("ACTION HISTORY", history_panel_x + 15, history_panel_y + 15, 16, silver);
    DrawText(TextFormat("Hand #%d", env->hand_number), history_panel_x + 15, history_panel_y + 40, 14, WHITE);
    
    int y_offset = 65;
    for (int i = 0; i < env->hand_history.count && i < 20; i++) {
        DrawText(env->hand_history.actions[i], history_panel_x + 15, history_panel_y + y_offset, 10, WHITE);
        y_offset += 12;
    }
    
    if (env->players[0].position == 0) {
        float button_x = hero_cards_x - 20;
        float button_y = hero_cards_y + 35;
        DrawCircle(button_x, button_y, 8, WHITE);
        DrawText("D", button_x - 3, button_y - 5, 10, BLACK);
    } else {
        float button_x = villain_cards_x - 20;
        float button_y = villain_cards_y + 35;
        DrawCircle(button_x, button_y, 8, WHITE);
        DrawText("D", button_x - 3, button_y - 5, 10, BLACK);
    }
    
    EndDrawing();
}

#endif