#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include "raylib.h"

// ============================================================================
// CONSTANTS
// ============================================================================

#define GRID_WIDTH 32
#define GRID_HEIGHT 24
#define GRID_SIZE (GRID_WIDTH * GRID_HEIGHT)

#define MAX_ENEMIES 100
#define MAX_BUILDINGS 50
#define MAX_HEROES 5
#define MAX_WAVES 20
#define MAX_PROJECTILES 50

#define OBS_SIZE (GRID_SIZE + 25)  // Grid + game state scalars
#define MAX_EPISODE_LENGTH 400000  // 100x longer episodes

#define ACTION_MIN 1
#define ACTION_MAX (GRID_WIDTH * GRID_HEIGHT)  // Full grid building

#define BUILDING_EMPTY 0
#define BUILDING_WALL 1
#define BUILDING_TOWER 2
#define BUILDING_MILL 3
#define BUILDING_FACTORY 4
#define BUILDING_REPAIR 5
#define BUILDING_GOAL 6
#define BUILDING_BASE_WALL 7  // Pre-built base walls
#define BUILDING_BARRICADE 8  // Indestructible barricades
#define BUILDING_MOUNTAIN 9   // Impassable terrain boundaries

#define ENEMY_NORMAL 1
#define ENEMY_AIR 2
#define ENEMY_INVISIBLE 3
#define ENEMY_BOSS 4

#define PHASE_SETUP 0
#define PHASE_WAVE_ACTIVE 1
#define PHASE_WAVE_COMPLETE 2
#define PHASE_GAME_OVER 3

// Base position (centered in bottom area)
#define BASE_CENTER_X (GRID_WIDTH / 2)
#define BASE_CENTER_Y (GRID_HEIGHT * 3 / 4)  
#define BASE_SIZE 16
#define FRONT_GAP_SIZE 4

// ============================================================================
// COLOR SCHEMES (inspired by rift)
// ============================================================================

typedef struct {
    Color background;
    Color ui_background_top, ui_background_bottom;
    Color ui_border_main, ui_border_bright, ui_border_dark;
    Color text_main, text_highlight, text_gold;
    Color health_low, health_mid, health_high;
} UIColorScheme;

typedef struct {
    Color ground_base, ground_highlight, ground_shadow;
    Color wall_base, wall_highlight, wall_mortar;
    Color base_wall, base_wall_highlight;
    Color tower_stone, tower_metal, tower_gem;
    Color mill_wood, mill_roof, mill_wheel;
    Color goal_core, goal_glow, goal_pulse;
} BuildingColorScheme;

typedef struct {
    Color normal_base, normal_eyes;
    Color air_base, air_wings;
    Color invisible_base, invisible_fade;
    Color boss_core, boss_armor, boss_glow;
} EnemyColorScheme;

static const UIColorScheme UI_COLORS = {
    .background = {15, 25, 15, 255},
    .ui_background_top = {20, 35, 25, 240},
    .ui_background_bottom = {10, 20, 15, 240},
    .ui_border_main = {80, 120, 90, 255},
    .ui_border_bright = {120, 180, 130, 200},
    .ui_border_dark = {40, 60, 45, 255},
    .text_main = {220, 240, 220, 255},
    .text_highlight = {100, 255, 120, 255},
    .text_gold = {255, 215, 0, 255}
};

static const BuildingColorScheme BUILDING_COLORS = {
    .ground_base = {45, 60, 35, 255},
    .ground_highlight = {60, 80, 45, 255},
    .ground_shadow = {30, 40, 25, 255},
    .wall_base = {120, 100, 80, 255},
    .wall_highlight = {150, 125, 100, 255},
    .wall_mortar = {80, 65, 50, 255},
    .base_wall = {160, 140, 120, 255},
    .base_wall_highlight = {190, 170, 150, 255},
    .tower_stone = {100, 110, 120, 255},
    .tower_metal = {150, 160, 170, 255},
    .tower_gem = {100, 200, 255, 255},
    .mill_wood = {139, 115, 85, 255},
    .mill_roof = {180, 100, 60, 255},
    .mill_wheel = {101, 67, 33, 255},
    .goal_core = {255, 215, 0, 255},
    .goal_glow = {255, 255, 150, 200},
    .goal_pulse = {255, 255, 255, 150}
};

static const EnemyColorScheme ENEMY_COLORS = {
    .normal_base = {180, 80, 80, 255},
    .normal_eyes = {255, 50, 50, 255},
    .air_base = {150, 150, 200, 255},
    .air_wings = {200, 200, 255, 180},
    .invisible_base = {120, 120, 120, 128},
    .invisible_fade = {160, 160, 160, 64},
    .boss_core = {128, 0, 128, 255},
    .boss_armor = {200, 50, 50, 255},
    .boss_glow = {255, 100, 255, 150}
};

// ============================================================================
// CONFIGURATION
// ============================================================================

typedef struct GameConfig {
    int starting_lumber;
    int starting_gold;
    float enemy_spawn_rate;
    float wave_difficulty_scale;
    int max_enemies_per_wave;
    int setup_time_ticks;
    float wall_build_cost;
    float tower_build_cost;
    float mill_build_cost;
    float completion_reward;
    float survival_reward_per_tick;
    float enemy_kill_reward;
    float death_penalty;
    float goal_death_penalty;
    int build_test_walls;
} GameConfig;

static const GameConfig DEFAULT_CONFIG = {
    .starting_lumber = 100,
    .starting_gold = 0,
    .enemy_spawn_rate = 0.3f,
    .wave_difficulty_scale = 1.15f,
    .max_enemies_per_wave = 12,
    .setup_time_ticks = 600,  // Longer setup time
    .wall_build_cost = 8.0f,
    .tower_build_cost = 20.0f,
    .mill_build_cost = 15.0f,
    .completion_reward = 50.0f,
    .survival_reward_per_tick = 0.15f,
    .enemy_kill_reward = 3.0f,
    .death_penalty = 15.0f,
    .goal_death_penalty = 25.0f,
    .build_test_walls = 1
};

// ============================================================================
// ENTITY STRUCTURES
// ============================================================================

typedef struct Enemy {
    float x, y;
    float target_x, target_y;
    float speed;
    uint8_t type;
    uint8_t active;
    uint16_t health;
    uint16_t max_health;
    uint16_t damage;
    uint16_t attack_cooldown;
    uint16_t id;
    uint16_t repath_timer;
    uint8_t stuck_counter;
} Enemy;

typedef struct Building {
    uint8_t x, y;
    uint8_t type;
    uint8_t active;
    uint16_t health;
    uint16_t max_health;
    uint8_t level;
    uint16_t attack_cooldown;
    float attack_range;
    uint16_t damage;
    uint16_t id;
} Building;

typedef struct Hero {
    float x, y;
    uint8_t active;
    uint16_t health;
    uint16_t max_health;
    uint16_t damage;
    uint8_t level;
    uint32_t experience;
    uint16_t id;
} Hero;

typedef struct Projectile {
    float x, y;
    float vel_x, vel_y;
    float target_x, target_y;
    uint8_t active;
    uint16_t damage;
    uint16_t lifetime;
    uint8_t type;
} Projectile;

typedef struct WaveManager {
    uint8_t current_wave;
    uint8_t enemies_spawned;
    uint8_t enemies_remaining;
    uint16_t spawn_timer;
    uint8_t wave_active;
    uint16_t wave_start_delay;
} WaveManager;

typedef struct Resources {
    uint16_t lumber;
    uint16_t gold;
    uint8_t lumber_production_rate;
} Resources;

typedef struct Log {
    float episode_return;
    float episode_length;
    float enemies_killed;
    float buildings_built;
    float waves_completed;
    float lumber_produced;
    float gold_earned;
    float damage_dealt;
    float damage_taken;
    float walls_destroyed;
    float towers_built;
    float mills_built;
    float heroes_leveled;
    float survival_time;
    float wave_completion_rewards;
    float enemy_kill_rewards;
    float death_penalties;
    float goal_death_penalties;
    float goal_deaths;
    float current_wave;
    float current_lumber;
    float current_gold;
    float goal_health_ratio;
    float games_lost;
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

typedef struct Goal {
    float x, y;
    uint8_t active;
    int32_t health;
    int32_t max_health;
} Goal;

typedef struct FortressSurvival {
    Client* client;
    Log log;
    float* observations;
    float* actions;
    float* rewards;
    unsigned char* terminals;
    
    unsigned char grid[GRID_SIZE];
    
    Enemy enemies[MAX_ENEMIES];
    Building buildings[MAX_BUILDINGS];
    Hero heroes[MAX_HEROES];
    Projectile projectiles[MAX_PROJECTILES];
    Goal goal;
    
    WaveManager wave_manager;
    Resources resources;
    
    GameConfig config;
    
    uint8_t current_phase;
    uint32_t tick;
    uint16_t episode_length;
    float episode_return;
    
    uint8_t human_control;
    uint8_t selected_building;  // 0=none, 1=wall, 2=tower, 3=mill
    uint8_t building_mode;
    
    uint16_t next_enemy_id;
    uint16_t next_building_id;
    uint16_t next_hero_id;
    
    uint16_t episode_games_lost;
    
    uint8_t num_enemies;
    uint8_t num_buildings;
    uint8_t num_heroes;
    uint8_t num_projectiles;
    uint8_t goal_under_attack;
    
    uint16_t episode_enemies_killed;
    uint16_t episode_buildings_built;
    uint8_t episode_waves_completed;
    uint16_t episode_lumber_produced;
    uint16_t episode_gold_earned;
    uint16_t episode_damage_dealt;
    uint16_t episode_damage_taken;
    uint16_t episode_walls_destroyed;
    uint16_t episode_towers_built;
    uint16_t episode_mills_built;
    uint8_t episode_heroes_leveled;
    uint16_t episode_survival_time;
    uint8_t episode_goal_deaths;
    
    float episode_wave_completion_rewards;
    float episode_enemy_kill_rewards;
    float episode_death_penalties;
    float episode_goal_death_penalties;
    
    float cell_size_render;
} FortressSurvival;

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

int count_active_enemies(FortressSurvival* env);
void render_building_bar(FortressSurvival* env);
void render_building_preview(FortressSurvival* env);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

static inline int get_grid_index(int x, int y) {
    if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT) return -1;
    return y * GRID_WIDTH + x;
}

static inline float distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

static inline int is_valid_position(int x, int y) {
    return x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT;
}

static inline int is_position_blocked(FortressSurvival* env, int x, int y) {
    if (!is_valid_position(x, y)) return 1;
    int idx = get_grid_index(x, y);
    if (idx < 0) return 1;
    
    // Mountains, base walls, barricades, and player walls block movement
    return (env->grid[idx] == BUILDING_MOUNTAIN ||
            env->grid[idx] == BUILDING_BASE_WALL || 
            env->grid[idx] == BUILDING_BARRICADE || 
            env->grid[idx] == BUILDING_WALL);
}

static inline int can_move_to(FortressSurvival* env, float from_x, float from_y, float to_x, float to_y) {
    int grid_to_x = (int)to_x;
    int grid_to_y = (int)to_y;
    
    // Check if destination is blocked by terrain
    if (!is_valid_position(grid_to_x, grid_to_y)) return 0;
    int idx = get_grid_index(grid_to_x, grid_to_y);
    if (idx < 0) return 0;
    
    // ALL BUILDINGS BLOCK MOVEMENT - mountains, walls, towers, mills, etc
    if (env->grid[idx] != BUILDING_EMPTY && env->grid[idx] != BUILDING_GOAL) {
        return 0;  // Blocked by any building except goal
    }
    
    // REMOVED ENEMY COLLISION - Let enemies overlap
    
    return 1;
}

static void find_path_to_goal(FortressSurvival* env, Enemy* enemy) {
    float goal_x = env->goal.x;
    float goal_y = env->goal.y;
    float old_x = enemy->x;
    float old_y = enemy->y;
    
    // DEBUG: Print pathfinding info occasionally
    if (enemy->repath_timer % 60 == 0) {
        fprintf(stderr, "PATHFINDING: enemy at (%.1f,%.1f) -> goal (%.1f,%.1f)\n", 
               enemy->x, enemy->y, goal_x, goal_y);
        fflush(stderr);
    }
    
    // Update repath timer
    enemy->repath_timer++;
    
    float dx = goal_x - enemy->x;
    float dy = goal_y - enemy->y;
    float dist = sqrtf(dx * dx + dy * dy);
    
    if (dist < 0.1f) return;
    
    // Try direct path to goal first
    float new_x = enemy->x + (dx / dist) * enemy->speed;
    float new_y = enemy->y + (dy / dist) * enemy->speed;
    
    if (can_move_to(env, enemy->x, enemy->y, new_x, new_y)) {
        enemy->x = new_x;
        enemy->y = new_y;
        enemy->stuck_counter = 0;
        
        // DEBUG: Print movement occasionally
        if (enemy->repath_timer % 60 == 0) {
            fprintf(stderr, "MOVING: enemy moved to (%.1f,%.1f)\n", enemy->x, enemy->y);
            fflush(stderr);
        }
        return;
    } else {
        // DEBUG: Print why movement failed
        if (enemy->repath_timer % 60 == 0) {
            fprintf(stderr, "BLOCKED: enemy at (%.1f,%.1f) can't move to (%.1f,%.1f)\n", 
                   enemy->x, enemy->y, new_x, new_y);
            fflush(stderr);
        }
    }
    
    // CHECK FOR BUILDINGS IN ATTACK RANGE - DON'T GLITCH INTO THEM
    // Look for buildings within 1.5 units (melee range)
    for (int j = 0; j < MAX_BUILDINGS; j++) {
        Building* building = &env->buildings[j];
        if (!building->active) continue;
        // Skip indestructible buildings
        if (building->type == BUILDING_MOUNTAIN || building->type == BUILDING_BARRICADE) continue;
        
        float build_dist = distance(enemy->x, enemy->y, building->x, building->y);
        if (build_dist <= 1.5f && enemy->attack_cooldown == 0) {
            // ATTACK THE BUILDING - don't move, just attack
            building->health -= enemy->damage;
            enemy->attack_cooldown = 30;
            env->episode_damage_dealt += enemy->damage;
            
            // Debug: Show attack
            fprintf(stderr, "ATTACKING: enemy at (%.1f,%.1f) attacks building at (%d,%d) for %d damage (HP: %d->%d)\n", 
                   enemy->x, enemy->y, building->x, building->y, enemy->damage, 
                   building->health + enemy->damage, building->health);
            fflush(stderr);
            
            if (building->health <= 0) {
                building->active = 0;
                env->num_buildings--;
                int grid_idx = get_grid_index(building->x, building->y);
                if (grid_idx >= 0) env->grid[grid_idx] = BUILDING_EMPTY;
                if (building->type == BUILDING_WALL) env->episode_walls_destroyed++;
                
                fprintf(stderr, "DESTROYED: building at (%d,%d) destroyed!\n", building->x, building->y);
                fflush(stderr);
            }
            
            enemy->stuck_counter = 0;
            return; // Don't move this turn, just attack
        }
    }
    
    // If direct path blocked, try alternative directions
    float moves[][2] = {
        {(dx > 0 ? enemy->speed : -enemy->speed), 0},  // x direction
        {0, (dy > 0 ? enemy->speed : -enemy->speed)},  // y direction
        {(dx > 0 ? enemy->speed * 0.7f : -enemy->speed * 0.7f), (dy > 0 ? enemy->speed * 0.7f : -enemy->speed * 0.7f)}, // diagonal
        {-(dx > 0 ? enemy->speed * 0.5f : -enemy->speed * 0.5f), (dy > 0 ? enemy->speed : -enemy->speed)}, // around left
        {(dx > 0 ? enemy->speed : -enemy->speed), -(dy > 0 ? enemy->speed * 0.5f : -enemy->speed * 0.5f)}, // around right
    };
    
    int moved = 0;
    for (int i = 0; i < 5; i++) {
        float test_x = enemy->x + moves[i][0];
        float test_y = enemy->y + moves[i][1];
        
        if (can_move_to(env, enemy->x, enemy->y, test_x, test_y)) {
            enemy->x = test_x;
            enemy->y = test_y;
            enemy->stuck_counter = 0;
            moved = 1;
            break;
        }
    }
    
    // Count stuck frames if didn't move
    if (!moved) {
        if (fabsf(enemy->x - old_x) < 0.01f && fabsf(enemy->y - old_y) < 0.01f) {
            enemy->stuck_counter++;
        }
        
        // If stuck for more than 1 second, ATTACK NEAREST BUILDING
        if (enemy->stuck_counter > 60 && enemy->attack_cooldown == 0) {
            float nearest_building_dist = 999.0f;
            Building* target_building = NULL;
            
            // Find nearest attackable building
            for (int j = 0; j < MAX_BUILDINGS; j++) {
                Building* building = &env->buildings[j];
                if (!building->active) continue;
                // Skip mountains and barricades (indestructible)
                if (building->type == BUILDING_MOUNTAIN || building->type == BUILDING_BARRICADE) continue;
                
                float build_dist = distance(enemy->x, enemy->y, building->x, building->y);
                if (build_dist < nearest_building_dist && build_dist < 2.5f) {
                    nearest_building_dist = build_dist;
                    target_building = building;
                }
            }
            
            // ATTACK THE BUILDING
            if (target_building) {
                target_building->health -= enemy->damage;
                enemy->attack_cooldown = 30;
                env->episode_damage_dealt += enemy->damage;
                
                if (target_building->health <= 0) {
                    target_building->active = 0;
                    env->num_buildings--;
                    int grid_idx = get_grid_index(target_building->x, target_building->y);
                    if (grid_idx >= 0) env->grid[grid_idx] = BUILDING_EMPTY;
                    if (target_building->type == BUILDING_WALL) env->episode_walls_destroyed++;
                }
                
                enemy->stuck_counter = 0; // Reset after successful attack
            }
        }
    }
}

// ============================================================================
// ENVIRONMENT LIFECYCLE
// ============================================================================

void init(FortressSurvival* env) {
    env->tick = 0;
    env->config = DEFAULT_CONFIG;
    env->current_phase = PHASE_SETUP;
    env->next_enemy_id = 1;
    env->next_building_id = 1;
    env->next_hero_id = 1;
}

void allocate(FortressSurvival* env) {
    env->tick = 0;
    env->episode_return = 0.0f;
    env->episode_games_lost = 0;
    // DON'T RESET human_control - keep it persistent
    env->selected_building = 0;
    env->building_mode = 0;
    env->client = NULL;
    memset(&env->log, 0, sizeof(Log));
    init(env);
}

void c_close(FortressSurvival* env) {
    
}

void free_allocated(FortressSurvival* env) {
    free(env->actions);
    free(env->observations);
    free(env->terminals);
    free(env->rewards);
    c_close(env);
}

// ============================================================================
// OBSERVATION SYSTEM
// ============================================================================

void setup_base(FortressSurvival* env) {
    // Clear the entire grid first
    for (int i = 0; i < GRID_SIZE; i++) {
        env->grid[i] = BUILDING_EMPTY;
    }
    
    // CREATE IMPASSABLE MOUNTAIN TERRAIN BOUNDARIES
    // Bottom edge - COMPLETE MOUNTAIN WALL
    for (int x = 0; x < GRID_WIDTH; x++) {
        int bottom_idx = get_grid_index(x, GRID_HEIGHT - 1);
        if (bottom_idx >= 0) env->grid[bottom_idx] = BUILDING_MOUNTAIN;
        // Make it 3 tiles thick
        int bottom_idx2 = get_grid_index(x, GRID_HEIGHT - 2);
        if (bottom_idx2 >= 0) env->grid[bottom_idx2] = BUILDING_MOUNTAIN;
        int bottom_idx3 = get_grid_index(x, GRID_HEIGHT - 3);
        if (bottom_idx3 >= 0) env->grid[bottom_idx3] = BUILDING_MOUNTAIN;
    }
    
    // Left edge - COMPLETE MOUNTAIN WALL
    for (int y = 0; y < GRID_HEIGHT; y++) {
        int left_idx = get_grid_index(0, y);
        if (left_idx >= 0) env->grid[left_idx] = BUILDING_MOUNTAIN;
        // Make it 3 tiles thick
        int left_idx2 = get_grid_index(1, y);
        if (left_idx2 >= 0) env->grid[left_idx2] = BUILDING_MOUNTAIN;
        int left_idx3 = get_grid_index(2, y);
        if (left_idx3 >= 0) env->grid[left_idx3] = BUILDING_MOUNTAIN;
    }
    
    // Right edge - COMPLETE MOUNTAIN WALL
    for (int y = 0; y < GRID_HEIGHT; y++) {
        int right_idx = get_grid_index(GRID_WIDTH - 1, y);
        if (right_idx >= 0) env->grid[right_idx] = BUILDING_MOUNTAIN;
        // Make it 3 tiles thick
        int right_idx2 = get_grid_index(GRID_WIDTH - 2, y);
        if (right_idx2 >= 0) env->grid[right_idx2] = BUILDING_MOUNTAIN;
        int right_idx3 = get_grid_index(GRID_WIDTH - 3, y);
        if (right_idx3 >= 0) env->grid[right_idx3] = BUILDING_MOUNTAIN;
    }
    
    // ADD INDESTRUCTIBLE FRONT WALL WITH 4-WIDE ENTRY
    // Place front wall much higher up to leave room for enemies
    int front_wall_y = 8;  // Plenty of room from top edge
    int gap_start = BASE_CENTER_X - 2;  // 4 wide entry
    int gap_end = BASE_CENTER_X + 1;
    
    for (int x = 3; x < GRID_WIDTH - 3; x++) {  // Leave side room too
        if (x < gap_start || x > gap_end) {
            int front_idx = get_grid_index(x, front_wall_y);
            if (front_idx >= 0) env->grid[front_idx] = BUILDING_BARRICADE;
        }
    }
    
    // Place goal in center
    env->goal.x = BASE_CENTER_X;
    env->goal.y = BASE_CENTER_Y;
    env->goal.active = 1;
    env->goal.health = 1000;
    env->goal.max_health = 1000;
    
    // Initialize wave manager
    env->wave_manager.wave_active = 0;
    env->wave_manager.current_wave = 0;
    env->wave_manager.enemies_remaining = 0;
    env->wave_manager.enemies_spawned = 0;
    env->wave_manager.wave_start_delay = 300;  // 5 seconds before first wave
    env->wave_manager.spawn_timer = 0;
    
    int goal_idx = get_grid_index(BASE_CENTER_X, BASE_CENTER_Y);
    if (goal_idx >= 0) env->grid[goal_idx] = BUILDING_GOAL;
    
    // ADD TEMPORARY USER WALLS TO BLOCK THE 4-WIDE ENTRY
    if (env->config.build_test_walls) {
        for (int x = gap_start; x <= gap_end; x++) {
            int wall_idx = get_grid_index(x, front_wall_y);
            if (wall_idx >= 0) {
                env->grid[wall_idx] = BUILDING_WALL;
                
                // Add building struct
                for (int b = 0; b < MAX_BUILDINGS; b++) {
                    if (!env->buildings[b].active) {
                        env->buildings[b].active = 1;
                        env->buildings[b].type = BUILDING_WALL;
                        env->buildings[b].x = x;
                        env->buildings[b].y = front_wall_y;
                        env->buildings[b].health = 100;
                        env->buildings[b].max_health = 100;
                        env->buildings[b].id = env->next_building_id++;
                        env->num_buildings++;
                        break;
                    }
                }
            }
        }
    }
}

void compute_observations(FortressSurvival* env) {
    int obs_idx = 0;
    
    for (int i = 0; i < GRID_SIZE; i++) {
        env->observations[obs_idx++] = (float)env->grid[i] / 10.0f;
    }
    
    env->observations[obs_idx++] = env->resources.lumber / 1000.0f;
    env->observations[obs_idx++] = env->resources.gold / 500.0f;
    env->observations[obs_idx++] = env->wave_manager.current_wave / (float)MAX_WAVES;
    env->observations[obs_idx++] = count_active_enemies(env) / (float)MAX_ENEMIES;
    env->observations[obs_idx++] = env->num_buildings / (float)MAX_BUILDINGS;
    env->observations[obs_idx++] = env->current_phase / 3.0f;
    env->observations[obs_idx++] = env->wave_manager.enemies_remaining / (float)env->config.max_enemies_per_wave;
    env->observations[obs_idx++] = env->wave_manager.spawn_timer / 100.0f;
    env->observations[obs_idx++] = env->episode_length / (float)MAX_EPISODE_LENGTH;
    
    int walls_count = 0;
    int towers_count = 0;
    int mills_count = 0;
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        if (env->buildings[i].active) {
            if (env->buildings[i].type == BUILDING_WALL) walls_count++;
            else if (env->buildings[i].type == BUILDING_TOWER) towers_count++;
            else if (env->buildings[i].type == BUILDING_MILL) mills_count++;
        }
    }
    
    env->observations[obs_idx++] = walls_count / 20.0f;
    env->observations[obs_idx++] = towers_count / 10.0f;
    env->observations[obs_idx++] = mills_count / 5.0f;
    
    float total_enemy_health = 0.0f;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (env->enemies[i].active) {
            total_enemy_health += env->enemies[i].health;
        }
    }
    env->observations[obs_idx++] = total_enemy_health / 1000.0f;
    
    float total_building_health = 0.0f;
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        if (env->buildings[i].active) {
            total_building_health += env->buildings[i].health;
        }
    }
    env->observations[obs_idx++] = total_building_health / 2000.0f;
    
    env->observations[obs_idx++] = env->resources.lumber_production_rate / 10.0f;
    env->observations[obs_idx++] = env->wave_manager.wave_active ? 1.0f : 0.0f;
    env->observations[obs_idx++] = (env->wave_manager.wave_start_delay > 0) ? 1.0f : 0.0f;
    env->observations[obs_idx++] = env->episode_return / 100.0f;
    env->observations[obs_idx++] = env->episode_enemies_killed / 100.0f;
    env->observations[obs_idx++] = env->goal.health / (float)env->goal.max_health;
    env->observations[obs_idx++] = env->goal_under_attack ? 1.0f : 0.0f;
    env->observations[obs_idx++] = distance(BASE_CENTER_X, BASE_CENTER_Y, 0, 0) / 50.0f; // Normalized distance to base
    
    // Add enemy proximity to goal
    float closest_enemy_to_goal = 999.0f;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (env->enemies[i].active) {
            float dist = distance(env->enemies[i].x, env->enemies[i].y, env->goal.x, env->goal.y);
            if (dist < closest_enemy_to_goal) {
                closest_enemy_to_goal = dist;
            }
        }
    }
    env->observations[obs_idx++] = closest_enemy_to_goal / 50.0f;
    env->observations[obs_idx++] = env->episode_goal_deaths / 10.0f;
}

// ============================================================================
// LOGGING SYSTEM
// ============================================================================

static void add_log(FortressSurvival* env) {
    env->log.episode_return += env->episode_return;
    env->log.episode_length += env->episode_length;
    env->log.enemies_killed += env->episode_enemies_killed;
    env->log.buildings_built += env->episode_buildings_built;
    env->log.waves_completed += env->episode_waves_completed;
    env->log.lumber_produced += env->episode_lumber_produced;
    env->log.gold_earned += env->episode_gold_earned;
    env->log.damage_dealt += env->episode_damage_dealt;
    env->log.damage_taken += env->episode_damage_taken;
    env->log.walls_destroyed += env->episode_walls_destroyed;
    env->log.towers_built += env->episode_towers_built;
    env->log.mills_built += env->episode_mills_built;
    env->log.heroes_leveled += env->episode_heroes_leveled;
    env->log.survival_time += env->episode_survival_time;
    env->log.wave_completion_rewards += env->episode_wave_completion_rewards;
    env->log.enemy_kill_rewards += env->episode_enemy_kill_rewards;
    env->log.death_penalties += env->episode_death_penalties;
    env->log.goal_death_penalties += env->episode_goal_death_penalties;
    env->log.goal_deaths += env->episode_goal_deaths;
    env->log.current_wave += env->wave_manager.current_wave;
    env->log.current_lumber += env->resources.lumber;
    env->log.current_gold += env->resources.gold;
    env->log.goal_health_ratio += (env->goal.active ? (float)env->goal.health / env->goal.max_health : 0.0f);
    env->log.games_lost += env->episode_games_lost;
    env->log.n += 1;
}

// ============================================================================
// RESET SYSTEM
// ============================================================================

void c_reset(FortressSurvival* env) {
    memset(env->grid, 0, GRID_SIZE);
    memset(env->enemies, 0, sizeof(env->enemies));
    memset(env->buildings, 0, sizeof(env->buildings));
    memset(env->heroes, 0, sizeof(env->heroes));
    memset(env->projectiles, 0, sizeof(env->projectiles));
    memset(&env->goal, 0, sizeof(env->goal));
    
    env->current_phase = PHASE_SETUP;
    env->tick = 0;
    env->episode_length = 0;
    env->episode_return = 0.0f;
    env->episode_games_lost = 0;
    // DON'T RESET human_control - keep it persistent
    env->selected_building = 0;
    env->building_mode = 0;
    env->rewards[0] = 0.0f;
    
    env->resources.lumber = env->config.starting_lumber;
    env->resources.gold = env->config.starting_gold;
    env->resources.lumber_production_rate = 1;
    
    env->wave_manager.current_wave = 0;
    env->wave_manager.enemies_spawned = 0;
    env->wave_manager.enemies_remaining = 0;
    env->wave_manager.spawn_timer = 0;
    env->wave_manager.wave_active = 0;
    env->wave_manager.wave_start_delay = env->config.setup_time_ticks;
    
    // Clear all enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        env->enemies[i].active = 0;
    }
    env->num_enemies = 0;
    env->num_buildings = 0;
    env->num_heroes = 0;
    env->num_projectiles = 0;
    env->goal_under_attack = 0;
    
    env->episode_enemies_killed = 0;
    env->episode_buildings_built = 0;
    env->episode_waves_completed = 0;
    env->episode_lumber_produced = 0;
    env->episode_gold_earned = 0;
    env->episode_damage_dealt = 0;
    env->episode_damage_taken = 0;
    env->episode_walls_destroyed = 0;
    env->episode_towers_built = 0;
    env->episode_mills_built = 0;
    env->episode_heroes_leveled = 0;
    env->episode_survival_time = 0;
    env->episode_goal_deaths = 0;
    
    env->episode_wave_completion_rewards = 0.0f;
    env->episode_enemy_kill_rewards = 0.0f;
    env->episode_death_penalties = 0.0f;
    env->episode_goal_death_penalties = 0.0f;
    
    setup_base(env);
    compute_observations(env);
}

// ============================================================================
// BUILDING SYSTEM
// ============================================================================

int can_build_at(FortressSurvival* env, int x, int y, int building_type) {
    if (!is_valid_position(x, y)) return 0;
    
    int grid_idx = get_grid_index(x, y);
    if (grid_idx == -1 || env->grid[grid_idx] != 0) return 0;
    
    float cost = 0.0f;
    switch (building_type) {
        case BUILDING_WALL: cost = env->config.wall_build_cost; break;
        case BUILDING_TOWER: cost = env->config.tower_build_cost; break;
        case BUILDING_MILL: cost = env->config.mill_build_cost; break;
        default: return 0;
    }
    
    return env->resources.lumber >= cost;
}

void build_structure(FortressSurvival* env, int x, int y, int building_type) {
    if (!can_build_at(env, x, y, building_type)) return;
    if (env->num_buildings >= MAX_BUILDINGS) return;
    
    float cost = 0.0f;
    uint16_t health = 100;
    uint16_t damage = 0;
    float range = 0.0f;
    
    switch (building_type) {
        case BUILDING_WALL:
            cost = env->config.wall_build_cost;
            health = 150;
            break;
        case BUILDING_TOWER:
            cost = env->config.tower_build_cost;
            health = 80;
            damage = 25;
            range = 3.0f;
            env->episode_towers_built++;
            break;
        case BUILDING_MILL:
            cost = env->config.mill_build_cost;
            health = 60;
            env->resources.lumber_production_rate++;
            env->episode_mills_built++;
            break;
        default: return;
    }
    
    env->resources.lumber -= (int)cost;
    
    Building* building = &env->buildings[env->num_buildings];
    building->x = x;
    building->y = y;
    building->type = building_type;
    building->active = 1;
    building->health = health;
    building->max_health = health;
    building->level = 1;
    building->attack_cooldown = 0;
    building->attack_range = range;
    building->damage = damage;
    building->id = env->next_building_id++;
    
    int grid_idx = get_grid_index(x, y);
    env->grid[grid_idx] = building_type;
    
    env->num_buildings++;
    env->episode_buildings_built++;
}

// ============================================================================
// WAVE MANAGEMENT SYSTEM
// ============================================================================


// ============================================================================
// ENEMY SYSTEM
// ============================================================================

void spawn_enemy(FortressSurvival* env, uint8_t enemy_type) {
    // DEBUG: Force print to stderr
    fprintf(stderr, "SPAWN_ENEMY CALLED: type=%d\n", enemy_type);
    fflush(stderr);
    
    // Find first inactive enemy slot
    int enemy_slot = -1;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!env->enemies[i].active) {
            enemy_slot = i;
            break;
        }
    }
    
    if (enemy_slot == -1) {
        fprintf(stderr, "SPAWN_ENEMY FAILED: No free slots\n");
        fflush(stderr);
        return;  // No free slots
    }
    
    fprintf(stderr, "SPAWN_ENEMY SUCCESS: slot=%d\n", enemy_slot);
    fflush(stderr);
    
    // SPAWN FROM TOP OF MAP (left, center, right randomly)
    float spawn_y = 0.0f;  // Always spawn from top edge
    float spawn_x;
    
    int spawn_zone = rand() % 3;
    if (spawn_zone == 0) {
        // Top left area
        spawn_x = 4 + (rand() % 8);
    } else if (spawn_zone == 1) {
        // Top center area
        spawn_x = (GRID_WIDTH / 2) - 4 + (rand() % 8);
    } else {
        // Top right area
        spawn_x = GRID_WIDTH - 12 + (rand() % 8);
    }
    
    Enemy* enemy = &env->enemies[enemy_slot];
    enemy->x = spawn_x;
    enemy->y = spawn_y;
    enemy->target_x = env->goal.x;  // Target the actual goal
    enemy->target_y = env->goal.y;
    enemy->type = enemy_type;
    enemy->active = 1;
    enemy->speed = 0.5f;  // MUCH FASTER FOR DEBUG
    enemy->attack_cooldown = 0;
    enemy->repath_timer = 0;
    enemy->stuck_counter = 0;
    enemy->id = env->next_enemy_id++;
    
    float wave_scale = powf(env->config.wave_difficulty_scale, env->wave_manager.current_wave);
    
    switch (enemy_type) {
        case ENEMY_NORMAL:
            enemy->health = (uint16_t)(30 * wave_scale);
            enemy->damage = (uint16_t)(25 * wave_scale);  // Increased from 10
            break;
        case ENEMY_AIR:
            enemy->health = (uint16_t)(20 * wave_scale);
            enemy->damage = (uint16_t)(20 * wave_scale);  // Increased from 8
            enemy->speed = 0.15f;
            break;
        case ENEMY_INVISIBLE:
            enemy->health = (uint16_t)(25 * wave_scale);
            enemy->damage = (uint16_t)(30 * wave_scale);  // Increased from 12
            enemy->speed = 0.12f;
            break;
        case ENEMY_BOSS:
            enemy->health = (uint16_t)(200 * wave_scale);
            enemy->damage = (uint16_t)(50 * wave_scale);  // Increased from 30
            enemy->speed = 0.08f;
            break;
    }
    
    enemy->max_health = enemy->health;
    env->num_enemies++;
}


// ============================================================================
// TOWER SYSTEM
// ============================================================================

void update_towers(FortressSurvival* env) {
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building* tower = &env->buildings[i];
        if (!tower->active || tower->type != BUILDING_TOWER) continue;
        
        if (tower->attack_cooldown > 0) {
            tower->attack_cooldown--;
            continue;
        }
        
        Enemy* target = NULL;
        float closest_dist = tower->attack_range + 1.0f;
        
        for (int j = 0; j < MAX_ENEMIES; j++) {
            Enemy* enemy = &env->enemies[j];
            if (!enemy->active) continue;
            
            float dist = distance(tower->x, tower->y, enemy->x, enemy->y);
            if (dist <= tower->attack_range && dist < closest_dist) {
                target = enemy;
                closest_dist = dist;
            }
        }
        
        if (target != NULL) {
            target->health -= tower->damage;
            tower->attack_cooldown = 20;
            env->episode_damage_dealt += tower->damage;
        }
    }
}

// ============================================================================
// WAVE SYSTEM
// ============================================================================

void update_wave_manager(FortressSurvival* env) {
    if (env->current_phase == PHASE_SETUP) {
        if (env->wave_manager.wave_start_delay > 0) {
            env->wave_manager.wave_start_delay--;
        } else {
            env->current_phase = PHASE_WAVE_ACTIVE;
            env->wave_manager.current_wave++;
            env->wave_manager.wave_active = 1;
            env->wave_manager.enemies_remaining = env->config.max_enemies_per_wave;
            env->wave_manager.enemies_spawned = 0;
        }
        return;
    }
    
    if (env->current_phase == PHASE_WAVE_ACTIVE) {
        if (env->wave_manager.enemies_spawned < env->config.max_enemies_per_wave) {
            if (env->wave_manager.spawn_timer <= 0) {
                uint8_t enemy_type = ENEMY_NORMAL;
                if (env->wave_manager.current_wave > 3 && rand() % 4 == 0) {
                    enemy_type = ENEMY_AIR;
                } else if (env->wave_manager.current_wave > 5 && rand() % 6 == 0) {
                    enemy_type = ENEMY_INVISIBLE;
                } else if (env->wave_manager.current_wave > 8 && env->wave_manager.enemies_spawned == 0) {
                    enemy_type = ENEMY_BOSS;
                }
                
                fprintf(stderr, "CALLING spawn_enemy: timer=%d, spawned=%d, type=%d\n", 
                       env->wave_manager.spawn_timer, env->wave_manager.enemies_spawned, enemy_type);
                fflush(stderr);
                
                spawn_enemy(env, enemy_type);
                env->wave_manager.enemies_spawned++;
                env->wave_manager.spawn_timer = 60 - (env->wave_manager.current_wave * 2);
                if (env->wave_manager.spawn_timer < 10) env->wave_manager.spawn_timer = 10;
            } else {
                env->wave_manager.spawn_timer--;
            }
        }
        
        if (env->wave_manager.enemies_remaining <= 0) {
            env->current_phase = PHASE_WAVE_COMPLETE;
            env->wave_manager.wave_active = 0;
            env->episode_waves_completed++;
            
            float wave_reward = env->config.completion_reward * env->wave_manager.current_wave;
            env->rewards[0] += wave_reward;
            env->episode_return += wave_reward;
            env->episode_wave_completion_rewards += wave_reward;
            
            if (env->wave_manager.current_wave >= MAX_WAVES) {
                env->current_phase = PHASE_GAME_OVER;
            } else {
                env->wave_manager.wave_start_delay = 120;
                env->current_phase = PHASE_SETUP;
            }
        }
    }
}

// ============================================================================
// RESOURCE SYSTEM  
// ============================================================================

void update_resources(FortressSurvival* env) {
    if (env->tick % 60 == 0) {
        env->resources.lumber += env->resources.lumber_production_rate;
        env->episode_lumber_produced += env->resources.lumber_production_rate;
    }
}

// ============================================================================
// GAME STEP
// ============================================================================

static void handle_episode_end(FortressSurvival* env) {
    if (env->current_phase == PHASE_GAME_OVER) {
        float completion_bonus = env->config.completion_reward * env->episode_waves_completed;
        env->rewards[0] += completion_bonus;
        env->episode_return += completion_bonus;
    }
    
    env->terminals[0] = 1;
    add_log(env);
    c_reset(env);
}

static int should_episode_end(FortressSurvival* env) {
    // ONLY END ON GOAL DEATH - not episode length
    if (env->current_phase == PHASE_GAME_OVER) return 1;
    
    // Only end episode if goal is destroyed
    if (!env->goal.active || env->goal.health <= 0) {
        return 1;
    }
    
    return 0;
}

void update_enemies(FortressSurvival* env) {
    env->goal_under_attack = 0;
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* enemy = &env->enemies[i];
        if (!enemy->active) continue;
        
        if (enemy->health <= 0) {
            enemy->active = 0;
            env->num_enemies--;
            env->episode_enemies_killed++;
            env->wave_manager.enemies_remaining--;
            
            env->rewards[0] += env->config.enemy_kill_reward;
            env->episode_return += env->config.enemy_kill_reward;
            env->episode_enemy_kill_rewards += env->config.enemy_kill_reward;
            continue;
        }
        
        // Set target to goal
        enemy->target_x = env->goal.x;
        enemy->target_y = env->goal.y;
        
        float dx = enemy->target_x - enemy->x;
        float dy = enemy->target_y - enemy->y;
        float dist = sqrtf(dx * dx + dy * dy);
        
        // Check if enemy reached goal
        if (dist < 1.2f && enemy->attack_cooldown == 0) {
            env->goal.health -= enemy->damage;
            enemy->attack_cooldown = 30;
            env->goal_under_attack = 1;
            env->episode_damage_taken += enemy->damage;
            
            if (env->goal.health <= 0) {
                env->rewards[0] -= env->config.goal_death_penalty;
                env->episode_return -= env->config.goal_death_penalty;
                env->episode_goal_death_penalties += env->config.goal_death_penalty;
                env->episode_goal_deaths++;
                env->episode_games_lost++;
                
                env->current_phase = PHASE_GAME_OVER;
                env->terminals[0] = 1;
                add_log(env);
                c_reset(env);
                return;
            }
        }
        
        // Use improved pathfinding to navigate to goal
        find_path_to_goal(env, enemy);
        
        
        if (enemy->attack_cooldown > 0) {
            enemy->attack_cooldown--;
        }
    }
}

int count_active_enemies(FortressSurvival* env) {
    int count = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (env->enemies[i].active) count++;
    }
    return count;
}

static int get_human_action(FortressSurvival* env) {
    Vector2 mouse_pos = GetMousePosition();
    int cell_size = (int)env->client->cell_size;
    int grid_x = (int)(mouse_pos.x / cell_size);
    int grid_y = (int)(mouse_pos.y / cell_size);
    
    // Handle building selection keys (1, 2, 3)
    if (IsKeyPressed(KEY_ONE)) {
        env->selected_building = (env->selected_building == 1) ? 0 : 1;
        env->building_mode = (env->selected_building > 0);
    }
    if (IsKeyPressed(KEY_TWO)) {
        env->selected_building = (env->selected_building == 2) ? 0 : 2;
        env->building_mode = (env->selected_building > 0);
    }
    if (IsKeyPressed(KEY_THREE)) {
        env->selected_building = (env->selected_building == 3) ? 0 : 3;
        env->building_mode = (env->selected_building > 0);
    }
    
    // Handle ESC to cancel building mode
    if (IsKeyPressed(KEY_ESCAPE)) {
        env->selected_building = 0;
        env->building_mode = 0;
    }
    
    // Handle mouse clicks for building placement
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        // Check if clicking on building bar
        int screen_height = env->client->height;
        int bar_y = screen_height - 80;
        
        if (mouse_pos.y >= bar_y) {
            // Clicked on building bar - handle selection
            int screen_width = env->client->width;
            int icon_spacing = 80;
            int start_x = screen_width/2 - icon_spacing;
            
            for (int i = 0; i < 3; i++) {
                int icon_x = start_x + i * icon_spacing;
                if (mouse_pos.x >= icon_x && mouse_pos.x <= icon_x + 48) {
                    int building_type = i + 1;
                    env->selected_building = (env->selected_building == building_type) ? 0 : building_type;
                    env->building_mode = (env->selected_building > 0);
                    break;
                }
            }
        } else if (env->building_mode && env->selected_building > 0) {
            // Place building at mouse position
            if (is_valid_position(grid_x, grid_y)) {
                // Convert to action index for building placement
                return grid_y * GRID_WIDTH + grid_x + 1;
            }
        }
    }
    
    // Right click to cancel building mode
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        env->selected_building = 0;
        env->building_mode = 0;
    }
    
    return 0;
}

void c_step(FortressSurvival* env) {
    env->terminals[0] = 0;
    env->rewards[0] = 0.0f;
    
    // Check for Shift key toggle for human control
    if (IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT)) {
        env->human_control = !env->human_control;
    }
    
    int action;
    if (env->human_control) {
        // Human control mode - get action from keyboard/mouse
        action = get_human_action(env);
    } else {
        // AI control mode - use action from network
        action = (int)env->actions[0];
    }
    
    if (action >= ACTION_MIN && action <= ACTION_MAX) {
        int build_x = (action - 1) % GRID_WIDTH;
        int build_y = (action - 1) / GRID_WIDTH;
        
        // Building placement logic
        if (env->human_control && env->building_mode && env->selected_building > 0) {
            // Human building mode - use selected building type
            int building_type = 0;
            float cost = 0;
            
            switch (env->selected_building) {
                case 1: building_type = BUILDING_WALL; cost = env->config.wall_build_cost; break;
                case 2: building_type = BUILDING_TOWER; cost = env->config.tower_build_cost; break;
                case 3: building_type = BUILDING_MILL; cost = env->config.mill_build_cost; break;
            }
            
            if (env->resources.lumber >= cost) {
                int grid_idx = get_grid_index(build_x, build_y);
                if (grid_idx >= 0 && env->grid[grid_idx] == BUILDING_EMPTY) {
                    build_structure(env, build_x, build_y, building_type);
                }
            }
        } else if (!env->human_control) {
            // AI building logic (simplified)
            if (env->current_phase == PHASE_SETUP || env->current_phase == PHASE_WAVE_COMPLETE) {
                if (env->resources.lumber >= env->config.tower_build_cost) {
                    int grid_idx = get_grid_index(build_x, build_y);
                    if (grid_idx >= 0 && env->grid[grid_idx] == BUILDING_EMPTY) {
                        build_structure(env, build_x, build_y, BUILDING_TOWER);
                    }
                }
            }
        }
    }
    
    update_resources(env);
    update_wave_manager(env);
    update_enemies(env);
    update_towers(env);
    
    env->rewards[0] += env->config.survival_reward_per_tick;
    env->episode_return += env->config.survival_reward_per_tick;
    env->episode_survival_time++;
    
    env->episode_length++;
    
    if (should_episode_end(env)) {
        handle_episode_end(env);
        return;
    }
    
    env->tick++;
    compute_observations(env);
}

// ============================================================================
// RENDERING SYSTEM
// ============================================================================

static Client* make_client(FortressSurvival* env) {
    Client* client = (Client*)calloc(1, sizeof(Client));
    client->cell_size = env->cell_size_render;
    client->width = GRID_WIDTH * client->cell_size;
    client->height = GRID_HEIGHT * client->cell_size;
    
    InitWindow(client->width, client->height, "PufferLib Fortress Survival");
    SetTargetFPS(60);
    
    client->camera.target = (Vector2){ client->width / 2.0f, client->height / 2.0f };
    client->camera.offset = (Vector2){ client->width / 2.0f, client->height / 2.0f };
    client->camera.rotation = 0.0f;
    client->camera.zoom = 1.0f;
    
    return client;
}

void c_close_client(Client* client) {
    CloseWindow();
    free(client);
}

static inline void draw_building_detailed(float x, float y, float size, int building_type, int tick) {
    switch (building_type) {
        case BUILDING_WALL: {
            // Player-built walls - LIGHT BROWN/TAN WOODEN LOOK
            Color user_wall_base = (Color){160, 120, 80, 255};  // Light brown
            Color user_wall_highlight = (Color){200, 160, 120, 255};  // Lighter brown
            Color user_wall_trim = (Color){100, 70, 40, 255};   // Dark brown
            
            DrawRectangle(x, y, size, size, user_wall_base);
            DrawRectangle(x + 2, y + 2, size - 4, size - 4, user_wall_highlight);
            DrawRectangleLines(x, y, size, size, user_wall_trim);
            
            // Wooden plank lines
            for (int i = 1; i < 4; i++) {
                DrawRectangle(x, y + i * size/4, size, 1, user_wall_trim);
            }
            DrawRectangle(x + size/2, y, 2, size, user_wall_trim);
            break;
        }
        case BUILDING_BASE_WALL: {
            // Base walls - DARK STONE FORTIFICATION
            Color base_wall_stone = (Color){60, 60, 70, 255};     // Dark grey stone
            Color base_wall_highlight = (Color){80, 80, 90, 255}; // Lighter grey
            Color base_wall_mortar = (Color){40, 40, 50, 255};    // Very dark grey
            
            DrawRectangle(x, y, size, size, base_wall_stone);
            DrawRectangle(x + 1, y + 1, size - 2, size - 2, base_wall_highlight);
            DrawRectangleLines(x, y, size, size, base_wall_mortar);
            
            // Stone block pattern
            for (int i = 0; i < 3; i++) {
                DrawRectangle(x + i * size/3, y, 1, size, base_wall_mortar);
                DrawRectangle(x, y + i * size/3, size, 1, base_wall_mortar);
            }
            break;
        }
        case BUILDING_BARRICADE: {
            // Indestructible barricades - dark red with reinforcement
            Color barricade_base = (Color){120, 40, 40, 255};
            Color barricade_highlight = (Color){150, 60, 60, 255};
            Color barricade_metal = (Color){80, 80, 80, 255};
            
            DrawRectangle(x, y, size, size, barricade_base);
            DrawRectangle(x + 1, y + 1, size - 2, size - 2, barricade_highlight);
            DrawRectangleLines(x, y, size, size, barricade_metal);
            
            // Heavy metal reinforcement pattern
            DrawRectangle(x + size/4, y, size/2, size, barricade_metal);
            DrawRectangle(x, y + size/4, size, size/2, barricade_metal);
            
            // Spikes on top for intimidation
            for (int i = 0; i < 3; i++) {
                int spike_x = x + (i + 1) * size / 4;
                DrawTriangle((Vector2){spike_x, y}, (Vector2){spike_x - 2, y + 4}, (Vector2){spike_x + 2, y + 4}, barricade_metal);
            }
            break;
        }
        case BUILDING_MOUNTAIN: {
            // IMPASSABLE MOUNTAIN TERRAIN - DARK ROCKY
            Color mountain_base = (Color){40, 35, 30, 255};    // Very dark brown
            Color mountain_rock = (Color){50, 45, 40, 255};    // Slightly lighter
            Color mountain_shadow = (Color){25, 20, 15, 255};  // Almost black
            
            DrawRectangle(x, y, size, size, mountain_base);
            
            // Rocky texture with random variations
            int rock_seed = ((int)x * 7 + (int)y * 13) % 8;
            if (rock_seed < 3) {
                DrawRectangle(x + 1, y + 1, size - 2, size - 2, mountain_rock);
            }
            if (rock_seed < 5) {
                DrawRectangle(x + 2, y + 2, size - 4, size - 4, mountain_shadow);
            }
            
            // Jagged mountain outline
            DrawRectangleLines(x, y, size, size, mountain_shadow);
            
            // Random rocky protrusions
            if (rock_seed % 3 == 0) {
                DrawRectangle(x, y, 3, 3, mountain_rock);
            }
            if (rock_seed % 3 == 1) {
                DrawRectangle(x + size - 3, y, 3, 3, mountain_rock);
            }
            if (rock_seed % 3 == 2) {
                DrawRectangle(x, y + size - 3, 3, 3, mountain_rock);
            }
            break;
        }
        case BUILDING_TOWER: {
            // Tower with gem on top
            DrawRectangle(x + size/6, y + size/6, 2*size/3, 2*size/3, BUILDING_COLORS.tower_stone);
            DrawRectangle(x + size/4, y + size/4, size/2, size/2, BUILDING_COLORS.tower_metal);
            DrawRectangleLines(x + size/6, y + size/6, 2*size/3, 2*size/3, BUILDING_COLORS.wall_mortar);
            
            // Pulsing gem
            float pulse = 0.8f + 0.2f * sinf(tick * 0.1f);
            int gem_size = (int)(size/6 * pulse);
            DrawCircle(x + size/2, y + size/3, gem_size, BUILDING_COLORS.tower_gem);
            DrawCircle(x + size/2, y + size/3, gem_size - 2, (Color){255, 255, 255, 100});
            break;
        }
        case BUILDING_MILL: {
            // Wooden mill building
            DrawRectangle(x, y, size, size, BUILDING_COLORS.mill_wood);
            DrawRectangle(x + 2, y + 2, size - 4, size - 4, BUILDING_COLORS.mill_roof);
            DrawRectangleLines(x, y, size, size, BUILDING_COLORS.wall_mortar);
            
            // Rotating wheel
            float rotation = tick * 2.0f;
            Vector2 center = {x + size/2, y + size/2};
            for (int i = 0; i < 4; i++) {
                float angle = rotation + i * 90 * DEG2RAD;
                float spoke_x = center.x + cosf(angle) * size/4;
                float spoke_y = center.y + sinf(angle) * size/4;
                DrawLineEx(center, (Vector2){spoke_x, spoke_y}, 2, BUILDING_COLORS.mill_wheel);
            }
            DrawCircle(center.x, center.y, 3, BUILDING_COLORS.mill_wheel);
            break;
        }
        case BUILDING_GOAL: {
            // Glowing goal with pulsing effect
            float pulse = 0.7f + 0.3f * sinf(tick * 0.05f);
            Color glow_color = BUILDING_COLORS.goal_glow;
            glow_color.a = (unsigned char)(150 * pulse);
            
            DrawCircle(x + size/2, y + size/2, size/2 * pulse, glow_color);
            DrawCircle(x + size/2, y + size/2, size/3, BUILDING_COLORS.goal_core);
            DrawCircle(x + size/2, y + size/2, size/4, BUILDING_COLORS.goal_pulse);
            
            // Orbiting particles
            for (int i = 0; i < 6; i++) {
                float orbit_angle = tick * 0.03f + i * 60 * DEG2RAD;
                float orbit_x = x + size/2 + cosf(orbit_angle) * size/2;
                float orbit_y = y + size/2 + sinf(orbit_angle) * size/2;
                DrawCircle(orbit_x, orbit_y, 2, BUILDING_COLORS.goal_pulse);
            }
            break;
        }
    }
}

static inline void draw_enemy_detailed(float x, float y, float size, int enemy_type, int tick, float health_ratio) {
    switch (enemy_type) {
        case ENEMY_NORMAL: {
            // Shadow
            DrawCircle(x + 2, y + 2, size, (Color){0, 0, 0, 100});
            // Body
            DrawCircle(x, y, size, ENEMY_COLORS.normal_base);
            DrawCircle(x, y, size * 0.7f, (Color){120, 60, 60, 255});
            // Eyes
            DrawCircle(x - size/3, y - size/4, 2, ENEMY_COLORS.normal_eyes);
            DrawCircle(x + size/3, y - size/4, 2, ENEMY_COLORS.normal_eyes);
            break;
        }
        case ENEMY_AIR: {
            // Shadow on ground
            DrawCircle(x + 2, y + size + 4, size * 0.5f, (Color){0, 0, 0, 80});
            // Floating body with wings
            float hover = sinf(tick * 0.2f) * 3;
            DrawCircle(x, y + hover, size, ENEMY_COLORS.air_base);
            DrawCircle(x, y + hover, size * 0.6f, (Color){100, 100, 150, 255});
            // Wings
            for (int i = 0; i < 2; i++) {
                float wing_angle = (i == 0 ? -30 : 30) * DEG2RAD + sinf(tick * 0.3f) * 0.2f;
                float wing_x = x + cosf(wing_angle) * size;
                float wing_y = y + hover + sinf(wing_angle) * size * 0.5f;
                DrawCircle(wing_x, wing_y, size * 0.4f, ENEMY_COLORS.air_wings);
            }
            break;
        }
        case ENEMY_INVISIBLE: {
            // Flickering visibility
            float flicker = sinf(tick * 0.4f) * 0.3f + 0.7f;
            Color fade_color = ENEMY_COLORS.invisible_base;
            fade_color.a = (unsigned char)(fade_color.a * flicker);
            DrawCircle(x, y, size, fade_color);
            DrawCircle(x, y, size * 0.6f, ENEMY_COLORS.invisible_fade);
            // Distortion effect
            DrawCircleLines(x, y, size * 1.2f, (Color){200, 200, 200, 30});
            break;
        }
        case ENEMY_BOSS: {
            // Large pulsing boss
            float pulse = 0.9f + 0.1f * sinf(tick * 0.1f);
            DrawCircle(x + 4, y + 4, size * 1.5f, (Color){0, 0, 0, 150});
            DrawCircle(x, y, size * 1.5f * pulse, ENEMY_COLORS.boss_armor);
            DrawCircle(x, y, size, ENEMY_COLORS.boss_core);
            DrawCircle(x, y, size * 0.5f, (Color){255, 100, 255, 200});
            
            // Energy aura
            Color glow = ENEMY_COLORS.boss_glow;
            glow.a = (unsigned char)(100 + 50 * sinf(tick * 0.15f));
            DrawCircleLines(x, y, size * 2.0f, glow);
            
            // Health bar
            if (health_ratio < 1.0f) {
                int bar_width = (int)(size * 2.5f);
                int bar_height = 6;
                int bar_x = x - bar_width/2;
                int bar_y = y - size * 2.0f;
                
                DrawRectangle(bar_x, bar_y, bar_width, bar_height, (Color){60, 60, 60, 255});
                DrawRectangle(bar_x, bar_y, (int)(bar_width * health_ratio), bar_height, (Color){255, 50, 50, 255});
                DrawRectangleLines(bar_x, bar_y, bar_width, bar_height, (Color){255, 255, 255, 255});
            }
            break;
        }
    }
}

void c_render(FortressSurvival* env) {
    if (env->client == NULL) {
        env->client = make_client(env);
    }
    
    if (IsKeyDown(KEY_ESCAPE)) {
        exit(0);
    }
    
    BeginDrawing();
    ClearBackground(UI_COLORS.background);
    
    BeginMode2D(env->client->camera);
    
    float cell_size = env->client->cell_size;
    
    // Draw ground/terrain
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            int idx = get_grid_index(x, y);
            Color ground_color = BUILDING_COLORS.ground_base;
            
            // Add some variation to ground
            if ((x + y) % 3 == 0) ground_color = BUILDING_COLORS.ground_highlight;
            if ((x + y) % 7 == 0) ground_color = BUILDING_COLORS.ground_shadow;
            
            DrawRectangle(x * cell_size, y * cell_size, cell_size - 1, cell_size - 1, ground_color);
            
            // Draw buildings
            if (idx >= 0 && env->grid[idx] != BUILDING_EMPTY) {
                draw_building_detailed(x * cell_size, y * cell_size, cell_size - 1, env->grid[idx], env->tick);
            }
        }
    }
    
    // Draw enemies with detailed graphics
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* enemy = &env->enemies[i];
        if (!enemy->active) continue;
        
        float enemy_x = enemy->x * cell_size + cell_size/2;
        float enemy_y = enemy->y * cell_size + cell_size/2;
        float enemy_size = cell_size/3;
        float health_ratio = (float)enemy->health / enemy->max_health;
        
        draw_enemy_detailed(enemy_x, enemy_y, enemy_size, enemy->type, env->tick, health_ratio);
    }
    
    EndMode2D();
    
    // Enhanced UI with rift-style theming - moved to top
    int ui_height = 80;
    int ui_y = 0;
    
    // UI background with gradient
    DrawRectangleGradientV(0, ui_y, env->client->width, ui_height, 
                          UI_COLORS.ui_background_top, UI_COLORS.ui_background_bottom);
    DrawRectangle(0, ui_y + ui_height, env->client->width, 2, UI_COLORS.ui_border_main);
    DrawRectangle(0, ui_y + ui_height + 1, env->client->width, 1, UI_COLORS.ui_border_bright);
    
    // Wave counter - NO BLINKING
    DrawText(TextFormat("Wave %d", env->wave_manager.current_wave), 20, ui_y + 10, 22, UI_COLORS.text_highlight);
    
    // Resources (lumber only)
    DrawText(TextFormat("Lumber: %d", env->resources.lumber), 20, ui_y + 35, 18, UI_COLORS.text_gold);
    DrawText(TextFormat("Production: +%d/sec", env->resources.lumber_production_rate), 20, ui_y + 55, 14, UI_COLORS.text_main);
    
    // Game state
    DrawText(TextFormat("Enemies: %d", count_active_enemies(env)), 200, ui_y + 15, 14, UI_COLORS.text_main);
    DrawText(TextFormat("Buildings: %d", env->num_buildings), 200, ui_y + 35, 14, UI_COLORS.text_main);
    
    // Control mode indicator
    const char* control_text = env->human_control ? "HUMAN CONTROL" : "AI CONTROL";
    Color control_color = env->human_control ? (Color){50, 255, 50, 255} : (Color){255, 255, 50, 255};
    DrawText(control_text, 200, ui_y + 55, 12, control_color);
    DrawText("(Shift to toggle)", 200, ui_y + 68, 10, UI_COLORS.text_main);
    
    // Goal health bar
    if (env->goal.active) {
        int goal_bar_x = 350;
        int goal_bar_y = ui_y + 15;
        int goal_bar_w = 120;
        int goal_bar_h = 8;
        float goal_health_ratio = (float)env->goal.health / env->goal.max_health;
        
        DrawText("Goal Health:", goal_bar_x, goal_bar_y - 15, 12, UI_COLORS.text_main);
        DrawRectangle(goal_bar_x, goal_bar_y, goal_bar_w, goal_bar_h, (Color){60, 60, 60, 255});
        
        Color health_color = (Color){50, 255, 50, 255};
        if (goal_health_ratio < 0.5f) health_color = (Color){255, 255, 50, 255};
        if (goal_health_ratio < 0.25f) health_color = (Color){255, 50, 50, 255};
        
        DrawRectangle(goal_bar_x, goal_bar_y, (int)(goal_bar_w * goal_health_ratio), goal_bar_h, health_color);
        DrawRectangleLines(goal_bar_x, goal_bar_y, goal_bar_w, goal_bar_h, UI_COLORS.ui_border_main);
        
        if (env->goal_under_attack) {
            Color warning_color = (Color){255, 100, 100, 255};
            DrawText("UNDER ATTACK!", goal_bar_x, goal_bar_y + 15, 12, warning_color);
        }
    }
    
    // Phase indicator
    const char* phase_text = "Unknown";
    Color phase_color = UI_COLORS.text_main;
    switch (env->current_phase) {
        case PHASE_SETUP: 
            phase_text = ">>> PREPARE DEFENSES <<<"; 
            phase_color = UI_COLORS.text_gold;
            break;
        case PHASE_WAVE_ACTIVE: 
            phase_text = ">>> WAVE ACTIVE <<<"; 
            phase_color = (Color){255, 100, 100, 255};
            break;
        case PHASE_WAVE_COMPLETE: 
            phase_text = ">>> WAVE COMPLETE <<<"; 
            phase_color = UI_COLORS.text_highlight;
            break;
        case PHASE_GAME_OVER: 
            phase_text = ">>> FORTRESS FALLEN <<<"; 
            phase_color = (Color){255, 50, 50, 255};
            break;
    }
    
    int phase_text_width = MeasureText(phase_text, 16);
    DrawText(phase_text, env->client->width/2 - phase_text_width/2, ui_y + 50, 16, phase_color);
    
    // TOWER BUILDING BAR - ALWAYS SHOW
    render_building_bar(env);
    
    // Show building preview if in building mode
    if (env->human_control && env->building_mode && env->selected_building > 0) {
        render_building_preview(env);
    }
    
    EndDrawing();
}

void render_building_bar(FortressSurvival* env) {
    int screen_width = env->client->width;
    int screen_height = env->client->height;
    
    // Bottom building bar - inspired by rift skill bar
    int bar_height = 80;
    int bar_y = screen_height - bar_height;
    
    // Dark background
    DrawRectangleGradientV(0, bar_y, screen_width, bar_height, 
                          (Color){20, 25, 30, 200}, (Color){10, 15, 20, 220});
    DrawRectangle(0, bar_y, screen_width, 2, (Color){100, 120, 140, 255});
    
    // Building icons (3 types: Wall, Tower, Mill)
    int icon_size = 48;
    int icon_spacing = 80;
    int start_x = screen_width/2 - icon_spacing;
    int icon_y = bar_y + 16;
    
    // Building costs and availability
    struct {
        const char* name;
        const char* key;
        int cost;
        int building_type;
        Color base_color;
    } buildings[] = {
        {"Wall", "1", (int)env->config.wall_build_cost, 1, (Color){160, 120, 80, 255}},
        {"Tower", "2", (int)env->config.tower_build_cost, 2, (Color){100, 100, 150, 255}},
        {"Mill", "3", (int)env->config.mill_build_cost, 3, (Color){80, 150, 80, 255}}
    };
    
    for (int i = 0; i < 3; i++) {
        int x = start_x + i * icon_spacing;
        
        // Check if affordable
        int affordable = env->resources.lumber >= buildings[i].cost;
        int selected = (env->selected_building == buildings[i].building_type);
        
        // Icon background
        Color bg_color = buildings[i].base_color;
        if (!affordable) {
            bg_color.r /= 3; bg_color.g /= 3; bg_color.b /= 3;
        }
        if (selected) {
            // Bright glow for selected
            DrawCircle(x + icon_size/2, icon_y + icon_size/2, icon_size/2 + 4, (Color){255, 255, 100, 100});
        }
        
        DrawCircle(x + icon_size/2, icon_y + icon_size/2, icon_size/2, bg_color);
        DrawCircleLines(x + icon_size/2, icon_y + icon_size/2, icon_size/2, (Color){200, 200, 200, 255});
        
        // Building icon (simple shapes)
        int center_x = x + icon_size/2;
        int center_y = icon_y + icon_size/2;
        
        if (i == 0) { // Wall
            DrawRectangle(center_x - 12, center_y - 8, 24, 16, (Color){120, 90, 60, 255});
            DrawRectangleLines(center_x - 12, center_y - 8, 24, 16, (Color){80, 60, 40, 255});
        } else if (i == 1) { // Tower
            DrawRectangle(center_x - 8, center_y - 12, 16, 24, (Color){70, 70, 120, 255});
            DrawCircle(center_x, center_y - 8, 4, (Color){150, 100, 255, 255});
        } else { // Mill
            DrawRectangle(center_x - 10, center_y - 6, 20, 12, (Color){60, 120, 60, 255});
            DrawCircle(center_x + 8, center_y, 6, (Color){100, 80, 60, 255});
        }
        
        // Key and cost text
        DrawText(buildings[i].key, x + 2, icon_y + 2, 14, (Color){255, 255, 255, 255});
        DrawText(buildings[i].name, x, icon_y + icon_size + 4, 12, (Color){200, 200, 200, 255});
        DrawText(TextFormat("%d lumber", buildings[i].cost), x, icon_y + icon_size + 18, 10, 
                affordable ? (Color){100, 255, 100, 255} : (Color){255, 100, 100, 255});
    }
}

void render_building_preview(FortressSurvival* env) {
    Vector2 mouse_pos = GetMousePosition();
    int cell_size = (int)env->client->cell_size;
    int grid_x = (int)(mouse_pos.x / cell_size);
    int grid_y = (int)(mouse_pos.y / cell_size);
    
    if (!is_valid_position(grid_x, grid_y)) return;
    
    int x = grid_x * cell_size;
    int y = grid_y * cell_size;
    
    // Check if position is valid for building
    int grid_idx = get_grid_index(grid_x, grid_y);
    int can_build = (grid_idx >= 0 && env->grid[grid_idx] == BUILDING_EMPTY);
    
    // Ghost building preview
    Color preview_color = can_build ? (Color){100, 255, 100, 100} : (Color){255, 100, 100, 100};
    
    if (env->selected_building == 1) { // Wall preview
        DrawRectangle(x, y, cell_size, cell_size, preview_color);
        DrawRectangleLines(x, y, cell_size, cell_size, (Color){255, 255, 255, 150});
    } else if (env->selected_building == 2) { // Tower preview
        DrawCircle(x + cell_size/2, y + cell_size/2, cell_size/3, preview_color);
        DrawCircleLines(x + cell_size/2, y + cell_size/2, cell_size/3, (Color){255, 255, 255, 150});
    } else if (env->selected_building == 3) { // Mill preview
        DrawRectangle(x + 4, y + 4, cell_size - 8, cell_size - 8, preview_color);
        DrawCircle(x + cell_size - 8, y + cell_size/2, 6, preview_color);
    }
}