#ifndef POKER_H
#define POKER_H

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
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
    uint32_t current_bet;
    uint8_t folded;
    uint8_t all_in;
    uint8_t position;
} Player;

typedef struct Log {
    float episode_return;
    float episode_length;
    float hands_played;
    float showdowns_reached;
    float hands_won;
    float big_blinds_won;
    float n;
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
    
    ActionHistory hand_history;
    
    GameConfig config;
    uint32_t tick;
    uint16_t episode_length;
    uint16_t hand_number;
    float episode_return;
    
    float width;
    float height;
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

void shuffle_deck(Poker* env) {
    for (int i = 0; i < 52; i++) {
        env->deck[i] = index_to_card(i);
    }
    
    for (int i = 51; i > 0; i--) {
        int j = rand() % (i + 1);
        Card temp = env->deck[i];
        env->deck[i] = env->deck[j];
        env->deck[j] = temp;
    }
    env->deck_position = 0;
}

Card deal_card(Poker* env) {
    return env->deck[env->deck_position++];
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
        uint8_t consecutive = 1;
        for (int j = 1; j < 5; j++) {
            if (ranks[i + j] > 0) consecutive++;
        }
        if (consecutive == 5) straight = 1;
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
    Card player1_cards[7];
    Card player2_cards[7];
    
    for (int i = 0; i < 2; i++) {
        player1_cards[i] = env->players[0].hole_cards[i];
        player2_cards[i] = env->players[1].hole_cards[i];
    }
    
    for (int i = 0; i < env->community_count; i++) {
        player1_cards[2 + i] = env->community_cards[i];
        player2_cards[2 + i] = env->community_cards[i];
    }
    
    uint32_t rank1 = hand_rank(player1_cards, 2 + env->community_count);
    uint32_t rank2 = hand_rank(player2_cards, 2 + env->community_count);
    
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
}

void allocate(Poker* env) {
    env->tick = 0;
    env->episode_return = 0.0f;
    env->hand_number = 0;
    env->client = NULL;
    memset(&env->log, 0, sizeof(Log));
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

void compute_observations(Poker* env) {
    int obs_idx = 0;
    
    for (int i = 0; i < 52; i++) {
        env->observations[obs_idx++] = 0.0f;
    }
    
    for (int i = 0; i < 2; i++) {
        uint8_t card_idx = card_to_index(env->players[0].hole_cards[i]);
        env->observations[card_idx] = 1.0f;
    }
    
    for (int i = 0; i < env->community_count; i++) {
        uint8_t card_idx = card_to_index(env->community_cards[i]);
        env->observations[card_idx] = 1.0f;
    }
    
    float max_stack = env->config.starting_stack;
    env->observations[obs_idx++] = env->pot / (float)(2 * max_stack);
    env->observations[obs_idx++] = env->current_bet / (float)max_stack;
    env->observations[obs_idx++] = env->players[0].stack / (float)max_stack;
    env->observations[obs_idx++] = env->players[1].stack / (float)max_stack;
    env->observations[obs_idx++] = env->players[0].current_bet / (float)max_stack;
    env->observations[obs_idx++] = env->players[1].current_bet / (float)max_stack;
    env->observations[obs_idx++] = env->phase / 4.0f;
    env->observations[obs_idx++] = env->players[0].position;
    env->observations[obs_idx++] = env->current_player;
    env->observations[obs_idx++] = env->players[1].folded;
    env->observations[obs_idx++] = env->players[0].all_in;
    env->observations[obs_idx++] = env->players[1].all_in;
    
    float call_amount = env->current_bet - env->players[0].current_bet;
    env->observations[obs_idx++] = call_amount / (float)max_stack;
    
    uint32_t min_raise = call_amount + env->current_bet;
    env->observations[obs_idx++] = min_raise / (float)max_stack;
    
    env->observations[obs_idx++] = env->hand_number / 1000.0f;
    env->observations[obs_idx++] = (env->players[0].stack + env->players[1].stack) / (float)(2 * max_stack);
    env->observations[obs_idx++] = env->episode_length / (float)POKER.max_episode_length;
    env->observations[obs_idx++] = env->episode_return / 100.0f;
    
    uint8_t can_call = (call_amount <= env->players[0].stack);
    uint8_t can_raise = (min_raise <= env->players[0].stack);
    env->observations[obs_idx++] = can_call;
    env->observations[obs_idx++] = can_raise;
}

static void add_log(Poker* env) {
    env->log.episode_return += env->episode_return;
    env->log.episode_length += env->episode_length;
    env->log.hands_played += env->hand_number;
    env->log.big_blinds_won += env->episode_return / (float)env->config.big_blind;
    env->log.n += 1;
}

void add_action_to_history(Poker* env, const char* action) {
    if (env->hand_history.count < 20) {
        strcpy(env->hand_history.actions[env->hand_history.count], action);
        env->hand_history.count++;
    }
}

void start_new_hand(Poker* env) {
    env->hand_number++;
    env->pot = 0;
    env->current_bet = 0;
    env->community_count = 0;
    env->phase = PHASE.preflop;
    env->betting_round_over = 0;
    
    env->hand_history.count = 0;
    
    for (int i = 0; i < 2; i++) {
        env->players[i].current_bet = 0;
        env->players[i].folded = 0;
        env->players[i].all_in = 0;
    }
    
    env->button = (env->button + 1) % 2;
    env->players[0].position = env->button;
    env->players[1].position = 1 - env->button;
    
    shuffle_deck(env);
    
    for (int i = 0; i < 2; i++) {
        env->players[0].hole_cards[i] = deal_card(env);
        env->players[1].hole_cards[i] = deal_card(env);
    }
    
    uint32_t small_blind_amt = env->config.small_blind;
    uint32_t big_blind_amt = env->config.big_blind;
    
    if (env->button == 0) {
        env->players[0].current_bet = small_blind_amt;
        env->players[1].current_bet = big_blind_amt;
        add_action_to_history(env, "Hero SB $1");
        add_action_to_history(env, "Villain BB $2");
    } else {
        env->players[1].current_bet = small_blind_amt;
        env->players[0].current_bet = big_blind_amt;
        add_action_to_history(env, "Villain SB $1");
        add_action_to_history(env, "Hero BB $2");
    }
    
    env->pot = small_blind_amt + big_blind_amt;
    env->current_bet = big_blind_amt;
    env->current_player = env->button;
    
    for (int i = 0; i < 2; i++) {
        env->players[i].stack -= env->players[i].current_bet;
        if (env->players[i].stack == 0) {
            env->players[i].all_in = 1;
        }
    }
}

void advance_phase(Poker* env) {
    env->phase++;
    env->current_bet = 0;
    env->betting_round_over = 0;
    
    for (int i = 0; i < 2; i++) {
        env->players[i].current_bet = 0;
    }
    
    if (env->phase == PHASE.flop) {
        for (int i = 0; i < 3; i++) {
            env->community_cards[i] = deal_card(env);
        }
        env->community_count = 3;
        add_action_to_history(env, "--- FLOP ---");
    } else if (env->phase == PHASE.turn) {
        env->community_cards[3] = deal_card(env);
        env->community_count = 4;
        add_action_to_history(env, "--- TURN ---");
    } else if (env->phase == PHASE.river) {
        env->community_cards[4] = deal_card(env);
        env->community_count = 5;
        add_action_to_history(env, "--- RIVER ---");
    } else if (env->phase == PHASE.showdown) {
        uint8_t winner = compare_hands(env);
        float chips_won = env->pot;
        
        if (winner == 0) {
            env->players[0].stack += env->pot;
            env->rewards[0] += chips_won;
            env->episode_return += chips_won;
            env->log.hands_won += 1;
            add_action_to_history(env, "Hero wins pot");
        } else if (winner == 1) {
            env->players[1].stack += env->pot;
            env->rewards[0] -= chips_won;
            env->episode_return -= chips_won;
            add_action_to_history(env, "Villain wins pot");
        } else {
            uint32_t split_pot = env->pot / 2;
            env->players[0].stack += split_pot;
            env->players[1].stack += split_pot;
            add_action_to_history(env, "Split pot");
        }
        
        env->log.showdowns_reached += 1;
        
        if (env->players[0].stack == 0 || env->players[1].stack == 0) {
            env->terminals[0] = 1;
            add_log(env);
            c_reset(env);
            return;
        }
        
        start_new_hand(env);
        return;
    }
    
    env->current_player = 1 - env->button;
}

uint8_t is_betting_round_over(Poker* env) {
    if (env->players[0].folded || env->players[1].folded) return 1;
    if (env->players[0].all_in || env->players[1].all_in) return 1;
    
    return (env->players[0].current_bet == env->players[1].current_bet);
}

void opponent_action(Poker* env) {
    uint32_t call_amount = env->current_bet - env->players[1].current_bet;
    
    if (call_amount == 0) {
        if (rand() % 4 == 0) {
            uint32_t bet_amount = env->pot / 2;
            if (bet_amount > env->players[1].stack) {
                bet_amount = env->players[1].stack;
                env->players[1].all_in = 1;
            }
            env->players[1].current_bet += bet_amount;
            env->players[1].stack -= bet_amount;
            env->pot += bet_amount;
            env->current_bet = env->players[1].current_bet;
            char action_str[50];
            snprintf(action_str, sizeof(action_str), "Villain bets $%d", bet_amount);
            add_action_to_history(env, action_str);
        } else {
            add_action_to_history(env, "Villain checks");
        }
    } else {
        if (call_amount <= env->players[1].stack) {
            if (rand() % 3 == 0) {
                env->players[1].folded = 1;
                add_action_to_history(env, "Villain folds");
            } else {
                env->players[1].current_bet += call_amount;
                env->players[1].stack -= call_amount;
                env->pot += call_amount;
                char action_str[50];
                snprintf(action_str, sizeof(action_str), "Villain calls $%d", call_amount);
                add_action_to_history(env, action_str);
                if (env->players[1].stack == 0) {
                    env->players[1].all_in = 1;
                }
            }
        } else {
            env->players[1].folded = 1;
            add_action_to_history(env, "Villain folds");
        }
    }
}

void c_reset(Poker* env) {
    env->tick = 0;
    env->episode_length = 0;
    env->episode_return = 0.0f;
    env->hand_number = 0;
    env->button = 0;
    
    for (int i = 0; i < 2; i++) {
        env->players[i].stack = env->config.starting_stack;
        env->players[i].current_bet = 0;
        env->players[i].folded = 0;
        env->players[i].all_in = 0;
    }
    
    start_new_hand(env);
    compute_observations(env);
}

void handle_episode_end(Poker* env) {
    env->terminals[0] = 1;
    add_log(env);
    c_reset(env);
}

void c_step(Poker* env) {
    env->terminals[0] = 0;
    env->rewards[0] = 0.0f;
    
    if (IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT)) {
        env->human_mode = !env->human_mode;
    }
    
    if (env->current_player != 0) {
        opponent_action(env);
        env->current_player = 0;
    } else {
        if (env->human_mode) {
            int human_action = -1;
            if (IsKeyPressed(KEY_ZERO) || IsKeyPressed(KEY_KP_0)) human_action = ACTIONS.fold;
            else if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1)) human_action = ACTIONS.call;
            else if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2)) human_action = ACTIONS.bet_quarter_pot;
            else if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3)) human_action = ACTIONS.bet_half_pot;
            else if (IsKeyPressed(KEY_FOUR) || IsKeyPressed(KEY_KP_4)) human_action = ACTIONS.bet_pot;
            else if (IsKeyPressed(KEY_FIVE) || IsKeyPressed(KEY_KP_5)) human_action = ACTIONS.bet_double_pot;
            else if (IsKeyPressed(KEY_SIX) || IsKeyPressed(KEY_KP_6)) human_action = ACTIONS.all_in;
            
            if (human_action == -1) {
                compute_observations(env);
                return;
            }
            env->actions[0] = human_action;
        }
        
        int action = (int)env->actions[0];
        
        if (action >= 0 && action < POKER.action_count) {
            uint32_t call_amount = env->current_bet - env->players[0].current_bet;
            
            if (action == ACTIONS.fold) {
                env->players[0].folded = 1;
                env->rewards[0] += env->config.fold_penalty;
                env->episode_return += env->config.fold_penalty;
                add_action_to_history(env, "Hero folds");
                
                env->players[1].stack += env->pot;
                env->rewards[0] -= env->pot;
                env->episode_return -= env->pot;
                
                if (env->players[0].stack == 0 || env->players[1].stack == 0) {
                    handle_episode_end(env);
                    return;
                }
                
                start_new_hand(env);
                compute_observations(env);
                return;
                
            } else if (action == ACTIONS.call) {
                if (call_amount <= env->players[0].stack) {
                    env->players[0].current_bet += call_amount;
                    env->players[0].stack -= call_amount;
                    env->pot += call_amount;
                    if (call_amount == 0) {
                        add_action_to_history(env, "Hero checks");
                    } else {
                        char action_str[50];
                        snprintf(action_str, sizeof(action_str), "Hero calls $%d", call_amount);
                        add_action_to_history(env, action_str);
                    }
                    if (env->players[0].stack == 0) {
                        env->players[0].all_in = 1;
                    }
                } else {
                    env->players[0].folded = 1;
                    add_action_to_history(env, "Hero folds");
                }
                
            } else {
                uint32_t bet_amount = 0;
                
                if (action == ACTIONS.bet_quarter_pot) {
                    bet_amount = call_amount + env->pot / 4;
                } else if (action == ACTIONS.bet_half_pot) {
                    bet_amount = call_amount + env->pot / 2;
                } else if (action == ACTIONS.bet_pot) {
                    bet_amount = call_amount + env->pot;
                } else if (action == ACTIONS.bet_double_pot) {
                    bet_amount = call_amount + env->pot * 2;
                } else if (action == ACTIONS.all_in) {
                    bet_amount = env->players[0].stack;
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
                    char action_str[50];
                    if (action == ACTIONS.all_in) {
                        snprintf(action_str, sizeof(action_str), "Hero all-in $%d", bet_amount);
                    } else {
                        snprintf(action_str, sizeof(action_str), "Hero bets $%d", bet_amount);
                    }
                    add_action_to_history(env, action_str);
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
            env->rewards[0] -= env->pot;
            env->episode_return -= env->pot;
            
            if (env->players[0].stack == 0 || env->players[1].stack == 0) {
                handle_episode_end(env);
                return;
            }
            
            start_new_hand(env);
        } else if (env->players[1].folded) {
            env->players[0].stack += env->pot;
            env->rewards[0] += env->pot;
            env->episode_return += env->pot;
            env->log.hands_won += 1;
            
            if (env->players[0].stack == 0 || env->players[1].stack == 0) {
                handle_episode_end(env);
                return;
            }
            
            start_new_hand(env);
        } else if (env->phase == PHASE.river || 
                   (env->players[0].all_in && env->players[1].all_in)) {
            advance_phase(env);
        } else {
            advance_phase(env);
        }
    }
    
    env->episode_length++;
    
    if (env->episode_length >= POKER.max_episode_length) {
        handle_episode_end(env);
        return;
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
                "0: Fold", "1: Call/Check", "2: Bet 1/4 Pot", 
                "3: Bet 1/2 Pot", "4: Bet Pot", "5: Bet 2x Pot", "6: All-in"
            };
            
            for (int i = 0; i < 7; i++) {
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
    float history_height = 200;
    
    DrawRectangle(history_panel_x, history_panel_y, history_width, history_height, (Color){25, 25, 25, 220});
    DrawRectangleLines(history_panel_x, history_panel_y, history_width, history_height, silver);
    
    DrawText("ACTION HISTORY", history_panel_x + 15, history_panel_y + 15, 16, silver);
    DrawText(TextFormat("Hand #%d", env->hand_number), history_panel_x + 15, history_panel_y + 40, 14, WHITE);
    
    int y_offset = 65;
    for (int i = 0; i < env->hand_history.count && i < 10; i++) {
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