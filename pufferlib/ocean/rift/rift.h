#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include "raylib.h"

// ============================================================================
// MAP CONSTANTS
// ============================================================================

#define MAP_WIDTH 160
#define MAP_HEIGHT 120
#define MAP_SIZE (MAP_WIDTH * MAP_HEIGHT)
#define MAP_BORDER_SIZE 5

// ============================================================================
// GAME PHASE CONSTANTS
// ============================================================================

#define PHASE_RIFT 0
#define PHASE_TOWN 1

// ============================================================================
// CELL TYPE CONSTANTS
// ============================================================================

#define CELL_EMPTY 0
#define CELL_WALL 1
#define CELL_FLOOR 2
#define CELL_DOOR 3
#define CELL_VENDOR 4

// ============================================================================
// ENTITY TYPE CONSTANTS
// ============================================================================

#define MONSTER_ZOMBIE 0
#define BOSS_RIFT_GUARDIAN 0
#define ITEM_GOLD 0
#define ITEM_HEALTH_POTION 1
#define ITEM_MANA_POTION 2

// ============================================================================
// ACTION CONSTANTS
// ============================================================================

#define ACTION_MOVE_UP 0
#define ACTION_MOVE_DOWN 1
#define ACTION_MOVE_LEFT 2
#define ACTION_MOVE_RIGHT 3
#define ACTION_MOVE_UP_LEFT 4
#define ACTION_MOVE_UP_RIGHT 5
#define ACTION_MOVE_DOWN_LEFT 6
#define ACTION_MOVE_DOWN_RIGHT 7
#define ACTION_WHIRLWIND 8
#define ACTION_USE_HEALTH_POTION 9
#define ACTION_USE_MANA_POTION 10
#define ACTION_INTERACT 11
#define ACTION_NOOP 12

// ============================================================================
// PLAYER CONSTANTS
// ============================================================================

#define PLAYER_MAX_HEALTH 100
#define PLAYER_MAX_MANA 50
#define PLAYER_BASE_DAMAGE 10
#define WHIRLWIND_DAMAGE 20
#define WHIRLWIND_MANA_COST 10
#define WHIRLWIND_RADIUS 2.0f
#define HEALTH_POTION_HEAL 1000
#define MANA_POTION_RESTORE 1000
#define HEALTH_POTION_COOLDOWN 60
#define MANA_POTION_COOLDOWN 60
#define MANA_REGEN_RATE 15

// ============================================================================
// MONSTER CONSTANTS
// ============================================================================

#define MAX_MONSTERS 200
#define MONSTERS_TO_SPAWN 100
#define MONSTER_BASE_HEALTH 20
#define MONSTER_BASE_DAMAGE 5
#define MONSTER_ATTACK_COOLDOWN 30
#define MONSTER_DETECTION_RANGE 8.0f
#define SAFE_SPAWN_RADIUS 15.0f
#define MONSTER_MOVEMENT_COOLDOWN 5
#define MONSTER_WANDER_COOLDOWN 10
#define MONSTER_WANDER_CHANCE 20

// ============================================================================
// PROJECTILE CONSTANTS
// ============================================================================

#define MAX_PROJECTILES 50
#define PROJECTILE_SPEED 3.0f
#define PROJECTILE_LIFETIME 60
#define PROJECTILE_FIREBALL 0

// ============================================================================
// BOSS CONSTANTS
// ============================================================================

#define BOSS_BASE_HEALTH 100
#define BOSS_BASE_DAMAGE 15
#define BOSS_ATTACK_COOLDOWN 15
#define BOSS_ATTACK_DISTANCE 2.0f

// ============================================================================
// ITEM CONSTANTS
// ============================================================================

#define MAX_ITEMS 100
#define MAX_INVENTORY_SIZE 20
#define ITEM_PICKUP_DISTANCE 1.0f
#define ITEM_SCAN_DISTANCE 5.0f
#define GOLD_DROP_MIN 5
#define GOLD_DROP_RANGE 15
#define GOLD_DROP_CHANCE 40
#define HEALTH_POTION_DROP_CHANCE 70

// ============================================================================
// VENDOR CONSTANTS
// ============================================================================

#define VENDOR_HEALTH_POTION_PRICE 20
#define VENDOR_MANA_POTION_PRICE 15
#define VENDOR_STOCK_AMOUNT 10
#define VENDOR_INTERACTION_DISTANCE 2.0f

// ============================================================================
// MAP GENERATION CONSTANTS
// ============================================================================

#define WALL_CLUSTER_COUNT 20
#define WALL_CLUSTER_SIZE 2
#define WALL_CLUSTER_DENSITY 3
#define SPAWN_BORDER_SIZE 10
#define SPAWN_AREA_MARGIN 20
#define MAP_SPAWN_BORDER 10
#define MAP_SPAWN_BORDER_OFFSET 20
#define SPAWN_CHECK_DISTANCE 8.0f

// ============================================================================
// OBSERVATION CONSTANTS
// ============================================================================

#define LOCAL_VIEW_SIZE 32
#define LOCAL_VIEW_AREA (LOCAL_VIEW_SIZE * LOCAL_VIEW_SIZE)
#define RIFT_OBS_SIZE (LOCAL_VIEW_AREA + 20)
#define TOWN_OBS_SIZE (LOCAL_VIEW_AREA + 30)
#define EMPTY_CELL_VALUE 0.0f
#define CELL_VALUE_FLOOR 0.2f
#define CELL_VALUE_DOOR 0.5f
#define CELL_VALUE_VENDOR 0.8f
#define CELL_VALUE_WALL 1.0f
#define GOLD_NORMALIZATION 1000.0f
#define NORMALIZATION_DIVISOR_1000 1000.0f
#define INVENTORY_NORMALIZATION 10.0f
#define DISTANCE_NORMALIZATION 50.0f
#define COOLDOWN_NORMALIZATION 10.0f

// ============================================================================
// GAMEPLAY CONSTANTS
// ============================================================================

#define RIFT_COMPLETION_THRESHOLD 0.8f
#define MAX_EPISODE_LENGTH 2000
#define WHIRLWIND_ACTIVATION_COOLDOWN 10
#define MOVEMENT_STEP 1.0f
#define MELEE_RANGE 1.0f
#define TWO_PI_MULTIPLIER 2.0f
#define FULL_CIRCLE_MULTIPLIER 2.0f
#define ZERO_VALUE 0.0f


// ============================================================================
// CONFIGURATION
// ============================================================================

typedef struct GameConfig {
    int max_monsters;
    int max_items;
    float monster_spawn_rate;
    float item_drop_rate;
    int player_start_health;
    int player_start_mana;
    int starting_gold;
    float monster_move_speed;
    int episode_length_limit;
    float completion_reward;
    float boss_kill_reward;
    float monster_kill_reward;
    float item_pickup_reward;
    float damage_taken_penalty;
    float time_penalty;
} GameConfig;

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
    .completion_reward = 100.0f,
    .boss_kill_reward = 50.0f,
    .monster_kill_reward = 2.0f,
    .item_pickup_reward = 1.0f,
    .damage_taken_penalty = -1.0f,
    .time_penalty = -0.01f
};

// ============================================================================
// ENTITY STRUCTURES
// ============================================================================

typedef struct Player {
    float x, y;
    int health, max_health;
    int mana, max_mana;
    int gold;
    int damage;
    uint8_t alive;
    uint8_t whirlwind_cooldown;
    uint8_t health_potion_cooldown;
    uint8_t mana_potion_cooldown;
    uint8_t mana_regen_timer;
    uint8_t inventory[MAX_INVENTORY_SIZE];
    uint8_t inventory_count;
} Player;

typedef struct Monster {
    float x, y;
    int health, max_health;
    int damage;
    uint8_t type;
    uint8_t alive;
    uint8_t move_cooldown;
    uint8_t attack_cooldown;
    float target_x, target_y;
} Monster;

typedef struct Boss {
    float x, y;
    int health, max_health;
    int damage;
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
} Item;

typedef struct Projectile {
    float x, y;
    float vel_x, vel_y;
    uint8_t type;
    uint8_t damage;
    uint8_t lifetime;
    uint8_t active;
} Projectile;

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
    float whirlwinds_used;
    float n;
} Log;

typedef struct Client {
    float cell_size;
    int width;
    int height;
    Camera2D camera;
} Client;

// ============================================================================
// MAIN ENVIRONMENT
// ============================================================================

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
    Vendor vendor;
    
    GameConfig config;
    uint32_t tick;
    uint8_t current_phase;  // PHASE_RIFT or PHASE_TOWN
    
    uint16_t monsters_spawned;
    uint16_t monsters_killed;
    uint8_t boss_spawned;
    uint8_t rift_completed;
    
    uint16_t episode_length;
    float episode_return;
    
    uint32_t total_monsters_killed;
    uint32_t total_boss_kills;
    uint32_t total_items_collected;
    uint32_t total_gold_earned;
    uint32_t total_damage_dealt;
    uint32_t total_damage_taken;
    uint32_t total_rift_completions;
    uint32_t total_vendor_transactions;
    uint32_t total_whirlwinds_used;
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

void spawn_monsters(Rift* env);
void update_monsters(Rift* env);
void update_player(Rift* env);
void handle_whirlwind(Rift* env);
void handle_item_pickup(Rift* env);
void handle_vendor_interaction(Rift* env);
void spawn_boss(Rift* env);
void drop_loot(Rift* env, float x, float y);

void spawn_projectile(Rift* env, float start_x, float start_y, float target_x, float target_y, uint8_t type, uint8_t damage);
void update_projectiles(Rift* env);

void compute_rift_observations(Rift* env);
void compute_town_observations(Rift* env);

void c_render(Rift* env);
void render_rift(Rift* env);
void render_town(Rift* env);
void c_close_client(Client* client);

static void render_projectiles(Rift* env, float cell_size);

static void add_log(Rift* env);

static inline float distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

static inline int is_valid_position(int x, int y) {
    return x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT;
}

static inline int is_floor(Rift* env, int x, int y) {
    int cell = env->map[y * MAP_WIDTH + x];
    return cell == CELL_FLOOR || cell == CELL_DOOR;
}

// ============================================================================
// IMPLEMENTATION
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
    memset(env->map, CELL_WALL, MAP_SIZE);
    
    for (int y = MAP_BORDER_SIZE; y < MAP_HEIGHT - MAP_BORDER_SIZE; y++) {
        for (int x = MAP_BORDER_SIZE; x < MAP_WIDTH - MAP_BORDER_SIZE; x++) {
            env->map[y * MAP_WIDTH + x] = CELL_FLOOR;
        }
    }
    
    for (int i = 0; i < WALL_CLUSTER_COUNT; i++) {
        int wall_x = MAP_BORDER_SIZE * 2 + rand() % (MAP_WIDTH - MAP_BORDER_SIZE * 4);
        int wall_y = MAP_BORDER_SIZE * 2 + rand() % (MAP_HEIGHT - MAP_BORDER_SIZE * 4);
        
        for (int dy = -WALL_CLUSTER_SIZE; dy <= WALL_CLUSTER_SIZE; dy++) {
            for (int dx = -WALL_CLUSTER_SIZE; dx <= WALL_CLUSTER_SIZE; dx++) {
                if (rand() % WALL_CLUSTER_DENSITY == 0) {
                    int wx = wall_x + dx;
                    int wy = wall_y + dy;
                    if (is_valid_position(wx, wy)) {
                        env->map[wy * MAP_WIDTH + wx] = CELL_WALL;
                    }
                }
            }
        }
    }
    
    env->map[(MAP_HEIGHT - 1) * MAP_WIDTH + (MAP_WIDTH / 2)] = CELL_DOOR;
}

void generate_town_map(Rift* env) {
    memset(env->map, CELL_FLOOR, MAP_SIZE);
    
    for (int x = 0; x < MAP_WIDTH; x++) {
        env->map[0 * MAP_WIDTH + x] = CELL_WALL;
        env->map[(MAP_HEIGHT - 1) * MAP_WIDTH + x] = CELL_WALL;
    }
    for (int y = 0; y < MAP_HEIGHT; y++) {
        env->map[y * MAP_WIDTH + 0] = CELL_WALL;
        env->map[y * MAP_WIDTH + (MAP_WIDTH - 1)] = CELL_WALL;
    }
    
    env->vendor.x = MAP_WIDTH / 4;
    env->vendor.y = MAP_HEIGHT / 2;
    env->map[(int)env->vendor.y * MAP_WIDTH + (int)env->vendor.x] = CELL_VENDOR;
    
    env->vendor.items_for_sale[0] = ITEM_HEALTH_POTION;
    env->vendor.prices[0] = VENDOR_HEALTH_POTION_PRICE;
    env->vendor.stock[0] = 10;
    
    env->vendor.items_for_sale[1] = ITEM_MANA_POTION;
    env->vendor.prices[1] = VENDOR_MANA_POTION_PRICE;
    env->vendor.stock[1] = VENDOR_STOCK_AMOUNT;
    
    env->map[0 * MAP_WIDTH + (MAP_WIDTH / 2)] = CELL_DOOR;
}

void spawn_monsters(Rift* env) {
    if (env->current_phase != PHASE_RIFT) return;
    if (env->monsters_spawned >= MONSTERS_TO_SPAWN) return;
    
    if ((float)rand() / RAND_MAX < env->config.monster_spawn_rate) {
        for (int i = 0; i < MAX_MONSTERS; i++) {
            if (!env->monsters[i].alive) {
                int spawn_x = MAP_SPAWN_BORDER + rand() % (MAP_WIDTH - MAP_SPAWN_BORDER_OFFSET);
                int spawn_y = MAP_SPAWN_BORDER + rand() % (MAP_HEIGHT - MAP_SPAWN_BORDER_OFFSET);
                
                if (is_floor(env, spawn_x, spawn_y)) {
                    env->monsters[i].x = (float)spawn_x;
                    env->monsters[i].y = (float)spawn_y;
                    env->monsters[i].type = MONSTER_ZOMBIE;
                    env->monsters[i].health = MONSTER_BASE_HEALTH;
                    env->monsters[i].max_health = MONSTER_BASE_HEALTH;
                    env->monsters[i].damage = MONSTER_BASE_DAMAGE;
                    env->monsters[i].alive = 1;
                    env->monsters[i].move_cooldown = 0;
                    env->monsters[i].attack_cooldown = 0;
                    env->monsters[i].target_x = env->monsters[i].x;
                    env->monsters[i].target_y = env->monsters[i].y;
                    env->monsters_spawned++;
                    break;
                }
            }
        }
    }
}

void update_monsters(Rift* env) {
    if (env->current_phase != PHASE_RIFT) return;
    
    for (int i = 0; i < MAX_MONSTERS; i++) {
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
                    
                    float new_x = monster->x + dx * env->config.monster_move_speed;
                    float new_y = monster->y + dy * env->config.monster_move_speed;
                    
                    if (is_floor(env, (int)new_x, (int)new_y)) {
                        monster->x = new_x;
                        monster->y = new_y;
                    }
                    
                    monster->move_cooldown = MONSTER_MOVEMENT_COOLDOWN;
                } else {
                    if (env->player.alive && monster->attack_cooldown == 0) {
                        spawn_projectile(env, monster->x, monster->y, env->player.x, env->player.y, 
                                       PROJECTILE_FIREBALL, monster->damage);
                        monster->attack_cooldown = MONSTER_ATTACK_COOLDOWN;
                    }
                }
            } else {
                if (rand() % MONSTER_WANDER_CHANCE == 0) {
                    float random_angle = ((float)rand() / RAND_MAX) * FULL_CIRCLE_MULTIPLIER * PI;
                    float new_x = monster->x + cos(random_angle) * env->config.monster_move_speed;
                    float new_y = monster->y + sin(random_angle) * env->config.monster_move_speed;
                    
                    if (is_floor(env, (int)new_x, (int)new_y)) {
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
    
    if (env->player.whirlwind_cooldown > 0) {
        env->player.whirlwind_cooldown--;
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
            break;
        case ACTION_MOVE_DOWN:
            new_y += 1.0f;
            break;
        case ACTION_MOVE_LEFT:
            new_x -= 1.0f;
            break;
        case ACTION_MOVE_RIGHT:
            new_x += 1.0f;
            break;
        case ACTION_MOVE_UP_LEFT:
            new_x -= 1.0f;
            new_y -= 1.0f;
            break;
        case ACTION_MOVE_UP_RIGHT:
            new_x += 1.0f;
            new_y -= 1.0f;
            break;
        case ACTION_MOVE_DOWN_LEFT:
            new_x -= 1.0f;
            new_y += 1.0f;
            break;
        case ACTION_MOVE_DOWN_RIGHT:
            new_x += 1.0f;
            new_y += 1.0f;
            break;
    }
    
    if (is_floor(env, (int)new_x, (int)new_y)) {
        env->player.x = new_x;
        env->player.y = new_y;
    }
    
    int player_cell = env->map[(int)env->player.y * MAP_WIDTH + (int)env->player.x];
    if (player_cell == CELL_DOOR) {
        if (env->current_phase == PHASE_RIFT) {
            env->current_phase = PHASE_TOWN;
            generate_town_map(env);
            env->player.x = MAP_WIDTH / 2;
            env->player.y = MAP_HEIGHT - FULL_CIRCLE_MULTIPLIER;
        } else if (env->current_phase == PHASE_TOWN) {
            env->current_phase = PHASE_RIFT;
            generate_rift_map(env);
            env->player.x = MAP_WIDTH / 2;
            env->player.y = FULL_CIRCLE_MULTIPLIER;
        }
    }
    
    switch (action) {
        case ACTION_WHIRLWIND:
            if (env->player.whirlwind_cooldown == 0 && env->player.mana >= WHIRLWIND_MANA_COST) {
                handle_whirlwind(env);
            }
            break;
        case ACTION_USE_HEALTH_POTION:
            if (env->player.health_potion_cooldown == 0 && env->player.health < env->player.max_health) {
                env->player.health = (env->player.health + HEALTH_POTION_HEAL > env->player.max_health) ? 
                                   env->player.max_health : env->player.health + HEALTH_POTION_HEAL;
                env->player.health_potion_cooldown = HEALTH_POTION_COOLDOWN;
            }
            break;
        case ACTION_USE_MANA_POTION:
            if (env->player.mana_potion_cooldown == 0 && env->player.mana < env->player.max_mana) {
                env->player.mana = (env->player.mana + MANA_POTION_RESTORE > env->player.max_mana) ? 
                                 env->player.max_mana : env->player.mana + MANA_POTION_RESTORE;
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

// ============================================================================
// COMBAT SYSTEM
// ============================================================================

void handle_whirlwind(Rift* env) {
    if (env->current_phase != PHASE_RIFT) return;
    
    env->player.mana -= WHIRLWIND_MANA_COST;
    env->player.whirlwind_cooldown = 10;
    env->total_whirlwinds_used++;
    
    for (int i = 0; i < MAX_MONSTERS; i++) {
        if (env->monsters[i].alive) {
            float dist = distance(env->player.x, env->player.y, 
                                env->monsters[i].x, env->monsters[i].y);
            if (dist <= WHIRLWIND_RADIUS) {
                env->monsters[i].health -= WHIRLWIND_DAMAGE;
                env->total_damage_dealt += WHIRLWIND_DAMAGE;
                
                if (env->monsters[i].health <= 0) {
                    env->monsters[i].alive = 0;
                    env->monsters_killed++;
                    env->total_monsters_killed++;
                    env->episode_return += env->config.monster_kill_reward;
                    
                    if ((float)rand() / RAND_MAX < env->config.item_drop_rate) {
                        drop_loot(env, env->monsters[i].x, env->monsters[i].y);
                    }
                }
            }
        }
    }
    
    float completion = (float)env->monsters_killed / MONSTERS_TO_SPAWN;
    if (completion >= RIFT_COMPLETION_THRESHOLD && !env->boss_spawned) {
        spawn_boss(env);
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
    
    for (int i = 0; i < MAX_MONSTERS; i++) {
        env->monsters[i].alive = 0;
    }
}

void drop_loot(Rift* env, float x, float y) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (!env->items[i].active) {
            env->items[i].x = (float)((int)x) + 0.5f;
            env->items[i].y = (float)((int)y) + 0.5f;
            env->items[i].active = 1;
            
            int rand_val = rand() % 100;
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

// ============================================================================
// PROJECTILE SYSTEM
// ============================================================================

void spawn_projectile(Rift* env, float start_x, float start_y, float target_x, float target_y, uint8_t type, uint8_t damage) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
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
    for (int i = 0; i < MAX_PROJECTILES; i++) {
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
            if (!is_valid_position(proj_x, proj_y) || !is_floor(env, proj_x, proj_y)) {
                proj->active = 0;
                continue;
            }
            
            float dist_to_player = distance(env->player.x, env->player.y, proj->x, proj->y);
            if (dist_to_player <= 0.8f && env->player.alive) {
                env->player.health -= proj->damage;
                env->episode_return += env->config.damage_taken_penalty;
                env->total_damage_taken += proj->damage;
                
                if (env->player.health <= 0) {
                    env->player.alive = 0;
                    env->player.health = 0;
                }
                
                proj->active = 0;
            }
        }
    }
}

// ============================================================================
// INTERACTION SYSTEM
// ============================================================================

void handle_item_pickup(Rift* env) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (env->items[i].active) {
            float dist = distance(env->player.x, env->player.y, 
                                env->items[i].x, env->items[i].y);
            if (dist <= 1.0f) {
                if (env->items[i].type == ITEM_GOLD) {
                    env->player.gold += env->items[i].value;
                    env->total_gold_earned += env->items[i].value;
                } else if (env->player.inventory_count < MAX_INVENTORY_SIZE) {
                    env->player.inventory[env->player.inventory_count] = env->items[i].type;
                    env->player.inventory_count++;
                }
                
                env->items[i].active = 0;
                env->total_items_collected++;
                env->episode_return += env->config.item_pickup_reward;
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
                    env->total_vendor_transactions++;
                    break;
                }
            }
        }
    }
}

// ============================================================================
// OBSERVATION SYSTEM
// ============================================================================

void compute_rift_observations(Rift* env) {
    int obs_idx = 0;
    
    int player_x = (int)env->player.x;
    int player_y = (int)env->player.y;
    int half_view = LOCAL_VIEW_SIZE / 2;
    
    for (int y = 0; y < LOCAL_VIEW_SIZE; y++) {
        for (int x = 0; x < LOCAL_VIEW_SIZE; x++) {
            int world_x = player_x - half_view + x;
            int world_y = player_y - half_view + y;
            
            float value;
            switch (env->map[world_y * MAP_WIDTH + world_x]) {
                case CELL_EMPTY: value = ZERO_VALUE; break;
                case CELL_WALL: value = CELL_VALUE_WALL; break;
                case CELL_FLOOR: value = CELL_VALUE_FLOOR; break;
                case CELL_DOOR: value = CELL_VALUE_DOOR; break;
                default: value = CELL_VALUE_WALL; break;
            }
            
            env->observations[obs_idx++] = value;
        }
    }
    
    // Player stats
    env->observations[obs_idx++] = (float)env->player.health / env->player.max_health;
    env->observations[obs_idx++] = (float)env->player.mana / env->player.max_mana;
    env->observations[obs_idx++] = env->player.x / MAP_WIDTH;
    env->observations[obs_idx++] = env->player.y / MAP_HEIGHT;
    env->observations[obs_idx++] = (float)env->player.gold / NORMALIZATION_DIVISOR_1000;
    
    int monster_count = 0;
    float nearest_monster_dist = 100.0f;
    for (int i = 0; i < MAX_MONSTERS; i++) {
        if (env->monsters[i].alive) {
            monster_count++;
            float dist = distance(env->player.x, env->player.y, 
                                env->monsters[i].x, env->monsters[i].y);
            if (dist < nearest_monster_dist) {
                nearest_monster_dist = dist;
            }
        }
    }
    env->observations[obs_idx++] = (float)monster_count / MAX_MONSTERS;
    env->observations[obs_idx++] = nearest_monster_dist / DISTANCE_NORMALIZATION;
    
    env->observations[obs_idx++] = env->boss.alive ? 1.0f : 0.0f;
    env->observations[obs_idx++] = env->boss.alive ? 
        ((float)env->boss.health / env->boss.max_health) : 0.0f;
    
    env->observations[obs_idx++] = (float)env->monsters_killed / MONSTERS_TO_SPAWN;
    
    int nearby_items = 0;
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (env->items[i].active) {
            float dist = distance(env->player.x, env->player.y, 
                                env->items[i].x, env->items[i].y);
            if (dist <= 5.0f) {
                nearby_items++;
            }
        }
    }
    env->observations[obs_idx++] = (float)nearby_items / 10.0f;
    
    env->observations[obs_idx++] = (float)env->player.inventory_count / MAX_INVENTORY_SIZE;
    
    // Phase
    env->observations[obs_idx++] = env->current_phase == PHASE_RIFT ? 1.0f : 0.0f;
    
    env->observations[obs_idx++] = (float)env->player.whirlwind_cooldown / 10.0f;
    
    while (obs_idx < RIFT_OBS_SIZE) {
        env->observations[obs_idx++] = 0.0f;
    }
}

void compute_town_observations(Rift* env) {
    int obs_idx = 0;
    
    int player_x = (int)env->player.x;
    int player_y = (int)env->player.y;
    int half_view = LOCAL_VIEW_SIZE / 2;
    
    for (int y = 0; y < LOCAL_VIEW_SIZE; y++) {
        for (int x = 0; x < LOCAL_VIEW_SIZE; x++) {
            int world_x = player_x - half_view + x;
            int world_y = player_y - half_view + y;
            
            float value;
            switch (env->map[world_y * MAP_WIDTH + world_x]) {
                case CELL_EMPTY: value = ZERO_VALUE; break;
                case CELL_WALL: value = CELL_VALUE_WALL; break;
                case CELL_FLOOR: value = CELL_VALUE_FLOOR; break;
                case CELL_DOOR: value = CELL_VALUE_DOOR; break;
                case CELL_VENDOR: value = CELL_VALUE_VENDOR; break;
                default: value = CELL_VALUE_WALL; break;
            }
            
            env->observations[obs_idx++] = value;
        }
    }
    
    // Player stats
    env->observations[obs_idx++] = (float)env->player.health / env->player.max_health;
    env->observations[obs_idx++] = (float)env->player.mana / env->player.max_mana;
    env->observations[obs_idx++] = env->player.x / MAP_WIDTH;
    env->observations[obs_idx++] = env->player.y / MAP_HEIGHT;
    env->observations[obs_idx++] = (float)env->player.gold / NORMALIZATION_DIVISOR_1000;
    
    for (int i = 0; i < MAX_INVENTORY_SIZE; i++) {
        if (i < env->player.inventory_count) {
            env->observations[obs_idx++] = (float)env->player.inventory[i] / 10.0f;
        } else {
            env->observations[obs_idx++] = 0.0f;
        }
    }
    
    env->observations[obs_idx++] = distance(env->player.x, env->player.y, 
                                          env->vendor.x, env->vendor.y) / DISTANCE_NORMALIZATION;
    
    // Phase
    env->observations[obs_idx++] = env->current_phase == PHASE_TOWN ? 1.0f : 0.0f;
    
    while (obs_idx < TOWN_OBS_SIZE) {
        env->observations[obs_idx++] = 0.0f;
    }
}

// ============================================================================
// LOGGING SYSTEM
// ============================================================================

static void add_log(Rift* env) {
    env->log.episode_return += env->episode_return;
    env->log.episode_length += env->episode_length;
    env->log.monsters_killed += env->total_monsters_killed;
    env->log.boss_kills += env->total_boss_kills;
    env->log.items_collected += env->total_items_collected;
    env->log.gold_earned += env->total_gold_earned;
    env->log.damage_dealt += env->total_damage_dealt;
    env->log.damage_taken += env->total_damage_taken;
    env->log.rift_completions += env->total_rift_completions;
    env->log.vendor_transactions += env->total_vendor_transactions;
    env->log.whirlwinds_used += env->total_whirlwinds_used;
    env->log.n += 1;
}

// ============================================================================
// RESET SYSTEM
// ============================================================================

void c_reset(Rift* env) {
    env->current_phase = PHASE_RIFT;
    generate_rift_map(env);
    
    env->player.x = MAP_WIDTH / 2;
    env->player.y = MAP_HEIGHT / 2;
    env->player.health = env->config.player_start_health;
    env->player.max_health = env->config.player_start_health;
    env->player.mana = env->config.player_start_mana;
    env->player.max_mana = env->config.player_start_mana;
    env->player.gold = env->config.starting_gold;
    env->player.damage = PLAYER_BASE_DAMAGE;
    env->player.alive = 1;
    env->player.whirlwind_cooldown = 0;
    env->player.health_potion_cooldown = 0;
    env->player.mana_potion_cooldown = 0;
    env->player.mana_regen_timer = 0;
    env->player.inventory_count = 0;
    memset(env->player.inventory, 0, sizeof(env->player.inventory));
    
    memset(env->monsters, 0, sizeof(env->monsters));
    env->monsters_spawned = 0;
    env->monsters_killed = 0;
    
    memset(&env->boss, 0, sizeof(Boss));
    env->boss_spawned = 0;
    
    memset(env->items, 0, sizeof(env->items));
    memset(env->projectiles, 0, sizeof(env->projectiles));
    
    env->tick = 0;
    env->episode_length = 0;
    env->episode_return = 0.0f;
    env->rift_completed = 0;
    
    env->rewards[0] = 0.0f;
    env->terminals[0] = 0;
    
    if (env->current_phase == PHASE_RIFT) {
        compute_rift_observations(env);
    } else {
        compute_town_observations(env);
    }
}

// ============================================================================
// GAME STEP
// ============================================================================

void c_step(Rift* env) {
    env->terminals[0] = 0;
    env->rewards[0] = 0.0f;
    
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
            env->actions[0] = ACTION_WHIRLWIND;
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
    
    if (env->current_phase == PHASE_RIFT) {
        spawn_monsters(env);
        update_monsters(env);
        update_projectiles(env);
        
        if (env->boss.alive) {
            float dist = distance(env->player.x, env->player.y, env->boss.x, env->boss.y);
            if (dist <= 2.0f && env->boss.attack_cooldown == 0) {
                env->player.health -= env->boss.damage;
                env->total_damage_taken += env->boss.damage;
                env->episode_return += env->config.damage_taken_penalty;
                env->boss.attack_cooldown = 15;
                
                if (env->player.health <= 0) {
                    env->player.alive = 0;
                    env->player.health = 0;
                }
            }
            if (env->boss.attack_cooldown > 0) {
                env->boss.attack_cooldown--;
            }
        }
        
        if (env->boss_spawned && !env->boss.alive && !env->rift_completed) {
            env->rift_completed = 1;
            env->total_rift_completions++;
            env->episode_return += env->config.completion_reward;
            
            drop_loot(env, env->boss.x, env->boss.y);
            drop_loot(env, env->boss.x + 1, env->boss.y);
            drop_loot(env, env->boss.x, env->boss.y + 1);
        }
    }
    
    env->episode_length++;
    env->episode_return += env->config.time_penalty;
    
    if (env->episode_length >= env->config.episode_length_limit) {
        env->terminals[0] = 1;
        add_log(env);
        c_reset(env);
        return;
    }
    
    if (env->current_phase == PHASE_RIFT) {
        compute_rift_observations(env);
    } else {
        compute_town_observations(env);
    }
    
    env->tick++;
}

// ============================================================================
// RENDERING SYSTEM
// ============================================================================

static Client* make_client(Rift* env) {
    Client* client = (Client*)calloc(1, sizeof(Client));
    client->cell_size = 32.0f;
    client->width = 1024;
    client->height = 768;
    
    InitWindow(client->width, client->height, "PufferLib Rift");
    SetTargetFPS(60);
    
    client->camera.target = (Vector2){ MAP_WIDTH * client->cell_size / 2, MAP_HEIGHT * client->cell_size / 2 };
    client->camera.offset = (Vector2){ client->width / 2, client->height / 2 };
    client->camera.rotation = 0.0f;
    client->camera.zoom = 1.0f;
    
    return client;
}

void c_close_client(Client* client) {
    CloseWindow();
    free(client);
}

Vector2 grid_to_screen(float grid_x, float grid_y, float cell_size) {
    Vector2 screen;
    screen.x = grid_x * cell_size;
    screen.y = grid_y * cell_size;
    return screen;
}

static void render_map(Rift* env, float cell_size) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            Vector2 screen_pos = grid_to_screen(x, y, cell_size);
            int cell = env->map[y * MAP_WIDTH + x];
            Color color;
            
            switch (cell) {
                case CELL_EMPTY:
                    color = BLACK;
                    break;
                case CELL_WALL:
                    color = (Color){60, 60, 60, 255};
                    break;
                case CELL_FLOOR:
                    color = (Color){40, 35, 30, 255};
                    break;
                case CELL_DOOR:
                    color = (Color){139, 69, 19, 255};
                    break;
                case CELL_VENDOR:
                    color = (Color){255, 215, 0, 255};
                    break;
                default:
                    color = GRAY;
                    break;
            }
            
            DrawRectangle(screen_pos.x, screen_pos.y, cell_size, cell_size, color);
            DrawRectangleLines(screen_pos.x, screen_pos.y, cell_size, cell_size, (Color){80, 80, 80, 255});
        }
    }
}

static void render_player(Rift* env, float cell_size) {
    Vector2 screen_pos = grid_to_screen(env->player.x, env->player.y, cell_size);
    
    screen_pos.x += cell_size / 2;
    screen_pos.y += cell_size / 2;
    
    Color player_color = (Color){205, 92, 92, 255};  // Indian red for barbarian
    Color armor_color = (Color){139, 69, 19, 255};   // Saddle brown for armor
    
    // Draw shadow
    DrawCircle(screen_pos.x + 2, screen_pos.y + 2, cell_size/3, (Color){0, 0, 0, 100});
    
    DrawCircle(screen_pos.x, screen_pos.y, cell_size/3, player_color);
    DrawCircle(screen_pos.x, screen_pos.y, cell_size/4, armor_color);
    
    if (env->player.whirlwind_cooldown > 0) {
        float radius = WHIRLWIND_RADIUS * cell_size;
        for (int i = 0; i < 8; i++) {
            float angle = i * 45 * DEG2RAD + env->tick * 0.3f;
            float x1 = screen_pos.x + cos(angle) * radius * 0.5f;
            float y1 = screen_pos.y + sin(angle) * radius * 0.5f;
            float x2 = screen_pos.x + cos(angle) * radius;
            float y2 = screen_pos.y + sin(angle) * radius;
            DrawLine(x1, y1, x2, y2, (Color){255, 255, 0, 150});
        }
        DrawCircleLines(screen_pos.x, screen_pos.y, radius, (Color){255, 255, 0, 200});
    }
}

static void render_monsters(Rift* env, float cell_size) {
    for (int i = 0; i < MAX_MONSTERS; i++) {
        if (env->monsters[i].alive) {
            Vector2 screen_pos = grid_to_screen(env->monsters[i].x, env->monsters[i].y, cell_size);
            screen_pos.x += cell_size / 2;
            screen_pos.y += cell_size / 2;
            
            Color monster_color;
            switch (env->monsters[i].type) {
                case MONSTER_ZOMBIE:
                    monster_color = (Color){34, 139, 34, 255};  // Forest green
                    break;
                default:
                    monster_color = (Color){128, 0, 0, 255};    // Maroon
                    break;
            }
            
            // Draw shadow
            DrawCircle(screen_pos.x + 1, screen_pos.y + 1, cell_size/4, (Color){0, 0, 0, 100});
            
            DrawCircle(screen_pos.x, screen_pos.y, cell_size/4, monster_color);
            DrawCircle(screen_pos.x, screen_pos.y, cell_size/6, (Color){255, 0, 0, 150});
            
            float health_ratio = (float)env->monsters[i].health / env->monsters[i].max_health;
            int bar_width = cell_size/2;
            int bar_height = 2;
            DrawRectangle(screen_pos.x - bar_width/2, screen_pos.y - cell_size/3 - 5, 
                         bar_width, bar_height, RED);
            DrawRectangle(screen_pos.x - bar_width/2, screen_pos.y - cell_size/3 - 5, 
                         bar_width * health_ratio, bar_height, GREEN);
            DrawRectangleLines(screen_pos.x - bar_width/2, screen_pos.y - cell_size/3 - 5, 
                              bar_width, bar_height, WHITE);
        }
    }
}

static void render_boss(Rift* env, float cell_size) {
    if (env->boss.alive) {
        Vector2 screen_pos = grid_to_screen(env->boss.x, env->boss.y, cell_size);
        screen_pos.x += cell_size / 2;
        screen_pos.y += cell_size / 2;
        
        Color boss_color = (Color){75, 0, 130, 255};  // Indigo for boss
        float size_multiplier = 1.5f;
        
        DrawCircle(screen_pos.x + 3, screen_pos.y + 3, cell_size/2 * size_multiplier, (Color){0, 0, 0, 150});
        
        float pulse = 0.9f + 0.1f * sinf(env->tick * 0.1f);
        DrawCircle(screen_pos.x, screen_pos.y, cell_size/2 * size_multiplier * pulse, boss_color);
        DrawCircle(screen_pos.x, screen_pos.y, cell_size/3 * size_multiplier, (Color){138, 43, 226, 255});
        DrawCircle(screen_pos.x, screen_pos.y, cell_size/6, (Color){255, 255, 255, 200});
        
        DrawCircleLines(screen_pos.x, screen_pos.y, cell_size * size_multiplier, 
                       (Color){75, 0, 130, 100});
        
        float health_ratio = (float)env->boss.health / env->boss.max_health;
        int bar_width = cell_size;
        int bar_height = 6;
        DrawRectangle(screen_pos.x - bar_width/2, screen_pos.y - cell_size/2 - 15, 
                     bar_width, bar_height, RED);
        DrawRectangle(screen_pos.x - bar_width/2, screen_pos.y - cell_size/2 - 15, 
                     bar_width * health_ratio, bar_height, GREEN);
        DrawRectangleLines(screen_pos.x - bar_width/2, screen_pos.y - cell_size/2 - 15, 
                          bar_width, bar_height, WHITE);
    }
}

static void render_items(Rift* env, float cell_size) {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (env->items[i].active) {
            Vector2 screen_pos = grid_to_screen(env->items[i].x, env->items[i].y, cell_size);
            screen_pos.x += cell_size / 2;
            screen_pos.y += cell_size / 2;
            
            Color item_color;
            switch (env->items[i].type) {
                case ITEM_GOLD:
                    item_color = GOLD;
                    break;
                case ITEM_HEALTH_POTION:
                    item_color = RED;
                    break;
                case ITEM_MANA_POTION:
                    item_color = BLUE;
                    break;
                default:
                    item_color = WHITE;
                    break;
            }
            
            float glow = 0.8f + 0.2f * sinf(env->tick * 0.2f + i);
            DrawCircle(screen_pos.x, screen_pos.y, cell_size/8 * glow, item_color);
            DrawCircle(screen_pos.x, screen_pos.y, cell_size/12, WHITE);
        }
    }
}

static void render_projectiles(Rift* env, float cell_size) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (env->projectiles[i].active) {
            Vector2 screen_pos = grid_to_screen(env->projectiles[i].x, env->projectiles[i].y, cell_size);
            screen_pos.x += cell_size / 2;
            screen_pos.y += cell_size / 2;
            
            Color projectile_color;
            float size_multiplier = 1.0f;
            
            switch (env->projectiles[i].type) {
                case PROJECTILE_FIREBALL:
                    projectile_color = (Color){255, 69, 0, 255};
                    size_multiplier = 0.8f + 0.3f * sinf(env->tick * 0.3f + i);
                    break;
                default:
                    projectile_color = RED;
                    break;
            }
            
            float lifetime_ratio = (float)env->projectiles[i].lifetime / PROJECTILE_LIFETIME;
            int alpha = (int)(255 * lifetime_ratio);
            projectile_color.a = alpha;
            
            float radius = cell_size / 6 * size_multiplier;
            DrawCircle(screen_pos.x, screen_pos.y, radius + 2, (Color){255, 140, 0, alpha/2});
            DrawCircle(screen_pos.x, screen_pos.y, radius, projectile_color);
            DrawCircle(screen_pos.x, screen_pos.y, radius/2, (Color){255, 255, 0, alpha});
            
            float trail_length = 10.0f;
            Vector2 trail_end = {
                screen_pos.x - env->projectiles[i].vel_x * trail_length,
                screen_pos.y - env->projectiles[i].vel_y * trail_length
            };
            DrawLineEx(screen_pos, trail_end, 2.0f, (Color){255, 100, 0, alpha/3});
        }
    }
}

static void render_ui(Rift* env, int screen_width, int screen_height) {
    DrawRectangle(0, screen_height - 80, screen_width, 80, (Color){20, 20, 20, 220});
    DrawLine(0, screen_height - 80, screen_width, screen_height - 80, (Color){60, 60, 60, 255});
    
    float health_ratio = (float)env->player.health / env->player.max_health;
    int globe_radius = 30;
    int health_globe_x = globe_radius + 10;
    int health_globe_y = screen_height - globe_radius - 10;
    
    DrawCircle(health_globe_x, health_globe_y, globe_radius, (Color){60, 0, 0, 255});
    
    if (health_ratio > 0) {
        int fill_height = (int)(globe_radius * 2 * health_ratio);
        int fill_y = health_globe_y + globe_radius - fill_height;
        
        for (int y = 0; y < fill_height; y++) {
            int current_y = fill_y + y;
            int dy = current_y - health_globe_y;
            if (dy * dy <= globe_radius * globe_radius) {
                int half_width = (int)sqrt(globe_radius * globe_radius - dy * dy);
                DrawRectangle(health_globe_x - half_width, current_y, half_width * 2, 1, RED);
            }
        }
    }
    
    DrawCircleLines(health_globe_x, health_globe_y, globe_radius, WHITE);
    DrawText(TextFormat("%d", env->player.health), health_globe_x - 10, health_globe_y - 5, 16, WHITE);
    
    float mana_ratio = (float)env->player.mana / env->player.max_mana;
    int mana_globe_x = screen_width - globe_radius - 10;
    int mana_globe_y = screen_height - globe_radius - 10;
    
    DrawCircle(mana_globe_x, mana_globe_y, globe_radius, (Color){0, 0, 60, 255});
    
    if (mana_ratio > 0) {
        int fill_height = (int)(globe_radius * 2 * mana_ratio);
        int fill_y = mana_globe_y + globe_radius - fill_height;
        
        for (int y = 0; y < fill_height; y++) {
            int current_y = fill_y + y;
            int dy = current_y - mana_globe_y;
            if (dy * dy <= globe_radius * globe_radius) {
                int half_width = (int)sqrt(globe_radius * globe_radius - dy * dy);
                DrawRectangle(mana_globe_x - half_width, current_y, half_width * 2, 1, BLUE);
            }
        }
    }
    
    DrawCircleLines(mana_globe_x, mana_globe_y, globe_radius, WHITE);
    DrawText(TextFormat("%d", env->player.mana), mana_globe_x - 10, mana_globe_y - 5, 16, WHITE);
    DrawText(TextFormat("Gold: %d", env->player.gold), 
             screen_width/2 - 30, 60, 16, GOLD);
    
    if (env->current_phase == PHASE_RIFT) {
        float progress = (float)env->monsters_killed / MONSTERS_TO_SPAWN;
        DrawText(TextFormat("Rift Progress: %.1f%%", progress * 100), 
                 screen_width/2 - 80, 10, 20, GREEN);
        
        if (env->boss_spawned) {
            DrawText("BOSS SPAWNED!", screen_width/2 - 60, 35, 18, RED);
        }
    }
    
    int potion_size = 25;
    int center_x = screen_width / 2;
    
    int hp_x = center_x - 40;
    int hp_y = screen_height - 40;
    Color hp_color = (env->player.health_potion_cooldown > 0) ? GRAY : RED;
    DrawCircle(hp_x, hp_y, potion_size, hp_color);
    DrawCircleLines(hp_x, hp_y, potion_size, WHITE);
    DrawText("Q", hp_x - 5, hp_y - 8, 16, WHITE);
    
    if (env->player.health_potion_cooldown > 0) {
        float cooldown_ratio = (float)env->player.health_potion_cooldown / HEALTH_POTION_COOLDOWN;
        Vector2 hp_center = {hp_x, hp_y};
        DrawCircleSector(hp_center, potion_size, 0, 360 * cooldown_ratio, 32, (Color){0, 0, 0, 150});
    }
    
    int mp_x = center_x + 40;
    int mp_y = screen_height - 40;
    Color mp_color = (env->player.mana_potion_cooldown > 0) ? GRAY : BLUE;
    DrawCircle(mp_x, mp_y, potion_size, mp_color);
    DrawCircleLines(mp_x, mp_y, potion_size, WHITE);
    DrawText("E", mp_x - 5, mp_y - 8, 16, WHITE);
    
    if (env->player.mana_potion_cooldown > 0) {
        float cooldown_ratio = (float)env->player.mana_potion_cooldown / MANA_POTION_COOLDOWN;
        Vector2 mp_center = {mp_x, mp_y};
        DrawCircleSector(mp_center, potion_size, 0, 360 * cooldown_ratio, 32, (Color){0, 0, 0, 150});
    }
    
    DrawText("Hold SHIFT for manual control: WASD=Move Space=Whirlwind Q/E=Potions (F=Interact in town)", 
             10, screen_height - 15, 12, YELLOW);
    
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        DrawText("MANUAL CONTROL ACTIVE", screen_width - 200, 10, 16, GREEN);
        DrawRectangle(screen_width - 220, 5, 200, 25, (Color){0, 255, 0, 50});
    } else {
        DrawText("AI CONTROL", screen_width - 120, 10, 16, GRAY);
    }
}

void render_rift(Rift* env) {
    float cell_size = env->client->cell_size;
    
    BeginMode2D(env->client->camera);
    
    render_map(env, cell_size);
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
    
    render_map(env, cell_size);
    render_player(env, cell_size);
    
    Vector2 vendor_pos = grid_to_screen(env->vendor.x, env->vendor.y, cell_size);
    DrawCircle(vendor_pos.x, vendor_pos.y, cell_size/3, GOLD);
    DrawCircle(vendor_pos.x, vendor_pos.y, cell_size/5, YELLOW);
    DrawText("VENDOR", vendor_pos.x - 25, vendor_pos.y - 40, 12, BLACK);
    
    EndMode2D();
}

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
    ClearBackground((Color){15, 10, 5, 255});
    
    if (env->current_phase == PHASE_RIFT) {
        render_rift(env);
    } else {
        render_town(env);
    }
    
    render_ui(env, env->client->width, env->client->height);
    
    EndDrawing();
}