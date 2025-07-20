#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include "raylib.h"

#define ACTION_BLIZZARD 8
#define ACTION_BLIZZARD_DOWN 14
#define ACTION_BLIZZARD_DOWN_LEFT 19
#define ACTION_BLIZZARD_DOWN_RIGHT 20
#define ACTION_BLIZZARD_LEFT 15
#define ACTION_BLIZZARD_RIGHT 16
#define ACTION_BLIZZARD_UP 13
#define ACTION_BLIZZARD_UP_LEFT 17
#define ACTION_BLIZZARD_UP_RIGHT 18
#define ACTION_INTERACT 11
#define ACTION_MOVE_DOWN 1
#define ACTION_MOVE_DOWN_LEFT 6
#define ACTION_MOVE_DOWN_RIGHT 7
#define ACTION_MOVE_LEFT 2
#define ACTION_MOVE_RIGHT 3
#define ACTION_MOVE_UP 0
#define ACTION_MOVE_UP_LEFT 4
#define ACTION_MOVE_UP_RIGHT 5
#define ACTION_NOOP 12
#define ACTION_USE_HEALTH_POTION 9
#define ACTION_USE_MANA_POTION 10
#define BOSS_RIFT_GUARDIAN 0
#define CELL_DOOR 3
#define CELL_EMPTY 0
#define CELL_FLOOR 2
#define CELL_VENDOR 4
#define CELL_WALL 1
#define ITEM_GOLD 0
#define ITEM_HEALTH_POTION 1
#define ITEM_MANA_POTION 2
#define MAP_BORDER_SIZE 5
#define MAP_HEIGHT 38
#define MAP_SIZE (MAP_WIDTH * MAP_HEIGHT)
#define MAP_WIDTH 50
#define MONSTER_ELITE 4
#define MONSTER_HEAVY_MELEE 2
#define MONSTER_LIGHT 3
#define MONSTER_MAGE 1
#define MONSTER_ZOMBIE 0
#define PHASE_RIFT 0
#define PHASE_TOWN 1

#define BLIZZARD_ACTIVATION_COOLDOWN 20
#define BLIZZARD_DAMAGE 15
#define BLIZZARD_DAMAGE_INTERVAL 15
#define BLIZZARD_DURATION 150
#define BLIZZARD_MANA_COST 15
#define BLIZZARD_RADIUS 2.0f
#define BOSS_ATTACK_COOLDOWN 15
#define BOSS_ATTACK_DISTANCE 2.0f
#define BOSS_BASE_DAMAGE 15
#define BOSS_BASE_HEALTH 100
#define COOLDOWN_NORMALIZATION 100.0f
#define DISTANCE_NORMALIZATION 50.0f
#define ELITE_DAMAGE 20
#define ELITE_HEALTH 15
#define ELITE_RANGE 4.0f
#define ELITE_SPEED 0.4f
#define FULL_CIRCLE_MULTIPLIER 2.0f
#define GOLD_DROP_CHANCE 40
#define GOLD_DROP_MIN 5
#define GOLD_DROP_RANGE 15
#define GOLD_NORMALIZATION 1000.0f
#define HEALTH_POTION_COOLDOWN 60
#define HEALTH_POTION_HEAL 1000
#define HEAVY_DAMAGE 15
#define HEAVY_HEALTH 15
#define HEAVY_RANGE 1.5f
#define HEAVY_SPEED 0.2f
#define LIGHT_DAMAGE 3
#define LIGHT_HEALTH 8
#define LIGHT_RANGE 2.0f
#define LIGHT_SPEED 0.8f
#define MAGE_DAMAGE 8
#define MAGE_HEALTH 10
#define MAGE_RANGE 6.0f
#define MAGE_SPEED 0.3f
#define MANA_POTION_COOLDOWN 60
#define MANA_POTION_RESTORE 1000
#define MANA_REGEN_RATE 15
#define MAX_MONSTERS 200
#define MONSTER_ATTACK_COOLDOWN 30
#define MONSTER_BASE_DAMAGE 5
#define MONSTER_BASE_HEALTH 15
#define MONSTER_DETECTION_RANGE 8.0f
#define MONSTER_MOVEMENT_COOLDOWN 5
#define MONSTER_WANDER_CHANCE 20
#define MONSTER_WANDER_COOLDOWN 10
#define MONSTERS_TO_SPAWN ((int)(MAP_WIDTH * MAP_HEIGHT * 0.02f))
#define PLAYER_BASE_DAMAGE 10
#define PLAYER_MAX_HEALTH 100
#define PLAYER_MAX_MANA 50
#define SAFE_SPAWN_RADIUS 15.0f
#define GRID_OBS_SIZE (GRID_SIZE * GRID_SIZE)
#define GRID_SIZE 10
#define HEALTH_POTION_DROP_CHANCE 70
#define ITEM_PICKUP_DISTANCE 1.0f
#define ITEM_SCAN_DISTANCE 5.0f
#define MAP_SPAWN_BORDER 10
#define MAP_SPAWN_BORDER_OFFSET 20
#define MAX_BLIZZARD_AREAS 10
#define MAX_EPISODE_LENGTH 2500
#define MAX_INVENTORY_SIZE 20
#define MAX_ITEMS 100
#define MAX_PROJECTILES 50
#define MELEE_RANGE 1.0f
#define MOVEMENT_STEP 1.0f
#define OBS_SIZE (PLAYER_OBS_SIZE + GRID_OBS_SIZE)
#define PLAYER_OBS_SIZE 17
#define PROJECTILE_FIREBALL 0
#define PROJECTILE_LIFETIME 60
#define PROJECTILE_SPEED 3.0f
#define RIFT_COMPLETION_THRESHOLD 0.8f
#define SPAWN_AREA_MARGIN 20
#define SPAWN_BORDER_SIZE 10
#define SPAWN_CHECK_DISTANCE 8.0f
#define TWO_PI_MULTIPLIER 2.0f
#define VENDOR_HEALTH_POTION_PRICE 20
#define VENDOR_INTERACTION_DISTANCE 2.0f
#define VENDOR_MANA_POTION_PRICE 15
#define VENDOR_STOCK_AMOUNT 10
#define WALL_CLUSTER_COUNT 20
#define WALL_CLUSTER_DENSITY 3
#define WALL_CLUSTER_SIZE 2
#define ZERO_VALUE 0.0f

typedef struct GameConfig {
    uint16_t max_monsters;
    uint16_t max_items;
    float monster_spawn_rate;
    float item_drop_rate;
    uint8_t player_start_health;
    uint8_t player_start_mana;
    uint16_t starting_gold;
    float monster_move_speed;
    uint16_t episode_length_limit;
    float completion_reward;
    float monster_kill_reward;
    float item_pickup_reward;
    float death_penalty;
} GameConfig;

// ============================================================================
// GAME CONFIGURATION
// ============================================================================

static const GameConfig DEFAULT_CONFIG = {
    .max_monsters = MAX_MONSTERS,
    .max_items = MAX_ITEMS,
    .monster_spawn_rate = 0.8f,
    .item_drop_rate = 0.3f,
    .player_start_health = PLAYER_MAX_HEALTH,
    .player_start_mana = PLAYER_MAX_MANA,
    .starting_gold = 100,
    .monster_move_speed = 0.5f,
    .episode_length_limit = MAX_EPISODE_LENGTH,
    .completion_reward = 1000.0f,
    .monster_kill_reward = 10.0f,
    .item_pickup_reward = 1.0f,
    .death_penalty = 0.0f,
};


typedef struct Player {
    float x, y;
    float prev_x, prev_y;
    float movement_x, movement_y;
    float facing_x, facing_y;
    uint8_t inventory[MAX_INVENTORY_SIZE];
    uint16_t gold;
    uint8_t health, max_health;
    uint8_t mana, max_mana;
    uint8_t damage;
    uint8_t alive;
    uint8_t blizzard_cooldown;
    uint8_t health_potion_cooldown;
    uint8_t mana_potion_cooldown;
    uint8_t mana_regen_timer;
    uint8_t inventory_count;
} Player;

typedef struct Monster {
    float x, y;
    float target_x, target_y;
    float speed;
    float attack_range;
    uint16_t id;
    uint8_t health, max_health;
    uint8_t damage;
    uint8_t type;
    uint8_t alive;
    uint8_t move_cooldown;
    uint8_t attack_cooldown;
} Monster;

typedef struct Boss {
    float x, y;
    uint8_t health, max_health;
    uint8_t damage;
    uint8_t type;
    uint8_t alive;
    uint8_t attack_cooldown;
    uint8_t special_attack_cooldown;
} Boss;

typedef struct Item {
    float x, y;
    uint8_t type;
    uint8_t value;
    uint8_t active;
    uint16_t id;
} Item;

typedef struct Projectile {
    float x, y;
    float vel_x, vel_y;
    uint8_t type;
    uint8_t damage;
    uint8_t lifetime;
    uint8_t active;
} Projectile;

typedef struct BlizzardArea {
    float x, y;
    uint8_t duration;
    uint8_t damage_timer;
    uint8_t active;
} BlizzardArea;

typedef struct Vendor {
    float x, y;
    uint8_t items_for_sale[10];
    uint8_t prices[10];
    uint8_t stock[10];
} Vendor;

typedef struct Log {
    float episode_return;
    float episode_length;
    float monsters_killed;
    float boss_kills;
    float items_collected;
    float gold_earned;
    float damage_dealt;
    float damage_taken;
    float rift_completions;
    float vendor_transactions;
    float blizzards_cast;
    float deaths;
    float completion_rewards;
    float monster_kill_rewards;
    float item_pickup_rewards;
    float death_penalties;
    float n;
} Log;

typedef struct Client {
    float cell_size;
    int width;
    int height;
    Camera2D camera;
} Client;


typedef struct Rift {
    Client* client;
    Log log;
    float* observations;
    float* actions;
    float* rewards;
    unsigned char* terminals;
    
    unsigned char map[MAP_SIZE];
    
    Player player;
    Monster monsters[MAX_MONSTERS];
    Boss boss;
    Item items[MAX_ITEMS];
    Projectile projectiles[MAX_PROJECTILES];
    BlizzardArea blizzard_areas[MAX_BLIZZARD_AREAS];
    Vendor vendor;
    
    GameConfig config;
    uint32_t tick;
    uint8_t current_phase;
    
    uint16_t monsters_spawned;
    uint16_t monsters_killed;
    uint8_t boss_spawned;
    uint8_t rift_completed;
    
    uint8_t elites_spawned;
    uint8_t max_elites;
    
    uint16_t next_monster_id;
    uint16_t next_item_id;
    
    uint16_t episode_length;
    float episode_return;
    float step_reward;
    
    uint16_t episode_monsters_killed;
    uint8_t episode_boss_kills;
    uint16_t episode_items_collected;
    uint16_t episode_gold_earned;
    uint16_t episode_damage_dealt;
    uint16_t episode_damage_taken;
    uint8_t episode_rift_completions;
    uint8_t episode_vendor_transactions;
    uint8_t episode_blizzards_cast;
    uint8_t episode_deaths;
    
    float episode_completion_rewards;
    float episode_monster_kill_rewards;
    float episode_item_pickup_rewards;
    float episode_death_penalties;
} Rift;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

void init(Rift* env);
void allocate(Rift* env);
void c_close(Rift* env);
void free_allocated(Rift* env);
void c_reset(Rift* env);
void c_step(Rift* env);

void generate_rift_map(Rift* env);
void generate_town_map(Rift* env);

// ============================================================================
// GAME LOGIC FUNCTIONS
// ============================================================================

void spawn_monsters(Rift* env);
void spawn_diverse_pack(Rift* env, float center_x, float center_y, uint8_t pack_size);
uint8_t get_random_monster_type(Rift* env, uint8_t allow_elite);
void update_monsters(Rift* env);
void update_player(Rift* env);
void handle_blizzard(Rift* env);
void handle_blizzard_with_direction(Rift* env, float facing_x, float facing_y);
void update_blizzard_areas(Rift* env);
void handle_item_pickup(Rift* env);
void handle_vendor_interaction(Rift* env);
void spawn_boss(Rift* env);
void drop_loot(Rift* env, float x, float y);

void spawn_projectile(Rift* env, float start_x, float start_y, float target_x, float target_y, uint8_t type, uint8_t damage);
void update_projectiles(Rift* env);

void compute_observations(Rift* env);

void c_render(Rift* env);

// ============================================================================
// STATIC INLINE UTILITY FUNCTIONS
// ============================================================================
void render_rift(Rift* env);
void render_town(Rift* env);
void c_close_client(Client* client);

static void render_projectiles(Rift* env, float cell_size);
static void render_blizzard_areas(Rift* env, float cell_size);

static void add_log(Rift* env);

static inline float distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

static inline float distance_squared(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return dx * dx + dy * dy;
}

static inline uint8_t clamp_uint8(int value, uint8_t min_val, uint8_t max_val) {
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return (uint8_t)value;
}

static inline float clampf(float value, float min_val, float max_val) {
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

static inline float normalize_to_unit(float value, float max_value) {
    return value / max_value;
}

static const float BLIZZARD_RADIUS_SQUARED = BLIZZARD_RADIUS * BLIZZARD_RADIUS;

// ============================================================================
// CORE GAME FUNCTIONS
// ============================================================================

void init(Rift* env) {
    env->tick = 0;
    env->config = DEFAULT_CONFIG;
    env->current_phase = PHASE_RIFT;
}

void allocate(Rift* env) {
    env->tick = 0;
    env->episode_return = 0.0f;
    env->client = NULL;
    
    memset(&env->log, 0, sizeof(Log));
    
    init(env);
}

void c_close(Rift* env) {
}

void free_allocated(Rift* env) {
    free(env->actions);
    free(env->observations);
    free(env->terminals);
    free(env->rewards);
    c_close(env);
}

void generate_rift_map(Rift* env) {
    memset(env->map, CELL_FLOOR, MAP_SIZE);
}

void generate_town_map(Rift* env) {
    memset(env->map, CELL_FLOOR, MAP_SIZE);
    
    env->vendor.x = MAP_WIDTH / 4;
    env->vendor.y = MAP_HEIGHT / 2;
    
    env->vendor.items_for_sale[0] = ITEM_HEALTH_POTION;
    env->vendor.prices[0] = VENDOR_HEALTH_POTION_PRICE;
    env->vendor.stock[0] = 10;
    
    env->vendor.items_for_sale[1] = ITEM_MANA_POTION;
    env->vendor.prices[1] = VENDOR_MANA_POTION_PRICE;
    env->vendor.stock[1] = VENDOR_STOCK_AMOUNT;
}

void spawn_monster_pack(Rift* env, float center_x, float center_y, uint8_t pack_size, uint8_t monster_type) {
    for (uint8_t p = 0; p < pack_size; p++) {
        for (uint16_t i = 0; i < MAX_MONSTERS; i++) {
            if (!env->monsters[i].alive && env->monsters_spawned < MONSTERS_TO_SPAWN) {
                float angle = ((float)rand() / RAND_MAX) * 2.0f * PI;
                float radius = 1.0f + ((float)rand() / RAND_MAX) * 2.0f;
                float spawn_x = center_x + cos(angle) * radius;
                float spawn_y = center_y + sin(angle) * radius;
                
                if (spawn_x < 1) spawn_x = 1;
                if (spawn_x >= MAP_WIDTH - 1) spawn_x = MAP_WIDTH - 2;
                if (spawn_y < 1) spawn_y = 1;
                if (spawn_y >= MAP_HEIGHT - 1) spawn_y = MAP_HEIGHT - 2;
                
                env->monsters[i].x = spawn_x;
                env->monsters[i].y = spawn_y;
                env->monsters[i].type = monster_type;
                env->monsters[i].alive = 1;
                env->monsters[i].move_cooldown = 0;
                env->monsters[i].attack_cooldown = 0;
                env->monsters[i].target_x = env->monsters[i].x;
                env->monsters[i].target_y = env->monsters[i].y;
                env->monsters[i].id = env->next_monster_id++;
                
                switch (monster_type) {
                    case MONSTER_MAGE:
                        env->monsters[i].health = MAGE_HEALTH;
                        env->monsters[i].max_health = MAGE_HEALTH;
                        env->monsters[i].damage = MAGE_DAMAGE;
                        env->monsters[i].speed = MAGE_SPEED;
                        env->monsters[i].attack_range = MAGE_RANGE;
                        break;
                    case MONSTER_HEAVY_MELEE:
                        env->monsters[i].health = HEAVY_HEALTH;
                        env->monsters[i].max_health = HEAVY_HEALTH;
                        env->monsters[i].damage = HEAVY_DAMAGE;
                        env->monsters[i].speed = HEAVY_SPEED;
                        env->monsters[i].attack_range = HEAVY_RANGE;
                        break;
                    case MONSTER_LIGHT:
                        env->monsters[i].health = LIGHT_HEALTH;
                        env->monsters[i].max_health = LIGHT_HEALTH;
                        env->monsters[i].damage = LIGHT_DAMAGE;
                        env->monsters[i].speed = LIGHT_SPEED;
                        env->monsters[i].attack_range = LIGHT_RANGE;
                        break;
                    case MONSTER_ELITE:
                        env->monsters[i].health = ELITE_HEALTH;
                        env->monsters[i].max_health = ELITE_HEALTH;
                        env->monsters[i].damage = ELITE_DAMAGE;
                        env->monsters[i].speed = ELITE_SPEED;
                        env->monsters[i].attack_range = ELITE_RANGE;
                        break;
                    default: // MONSTER_ZOMBIE
                        env->monsters[i].health = MONSTER_BASE_HEALTH;
                        env->monsters[i].max_health = MONSTER_BASE_HEALTH;
                        env->monsters[i].damage = MONSTER_BASE_DAMAGE;
                        env->monsters[i].speed = 0.5f;
                        env->monsters[i].attack_range = 2.0f;
                        break;
                }
                
                env->monsters_spawned++;
                break;
            }
        }
    }
}

uint8_t get_random_monster_type(Rift* env, uint8_t allow_elite) {
    uint8_t type_roll = rand() % 100;
    
    if (allow_elite && type_roll < 10 && env->elites_spawned < env->max_elites) {
        return MONSTER_ELITE;
    } else if (type_roll < 30) {
        return MONSTER_HEAVY_MELEE;  // 30% chance
    } else if (type_roll < 55) {
        return MONSTER_MAGE;  // 25% chance
    } else if (type_roll < 80) {
        return MONSTER_LIGHT;  // 25% chance
    } else {
        return MONSTER_ZOMBIE;  // 20% chance
    }
}

void spawn_diverse_pack(Rift* env, float center_x, float center_y, uint8_t pack_size) {
    for (uint8_t p = 0; p < pack_size; p++) {
        for (uint16_t i = 0; i < MAX_MONSTERS; i++) {
            if (!env->monsters[i].alive && env->monsters_spawned < MONSTERS_TO_SPAWN) {
                float angle = ((float)rand() / RAND_MAX) * 2.0f * PI;
                float radius = 1.0f + ((float)rand() / RAND_MAX) * 2.0f;
                float spawn_x = center_x + cos(angle) * radius;
                float spawn_y = center_y + sin(angle) * radius;
                
                if (spawn_x < 1) spawn_x = 1;
                if (spawn_x >= MAP_WIDTH - 1) spawn_x = MAP_WIDTH - 2;
                if (spawn_y < 1) spawn_y = 1;
                if (spawn_y >= MAP_HEIGHT - 1) spawn_y = MAP_HEIGHT - 2;
                
                uint8_t monster_type = get_random_monster_type(env, 1);
                if (monster_type == MONSTER_ELITE) {
                    env->elites_spawned++;
                }
                
                env->monsters[i].x = spawn_x;
                env->monsters[i].y = spawn_y;
                env->monsters[i].type = monster_type;
                env->monsters[i].alive = 1;
                env->monsters[i].move_cooldown = 0;
                env->monsters[i].attack_cooldown = 0;
                env->monsters[i].target_x = env->monsters[i].x;
                env->monsters[i].target_y = env->monsters[i].y;
                env->monsters[i].id = env->next_monster_id++;
                
                switch (monster_type) {
                    case MONSTER_MAGE:
                        env->monsters[i].health = MAGE_HEALTH;
                        env->monsters[i].max_health = MAGE_HEALTH;
                        env->monsters[i].damage = (int)(MAGE_DAMAGE * 0.75f);
                        env->monsters[i].speed = MAGE_SPEED;
                        env->monsters[i].attack_range = MAGE_RANGE;
                        break;
                    case MONSTER_HEAVY_MELEE:
                        env->monsters[i].health = HEAVY_HEALTH;
                        env->monsters[i].max_health = HEAVY_HEALTH;
                        env->monsters[i].damage = (int)(HEAVY_DAMAGE * 0.75f);
                        env->monsters[i].speed = HEAVY_SPEED;
                        env->monsters[i].attack_range = HEAVY_RANGE;
                        break;
                    case MONSTER_LIGHT:
                        env->monsters[i].health = LIGHT_HEALTH;
                        env->monsters[i].max_health = LIGHT_HEALTH;
                        env->monsters[i].damage = (int)(LIGHT_DAMAGE * 0.75f);
                        env->monsters[i].speed = LIGHT_SPEED;
                        env->monsters[i].attack_range = LIGHT_RANGE;
                        break;
                    case MONSTER_ELITE:
                        env->monsters[i].health = ELITE_HEALTH;
                        env->monsters[i].max_health = ELITE_HEALTH;
                        env->monsters[i].damage = (int)(ELITE_DAMAGE * 0.75f);
                        env->monsters[i].speed = ELITE_SPEED;
                        env->monsters[i].attack_range = ELITE_RANGE;
                        break;
                    default: // MONSTER_ZOMBIE
                        env->monsters[i].health = MONSTER_BASE_HEALTH;
                        env->monsters[i].max_health = MONSTER_BASE_HEALTH;
                        env->monsters[i].damage = (int)(MONSTER_BASE_DAMAGE * 0.75f);
                        env->monsters[i].speed = 0.5f;
                        env->monsters[i].attack_range = 2.0f;
                        break;
                }
                
                env->monsters_spawned++;
                break;
            }
        }
    }
}

void spawn_monsters(Rift* env) {
    if (env->current_phase != PHASE_RIFT) return;
    if (env->monsters_spawned >= MONSTERS_TO_SPAWN) return;
    
    if ((float)rand() / RAND_MAX < env->config.monster_spawn_rate) {
        float spawn_x = 3.0f + ((float)rand() / RAND_MAX) * (MAP_WIDTH - 6.0f);
        float spawn_y = 3.0f + ((float)rand() / RAND_MAX) * (MAP_HEIGHT - 6.0f);
        
        uint8_t pack_size = 2 + rand() % 3;
        
        spawn_diverse_pack(env, spawn_x, spawn_y, pack_size);
    }
}

void update_monsters(Rift* env) {
    if (env->current_phase != PHASE_RIFT) return;
    
    for (uint16_t i = 0; i < MAX_MONSTERS; i++) {
        if (env->monsters[i].alive) {
            Monster* monster = &env->monsters[i];
            
            if (monster->move_cooldown > 0) {
                monster->move_cooldown--;
            }
            if (monster->attack_cooldown > 0) {
                monster->attack_cooldown--;
            }
            
            if (monster->move_cooldown > 0) {
                continue;
            }
            
            float dist_to_player = distance(env->player.x, env->player.y, monster->x, monster->y);
            
            if (dist_to_player <= MONSTER_DETECTION_RANGE) {
                float dx = env->player.x - monster->x;
                float dy = env->player.y - monster->y;
                
                if (dist_to_player > SPAWN_CHECK_DISTANCE) {
                    dx /= dist_to_player;
                    dy /= dist_to_player;
                    
                    float new_x = monster->x + dx * monster->speed;
                    float new_y = monster->y + dy * monster->speed;
                    
                    if (new_x >= 0 && new_x < MAP_WIDTH && new_y >= 0 && new_y < MAP_HEIGHT) {
                        monster->x = new_x;
                        monster->y = new_y;
                    }
                    
                    monster->move_cooldown = MONSTER_MOVEMENT_COOLDOWN;
                } else {
                    if (env->player.alive && monster->attack_cooldown == 0 && dist_to_player <= monster->attack_range) {
                        spawn_projectile(env, monster->x, monster->y, env->player.x, env->player.y, 
                                       PROJECTILE_FIREBALL, monster->damage);
                        monster->attack_cooldown = MONSTER_ATTACK_COOLDOWN;
                    }
                }
            } else {
                if (rand() % MONSTER_WANDER_CHANCE == 0) {
                    float random_angle = ((float)rand() / RAND_MAX) * FULL_CIRCLE_MULTIPLIER * PI;
                    float new_x = monster->x + cos(random_angle) * monster->speed;
                    float new_y = monster->y + sin(random_angle) * monster->speed;
                    
                    if (new_x >= 0 && new_x < MAP_WIDTH && new_y >= 0 && new_y < MAP_HEIGHT) {
                        monster->x = new_x;
                        monster->y = new_y;
                    }
                    monster->move_cooldown = MONSTER_WANDER_COOLDOWN;
                }
            }
        }
    }
}

void update_player(Rift* env) {
    int action = (int)env->actions[0];
    
    if (env->player.blizzard_cooldown > 0) {
        env->player.blizzard_cooldown--;
    }
    if (env->player.health_potion_cooldown > 0) {
        env->player.health_potion_cooldown--;
    }
    if (env->player.mana_potion_cooldown > 0) {
        env->player.mana_potion_cooldown--;
    }
    
    env->player.mana_regen_timer++;
    if (env->player.mana_regen_timer >= MANA_REGEN_RATE) {
        env->player.mana_regen_timer = 0;
        if (env->player.mana < env->player.max_mana) {
            env->player.mana++;
        }
    }
    
    float new_x = env->player.x;
    float new_y = env->player.y;
    
    switch (action) {
        case ACTION_MOVE_UP:
            new_y -= 1.0f;
            env->player.facing_x = 0.0f;
            env->player.facing_y = -1.0f;
            break;
        case ACTION_MOVE_DOWN:
            new_y += 1.0f;
            env->player.facing_x = 0.0f;
            env->player.facing_y = 1.0f;
            break;
        case ACTION_MOVE_LEFT:
            new_x -= 1.0f;
            env->player.facing_x = -1.0f;
            env->player.facing_y = 0.0f;
            break;
        case ACTION_MOVE_RIGHT:
            new_x += 1.0f;
            env->player.facing_x = 1.0f;
            env->player.facing_y = 0.0f;
            break;
        case ACTION_MOVE_UP_LEFT:
            new_x -= 1.0f;
            new_y -= 1.0f;
            env->player.facing_x = -0.707f;
            env->player.facing_y = -0.707f;
            break;
        case ACTION_MOVE_UP_RIGHT:
            new_x += 1.0f;
            new_y -= 1.0f;
            env->player.facing_x = 0.707f;
            env->player.facing_y = -0.707f;
            break;
        case ACTION_MOVE_DOWN_LEFT:
            new_x -= 1.0f;
            new_y += 1.0f;
            env->player.facing_x = -0.707f;
            env->player.facing_y = 0.707f;
            break;
        case ACTION_MOVE_DOWN_RIGHT:
            new_x += 1.0f;
            new_y += 1.0f;
            env->player.facing_x = 0.707f;
            env->player.facing_y = 0.707f;
            break;
    }
    
    if (new_x >= 0 && new_x < MAP_WIDTH && new_y >= 0 && new_y < MAP_HEIGHT) {
        env->player.x = new_x;
        env->player.y = new_y;
    }
    
    
    switch (action) {
        case ACTION_BLIZZARD:
            if (env->player.blizzard_cooldown == 0 && env->player.mana >= BLIZZARD_MANA_COST) {
                handle_blizzard(env);
            }
            break;
        case ACTION_BLIZZARD_UP:
            if (env->player.blizzard_cooldown == 0 && env->player.mana >= BLIZZARD_MANA_COST) {
                handle_blizzard_with_direction(env, 0.0f, -1.0f);
            }
            break;
        case ACTION_BLIZZARD_DOWN:
            if (env->player.blizzard_cooldown == 0 && env->player.mana >= BLIZZARD_MANA_COST) {
                handle_blizzard_with_direction(env, 0.0f, 1.0f);
            }
            break;
        case ACTION_BLIZZARD_LEFT:
            if (env->player.blizzard_cooldown == 0 && env->player.mana >= BLIZZARD_MANA_COST) {
                handle_blizzard_with_direction(env, -1.0f, 0.0f);
            }
            break;
        case ACTION_BLIZZARD_RIGHT:
            if (env->player.blizzard_cooldown == 0 && env->player.mana >= BLIZZARD_MANA_COST) {
                handle_blizzard_with_direction(env, 1.0f, 0.0f);
            }
            break;
        case ACTION_BLIZZARD_UP_LEFT:
            if (env->player.blizzard_cooldown == 0 && env->player.mana >= BLIZZARD_MANA_COST) {
                handle_blizzard_with_direction(env, -0.707f, -0.707f);
            }
            break;
        case ACTION_BLIZZARD_UP_RIGHT:
            if (env->player.blizzard_cooldown == 0 && env->player.mana >= BLIZZARD_MANA_COST) {
                handle_blizzard_with_direction(env, 0.707f, -0.707f);
            }
            break;
        case ACTION_BLIZZARD_DOWN_LEFT:
            if (env->player.blizzard_cooldown == 0 && env->player.mana >= BLIZZARD_MANA_COST) {
                handle_blizzard_with_direction(env, -0.707f, 0.707f);
            }
            break;
        case ACTION_BLIZZARD_DOWN_RIGHT:
            if (env->player.blizzard_cooldown == 0 && env->player.mana >= BLIZZARD_MANA_COST) {
                handle_blizzard_with_direction(env, 0.707f, 0.707f);
            }
            break;
        case ACTION_USE_HEALTH_POTION:
            if (env->player.health_potion_cooldown == 0 && env->player.health < env->player.max_health) {
                env->player.health = clamp_uint8(env->player.health + HEALTH_POTION_HEAL, 0, env->player.max_health);
                env->player.health_potion_cooldown = HEALTH_POTION_COOLDOWN;
            }
            break;
        case ACTION_USE_MANA_POTION:
            if (env->player.mana_potion_cooldown == 0 && env->player.mana < env->player.max_mana) {
                env->player.mana = clamp_uint8(env->player.mana + MANA_POTION_RESTORE, 0, env->player.max_mana);
                env->player.mana_potion_cooldown = MANA_POTION_COOLDOWN;
            }
            break;
        case ACTION_INTERACT:
            if (env->current_phase == PHASE_TOWN) {
                handle_vendor_interaction(env);
            }
            break;
    }
    
    if (env->current_phase == PHASE_RIFT) {
        handle_item_pickup(env);
    }
}


void handle_blizzard(Rift* env) {
    handle_blizzard_with_direction(env, env->player.facing_x, env->player.facing_y);
}

void handle_blizzard_with_direction(Rift* env, float facing_x, float facing_y) {
    if (env->current_phase != PHASE_RIFT) return;
    
    env->player.mana -= BLIZZARD_MANA_COST;
    env->player.blizzard_cooldown = BLIZZARD_ACTIVATION_COOLDOWN;
    env->episode_blizzards_cast++;
    
    // Update player facing direction
    env->player.facing_x = facing_x;
    env->player.facing_y = facing_y;
    
    float blizzard_x = env->player.x + facing_x * 2.5f;
    float blizzard_y = env->player.y + facing_y * 2.5f;
    
    // Instant initial hit damage
    for (uint16_t j = 0; j < MAX_MONSTERS; j++) {
        if (env->monsters[j].alive) {
            float dist_sq = distance_squared(blizzard_x, blizzard_y, 
                                            env->monsters[j].x, env->monsters[j].y);
            if (dist_sq <= BLIZZARD_RADIUS_SQUARED) {
                uint8_t damage_amount = (env->monsters[j].health <= BLIZZARD_DAMAGE) ? 
                                       env->monsters[j].health : BLIZZARD_DAMAGE;
                if (env->monsters[j].health <= BLIZZARD_DAMAGE) {
                    env->monsters[j].health = 0;
                    env->monsters[j].alive = 0;
                    env->monsters_killed++;
                    env->episode_monsters_killed++;
                    env->step_reward += env->config.monster_kill_reward;
                    env->episode_return += env->config.monster_kill_reward;
                    env->episode_monster_kill_rewards += env->config.monster_kill_reward;
                    
                    if ((float)rand() / RAND_MAX < env->config.item_drop_rate) {
                        drop_loot(env, env->monsters[j].x, env->monsters[j].y);
                    }
                } else {
                    env->monsters[j].health -= damage_amount;
                }
                env->episode_damage_dealt += damage_amount;
            }
        }
    }
    
    if (env->boss.alive) {
        float dist_sq = distance_squared(blizzard_x, blizzard_y, env->boss.x, env->boss.y);
        if (dist_sq <= BLIZZARD_RADIUS_SQUARED) {
            uint8_t damage_amount = (env->boss.health <= BLIZZARD_DAMAGE) ? 
                                   env->boss.health : BLIZZARD_DAMAGE;
            if (env->boss.health <= BLIZZARD_DAMAGE) {
                env->boss.health = 0;
                env->boss.alive = 0;
                env->episode_boss_kills++;
            } else {
                env->boss.health -= damage_amount;
            }
            env->episode_damage_dealt += damage_amount;
        }
    }
    
    // Create the blizzard area for continued DOT
    for (uint8_t i = 0; i < MAX_BLIZZARD_AREAS; i++) {
        if (!env->blizzard_areas[i].active) {
            env->blizzard_areas[i].x = blizzard_x;
            env->blizzard_areas[i].y = blizzard_y;
            env->blizzard_areas[i].duration = BLIZZARD_DURATION;
            env->blizzard_areas[i].damage_timer = 0;
            env->blizzard_areas[i].active = 1;
            break;
        }
    }
    
    // Check for boss spawn after initial damage
    float completion = (float)env->monsters_killed / MONSTERS_TO_SPAWN;
    if (completion >= RIFT_COMPLETION_THRESHOLD && !env->boss_spawned) {
        spawn_boss(env);
    }
}

void update_blizzard_areas(Rift* env) {
    for (uint8_t i = 0; i < MAX_BLIZZARD_AREAS; i++) {
        if (env->blizzard_areas[i].active) {
            BlizzardArea* area = &env->blizzard_areas[i];
            
            area->duration--;
            area->damage_timer++;
            
            if (area->duration <= 0) {
                area->active = 0;
                continue;
            }
            
            if (area->damage_timer >= BLIZZARD_DAMAGE_INTERVAL) {
                area->damage_timer = 0;
                
                for (uint16_t j = 0; j < MAX_MONSTERS; j++) {
                    if (env->monsters[j].alive) {
                        float dist_sq = distance_squared(area->x, area->y, 
                                                        env->monsters[j].x, env->monsters[j].y);
                        if (dist_sq <= BLIZZARD_RADIUS_SQUARED) {
                            uint8_t damage_amount = (env->monsters[j].health <= BLIZZARD_DAMAGE) ? 
                                                   env->monsters[j].health : BLIZZARD_DAMAGE;
                            if (env->monsters[j].health <= BLIZZARD_DAMAGE) {
                                env->monsters[j].health = 0;
                                env->monsters[j].alive = 0;
                                env->monsters_killed++;
                                env->episode_monsters_killed++;
                                env->step_reward += env->config.monster_kill_reward;
                                env->episode_return += env->config.monster_kill_reward;
                                env->episode_monster_kill_rewards += env->config.monster_kill_reward;
                                
                                if ((float)rand() / RAND_MAX < env->config.item_drop_rate) {
                                    drop_loot(env, env->monsters[j].x, env->monsters[j].y);
                                }
                            } else {
                                env->monsters[j].health -= damage_amount;
                            }
                            env->episode_damage_dealt += damage_amount;
                        }
                    }
                }
                
                if (env->boss.alive) {
                    float dist_sq = distance_squared(area->x, area->y, env->boss.x, env->boss.y);
                    if (dist_sq <= BLIZZARD_RADIUS_SQUARED) {
                        uint8_t damage_amount = (env->boss.health <= BLIZZARD_DAMAGE) ? 
                                               env->boss.health : BLIZZARD_DAMAGE;
                        if (env->boss.health <= BLIZZARD_DAMAGE) {
                            env->boss.health = 0;
                            env->boss.alive = 0;
                            env->episode_boss_kills++;
                        } else {
                            env->boss.health -= damage_amount;
                        }
                        env->episode_damage_dealt += damage_amount;
                    }
                }
            }
            
            float completion = (float)env->monsters_killed / MONSTERS_TO_SPAWN;
            if (completion >= RIFT_COMPLETION_THRESHOLD && !env->boss_spawned) {
                spawn_boss(env);
            }
        }
    }
}

void spawn_boss(Rift* env) {
    env->boss.x = MAP_WIDTH / 2;
    env->boss.y = MAP_HEIGHT / 2;
    env->boss.type = BOSS_RIFT_GUARDIAN;
    env->boss.health = BOSS_BASE_HEALTH;
    env->boss.max_health = BOSS_BASE_HEALTH;
    env->boss.damage = BOSS_BASE_DAMAGE;
    env->boss.alive = 1;
    env->boss.attack_cooldown = 0;
    env->boss.special_attack_cooldown = 0;
    env->boss_spawned = 1;
    
    for (uint16_t i = 0; i < MAX_MONSTERS; i++) {
        env->monsters[i].alive = 0;
    }
}

void drop_loot(Rift* env, float x, float y) {
    for (uint8_t i = 0; i < MAX_ITEMS; i++) {
        if (!env->items[i].active) {
            env->items[i].x = (float)((int)x) + 0.5f;
            env->items[i].y = (float)((int)y) + 0.5f;
            env->items[i].active = 1;
            env->items[i].id = env->next_item_id++;
            
            uint8_t rand_val = rand() % 100;
            if (rand_val < GOLD_DROP_CHANCE) {
                env->items[i].type = ITEM_GOLD;
                env->items[i].value = GOLD_DROP_MIN + rand() % GOLD_DROP_RANGE;
            } else if (rand_val < HEALTH_POTION_DROP_CHANCE) {
                env->items[i].type = ITEM_HEALTH_POTION;
                env->items[i].value = 1;
            } else {
                env->items[i].type = ITEM_MANA_POTION;
                env->items[i].value = 1;
            }
            break;
        }
    }
}


void spawn_projectile(Rift* env, float start_x, float start_y, float target_x, float target_y, uint8_t type, uint8_t damage) {
    for (uint8_t i = 0; i < MAX_PROJECTILES; i++) {
        if (!env->projectiles[i].active) {
            env->projectiles[i].x = start_x;
            env->projectiles[i].y = start_y;
            env->projectiles[i].type = type;
            env->projectiles[i].damage = damage;
            env->projectiles[i].lifetime = PROJECTILE_LIFETIME;
            env->projectiles[i].active = 1;
            
            float dx = target_x - start_x;
            float dy = target_y - start_y;
            float dist = sqrtf(dx * dx + dy * dy);
            
            if (dist > 0) {
                env->projectiles[i].vel_x = (dx / dist) * PROJECTILE_SPEED;
                env->projectiles[i].vel_y = (dy / dist) * PROJECTILE_SPEED;
            } else {
                env->projectiles[i].vel_x = 0;
                env->projectiles[i].vel_y = 0;
            }
            break;
        }
    }
}

void update_projectiles(Rift* env) {
    for (uint8_t i = 0; i < MAX_PROJECTILES; i++) {
        if (env->projectiles[i].active) {
            Projectile* proj = &env->projectiles[i];
            
            proj->x += proj->vel_x;
            proj->y += proj->vel_y;
            proj->lifetime--;
            
            if (proj->lifetime <= 0) {
                proj->active = 0;
                continue;
            }
            
            int proj_x = (int)proj->x;
            int proj_y = (int)proj->y;
            if (proj_x < 0 || proj_x >= MAP_WIDTH || proj_y < 0 || proj_y >= MAP_HEIGHT) {
                proj->active = 0;
                continue;
            }
            
            float dist_to_player = distance(env->player.x, env->player.y, proj->x, proj->y);
            if (dist_to_player <= 0.8f && env->player.alive) {
                env->player.health -= proj->damage;
                env->episode_damage_taken += proj->damage;
                
                if (env->player.health <= 0) {
                    env->player.alive = 0;
                    env->player.health = 0;
                    env->step_reward += env->config.death_penalty;
                    env->episode_return += env->config.death_penalty;
                    env->episode_death_penalties += env->config.death_penalty;
                    env->episode_deaths++;
                }
                
                proj->active = 0;
            }
        }
    }
}


void handle_item_pickup(Rift* env) {
    for (uint8_t i = 0; i < MAX_ITEMS; i++) {
        if (env->items[i].active) {
            float dist = distance(env->player.x, env->player.y, 
                                env->items[i].x, env->items[i].y);
            if (dist <= 1.0f) {
                if (env->items[i].type == ITEM_GOLD) {
                    env->player.gold += env->items[i].value;
                    env->episode_gold_earned += env->items[i].value;
                } else if (env->player.inventory_count < MAX_INVENTORY_SIZE) {
                    env->player.inventory[env->player.inventory_count] = env->items[i].type;
                    env->player.inventory_count++;
                }
                
                env->items[i].active = 0;
                env->episode_items_collected++;
                env->step_reward += env->config.item_pickup_reward;
                env->episode_return += env->config.item_pickup_reward;
                env->episode_item_pickup_rewards += env->config.item_pickup_reward;
                break;
            }
        }
    }
}

void handle_vendor_interaction(Rift* env) {
    float dist = distance(env->player.x, env->player.y, env->vendor.x, env->vendor.y);
    if (dist <= 2.0f) {
        for (int i = 0; i < 10; i++) {
            if (env->vendor.stock[i] > 0 && env->player.gold >= env->vendor.prices[i]) {
                if (env->player.inventory_count < MAX_INVENTORY_SIZE) {
                    env->player.gold -= env->vendor.prices[i];
                    env->vendor.stock[i]--;
                    env->player.inventory[env->player.inventory_count] = env->vendor.items_for_sale[i];
                    env->player.inventory_count++;
                    env->episode_vendor_transactions++;
                    break;
                }
            }
        }
    }
}


void compute_observations(Rift* env) {
    uint8_t obs_idx = 0;
    
    env->observations[obs_idx++] = (float)env->player.health / env->player.max_health;
    env->observations[obs_idx++] = (float)env->player.mana / env->player.max_mana;
    env->observations[obs_idx++] = env->player.x / MAP_WIDTH;
    env->observations[obs_idx++] = env->player.y / MAP_HEIGHT;
    env->observations[obs_idx++] = (float)env->player.gold / GOLD_NORMALIZATION;
    env->observations[obs_idx++] = env->player.facing_x;
    env->observations[obs_idx++] = env->player.facing_y;
    env->observations[obs_idx++] = env->player.movement_x;
    env->observations[obs_idx++] = env->player.movement_y;
    
    float progress = (float)env->monsters_killed / MONSTERS_TO_SPAWN;
    env->observations[obs_idx++] = progress;
    
    float nearest_enemy_dist = 100.0f;
    int nearby_enemies = 0;
    for (uint16_t i = 0; i < MAX_MONSTERS; i++) {
        if (env->monsters[i].alive) {
            float dist = distance(env->player.x, env->player.y, env->monsters[i].x, env->monsters[i].y);
            if (dist < nearest_enemy_dist) {
                nearest_enemy_dist = dist;
            }
            if (dist <= 8.0f) {
                nearby_enemies++;
            }
        }
    }
    if (env->boss.alive) {
        float boss_dist = distance(env->player.x, env->player.y, env->boss.x, env->boss.y);
        if (boss_dist < nearest_enemy_dist) {
            nearest_enemy_dist = boss_dist;
        }
        if (boss_dist <= 8.0f) {
            nearby_enemies++;
        }
    }
    
    env->observations[obs_idx++] = nearest_enemy_dist / DISTANCE_NORMALIZATION;
    env->observations[obs_idx++] = (float)nearby_enemies / 10.0f;
    
    env->observations[obs_idx++] = env->player.x / MAP_WIDTH;
    env->observations[obs_idx++] = (MAP_WIDTH - 1 - env->player.x) / MAP_WIDTH;
    env->observations[obs_idx++] = env->player.y / MAP_HEIGHT;
    env->observations[obs_idx++] = (MAP_HEIGHT - 1 - env->player.y) / MAP_HEIGHT;
    env->observations[obs_idx++] = env->current_phase == PHASE_TOWN ? 1.0f : 0.0f;
    
    // 10x10 Grid-based observations centered on player
    float grid[GRID_OBS_SIZE];
    int player_grid_center_x = (int)env->player.x;
    int player_grid_center_y = (int)env->player.y;
    
    // Initialize grid with base values
    for (int i = 0; i < GRID_OBS_SIZE; i++) {
        int grid_x = i % GRID_SIZE;
        int grid_y = i / GRID_SIZE;
        int world_x = player_grid_center_x + grid_x - (GRID_SIZE / 2);
        int world_y = player_grid_center_y + grid_y - (GRID_SIZE / 2);
        
        if (world_x < 0 || world_x >= MAP_WIDTH || world_y < 0 || world_y >= MAP_HEIGHT) {
            grid[i] = 0.1f;
        } else {
            grid[i] = 0.05f;
        }
    }
    
    // Place monsters in grid (iterate through monsters once)
    for (uint16_t i = 0; i < MAX_MONSTERS; i++) {
        if (env->monsters[i].alive) {
            int grid_x = (int)env->monsters[i].x - player_grid_center_x + (GRID_SIZE / 2);
            int grid_y = (int)env->monsters[i].y - player_grid_center_y + (GRID_SIZE / 2);
            
            if (grid_x >= 0 && grid_x < GRID_SIZE && grid_y >= 0 && grid_y < GRID_SIZE) {
                int grid_idx = grid_y * GRID_SIZE + grid_x;
                float monster_value = 0.5f + (env->monsters[i].type * 0.1f) + 
                                    ((float)env->monsters[i].health / env->monsters[i].max_health * 0.1f);
                if (monster_value > grid[grid_idx]) {
                    grid[grid_idx] = monster_value;
                }
            }
        }
    }
    
    // Place boss in grid
    if (env->boss.alive && env->current_phase == PHASE_RIFT) {
        int grid_x = (int)env->boss.x - player_grid_center_x + (GRID_SIZE / 2);
        int grid_y = (int)env->boss.y - player_grid_center_y + (GRID_SIZE / 2);
        
        if (grid_x >= 0 && grid_x < GRID_SIZE && grid_y >= 0 && grid_y < GRID_SIZE) {
            int grid_idx = grid_y * GRID_SIZE + grid_x;
            float boss_value = 0.9f + ((float)env->boss.health / env->boss.max_health * 0.1f);
            if (boss_value > grid[grid_idx]) {
                grid[grid_idx] = boss_value;
            }
        }
    }
    
    // Place vendor in grid  
    if (env->current_phase == PHASE_TOWN) {
        int grid_x = (int)env->vendor.x - player_grid_center_x + (GRID_SIZE / 2);
        int grid_y = (int)env->vendor.y - player_grid_center_y + (GRID_SIZE / 2);
        
        if (grid_x >= 0 && grid_x < GRID_SIZE && grid_y >= 0 && grid_y < GRID_SIZE) {
            int grid_idx = grid_y * GRID_SIZE + grid_x;
            if (0.85f > grid[grid_idx]) {
                grid[grid_idx] = 0.85f;
            }
        }
    }
    
    // Place items in grid
    for (uint8_t i = 0; i < MAX_ITEMS; i++) {
        if (env->items[i].active) {
            int grid_x = (int)env->items[i].x - player_grid_center_x + (GRID_SIZE / 2);
            int grid_y = (int)env->items[i].y - player_grid_center_y + (GRID_SIZE / 2);
            
            if (grid_x >= 0 && grid_x < GRID_SIZE && grid_y >= 0 && grid_y < GRID_SIZE) {
                int grid_idx = grid_y * GRID_SIZE + grid_x;
                float item_value = 0.2f + (env->items[i].type * 0.05f);
                if (item_value > grid[grid_idx]) {
                    grid[grid_idx] = item_value;
                }
            }
        }
    }
    
    // Copy grid to observations
    for (int i = 0; i < GRID_OBS_SIZE; i++) {
        env->observations[obs_idx++] = grid[i];
    }
}

// ============================================================================
// LOGGING FUNCTIONS
// ============================================================================

static void add_log(Rift* env) {
    env->log.episode_return += env->episode_return;
    env->log.episode_length += env->episode_length;
    env->log.monsters_killed += env->episode_monsters_killed;
    env->log.boss_kills += env->episode_boss_kills;
    env->log.items_collected += env->episode_items_collected;
    env->log.gold_earned += env->episode_gold_earned;
    env->log.damage_dealt += env->episode_damage_dealt;
    env->log.damage_taken += env->episode_damage_taken;
    env->log.rift_completions += env->episode_rift_completions;
    env->log.vendor_transactions += env->episode_vendor_transactions;
    env->log.blizzards_cast += env->episode_blizzards_cast;
    env->log.deaths += env->episode_deaths;
    env->log.completion_rewards += env->episode_completion_rewards;
    env->log.monster_kill_rewards += env->episode_monster_kill_rewards;
    env->log.item_pickup_rewards += env->episode_item_pickup_rewards;
    env->log.death_penalties += env->episode_death_penalties;
    env->log.n += 1;
}


void c_reset(Rift* env) {
    env->current_phase = PHASE_RIFT;
    generate_rift_map(env);
    
    env->player.x = MAP_WIDTH / 2;
    env->player.y = MAP_HEIGHT / 2;
    env->player.prev_x = env->player.x;
    env->player.prev_y = env->player.y;
    env->player.movement_x = 0.0f;
    env->player.movement_y = 0.0f;
    env->player.health = env->config.player_start_health;
    env->player.max_health = env->config.player_start_health;
    env->player.mana = env->config.player_start_mana;
    env->player.max_mana = env->config.player_start_mana;
    env->player.gold = env->config.starting_gold;
    env->player.damage = PLAYER_BASE_DAMAGE;
    env->player.alive = 1;
    env->player.blizzard_cooldown = 0;
    env->player.health_potion_cooldown = 0;
    env->player.mana_potion_cooldown = 0;
    env->player.mana_regen_timer = 0;
    env->player.inventory_count = 0;
    env->player.facing_x = 0.0f;
    env->player.facing_y = -1.0f;
    memset(env->player.inventory, 0, sizeof(env->player.inventory));
    
    memset(env->monsters, 0, sizeof(env->monsters));
    env->monsters_spawned = 0;
    env->monsters_killed = 0;
    env->next_monster_id = 0;
    env->next_item_id = 0;
    
    env->elites_spawned = 0;
    env->max_elites = 1 + (MAP_WIDTH * MAP_HEIGHT) / 400;
    
    memset(&env->boss, 0, sizeof(Boss));
    env->boss_spawned = 0;
    
    memset(env->items, 0, sizeof(env->items));
    memset(env->projectiles, 0, sizeof(env->projectiles));
    memset(env->blizzard_areas, 0, sizeof(env->blizzard_areas));
    
    memset(&env->vendor, 0, sizeof(Vendor));
    
    env->tick = 0;
    env->episode_length = 0;
    env->episode_return = 0.0f;
    env->step_reward = 0.0f;
    env->rift_completed = 0;
    
    env->rewards[0] = 0.0f;
    env->terminals[0] = 0;
    
    env->episode_monsters_killed = 0;
    env->episode_boss_kills = 0;
    env->episode_items_collected = 0;
    env->episode_gold_earned = 0;
    env->episode_damage_dealt = 0;
    env->episode_damage_taken = 0;
    env->episode_rift_completions = 0;
    env->episode_vendor_transactions = 0;
    env->episode_blizzards_cast = 0;
    env->episode_deaths = 0;
    env->episode_completion_rewards = 0.0f;
    env->episode_monster_kill_rewards = 0.0f;
    env->episode_item_pickup_rewards = 0.0f;
    env->episode_death_penalties = 0.0f;
    
    compute_observations(env);
}


void c_step(Rift* env) {
    env->terminals[0] = 0;
    env->rewards[0] = 0.0f;
    
    env->step_reward = 0.0f;
    
    if (!env->player.alive) {
        env->terminals[0] = 1;
        add_log(env);
        c_reset(env);
        return;
    }
    
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        bool w = IsKeyDown(KEY_W);
        bool s = IsKeyDown(KEY_S);
        bool a = IsKeyDown(KEY_A);
        bool d = IsKeyDown(KEY_D);
        
        if (w && a) {
            env->actions[0] = ACTION_MOVE_UP_LEFT;
        } else if (w && d) {
            env->actions[0] = ACTION_MOVE_UP_RIGHT;
        } else if (s && a) {
            env->actions[0] = ACTION_MOVE_DOWN_LEFT;
        } else if (s && d) {
            env->actions[0] = ACTION_MOVE_DOWN_RIGHT;
        } else if (w) {
            env->actions[0] = ACTION_MOVE_UP;
        } else if (s) {
            env->actions[0] = ACTION_MOVE_DOWN;
        } else if (a) {
            env->actions[0] = ACTION_MOVE_LEFT;
        } else if (d) {
            env->actions[0] = ACTION_MOVE_RIGHT;
        } else if (IsKeyDown(KEY_SPACE)) {
            env->actions[0] = ACTION_BLIZZARD;
        } else if (IsKeyDown(KEY_Q)) {
            env->actions[0] = ACTION_USE_HEALTH_POTION;
        } else if (IsKeyDown(KEY_E)) {
            env->actions[0] = ACTION_USE_MANA_POTION;
        } else if (IsKeyDown(KEY_F)) {
            env->actions[0] = ACTION_INTERACT;
        } else {
            env->actions[0] = ACTION_NOOP;
        }
    }
    
    update_player(env);
    
    // Calculate and store movement vector
    env->player.movement_x = env->player.x - env->player.prev_x;
    env->player.movement_y = env->player.y - env->player.prev_y;
    
    // Update previous position for next frame
    env->player.prev_x = env->player.x;
    env->player.prev_y = env->player.y;
    
    if (env->current_phase == PHASE_RIFT) {
        spawn_monsters(env);
        update_monsters(env);
        update_projectiles(env);
        update_blizzard_areas(env);
        
        if (env->boss.alive) {
            float dist = distance(env->player.x, env->player.y, env->boss.x, env->boss.y);
            if (dist <= 2.0f && env->boss.attack_cooldown == 0) {
                env->player.health -= env->boss.damage;
                env->episode_damage_taken += env->boss.damage;
                env->boss.attack_cooldown = 15;
                
                if (env->player.health <= 0) {
                    env->player.alive = 0;
                    env->player.health = 0;
                    env->step_reward += env->config.death_penalty;
                    env->episode_return += env->config.death_penalty;
                    env->episode_death_penalties += env->config.death_penalty;
                    env->episode_deaths++;
                }
            }
            if (env->boss.attack_cooldown > 0) {
                env->boss.attack_cooldown--;
            }
        }
        
        if (env->boss_spawned && !env->boss.alive && !env->rift_completed) {
            env->rift_completed = 1;
            env->episode_rift_completions++;
            env->step_reward += env->config.completion_reward;
            env->episode_return += env->config.completion_reward;
            env->episode_completion_rewards += env->config.completion_reward;
            
            drop_loot(env, env->boss.x, env->boss.y);
            drop_loot(env, env->boss.x + 1, env->boss.y);
            drop_loot(env, env->boss.x, env->boss.y + 1);
            
            env->terminals[0] = 1;
            add_log(env);
            c_reset(env);
            return;
        }
    }
    
    env->episode_length++;
    
    if (env->episode_length >= env->config.episode_length_limit) {
        env->terminals[0] = 1;
        add_log(env);
        c_reset(env);
        return;
    }
    
    compute_observations(env);
    
    env->rewards[0] = env->step_reward;
    
    env->tick++;
}

// ============================================================================
// RENDERING SYSTEM
// ============================================================================

#define CLIENT_CELL_SIZE 32.0f
#define CLIENT_FPS 60
#define CLIENT_HEIGHT 768
#define CLIENT_WIDTH 1024

static Client* make_client(Rift* env) {
    Client* client = (Client*)calloc(1, sizeof(Client));
    client->cell_size = CLIENT_CELL_SIZE;
    client->width = CLIENT_WIDTH;
    client->height = CLIENT_HEIGHT;
    
    InitWindow(client->width, client->height, "PufferLib Rift");
    SetTargetFPS(CLIENT_FPS);
    
    client->camera.target = (Vector2){ MAP_WIDTH * client->cell_size / 2, MAP_HEIGHT * client->cell_size / 2 };
    client->camera.offset = (Vector2){ client->width / 2, client->height / 2 };
    client->camera.rotation = ZERO_VALUE;
    client->camera.zoom = 1.0f;
    
    return client;
}

void c_close_client(Client* client) {
    CloseWindow();
    free(client);
}

static inline Vector2 grid_to_screen(float grid_x, float grid_y, float cell_size) {
    return (Vector2){ grid_x * cell_size, grid_y * cell_size };
}

#define BG_COLOR_R 40
#define BG_COLOR_G 35
#define BG_COLOR_B 30
#define BORDER_COLOR_RGB 80

static void render_background(Rift* env, float cell_size) {
    Color bg_color = (Color){BG_COLOR_R, BG_COLOR_G, BG_COLOR_B, 255};
    DrawRectangle(0, 0, MAP_WIDTH * cell_size, MAP_HEIGHT * cell_size, bg_color);
    DrawRectangleLines(0, 0, MAP_WIDTH * cell_size, MAP_HEIGHT * cell_size, (Color){BORDER_COLOR_RGB, BORDER_COLOR_RGB, BORDER_COLOR_RGB, 255});
}

#define ARMOR_COLOR_B 130
#define ARMOR_COLOR_R 75
#define PLAYER_COLOR_B 226
#define PLAYER_COLOR_G 43
#define PLAYER_COLOR_R 138
#define SHADOW_ALPHA 100
#define SHADOW_OFFSET 2

static void render_player(Rift* env, float cell_size) {
    Vector2 screen_pos = grid_to_screen(env->player.x, env->player.y, cell_size);
    float half_cell = cell_size * 0.5f;
    screen_pos.x += half_cell;
    screen_pos.y += half_cell;
    
    Color player_color = (Color){PLAYER_COLOR_R, PLAYER_COLOR_G, PLAYER_COLOR_B, 255};
    Color armor_color = (Color){ARMOR_COLOR_R, 0, ARMOR_COLOR_B, 255};
    
    DrawCircle(screen_pos.x + SHADOW_OFFSET, screen_pos.y + SHADOW_OFFSET, cell_size/3, (Color){0, 0, 0, SHADOW_ALPHA});
    DrawCircle(screen_pos.x, screen_pos.y, cell_size/3, player_color);
    DrawCircle(screen_pos.x, screen_pos.y, cell_size/4, armor_color);
}

#define ELITE_COLOR_B 60
#define ELITE_COLOR_G 20
#define ELITE_COLOR_R 220
#define ELITE_SIZE_MULT 1.4f
#define GLOW_ALPHA 100
#define GLOW_AMPLITUDE 0.2f
#define GLOW_BASE 0.8f
#define GLOW_SPEED 0.1f
#define HEALTH_BAR_HEIGHT 2
#define HEALTH_BAR_OFFSET 5
#define HEAVY_COLOR_B 19
#define HEAVY_COLOR_G 69
#define HEAVY_COLOR_R 139
#define HEAVY_SIZE_MULT 1.3f
#define LIGHT_COLOR_G 215
#define LIGHT_COLOR_R 255
#define LIGHT_SIZE_MULT 0.8f
#define MAGE_SIZE_MULT 0.9f
#define MAROON_COLOR_R 128
#define ZOMBIE_COLOR_B 34
#define ZOMBIE_COLOR_G 139
#define ZOMBIE_COLOR_R 34

static void render_monsters(Rift* env, float cell_size) {
    float half_cell = cell_size * 0.5f;
    float quarter_cell = cell_size * 0.25f;
    float sixth_cell = cell_size / 6.0f;
    float third_cell = cell_size / 3.0f;
    
    for (uint16_t i = 0; i < MAX_MONSTERS; i++) {
        if (!env->monsters[i].alive) continue;
        
        Vector2 screen_pos = grid_to_screen(env->monsters[i].x, env->monsters[i].y, cell_size);
        screen_pos.x += half_cell;
        screen_pos.y += half_cell;
        
        Color monster_color;
        float size_multiplier;
        uint8_t monster_type = env->monsters[i].type;
        
        if (monster_type == MONSTER_ZOMBIE) {
            monster_color = (Color){ZOMBIE_COLOR_R, ZOMBIE_COLOR_G, ZOMBIE_COLOR_B, 255};
            size_multiplier = 1.0f;
        } else if (monster_type == MONSTER_MAGE) {
            monster_color = (Color){PLAYER_COLOR_R, PLAYER_COLOR_G, PLAYER_COLOR_B, 255};
            size_multiplier = MAGE_SIZE_MULT;
        } else if (monster_type == MONSTER_HEAVY_MELEE) {
            monster_color = (Color){HEAVY_COLOR_R, HEAVY_COLOR_G, HEAVY_COLOR_B, 255};
            size_multiplier = HEAVY_SIZE_MULT;
        } else if (monster_type == MONSTER_LIGHT) {
            monster_color = (Color){LIGHT_COLOR_R, LIGHT_COLOR_G, 0, 255};
            size_multiplier = LIGHT_SIZE_MULT;
        } else if (monster_type == MONSTER_ELITE) {
            monster_color = (Color){ELITE_COLOR_R, ELITE_COLOR_G, ELITE_COLOR_B, 255};
            size_multiplier = ELITE_SIZE_MULT;
        } else {
            monster_color = (Color){MAROON_COLOR_R, 0, 0, 255};
            size_multiplier = 1.0f;
        }
        
        float scaled_quarter = quarter_cell * size_multiplier;
        float scaled_sixth = sixth_cell * size_multiplier;
        
        DrawCircle(screen_pos.x + 1, screen_pos.y + 1, scaled_quarter, (Color){0, 0, 0, SHADOW_ALPHA});
        DrawCircle(screen_pos.x, screen_pos.y, scaled_quarter, monster_color);
        DrawCircle(screen_pos.x, screen_pos.y, scaled_sixth, (Color){255, 0, 0, 150});
        
        if (monster_type == MONSTER_ELITE) {
            float glow = GLOW_BASE + GLOW_AMPLITUDE * sinf(env->tick * GLOW_SPEED);
            DrawCircleLines(screen_pos.x, screen_pos.y, third_cell * size_multiplier * glow, (Color){255, 255, 255, GLOW_ALPHA});
        }
        
        float health_ratio = (float)env->monsters[i].health / env->monsters[i].max_health;
        uint8_t bar_width = (uint8_t)half_cell;
        uint8_t half_bar_width = bar_width >> 1;
        int16_t bar_x = screen_pos.x - half_bar_width;
        int16_t bar_y = screen_pos.y - third_cell - HEALTH_BAR_OFFSET;
        
        DrawRectangle(bar_x, bar_y, bar_width, HEALTH_BAR_HEIGHT, RED);
        DrawRectangle(bar_x, bar_y, (uint8_t)(bar_width * health_ratio), HEALTH_BAR_HEIGHT, GREEN);
        DrawRectangleLines(bar_x, bar_y, bar_width, HEALTH_BAR_HEIGHT, WHITE);
    }
}

#define BOSS_AURA_ALPHA 100
#define BOSS_CORE_ALPHA 200
#define BOSS_HEALTH_BAR_HEIGHT 6
#define BOSS_HEALTH_BAR_OFFSET 15
#define BOSS_PULSE_AMPLITUDE 0.1f
#define BOSS_PULSE_BASE 0.9f
#define BOSS_SHADOW_ALPHA 150
#define BOSS_SHADOW_OFFSET 3
#define BOSS_SIZE_MULT 1.5f

static void render_boss(Rift* env, float cell_size) {
    if (!env->boss.alive) return;
    
    Vector2 screen_pos = grid_to_screen(env->boss.x, env->boss.y, cell_size);
    float half_cell = cell_size * 0.5f;
    screen_pos.x += half_cell;
    screen_pos.y += half_cell;
    
    Color boss_color = (Color){ARMOR_COLOR_R, 0, ARMOR_COLOR_B, 255};
    float scaled_half = half_cell * BOSS_SIZE_MULT;
    float scaled_third = (cell_size / 3.0f) * BOSS_SIZE_MULT;
    
    DrawCircle(screen_pos.x + BOSS_SHADOW_OFFSET, screen_pos.y + BOSS_SHADOW_OFFSET, scaled_half, (Color){0, 0, 0, BOSS_SHADOW_ALPHA});
    
    float pulse = BOSS_PULSE_BASE + BOSS_PULSE_AMPLITUDE * sinf(env->tick * GLOW_SPEED);
    DrawCircle(screen_pos.x, screen_pos.y, scaled_half * pulse, boss_color);
    DrawCircle(screen_pos.x, screen_pos.y, scaled_third, (Color){PLAYER_COLOR_R, PLAYER_COLOR_G, PLAYER_COLOR_B, 255});
    DrawCircle(screen_pos.x, screen_pos.y, cell_size / 6.0f, (Color){255, 255, 255, BOSS_CORE_ALPHA});
    
    DrawCircleLines(screen_pos.x, screen_pos.y, cell_size * BOSS_SIZE_MULT, (Color){ARMOR_COLOR_R, 0, ARMOR_COLOR_B, BOSS_AURA_ALPHA});
    
    float health_ratio = (float)env->boss.health / env->boss.max_health;
    uint8_t bar_width = (uint8_t)cell_size;
    uint8_t half_bar_width = bar_width >> 1;
    int16_t bar_x = screen_pos.x - half_bar_width;
    int16_t bar_y = screen_pos.y - half_cell - BOSS_HEALTH_BAR_OFFSET;
    
    DrawRectangle(bar_x, bar_y, bar_width, BOSS_HEALTH_BAR_HEIGHT, RED);
    DrawRectangle(bar_x, bar_y, (uint8_t)(bar_width * health_ratio), BOSS_HEALTH_BAR_HEIGHT, GREEN);
    DrawRectangleLines(bar_x, bar_y, bar_width, BOSS_HEALTH_BAR_HEIGHT, WHITE);
}

#define ITEM_GLOW_SPEED 0.2f
#define ITEM_SIZE_EIGHTH 0.125f
#define ITEM_SIZE_TWELFTH 0.0833f

static void render_items(Rift* env, float cell_size) {
    float half_cell = cell_size * 0.5f;
    float eighth_cell = cell_size * ITEM_SIZE_EIGHTH;
    float twelfth_cell = cell_size * ITEM_SIZE_TWELFTH;
    
    for (uint8_t i = 0; i < MAX_ITEMS; i++) {
        if (!env->items[i].active) continue;
        
        Vector2 screen_pos = grid_to_screen(env->items[i].x, env->items[i].y, cell_size);
        screen_pos.x += half_cell;
        screen_pos.y += half_cell;
        
        Color item_color;
        uint8_t item_type = env->items[i].type;
        
        if (item_type == ITEM_GOLD) {
            item_color = GOLD;
        } else if (item_type == ITEM_HEALTH_POTION) {
            item_color = RED;
        } else if (item_type == ITEM_MANA_POTION) {
            item_color = BLUE;
        } else {
            item_color = WHITE;
        }
        
        float glow = GLOW_BASE + GLOW_AMPLITUDE * sinf(env->tick * ITEM_GLOW_SPEED + i);
        DrawCircle(screen_pos.x, screen_pos.y, eighth_cell * glow, item_color);
        DrawCircle(screen_pos.x, screen_pos.y, twelfth_cell, WHITE);
    }
}

#define CORE_COLOR_G 255
#define CORE_COLOR_R 255
#define FIREBALL_COLOR_G 69
#define FIREBALL_COLOR_R 255
#define FIREBALL_PULSE_AMPLITUDE 0.3f
#define FIREBALL_PULSE_BASE 0.8f
#define FIREBALL_PULSE_SPEED 0.3f
#define HALO_COLOR_G 140
#define HALO_COLOR_R 255
#define PROJECTILE_HALO_OFFSET 2
#define PROJECTILE_SIXTH 0.1667f
#define TRAIL_COLOR_G 100
#define TRAIL_COLOR_R 255
#define TRAIL_LENGTH 10.0f
#define TRAIL_WIDTH 2.0f

static void render_projectiles(Rift* env, float cell_size) {
    float half_cell = cell_size * 0.5f;
    float sixth_cell = cell_size * PROJECTILE_SIXTH;
    
    for (uint8_t i = 0; i < MAX_PROJECTILES; i++) {
        if (!env->projectiles[i].active) continue;
        
        Vector2 screen_pos = grid_to_screen(env->projectiles[i].x, env->projectiles[i].y, cell_size);
        screen_pos.x += half_cell;
        screen_pos.y += half_cell;
        
        Color projectile_color;
        float size_multiplier;
        
        if (env->projectiles[i].type == PROJECTILE_FIREBALL) {
            projectile_color = (Color){FIREBALL_COLOR_R, FIREBALL_COLOR_G, 0, 255};
            size_multiplier = FIREBALL_PULSE_BASE + FIREBALL_PULSE_AMPLITUDE * sinf(env->tick * FIREBALL_PULSE_SPEED + i);
        } else {
            projectile_color = RED;
            size_multiplier = 1.0f;
        }
        
        float lifetime_ratio = (float)env->projectiles[i].lifetime / PROJECTILE_LIFETIME;
        uint8_t alpha = (uint8_t)(255 * lifetime_ratio);
        projectile_color.a = alpha;
        
        float radius = sixth_cell * size_multiplier;
        float half_radius = radius * 0.5f;
        uint8_t half_alpha = alpha >> 1;
        uint8_t third_alpha = alpha / 3;
        
        DrawCircle(screen_pos.x, screen_pos.y, radius + PROJECTILE_HALO_OFFSET, (Color){HALO_COLOR_R, HALO_COLOR_G, 0, half_alpha});
        DrawCircle(screen_pos.x, screen_pos.y, radius, projectile_color);
        DrawCircle(screen_pos.x, screen_pos.y, half_radius, (Color){CORE_COLOR_R, CORE_COLOR_G, 0, alpha});
        
        Vector2 trail_end = {
            screen_pos.x - env->projectiles[i].vel_x * TRAIL_LENGTH,
            screen_pos.y - env->projectiles[i].vel_y * TRAIL_LENGTH
        };
        DrawLineEx(screen_pos, trail_end, TRAIL_WIDTH, (Color){TRAIL_COLOR_R, TRAIL_COLOR_G, 0, third_alpha});
    }
}

#define BLIZZARD_BASE_ALPHA 200
#define BLIZZARD_BASE_COLOR_R 200
#define BLIZZARD_BASE_COLOR_G 230
#define BLIZZARD_BASE_COLOR_B 255
#define FROST_COLOR_B 230
#define FROST_COLOR_G 216
#define FROST_COLOR_R 173
#define FROST_LINE_CHANCE 4
#define FROST_LINE_SIZE 2
#define ICE_SHARD_COUNT 24
#define SHARD_CYCLE_FRAMES 60
#define SHARD_FALL_SPEED 0.3f
#define SHARD_SIZE_MIN 1
#define SHARD_SIZE_RANGE 3
#define SHARD_TIMING_MULT 7

static void render_blizzard_areas(Rift* env, float cell_size) {
    float half_cell = cell_size * 0.5f;
    
    for (uint8_t i = 0; i < MAX_BLIZZARD_AREAS; i++) {
        if (!env->blizzard_areas[i].active) continue;
        
        Vector2 screen_pos = grid_to_screen(env->blizzard_areas[i].x, env->blizzard_areas[i].y, cell_size);
        screen_pos.x += half_cell;
        screen_pos.y += half_cell;
        
        float duration_ratio = (float)env->blizzard_areas[i].duration / BLIZZARD_DURATION;
        uint8_t alpha = (uint8_t)(BLIZZARD_BASE_ALPHA * duration_ratio);
        float radius = BLIZZARD_RADIUS * cell_size;
        uint8_t third_alpha = alpha / 3;
        uint8_t half_alpha = alpha >> 1;
        
        Color base_color = (Color){BLIZZARD_BASE_COLOR_R, BLIZZARD_BASE_COLOR_G, BLIZZARD_BASE_COLOR_B, third_alpha};
        Color ice_shard_color = (Color){255, 255, 255, alpha};
        Color frost_color = (Color){FROST_COLOR_R, FROST_COLOR_G, FROST_COLOR_B, half_alpha};
        
        DrawCircle(screen_pos.x, screen_pos.y, radius, base_color);
        
        uint16_t radius_int = (uint16_t)radius;
        uint16_t radius_double = radius_int << 1;
        
        for (uint8_t j = 0; j < ICE_SHARD_COUNT; j++) {
            int16_t random_offset_x = (rand() % radius_double) - radius_int;
            int16_t random_offset_y = (rand() % radius_double) - radius_int;
            float distance_squared = random_offset_x * random_offset_x + random_offset_y * random_offset_y;
            
            if (distance_squared <= radius * radius) {
                float x = screen_pos.x + random_offset_x;
                float y = screen_pos.y + random_offset_y;
                
                uint16_t fall_offset = ((env->tick + j * SHARD_TIMING_MULT) % SHARD_CYCLE_FRAMES);
                y += fall_offset * SHARD_FALL_SPEED;
                
                uint8_t shard_size = SHARD_SIZE_MIN + (rand() % SHARD_SIZE_RANGE);
                DrawCircle(x, y, shard_size, ice_shard_color);
                
                if ((rand() & 3) == 0) {
                    DrawLine(x - FROST_LINE_SIZE, y - FROST_LINE_SIZE, x + FROST_LINE_SIZE, y + FROST_LINE_SIZE, frost_color);
                    DrawLine(x - FROST_LINE_SIZE, y + FROST_LINE_SIZE, x + FROST_LINE_SIZE, y - FROST_LINE_SIZE, frost_color);
                }
            }
        }
        
        DrawCircleLines(screen_pos.x, screen_pos.y, radius, frost_color);
    }
}

#define AI_CONTROL_WIDTH 120
#define GLOBE_MARGIN 10
#define GLOBE_RADIUS 30
#define HEALTH_GLOBE_BG_R 60
#define MANUAL_CONTROL_HEIGHT 25
#define MANUAL_CONTROL_OFFSET 220
#define MANUAL_CONTROL_WIDTH 200
#define POTION_SIZE 25
#define POTION_SPACING 40
#define TEXT_SIZE_12 12
#define TEXT_SIZE_14 14
#define TEXT_SIZE_16 16
#define TEXT_SIZE_18 18
#define TEXT_SIZE_20 20
#define UI_BG_ALPHA 220
#define UI_BG_COLOR_RGB 20
#define UI_HEIGHT 120
#define UI_LINE_COLOR_RGB 60
#define UI_STATS_LINE_HEIGHT 18
#define UI_STATS_OFFSET 55

static void render_ui(Rift* env, uint16_t screen_width, uint16_t screen_height) {
    uint16_t ui_bottom = screen_height - UI_HEIGHT;
    DrawRectangle(0, ui_bottom, screen_width, UI_HEIGHT, (Color){UI_BG_COLOR_RGB, UI_BG_COLOR_RGB, UI_BG_COLOR_RGB, UI_BG_ALPHA});
    DrawLine(0, ui_bottom, screen_width, ui_bottom, (Color){UI_LINE_COLOR_RGB, UI_LINE_COLOR_RGB, UI_LINE_COLOR_RGB, 255});
    
    float health_ratio = (float)env->player.health / env->player.max_health;
    uint16_t health_globe_x = GLOBE_RADIUS + GLOBE_MARGIN;
    uint16_t health_globe_y = screen_height - GLOBE_RADIUS - GLOBE_MARGIN;
    
    DrawCircle(health_globe_x, health_globe_y, GLOBE_RADIUS, (Color){HEALTH_GLOBE_BG_R, 0, 0, 255});
    
    if (health_ratio > ZERO_VALUE) {
        uint16_t fill_height = (uint16_t)(GLOBE_RADIUS * 2 * health_ratio);
        uint16_t fill_y = health_globe_y + GLOBE_RADIUS - fill_height;
        uint16_t globe_radius_sq = GLOBE_RADIUS * GLOBE_RADIUS;
        
        for (uint16_t y = 0; y < fill_height; y++) {
            uint16_t current_y = fill_y + y;
            int16_t dy = current_y - health_globe_y;
            uint16_t dy_sq = dy * dy;
            if (dy_sq <= globe_radius_sq) {
                uint16_t half_width = (uint16_t)sqrt(globe_radius_sq - dy_sq);
                uint16_t full_width = half_width << 1;
                DrawRectangle(health_globe_x - half_width, current_y, full_width, 1, RED);
            }
        }
    }
    
    DrawCircleLines(health_globe_x, health_globe_y, GLOBE_RADIUS, WHITE);
    DrawText(TextFormat("%d", env->player.health), health_globe_x - GLOBE_MARGIN, health_globe_y - 5, TEXT_SIZE_16, WHITE);
    
    float mana_ratio = (float)env->player.mana / env->player.max_mana;
    uint16_t mana_globe_x = screen_width - GLOBE_RADIUS - GLOBE_MARGIN;
    uint16_t mana_globe_y = screen_height - GLOBE_RADIUS - GLOBE_MARGIN;
    
    DrawCircle(mana_globe_x, mana_globe_y, GLOBE_RADIUS, (Color){0, 0, HEALTH_GLOBE_BG_R, 255});
    
    if (mana_ratio > ZERO_VALUE) {
        uint16_t fill_height = (uint16_t)(GLOBE_RADIUS * 2 * mana_ratio);
        uint16_t fill_y = mana_globe_y + GLOBE_RADIUS - fill_height;
        uint16_t globe_radius_sq = GLOBE_RADIUS * GLOBE_RADIUS;
        
        for (uint16_t y = 0; y < fill_height; y++) {
            uint16_t current_y = fill_y + y;
            int16_t dy = current_y - mana_globe_y;
            uint16_t dy_sq = dy * dy;
            if (dy_sq <= globe_radius_sq) {
                uint16_t half_width = (uint16_t)sqrt(globe_radius_sq - dy_sq);
                uint16_t full_width = half_width << 1;
                DrawRectangle(mana_globe_x - half_width, current_y, full_width, 1, BLUE);
            }
        }
    }
    
    DrawCircleLines(mana_globe_x, mana_globe_y, GLOBE_RADIUS, WHITE);
    DrawText(TextFormat("%d", env->player.mana), mana_globe_x - GLOBE_MARGIN, mana_globe_y - 5, TEXT_SIZE_16, WHITE);
    uint16_t screen_center = screen_width >> 1;
    DrawText(TextFormat("Gold: %d", env->player.gold), screen_center - 30, 60, TEXT_SIZE_16, GOLD);
    
    if (env->current_phase == PHASE_RIFT) {
        float progress = (float)env->monsters_killed / MONSTERS_TO_SPAWN;
        DrawText(TextFormat("Rift Progress: %.1f%%", progress * 100), screen_center - 80, 10, TEXT_SIZE_20, GREEN);
        
        if (env->boss_spawned) {
            DrawText("BOSS SPAWNED!", screen_center - 60, 35, TEXT_SIZE_18, RED);
        }
    }
    
    uint16_t hp_x = screen_center - POTION_SPACING;
    uint16_t hp_y = screen_height - POTION_SPACING;
    Color hp_color = (env->player.health_potion_cooldown > 0) ? GRAY : RED;
    DrawCircle(hp_x, hp_y, POTION_SIZE, hp_color);
    DrawCircleLines(hp_x, hp_y, POTION_SIZE, WHITE);
    DrawText("Q", hp_x - 5, hp_y - 8, TEXT_SIZE_16, WHITE);
    
    if (env->player.health_potion_cooldown > 0) {
        float cooldown_ratio = (float)env->player.health_potion_cooldown / HEALTH_POTION_COOLDOWN;
        Vector2 hp_center = {hp_x, hp_y};
        DrawCircleSector(hp_center, POTION_SIZE, 0, 360 * cooldown_ratio, 32, (Color){0, 0, 0, 150});
    }
    
    uint16_t mp_x = screen_center + POTION_SPACING;
    uint16_t mp_y = screen_height - POTION_SPACING;
    Color mp_color = (env->player.mana_potion_cooldown > 0) ? GRAY : BLUE;
    DrawCircle(mp_x, mp_y, POTION_SIZE, mp_color);
    DrawCircleLines(mp_x, mp_y, POTION_SIZE, WHITE);
    DrawText("E", mp_x - 5, mp_y - 8, TEXT_SIZE_16, WHITE);
    
    if (env->player.mana_potion_cooldown > 0) {
        float cooldown_ratio = (float)env->player.mana_potion_cooldown / MANA_POTION_COOLDOWN;
        Vector2 mp_center = {mp_x, mp_y};
        DrawCircleSector(mp_center, POTION_SIZE, 0, 360 * cooldown_ratio, 32, (Color){0, 0, 0, 150});
    }
    
    if (env->current_phase == PHASE_RIFT) {
        uint16_t stats_y = screen_height - UI_STATS_OFFSET;
        DrawText(TextFormat("Episode Stats - Monsters: %d/%d  Kills: %d  Items: %d  Damage: %d  Return: %.1f", 
                 env->monsters_killed, MONSTERS_TO_SPAWN, env->episode_monsters_killed, 
                 env->episode_items_collected, env->episode_damage_dealt, env->episode_return), 
                 GLOBE_MARGIN, stats_y, TEXT_SIZE_14, WHITE);
        
        DrawText(TextFormat("Step Reward: %.1f  Boss: %s  Length: %d/%d", 
                 env->step_reward, env->boss_spawned ? (env->boss.alive ? "ALIVE" : "DEAD") : "NO", 
                 env->episode_length, env->config.episode_length_limit), 
                 GLOBE_MARGIN, stats_y + UI_STATS_LINE_HEIGHT, TEXT_SIZE_14, (Color){0, 255, 255, 255});
    }
    
    DrawText("Hold SHIFT for manual control: WASD=Move Space=Blizzard Q/E=Potions (F=Interact in town)", 
             GLOBE_MARGIN, screen_height - 15, TEXT_SIZE_12, YELLOW);
    
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        DrawText("MANUAL CONTROL ACTIVE", screen_width - MANUAL_CONTROL_WIDTH, GLOBE_MARGIN, TEXT_SIZE_16, GREEN);
        DrawRectangle(screen_width - MANUAL_CONTROL_OFFSET, 5, MANUAL_CONTROL_WIDTH, MANUAL_CONTROL_HEIGHT, (Color){0, 255, 0, 50});
    } else {
        DrawText("AI CONTROL", screen_width - AI_CONTROL_WIDTH, GLOBE_MARGIN, TEXT_SIZE_16, GRAY);
    }
}

#define VENDOR_INNER_SIZE 0.2f
#define VENDOR_OUTER_SIZE 0.333f
#define VENDOR_TEXT_OFFSET_X 25
#define VENDOR_TEXT_OFFSET_Y 40

void render_rift(Rift* env) {
    float cell_size = env->client->cell_size;
    
    BeginMode2D(env->client->camera);
    
    render_background(env, cell_size);
    render_blizzard_areas(env, cell_size);
    render_items(env, cell_size);
    render_monsters(env, cell_size);
    render_boss(env, cell_size);
    render_projectiles(env, cell_size);
    render_player(env, cell_size);
    
    EndMode2D();
}

void render_town(Rift* env) {
    float cell_size = env->client->cell_size;
    
    BeginMode2D(env->client->camera);
    
    render_background(env, cell_size);
    render_player(env, cell_size);
    
    Vector2 vendor_pos = grid_to_screen(env->vendor.x, env->vendor.y, cell_size);
    float half_cell = cell_size * 0.5f;
    vendor_pos.x += half_cell;
    vendor_pos.y += half_cell;
    DrawCircle(vendor_pos.x, vendor_pos.y, cell_size * VENDOR_OUTER_SIZE, GOLD);
    DrawCircle(vendor_pos.x, vendor_pos.y, cell_size * VENDOR_INNER_SIZE, YELLOW);
    DrawText("VENDOR", vendor_pos.x - VENDOR_TEXT_OFFSET_X, vendor_pos.y - VENDOR_TEXT_OFFSET_Y, TEXT_SIZE_12, BLACK);
    
    EndMode2D();
}

#define CLEAR_BG_B 5
#define CLEAR_BG_G 10
#define CLEAR_BG_R 15

void c_render(Rift* env) {
    if (env->client == NULL) {
        env->client = make_client(env);
    }
    
    if (IsKeyDown(KEY_ESCAPE)) {
        exit(0);
    }
    
    Vector2 player_screen = grid_to_screen(env->player.x, env->player.y, env->client->cell_size);
    env->client->camera.target = player_screen;
    
    BeginDrawing();
    ClearBackground((Color){CLEAR_BG_R, CLEAR_BG_G, CLEAR_BG_B, 255});
    
    if (env->current_phase == PHASE_RIFT) {
        render_rift(env);
    } else {
        render_town(env);
    }
    
    render_ui(env, env->client->width, env->client->height);
    
    EndDrawing();
}