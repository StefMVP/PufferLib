#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include "raylib.h"

// ============================================================================
// CONSTANTS
// ============================================================================

#define GRID_WIDTH 16
#define GRID_HEIGHT 12
#define GRID_SIZE (GRID_WIDTH * GRID_HEIGHT)

#define CELL_EMPTY 0
#define CELL_PATH 1
#define CELL_TOWER 2
#define CELL_SPLASH_TOWER 3
#define CELL_SNIPER_TOWER 4
#define CELL_SPAWNER 5
#define CELL_GOAL 6

#define MAX_ENEMIES 100
#define MAX_PROJECTILES 200
#define MAX_SPAWNERS 5
#define MAX_GOALS 5

#define OBS_SIZE (GRID_SIZE + 3)
#define TOWER_PLACEMENT_PENALTY -0.1f
#define EPISODE_DECAY_RATE -0.001f

#define ENEMY_TYPE_NORMAL 0
#define ENEMY_TYPE_HEAVY 1
#define HEAVY_ENEMY_CHANCE 4
#define MIN_SPAWNERS 2
#define MAX_SPAWNERS_PER_MAP 2
#define MIN_SPAWN_DISTANCE 8
#define MAX_EPISODE_LENGTH 1500
#define MIN_ENEMIES_TO_SPAWN 15
#define MAX_EXTRA_ENEMIES 20
#define BURST_SPAWN_CHANCE 3
#define MIN_BURST_SIZE 2
#define MAX_BURST_SIZE 3

#define ACTION_MIN 1
#define NORMAL_TOWER_ACTION_MAX 192
#define SPLASH_TOWER_ACTION_MIN 193
#define SPLASH_TOWER_ACTION_MAX 384
#define SNIPER_TOWER_ACTION_MIN 385
#define ACTION_MAX 576

#define SPAWN_MARGIN 2
#define SPAWN_EDGE_MARGIN 6
#define MAX_SPAWN_ATTEMPTS 20
#define PROJECTILE_LIFETIME 30

#define OBS_PATH_VALUE 0.1f
#define OBS_TOWER_VALUE 0.3f
#define OBS_SPLASH_VALUE 0.5f
#define OBS_SNIPER_VALUE 0.7f
#define OBS_SPAWNER_VALUE 0.9f
#define OBS_GOAL_VALUE 1.0f

#define GOLD_NORMALIZE_FACTOR 200.0f
#define WIN_TIME_BONUS_BASE 1000
#define DAMAGE_REWARD 0.02f
#define ATTACK_REWARD 0.01f
#define PATH_COVERAGE_MULTIPLIER 0.1f

#define ENEMY_SEARCH_DISTANCE 999999.0f
#define PROJECTILE_HIT_DISTANCE 0.1f
#define SPEED_VARIATION_BASE 0.8f
#define SPEED_VARIATION_RANGE 40

#define CORRIDOR_CENTER_DIVISOR 2
#define NUM_MOVEMENT_DIRECTIONS 4
#define NUM_SPAWN_SIDES 3
#define SPAWNER_ANIMATION_RAYS 8
#define SPAWNER_RAY_ANGLE_STEP 45

#define TOWER_TYPE_NORMAL 0
#define TOWER_TYPE_SPLASH 1
#define TOWER_TYPE_SNIPER 2

#define INITIAL_GOAL_COUNT 1
#define SPAWN_INTERVAL_MULTIPLIER 2
#define HEAVY_ENEMY_HP 2

// ============================================================================
// CONFIGURATION
// ============================================================================

typedef struct GameConfig {
    int tower_cost;
    int tower_range;
    int tower_damage;
    int tower_fire_cooldown;
    
    int enemy_base_hp;
    float enemy_speed;
    int enemy_kill_gold;
    float enemy_kill_reward;
    float enemy_escape_penalty;
    int enemy_hp_increase_per_wave;
    
    int min_towers_per_episode;
    int max_towers_per_episode;
    
    int starting_enemies_per_wave;
    int enemies_increase_per_wave;
    int spawn_interval;
    int wave_start_delay;
    float wave_complete_bonus;
    
    int starting_gold;
    int starting_lives;
    
    float projectile_speed;
    
    int splash_tower_cost;
    float splash_radius;
    int splash_damage;
    
    int sniper_tower_cost;
    int sniper_range;
    int sniper_damage;
    int sniper_fire_cooldown;
    
    float reward_episode_win_multiplier;
    float reward_episode_win_time_bonus;
    float reward_episode_loss;
} GameConfig;

static const GameConfig DEFAULT_CONFIG = {
    .tower_cost = 30,
    .tower_range = 2,
    .tower_damage = 1,
    .tower_fire_cooldown = 8,
    
    .enemy_base_hp = 1,
    .enemy_speed = 0.3f,
    .enemy_kill_gold = 0,
    .enemy_kill_reward = 1.0f,
    .enemy_escape_penalty = -5.0f,
    .enemy_hp_increase_per_wave = 0,
    
    .min_towers_per_episode = 5,
    .max_towers_per_episode = 10,
    
    .starting_enemies_per_wave = 8,
    .enemies_increase_per_wave = 3,
    .spawn_interval = 8,
    .wave_start_delay = 0,
    .wave_complete_bonus = 0.0f,
    
    .starting_gold = 150,
    .starting_lives = 5,
    
    .projectile_speed = 0.6f,
    
    .splash_tower_cost = 30,
    .splash_radius = 3.0f,
    .splash_damage = 1,
    
    .sniper_tower_cost = 30,
    .sniper_range = 4,
    .sniper_damage = 2,
    .sniper_fire_cooldown = 35,
    
    .reward_episode_win_multiplier = 2.0f,
    .reward_episode_win_time_bonus = 0.01f,
    .reward_episode_loss = -10.0f
};

// ============================================================================
// ENTITY_STRUCTURES
// ============================================================================

typedef struct Enemy {
    float x, y;
    uint16_t hp, max_hp;
    uint16_t path_index;
    uint8_t active;
    float progress;
    float speed_multiplier;
    uint8_t type;
} Enemy;

typedef struct Projectile {
    float x, y;
    float target_x, target_y;
    uint16_t damage;
    uint8_t active;
    uint8_t is_splash;
    uint8_t lifetime;
} Projectile;

typedef struct Tower {
    uint8_t x, y;
    uint8_t cooldown;
    uint8_t type;
} Tower;

typedef struct Spawner {
    uint8_t x, y;
    uint8_t active;
} Spawner;

typedef struct Goal {
    uint8_t x, y;
    uint8_t active;
} Goal;

typedef struct Log {
    float episode_return;
    float episode_length;
    float enemies_killed;
    float enemies_escaped;
    float towers_placed;
    float win_rate;
    float lives_remaining;
    float normal_towers;
    float splash_towers;
    float sniper_towers;
    float gold_efficiency;
    float n;
} Log;

typedef struct Client {
    float cell_size;
    int width;
    int height;
} Client;

// ============================================================================
// MAIN_ENVIRONMENT
// ============================================================================

typedef struct MinimalTD {
    Client* client;
    Log log;
    float* observations;
    float* actions;
    float* rewards;
    unsigned char* terminals;
    
    unsigned char grid[GRID_SIZE];
    
    Spawner spawners[MAX_SPAWNERS];
    uint8_t num_spawners;
    Goal goals[MAX_GOALS];
    uint8_t num_goals;
    
    Enemy enemies[MAX_ENEMIES];
    uint16_t num_enemies;
    uint8_t spawn_cooldown;
    uint16_t total_spawned;
    
    uint16_t gold;
    uint8_t lives;
    
    Tower towers[GRID_SIZE];
    uint8_t num_towers;
    
    Projectile projectiles[MAX_PROJECTILES];
    uint16_t num_projectiles;
    
    GameConfig config;
    uint32_t tick;
    uint16_t enemies_killed;
    uint16_t total_enemies_killed;
    uint16_t enemies_escaped;
    uint8_t towers_placed;
    uint8_t normal_towers_placed;
    uint8_t splash_towers_placed;
    uint8_t sniper_towers_placed;
    uint16_t starting_gold;
    uint16_t episode_length;
    uint16_t max_episode_length;
    float episode_return;
    
    uint16_t total_enemies_to_spawn;
    uint16_t enemies_spawned;
    uint16_t enemies_dealt_with;
    uint8_t episode_tower_limit;
    
    uint16_t valid_positions[GRID_SIZE];
    uint8_t num_valid_positions;
    
    float cell_size_render;
} MinimalTD;

// ============================================================================
// UTILITY_FUNCTIONS
// ============================================================================

static void update_valid_positions(MinimalTD* env);

static inline int manhattan_distance(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

static inline float calculate_path_coverage_reward(MinimalTD* env, int x, int y, int tower_type) {
    float tower_range = (tower_type == TOWER_TYPE_NORMAL) ? env->config.tower_range :
                       (tower_type == TOWER_TYPE_SPLASH) ? env->config.splash_radius :
                                          env->config.sniper_range;
    
    int path_cells_covered = 0;
    for (int dy = -((int)tower_range); dy <= (int)tower_range; dy++) {
        for (int dx = -((int)tower_range); dx <= (int)tower_range; dx++) {
            int check_x = x + dx;
            int check_y = y + dy;
            
            if (check_x >= 0 && check_x < GRID_WIDTH && check_y >= 0 && check_y < GRID_HEIGHT) {
                float dist = sqrtf(dx * dx + dy * dy);
                if (dist <= tower_range) {
                    int check_idx = check_y * GRID_WIDTH + check_x;
                    if (env->grid[check_idx] == CELL_PATH) {
                        path_cells_covered++;
                    }
                }
            }
        }
    }
    
    return (path_cells_covered > 0) ? path_cells_covered * PATH_COVERAGE_MULTIPLIER : 0.0f;
}

// ============================================================================
// MAP_GENERATION
// ============================================================================

static void place_goal(MinimalTD* env) {
    env->num_goals = 0;
    int goal_y = SPAWN_MARGIN + rand() % (GRID_HEIGHT - (SPAWN_MARGIN * 2));
    env->goals[0].x = GRID_WIDTH - 1;
    env->goals[0].y = goal_y;
    env->goals[0].active = 1;
    env->grid[goal_y * GRID_WIDTH + (GRID_WIDTH - 1)] = CELL_GOAL;
    env->num_goals = INITIAL_GOAL_COUNT;
}

static void place_spawners(MinimalTD* env) {
    env->num_spawners = 0;
    int num_spawn = MIN_SPAWNERS + rand() % MAX_SPAWNERS_PER_MAP;
    int min_distance = MIN_SPAWN_DISTANCE;
    
    for (int i = 0; i < num_spawn && env->num_spawners < MAX_SPAWNERS; i++) {
        int attempts = 0;
        int spawn_x, spawn_y;
        
        do {
            int side = rand() % NUM_SPAWN_SIDES;
            if (side == 0) {
                spawn_x = 0;
                spawn_y = SPAWN_MARGIN + rand() % (GRID_HEIGHT - (SPAWN_MARGIN * 2));
            } else if (side == 1) {
                spawn_x = SPAWN_MARGIN + rand() % (GRID_WIDTH - SPAWN_EDGE_MARGIN);
                spawn_y = 0;
            } else {
                spawn_x = SPAWN_MARGIN + rand() % (GRID_WIDTH - SPAWN_EDGE_MARGIN);
                spawn_y = GRID_HEIGHT - 1;
            }
            attempts++;
        } while (manhattan_distance(spawn_x, spawn_y, env->goals[0].x, env->goals[0].y) < min_distance && attempts < MAX_SPAWN_ATTEMPTS);
        
        if (env->grid[spawn_y * GRID_WIDTH + spawn_x] == CELL_EMPTY) {
            env->spawners[env->num_spawners].x = spawn_x;
            env->spawners[env->num_spawners].y = spawn_y;
            env->spawners[env->num_spawners].active = 1;
            env->grid[spawn_y * GRID_WIDTH + spawn_x] = CELL_SPAWNER;
            env->num_spawners++;
        }
    }
}

static void create_paths_to_goal(MinimalTD* env) {
    int gx = env->goals[0].x;
    int gy = env->goals[0].y;
    
    for (int s = 0; s < env->num_spawners; s++) {
        int x = env->spawners[s].x;
        int y = env->spawners[s].y;
        
        env->grid[y * GRID_WIDTH + x] = CELL_PATH;
        
        while (x != gx) {
            if (x < gx) x++;
            else x--;
            if (env->grid[y * GRID_WIDTH + x] == CELL_EMPTY) {
                env->grid[y * GRID_WIDTH + x] = CELL_PATH;
            }
        }
        
        while (y != gy) {
            if (y < gy) y++;
            else y--;
            if (env->grid[y * GRID_WIDTH + x] == CELL_EMPTY) {
                env->grid[y * GRID_WIDTH + x] = CELL_PATH;
            }
        }
        
        env->grid[env->spawners[s].y * GRID_WIDTH + env->spawners[s].x] = CELL_SPAWNER;
    }
}

static void connect_spawners(MinimalTD* env) {
    if (env->num_spawners <= 1) return;
    
    int corridor_y = GRID_HEIGHT / CORRIDOR_CENTER_DIVISOR;
    for (int x = 1; x < GRID_WIDTH - 1; x++) {
        if (env->grid[corridor_y * GRID_WIDTH + x] == CELL_EMPTY) {
            env->grid[corridor_y * GRID_WIDTH + x] = CELL_PATH;
        }
    }
    
    for (int s = 0; s < env->num_spawners; s++) {
        int sx = env->spawners[s].x;
        int sy = env->spawners[s].y;
        
        int y = sy;
        while (y != corridor_y) {
            if (y < corridor_y) y++;
            else y--;
            if (env->grid[y * GRID_WIDTH + sx] == CELL_EMPTY) {
                env->grid[y * GRID_WIDTH + sx] = CELL_PATH;
            }
        }
    }
}

static void generate_random_map(MinimalTD* env) {
    place_goal(env);
    place_spawners(env);
    create_paths_to_goal(env);
    connect_spawners(env);
}

// ============================================================================
// ENEMY_MANAGEMENT
// ============================================================================

static void spawn_enemy(MinimalTD* env) {
    if (env->num_enemies >= MAX_ENEMIES) return;
    if (env->num_spawners == 0) return;
    if (env->enemies_spawned >= env->total_enemies_to_spawn) return;  
    
    
    int spawner_idx = rand() % env->num_spawners;
    Spawner* spawner = &env->spawners[spawner_idx];
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!env->enemies[i].active) {
            env->enemies[i].x = (float)spawner->x;
            env->enemies[i].y = (float)spawner->y;
            
            
            if (rand() % HEAVY_ENEMY_CHANCE == 0) {
                env->enemies[i].type = ENEMY_TYPE_HEAVY;  
                env->enemies[i].hp = HEAVY_ENEMY_HP;
                env->enemies[i].max_hp = HEAVY_ENEMY_HP;
            } else {
                env->enemies[i].type = ENEMY_TYPE_NORMAL;  
                env->enemies[i].hp = env->config.enemy_base_hp;
                env->enemies[i].max_hp = env->enemies[i].hp;
            }
            
            env->enemies[i].path_index = 0;
            env->enemies[i].progress = 0.0f;
            env->enemies[i].active = 1;
            
            env->enemies[i].speed_multiplier = SPEED_VARIATION_BASE + (rand() % SPEED_VARIATION_RANGE) / 100.0f;
            env->num_enemies++;
            env->enemies_spawned++;
            break;
        }
    }
}

static void move_enemies(MinimalTD* env) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (env->enemies[i].active) {
            
            float best_dist = ENEMY_SEARCH_DISTANCE;
            int best_dx = 0, best_dy = 0;
            
            
            int dirs[NUM_MOVEMENT_DIRECTIONS][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};
            for (int d = 0; d < NUM_MOVEMENT_DIRECTIONS; d++) {
                int nx = (int)env->enemies[i].x + dirs[d][0];
                int ny = (int)env->enemies[i].y + dirs[d][1];
                
                
                if (nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT) {
                    int cell = env->grid[ny * GRID_WIDTH + nx];
                    if (cell == CELL_PATH || cell == CELL_GOAL || cell == CELL_SPAWNER) {
                        
                        for (int g = 0; g < env->num_goals; g++) {
                            float dist = abs(nx - env->goals[g].x) + abs(ny - env->goals[g].y);
                            if (dist < best_dist) {
                                best_dist = dist;
                                best_dx = dirs[d][0];
                                best_dy = dirs[d][1];
                            }
                        }
                    }
                }
            }
            
            
            float speed = env->config.enemy_speed * env->enemies[i].speed_multiplier;
            env->enemies[i].x += best_dx * speed;
            env->enemies[i].y += best_dy * speed;
            
            
            int ex = (int)env->enemies[i].x;
            int ey = (int)env->enemies[i].y;
            if (env->grid[ey * GRID_WIDTH + ex] == CELL_GOAL) {
                env->enemies[i].active = 0;
                env->num_enemies--;
                env->lives--;
                env->enemies_dealt_with++;
                env->rewards[0] += env->config.enemy_escape_penalty;
                env->episode_return += env->config.enemy_escape_penalty;
                env->enemies_escaped++;  
            }
        }
    }
}

// ============================================================================
// PROJECTILE_SYSTEM
// ============================================================================

static void spawn_projectile(MinimalTD* env, float from_x, float from_y, float to_x, float to_y, int damage, int is_splash) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!env->projectiles[i].active) {
            env->projectiles[i].x = from_x;
            env->projectiles[i].y = from_y;
            env->projectiles[i].target_x = to_x;
            env->projectiles[i].target_y = to_y;
            env->projectiles[i].damage = damage;
            env->projectiles[i].active = 1;
            env->projectiles[i].is_splash = is_splash;
            env->projectiles[i].lifetime = PROJECTILE_LIFETIME;  
            env->num_projectiles++;
            break;
        }
    }
}

static void update_projectiles(MinimalTD* env) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (env->projectiles[i].active) {
            
            env->projectiles[i].lifetime--;
            if (env->projectiles[i].lifetime <= 0) {
                env->projectiles[i].active = 0;
                env->num_projectiles--;
                continue;
            }
            
            
            float dx = env->projectiles[i].target_x - env->projectiles[i].x;
            float dy = env->projectiles[i].target_y - env->projectiles[i].y;
            float dist = sqrtf(dx * dx + dy * dy);
            
            if (dist < PROJECTILE_HIT_DISTANCE) {
                
                env->projectiles[i].active = 0;
                env->num_projectiles--;
            } else {
                
                env->projectiles[i].x += (dx / dist) * env->config.projectile_speed;
                env->projectiles[i].y += (dy / dist) * env->config.projectile_speed;
            }
        }
    }
}

// ============================================================================
// TOWER_COMBAT
// ============================================================================

static inline float get_tower_range(const Tower* tower, const GameConfig* config) {
    if (tower->type == TOWER_TYPE_SPLASH) return config->splash_radius;
    if (tower->type == TOWER_TYPE_SNIPER) return config->sniper_range;
    return config->tower_range;
}

static inline int get_tower_cooldown(const Tower* tower, const GameConfig* config) {
    return (tower->type == TOWER_TYPE_SNIPER) ? config->sniper_fire_cooldown : config->tower_fire_cooldown;
}

static inline void handle_enemy_damage(MinimalTD* env, int enemy_idx, int damage) {
    Enemy* enemy = &env->enemies[enemy_idx];
    enemy->hp -= damage;
    if (enemy->hp <= 0) {
        enemy->active = 0;
        env->num_enemies--;
        env->enemies_killed++;
        env->total_enemies_killed++;
        env->enemies_dealt_with++;
        env->gold += env->config.enemy_kill_gold;
        env->rewards[0] += env->config.enemy_kill_reward;
        env->episode_return += env->config.enemy_kill_reward;
    } else {
        env->episode_return += DAMAGE_REWARD;
    }
}

static int find_best_target(const MinimalTD* env, const Tower* tower) {
    int best_enemy = -1;
    float best_progress = -1.0f;
    float max_range = get_tower_range(tower, &env->config);
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (env->enemies[i].active) {
            float dist = sqrtf(
                powf(tower->x - env->enemies[i].x, 2) +
                powf(tower->y - env->enemies[i].y, 2)
            );
            
            if (dist <= max_range) {
                float total_progress = env->enemies[i].path_index + env->enemies[i].progress;
                if (total_progress > best_progress) {
                    best_enemy = i;
                    best_progress = total_progress;
                }
            }
        }
    }
    return best_enemy;
}

static void process_tower_attack(MinimalTD* env, Tower* tower, int target_idx) {
    tower->cooldown = get_tower_cooldown(tower, &env->config);
    env->episode_return += ATTACK_REWARD;
    
    Enemy* target = &env->enemies[target_idx];
    float target_x = target->x;
    float target_y = target->y;
    
    if (tower->type == TOWER_TYPE_SPLASH) {
        handle_enemy_damage(env, target_idx, env->config.splash_damage);
        
        for (int e = 0; e < MAX_ENEMIES; e++) {
            if (env->enemies[e].active && e != target_idx) {
                float dist = sqrtf(
                    powf(env->enemies[e].x - target_x, 2) +
                    powf(env->enemies[e].y - target_y, 2)
                );
                if (dist <= env->config.splash_radius) {
                    handle_enemy_damage(env, e, env->config.splash_damage);
                }
            }
        }
        spawn_projectile(env, tower->x + 0.5f, tower->y + 0.5f, target_x, target_y, 0, 1);
    } else if (tower->type == TOWER_TYPE_SNIPER) {
        handle_enemy_damage(env, target_idx, env->config.sniper_damage);
        spawn_projectile(env, tower->x + 0.5f, tower->y + 0.5f, target_x, target_y, 0, 0);
    } else {
        handle_enemy_damage(env, target_idx, env->config.tower_damage);
        spawn_projectile(env, tower->x + 0.5f, tower->y + 0.5f, target_x, target_y, 0, 0);
    }
}

static void towers_attack(MinimalTD* env) {
    for (int t = 0; t < env->num_towers; t++) {
        Tower* tower = &env->towers[t];
        
        if (tower->cooldown > 0) {
            tower->cooldown--;
            continue;
        }
        
        int target = find_best_target(env, tower);
        if (target >= 0) {
            process_tower_attack(env, tower, target);
        }
    }
}

// ============================================================================
// OBSERVATION_SYSTEM
// ============================================================================

void compute_observations(MinimalTD* env) {
    int obs_idx = 0;
    
    for (int i = 0; i < GRID_SIZE; i++) {
        float value = 0.0f;
        
        if (env->grid[i] == CELL_EMPTY) {
            value = 0.0f;
        } else if (env->grid[i] == CELL_PATH) {
            value = OBS_PATH_VALUE;
        } else if (env->grid[i] == CELL_TOWER) {
            value = OBS_TOWER_VALUE;
        } else if (env->grid[i] == CELL_SPLASH_TOWER) {
            value = OBS_SPLASH_VALUE;
        } else if (env->grid[i] == CELL_SNIPER_TOWER) {
            value = OBS_SNIPER_VALUE;
        } else if (env->grid[i] == CELL_SPAWNER) {
            value = OBS_SPAWNER_VALUE;
        } else if (env->grid[i] == CELL_GOAL) {
            value = OBS_GOAL_VALUE;
        }
        
        env->observations[obs_idx++] = value;
    }
    
    env->observations[obs_idx++] = fminf(env->gold / GOLD_NORMALIZE_FACTOR, 1.0f);
    env->observations[obs_idx++] = (float)env->lives / (float)env->config.starting_lives;
    env->observations[obs_idx++] = (float)env->enemies_dealt_with / (float)env->total_enemies_to_spawn;
}

// ============================================================================
// ENVIRONMENT_LIFECYCLE
// ============================================================================

void init(MinimalTD* env) {
    env->tick = 0;
    env->config = DEFAULT_CONFIG;
    env->max_episode_length = MAX_EPISODE_LENGTH;  
}

void allocate(MinimalTD* env) {
    env->tick = 0;
    env->episode_return = 0.0f;
    env->client = NULL;
    
    memset(&env->log, 0, sizeof(Log));
    
    init(env);
}

void c_close(MinimalTD* env) {
    
}

void free_allocated(MinimalTD* env) {
    free(env->actions);
    free(env->observations);
    free(env->terminals);
    free(env->rewards);
    c_close(env);
}

// ============================================================================
// LOGGING_SYSTEM
// ============================================================================

static void add_log(MinimalTD* env) {
    
    float win_rate = (env->lives > 0) ? 1.0f : 0.0f;
    float gold_efficiency = (env->starting_gold > 0) ? 
        (float)(env->starting_gold - env->gold) / (float)env->starting_gold : 0.0f;
    
    env->log.episode_return += env->episode_return;
    env->log.episode_length += env->episode_length;
    env->log.enemies_killed += env->total_enemies_killed;
    env->log.enemies_escaped += env->enemies_escaped;
    env->log.towers_placed += env->towers_placed;
    env->log.win_rate += win_rate;
    env->log.lives_remaining += env->lives;
    env->log.normal_towers += env->normal_towers_placed;
    env->log.splash_towers += env->splash_towers_placed;
    env->log.sniper_towers += env->sniper_towers_placed;
    env->log.gold_efficiency += gold_efficiency;
    env->log.n += 1;
}

// ============================================================================
// RESET_SYSTEM
// ============================================================================

void c_reset(MinimalTD* env) {
    
    memset(env->grid, CELL_EMPTY, GRID_SIZE);
    
    
    generate_random_map(env);
    
    
    memset(env->enemies, 0, sizeof(env->enemies));
    env->num_enemies = 0;
    env->spawn_cooldown = 0;
    env->total_spawned = 0;
    
    
    memset(env->towers, 0, sizeof(env->towers));
    env->num_towers = 0;
    memset(env->projectiles, 0, sizeof(env->projectiles));
    env->num_projectiles = 0;
    
    
    
    env->episode_tower_limit = env->config.min_towers_per_episode + 
        rand() % (env->config.max_towers_per_episode - env->config.min_towers_per_episode + 1);
    
    
    env->gold = env->config.starting_gold;
    env->lives = env->config.starting_lives;
    
    
    env->tick = 0;
    env->enemies_killed = 0;
    env->total_enemies_killed = 0;
    env->enemies_escaped = 0;
    env->normal_towers_placed = 0;
    env->splash_towers_placed = 0;
    env->sniper_towers_placed = 0;
    env->starting_gold = env->gold;
    env->towers_placed = 0;
    env->episode_length = 0;
    env->episode_return = 0.0f;
    
    
    env->total_enemies_to_spawn = MIN_ENEMIES_TO_SPAWN + rand() % MAX_EXTRA_ENEMIES;  
    env->enemies_spawned = 0;
    env->enemies_dealt_with = 0;
    
    
    env->rewards[0] = 0.0f;
    
    
    
    
    update_valid_positions(env);
    
    compute_observations(env);
}

static void update_valid_positions(MinimalTD* env) {
    env->num_valid_positions = 0;
    
    
    for (int i = 0; i < GRID_SIZE; i++) {
        if (env->grid[i] == CELL_EMPTY) {
            int x = i % GRID_WIDTH;
            int y = i / GRID_WIDTH;
            
            
            int adjacent_to_path = 0;
            int dirs[NUM_MOVEMENT_DIRECTIONS][2] = {{1,0}, {-1,0}, {0,1}, {0,-1}};
            for (int d = 0; d < NUM_MOVEMENT_DIRECTIONS; d++) {
                int nx = x + dirs[d][0];
                int ny = y + dirs[d][1];
                if (nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT) {
                    int idx = ny * GRID_WIDTH + nx;
                    if (env->grid[idx] == CELL_PATH) {
                        adjacent_to_path = 1;
                        break;
                    }
                }
            }
            
            if (adjacent_to_path) {
                env->valid_positions[env->num_valid_positions++] = i;
            }
        }
    }
    
    
    if (env->num_valid_positions == 0) {
        for (int i = 0; i < GRID_SIZE; i++) {
            if (env->grid[i] == CELL_EMPTY) {
                env->valid_positions[env->num_valid_positions++] = i;
            }
        }
    }
}

// ============================================================================
// GAME_STEP
// ============================================================================

static void handle_tower_placement(MinimalTD* env, int action) {
    int tower_type, cost, cell_type, grid_idx;
    
    if (action >= ACTION_MIN && action <= NORMAL_TOWER_ACTION_MAX) {
        tower_type = TOWER_TYPE_NORMAL;
        cost = env->config.tower_cost;
        cell_type = CELL_TOWER;
        grid_idx = action - ACTION_MIN;
    } else if (action >= SPLASH_TOWER_ACTION_MIN && action <= SPLASH_TOWER_ACTION_MAX) {
        tower_type = TOWER_TYPE_SPLASH;
        cost = env->config.splash_tower_cost;
        cell_type = CELL_SPLASH_TOWER;
        grid_idx = action - SPLASH_TOWER_ACTION_MIN;
    } else {
        tower_type = TOWER_TYPE_SNIPER;
        cost = env->config.sniper_tower_cost;
        cell_type = CELL_SNIPER_TOWER;
        grid_idx = action - SNIPER_TOWER_ACTION_MIN;
    }
    
    if (env->grid[grid_idx] == CELL_EMPTY && env->gold >= cost) {
        int x = grid_idx % GRID_WIDTH;
        int y = grid_idx / GRID_WIDTH;
        
        env->grid[grid_idx] = cell_type;
        env->gold -= cost;
        env->towers_placed++;
        
        env->towers[env->num_towers].x = x;
        env->towers[env->num_towers].y = y;
        env->towers[env->num_towers].cooldown = 0;
        env->towers[env->num_towers].type = tower_type;
        env->num_towers++;
        
        if (tower_type == TOWER_TYPE_NORMAL) env->normal_towers_placed++;
        else if (tower_type == TOWER_TYPE_SPLASH) env->splash_towers_placed++;
        else if (tower_type == TOWER_TYPE_SNIPER) env->sniper_towers_placed++;
        
        float path_coverage_reward = calculate_path_coverage_reward(env, x, y, tower_type);
        if (path_coverage_reward > 0.0f) {
            env->episode_return += path_coverage_reward;
            env->rewards[0] += path_coverage_reward;
        }
    } else {
        env->rewards[0] += TOWER_PLACEMENT_PENALTY;
        env->episode_return += TOWER_PLACEMENT_PENALTY;
    }
}

static void handle_enemy_spawning(MinimalTD* env) {
    if (env->spawn_cooldown > 0) {
        env->spawn_cooldown--;
        return;
    }
    
    int spawn_count = 1;
    if (rand() % BURST_SPAWN_CHANCE == 0) {
        spawn_count = MIN_BURST_SIZE + rand() % MAX_BURST_SIZE;
    }
    
    for (int i = 0; i < spawn_count; i++) {
        spawn_enemy(env);
    }
    
    env->spawn_cooldown = (spawn_count > 1) ? 
        env->config.spawn_interval * SPAWN_INTERVAL_MULTIPLIER : env->config.spawn_interval;
}

static void handle_episode_end(MinimalTD* env) {
    if (env->lives > 0 && env->enemies_dealt_with >= env->total_enemies_to_spawn) {
        env->rewards[0] += env->lives * env->config.reward_episode_win_multiplier;
        env->episode_return += env->lives * env->config.reward_episode_win_multiplier;
        env->rewards[0] += (float)(WIN_TIME_BONUS_BASE - env->episode_length) * env->config.reward_episode_win_time_bonus;
        env->episode_return += (float)(WIN_TIME_BONUS_BASE - env->episode_length) * env->config.reward_episode_win_time_bonus;
    } else if (env->lives <= 0) {
        env->rewards[0] += env->config.reward_episode_loss;
        env->episode_return += env->config.reward_episode_loss;
    }
    
    env->terminals[0] = 1;
    add_log(env);
    c_reset(env);
}

static int should_episode_end(MinimalTD* env) {
    return (env->lives <= 0) || 
           (env->enemies_dealt_with >= env->total_enemies_to_spawn) ||
           (env->episode_length >= env->max_episode_length);
}

void c_step(MinimalTD* env) {
    env->terminals[0] = 0;
    env->rewards[0] = 0.0f;
    
    int action = (int)env->actions[0];
    
    if (action >= ACTION_MIN && action <= ACTION_MAX) {
        handle_tower_placement(env, action);
    }
    
    handle_enemy_spawning(env);
    move_enemies(env);
    towers_attack(env);
    update_projectiles(env);
    
    env->episode_length++;
    
    if (should_episode_end(env)) {
        handle_episode_end(env);
        return;
    }
    
    env->episode_return += EPISODE_DECAY_RATE;
    env->tick++;
    compute_observations(env);
}

// ============================================================================
// RENDERING_SYSTEM
// ============================================================================

static void render_grid(MinimalTD* env, float cell_size) {
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            int idx = y * GRID_WIDTH + x;
            Color color = (Color){25, 35, 40, 255};
            
            if (env->grid[idx] == CELL_PATH) {
                color = (Color){80, 120, 160, 255};
            } else if (env->grid[idx] == CELL_TOWER) {
                color = (Color){60, 60, 60, 255};
            } else if (env->grid[idx] == CELL_SPLASH_TOWER) {
                color = (Color){40, 80, 120, 255};
            } else if (env->grid[idx] == CELL_SNIPER_TOWER) {
                color = (Color){80, 40, 80, 255};
            } else if (env->grid[idx] == CELL_SPAWNER) {
                color = (Color){120, 40, 40, 255};
            } else if (env->grid[idx] == CELL_GOAL) {
                color = (Color){120, 100, 40, 255};
            }
            
            DrawRectangle(x * cell_size, y * cell_size, cell_size - 1, cell_size - 1, color);
        }
    }
}

static void render_grid_lines(float cell_size) {
    for (int x = 0; x <= GRID_WIDTH; x++) {
        DrawLine(x * cell_size, 0, x * cell_size, GRID_HEIGHT * cell_size, (Color){40, 45, 50, 80});
    }
    for (int y = 0; y <= GRID_HEIGHT; y++) {
        DrawLine(0, y * cell_size, GRID_WIDTH * cell_size, y * cell_size, (Color){40, 45, 50, 80});
    }
}

static void render_tower(MinimalTD* env, Tower* tower, float cell_size) {
    int cx = tower->x * cell_size + cell_size/2;
    int cy = tower->y * cell_size + cell_size/2;
    
    if (tower->type == TOWER_TYPE_SPLASH) {
        DrawCircle(cx, cy, cell_size/2.5, (Color){30, 144, 255, 200});
        DrawCircle(cx, cy, cell_size/3.5, (Color){70, 130, 180, 255});
        DrawCircle(cx, cy, cell_size/5, (Color){255, 255, 255, 180});
        DrawCircleLines(cx, cy, env->config.splash_radius * cell_size, (Color){135, 206, 235, 150});
    } else if (tower->type == TOWER_TYPE_SNIPER) {
        DrawCircle(cx, cy, cell_size/2.5, (Color){138, 43, 226, 200});
        DrawCircle(cx, cy, cell_size/3.5, (Color){75, 0, 130, 255});
        DrawCircle(cx, cy, cell_size/5, (Color){255, 255, 255, 180});
        DrawCircleLines(cx, cy, env->config.sniper_range * cell_size, (Color){218, 112, 214, 120});
        DrawLine(cx-cell_size/4, cy, cx+cell_size/4, cy, WHITE);
        DrawLine(cx, cy-cell_size/4, cx, cy+cell_size/4, WHITE);
    } else {
        DrawCircle(cx, cy, cell_size/2.5, (Color){105, 105, 105, 200});
        DrawCircle(cx, cy, cell_size/3.5, (Color){169, 169, 169, 255});
        DrawCircle(cx, cy, cell_size/5, (Color){255, 255, 255, 180});
        DrawCircleLines(cx, cy, env->config.tower_range * cell_size, (Color){211, 211, 211, 100});
    }
}

static void render_spawners(MinimalTD* env, float cell_size) {
    for (int i = 0; i < env->num_spawners; i++) {
        if (env->spawners[i].active) {
            int cx = env->spawners[i].x * cell_size + cell_size/2;
            int cy = env->spawners[i].y * cell_size + cell_size/2;
            
            float pulse = 0.8f + 0.2f * sinf(env->tick * 0.1f);
            DrawCircle(cx, cy, cell_size/2.2 * pulse, (Color){220, 20, 60, 180});
            DrawCircle(cx, cy, cell_size/3, (Color){139, 0, 0, 255});
            DrawCircle(cx, cy, cell_size/5, (Color){255, 69, 0, 200});
            
            for (int j = 0; j < SPAWNER_ANIMATION_RAYS; j++) {
                float angle = j * SPAWNER_RAY_ANGLE_STEP * DEG2RAD + env->tick * 0.02f;
                int x1 = cx + cos(angle) * cell_size/4;
                int y1 = cy + sin(angle) * cell_size/4;
                int x2 = cx + cos(angle) * cell_size/3;
                int y2 = cy + sin(angle) * cell_size/3;
                DrawLine(x1, y1, x2, y2, (Color){255, 0, 0, 150});
            }
        }
    }
}

static void render_goals(MinimalTD* env, float cell_size) {
    for (int i = 0; i < env->num_goals; i++) {
        if (env->goals[i].active) {
            int cx = env->goals[i].x * cell_size + cell_size/2;
            int cy = env->goals[i].y * cell_size + cell_size/2;
            
            float pulse = 0.9f + 0.1f * sinf(env->tick * 0.08f);
            DrawCircle(cx, cy, cell_size/2.2 * pulse, (Color){255, 215, 0, 180});
            DrawCircle(cx, cy, cell_size/3, (Color){255, 165, 0, 255});
            DrawCircle(cx, cy, cell_size/5, (Color){255, 255, 255, 200});
            
            float ring_pulse = 0.8f + 0.2f * sinf(env->tick * 0.12f);
            DrawCircleLines(cx, cy, cell_size/2.5 * ring_pulse, (Color){255, 215, 0, 120});
            DrawCircleLines(cx, cy, cell_size/3.5, (Color){255, 215, 0, 120});
        }
    }
}

static void render_projectiles(MinimalTD* env, float cell_size) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (env->projectiles[i].active) {
            int px = env->projectiles[i].x * cell_size;
            int py = env->projectiles[i].y * cell_size;
            
            if (env->projectiles[i].is_splash) {
                DrawCircle(px, py, cell_size/5, (Color){135, 206, 235, 100});
                DrawCircle(px, py, cell_size/6, SKYBLUE);
                DrawCircle(px, py, cell_size/8, (Color){255, 255, 255, 180});
            } else {
                DrawCircle(px, py, cell_size/6, (Color){255, 255, 0, 100});
                DrawCircle(px, py, cell_size/8, YELLOW);
                DrawCircle(px, py, cell_size/12, (Color){255, 255, 255, 200});
            }
        }
    }
}

static void render_enemies(MinimalTD* env, float cell_size) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (env->enemies[i].active) {
            int cx = env->enemies[i].x * cell_size + cell_size/2;
            int cy = env->enemies[i].y * cell_size + cell_size/2;
            
            if (env->enemies[i].type == ENEMY_TYPE_HEAVY) {
                if (env->enemies[i].hp >= HEAVY_ENEMY_HP) {
                    DrawCircle(cx, cy, cell_size/3.5, (Color){0, 100, 255, 100});
                    DrawCircle(cx, cy, cell_size/4, BLUE);
                    DrawCircle(cx, cy, cell_size/6, (Color){100, 150, 255, 255});
                } else {
                    DrawCircle(cx, cy, cell_size/3.5, (Color){0, 50, 150, 100});
                    DrawCircle(cx, cy, cell_size/4, DARKBLUE);
                    DrawCircle(cx, cy, cell_size/6, (Color){50, 100, 200, 255});
                }
            } else {
                DrawCircle(cx, cy, cell_size/3.5, (Color){255, 100, 100, 100});
                DrawCircle(cx, cy, cell_size/4, RED);
                DrawCircle(cx, cy, cell_size/6, (Color){255, 150, 150, 255});
            }
            
            DrawCircleLines(cx, cy, cell_size/4, BLACK);
            
            if (env->enemies[i].max_hp > ENEMY_TYPE_HEAVY) {
                DrawText(TextFormat("%d", env->enemies[i].hp), cx-5, cy-8, 12, WHITE);
                DrawText(TextFormat("%d", env->enemies[i].hp), cx-6, cy-9, 12, BLACK);
            }
        }
    }
}

static void draw_status_icon(int x, int y, const char* icon, Color icon_color, Color text_color, const char* value) {
    DrawCircle(x, y + 8, 8, icon_color);
    DrawText(icon, x - 3, y + 4, 12, text_color);
    DrawText(value, x + 15, y, 20, icon_color);
}

static void draw_tower_icon(int x, int y, const char* number, Color color, int cost) {
    DrawCircle(x, y + 8, 8, color);
    DrawText(number, x - 3, y + 4, 12, WHITE);
    DrawText(TextFormat("%dg", cost), x + 15, y, 18, color);
}

static void render_ui(MinimalTD* env, float cell_size) {
    int ui_height = 80;
    int ui_y = GRID_HEIGHT * cell_size + 5;
    
    DrawRectangle(0, GRID_HEIGHT * cell_size, GRID_WIDTH * cell_size, ui_height, (Color){12, 18, 24, 255});
    DrawRectangle(0, GRID_HEIGHT * cell_size, GRID_WIDTH * cell_size, 3, (Color){60, 120, 180, 255});
    
    int status_y = ui_y + 5;
    
    draw_status_icon(20, status_y, "G", YELLOW, BLACK, TextFormat("%d", env->gold));
    draw_status_icon(120, status_y, "♥", RED, WHITE, TextFormat("%d", env->lives));
    draw_status_icon(220, status_y, "X", GREEN, BLACK, TextFormat("%d", env->total_enemies_killed));
    
    draw_tower_icon(350, status_y, "1", GRAY, env->config.tower_cost);
    draw_tower_icon(430, status_y, "2", SKYBLUE, env->config.splash_tower_cost);
    draw_tower_icon(510, status_y, "3", PURPLE, env->config.sniper_tower_cost);
    
    int instruction_y = ui_y + 35;
    DrawText("Press 1-3 to place towers", 10, instruction_y, 16, (Color){200, 200, 200, 255});
    DrawText("• Gray=Rapid  • Blue=Splash  • Purple=Sniper", 10, instruction_y + 20, 14, (Color){170, 170, 170, 255});
    
    if (env->total_enemies_to_spawn > 0) {
        int progress_x = 400;
        int progress_width = 200;
        int progress_height = 8;
        float progress = (float)env->enemies_dealt_with / env->total_enemies_to_spawn;
        
        DrawRectangle(progress_x, instruction_y + 10, progress_width, progress_height, (Color){40, 40, 40, 255});
        DrawRectangle(progress_x, instruction_y + 10, progress_width * progress, progress_height, (Color){60, 180, 60, 255});
        DrawText(TextFormat("Wave: %d/%d", env->enemies_dealt_with, env->total_enemies_to_spawn), 
                 progress_x + progress_width + 10, instruction_y + 5, 14, WHITE);
    }
}

static Client* make_client(MinimalTD* env) {
    Client* client = (Client*)calloc(1, sizeof(Client));
    client->cell_size = 40.0f;  
    client->width = GRID_WIDTH * client->cell_size;
    client->height = GRID_HEIGHT * client->cell_size + 90;  
    
    InitWindow(client->width, client->height, "PufferLib Tower Defense");
    SetTargetFPS(60);
    
    return client;
}

void c_close_client(Client* client) {
    CloseWindow();
    free(client);
}

void c_render(MinimalTD* env) {
    if (env->client == NULL) {
        env->client = make_client(env);
    }
    
    if (IsKeyDown(KEY_ESCAPE)) {
        exit(0);
    }
    
    BeginDrawing();
    ClearBackground((Color){15, 20, 25, 255});
    
    float cell_size = env->client->cell_size;
    
    render_grid(env, cell_size);
    render_grid_lines(cell_size);
    
    for (int t = 0; t < env->num_towers; t++) {
        render_tower(env, &env->towers[t], cell_size);
    }
    
    render_spawners(env, cell_size);
    render_goals(env, cell_size);
    render_projectiles(env, cell_size);
    render_enemies(env, cell_size);
    render_ui(env, cell_size);
    
    EndDrawing();
}
