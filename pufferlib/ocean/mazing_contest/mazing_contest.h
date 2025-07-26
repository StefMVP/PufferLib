#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <raylib.h>

// Grid constants - adjustable for different maze sizes
#define GRID_WIDTH 10
#define GRID_HEIGHT 10
#define GRID_SIZE (GRID_WIDTH * GRID_HEIGHT)  // 100 for 10x10
#define MAX_TOWERS 80  // Scale up for larger grid (roughly 0.8 towers per cell)
#define MAX_THUNDERCLAP_TOWERS 2

// Action space constants (walls + thunderclaps)
#define ACTION_BUILD_WALL_START 0
#define ACTION_BUILD_WALL_END (GRID_SIZE - 1)  // 0 to 99 for 10x10
#define ACTION_BUILD_THUNDERCLAP_START GRID_SIZE
#define ACTION_BUILD_THUNDERCLAP_END (2 * GRID_SIZE - 1)  // 100 to 199 for 10x10
#define TOTAL_ACTIONS (2 * GRID_SIZE)  // Wall + thunderclap actions (200 for 10x10)

// Game phases
typedef enum {
    PHASE_BUILD = 0,
    PHASE_RUN = 1
} GamePhase;

// Tower types
typedef enum {
    TOWER_WALL = 0,
    TOWER_THUNDERCLAP = 1
} TowerType;

// Core data structures
typedef struct {
    int active;
    TowerType type;
    int grid_x, grid_y;
    float x, y;
    int cooldown;
    int last_activated_tick;
    int touched_by_runner;  // Track if runner has been within 1 grid space
    int touch_count;        // Count how many times runner has been near
    int currently_touching_edges[9];  // 1 if runner is currently touching this edge, 0 otherwise
    int previously_touching_edges[9]; // 1 if runner was touching this edge last frame, 0 otherwise
    int edge_touch_cooldown[9];       // Cooldown timer for each edge to prevent spam counting
    int is_random_obstacle; // 1 if this is a random obstacle, 0 if player-placed
    int caused_new_slow;    // 1 if this thunderclap ever caused a NEW slow (not when already slowed)
} Tower;

typedef struct {
    int active;
    float x, y;
    int duration;
    int max_duration;
} ThunderclapEffect;

typedef struct {
    float x, y;
    float speed;
    int slowed_until_tick;
    int target_x, target_y;
    int stuck_counter;
} Runner;

typedef struct {
    // Resources
    int gold;
    int lumber;
    
    // Game state
    GamePhase phase;
    int phase_timer;
    int round_number;
    float runner_time;
    float best_time;
    int maze_completed;
    
    // Configuration
    int build_time_limit;
    int max_moves;  // Maximum moves before episode terminates (like drone_race)
    int max_rounds; // Maximum rounds before episode terminates (alternative to max_moves)
    int min_gold, max_gold;
    int min_lumber, max_lumber;
    int num_thunderclap_towers;
    
    
    // Thunderclap settings
    float thunderclap_slowdown_factor;  // 0.3 = 30% speed (70% slower)
    int thunderclap_slowdown_duration;  // in ticks (60 ticks = 1 second)
    float thunderclap_range;           // activation range in pixels
    int thunderclap_cooldown;          // cooldown in ticks
    
    // Runner settings
    float runner_base_speed;           // base movement speed
    
    // Reward values
    float path_length_reward_multiplier; // MAIN REWARD: multiplier for path length increases
    // tower_utilization_reward removed - no longer used
    float wall_touch_base_reward;      // base reward for wall touches (exponential multiplier)
    float thunderclap_activation_reward; // reward for thunderclap activation
    float thunderclap_slowdown_reward;  // reward per tick of slowdown
    float thunderclap_unused_penalty;  // penalty for thunderclap towers never activated
    float runner_time_reward_multiplier; // multiplier for runner time reward
    
    // Grid and world constants
    float grid_cell_size;              // size of each grid cell in pixels
    float grid_cell_center_offset;     // offset to center of grid cell
    
    // Observation normalization constants
    float gold_normalization;          // divisor for gold normalization
    float lumber_normalization;        // divisor for lumber normalization
    float phase_normalization;         // divisor for phase normalization
    float build_time_normalization;    // divisor for build time normalization
    float runner_time_normalization;   // divisor for runner time normalization
    
    // Timing constants
    float fps;                         // frames per second (for time conversion)
    int stuck_teleport_ticks;          // ticks before teleporting stuck runner
    int tower_touch_cooldown_ticks;    // ticks before tower can be touched again
    
    // Rendering constants
    float cell_size_render;            // cell size for rendering
    
    // Random obstacle generation
    int min_random_walls;              // minimum number of random walls to place
    int max_random_walls;              // maximum number of random walls to place
    int min_random_thunderclaps;       // minimum number of random thunderclaps to place
    int max_random_thunderclaps;       // maximum number of random thunderclaps to place
    
    // Goal system
    int goal_enabled;                  // 1 if goal system is enabled, 0 if disabled
} GameConfig;

typedef struct {
    float episode_return;
    float episode_length;
    float n;
    
    // Mazing contest specific stats
    float towers_built;
    float total_gold_spent;
    float best_time;              // Best (longest) runner time achieved this episode
    float rounds_completed;
    float average_path_length;    // Average path length across all rounds this episode
    float walls_built;
    float final_path_length;      // Final path length achieved
    float path_length_rewards;    // Total rewards from path length increases
    // towers_touched, towers_untouched, tower_utilization_rewards removed
    float thunderclap_towers_built;   // Number of thunderclap towers built
    float thunderclap_slowdown_time;  // Total time runner was slowed (in seconds)
    float thunderclap_rewards;        // Total rewards from thunderclap activations and slowdowns
    float wall_touches;               // Total number of wall touches across all walls
    float wall_touch_rewards;         // Total rewards from wall touch exponential system
    // Detailed wall touch breakdown by number of touches per wall
    float wall_touch_1;               // Number of walls touched exactly 1 time
    float wall_touch_2;               // Number of walls touched exactly 2 times
    float wall_touch_3;               // Number of walls touched exactly 3 times
    float wall_touch_4;               // Number of walls touched exactly 4 times
    float wall_touch_5;               // Number of walls touched exactly 5 times
    float wall_touch_6;               // Number of walls touched 6+ times
    // Goal system stats
    float goal_visits;                // Number of times goal was visited (if goal_enabled)
    // Efficiency system stats
    float efficiency_rewards;         // Total rewards from efficiency system (0-10 per round)
    float efficiency_percentage;      // Average efficiency percentage (touches / possible edges)
} Log;

typedef struct Client {
    float cell_size;
    int width;
    int height;
} Client;

typedef struct {
    // Core game state
    int tick;
    GameConfig config;
    
    // Game objects
    Tower towers[MAX_TOWERS];
    int num_towers;
    Runner runner;
    
    // Visual effects
    ThunderclapEffect thunderclap_effects[MAX_TOWERS];
    int num_thunderclap_effects;
    
    // Grid representation
    int grid[GRID_WIDTH][GRID_HEIGHT];  // 0=empty, 1=wall, 2=thunderclap
    
    // Performance optimization: Tower lookup grid
    int tower_grid[GRID_WIDTH][GRID_HEIGHT];  // Direct tower index lookup (-1 = none)
    
    // Path cache removed - using simple pathfinding instead
    
    // Episode tracking (like drone_race)
    int moves_left;
    float episode_return;
    
    // Pathfinding
    int entrance_x, entrance_y;
    int exit_x, exit_y;
    int goal_x, goal_y;            // Goal position (if goal_enabled)
    int goal_visited;              // 1 if runner has visited goal, 0 otherwise
    int previous_path_length;      // Track path length for immediate rewards
    
    // Performance tracking
    int rounds_completed;
    float total_path_length_rewards;  // Track total rewards from path length increases
    float total_path_length;          // Track cumulative path lengths for averaging
    // total_tower_utilization_rewards removed
    float total_thunderclap_rewards;  // Track total rewards from thunderclap activations and slowdowns
    float total_wall_touch_rewards;   // Track total rewards from wall touch exponential system
    float total_efficiency_rewards;   // Track total rewards from efficiency system
    float total_efficiency_percentage; // Track cumulative efficiency percentage for averaging
    
    // PufferLib interface
    float* observations;
    int* actions;  // Changed from float* to int* to match target
    float* rewards;
    int* terminals;
    
    // Memory management flag
    int buffers_allocated_by_c;
    
    // Rendering
    Client* client;
    
    // Logging
    Log log;
} mazing_contest;

// Default configuration
extern const GameConfig DEFAULT_CONFIG;

// Observation size: grid + resources (2) + phase info (3) + goal info (3)
#define OBS_SIZE (GRID_SIZE + 8)  // 100 + 8 = 108 for 10x10 grid

// Function declarations
float distance(float x1, float y1, float x2, float y2);
int get_tower_cost(TowerType type);
int get_tower_lumber_cost(TowerType type);
void grid_to_world(int grid_x, int grid_y, float* world_x, float* world_y, GameConfig* config);
void world_to_grid(float world_x, float world_y, int* grid_x, int* grid_y, GameConfig* config);
int is_valid_position(int x, int y);
int can_place_tower(mazing_contest* env, int x, int y, TowerType type);
int find_next_step(mazing_contest* env, int start_x, int start_y, int end_x, int end_y, int* next_x, int* next_y);
int find_path(mazing_contest* env, int start_x, int start_y, int end_x, int end_y);
int calculate_path_length(mazing_contest* env, int start_x, int start_y, int end_x, int end_y);
void place_tower(mazing_contest* env, int x, int y, TowerType type);
void place_tower_with_cost(mazing_contest* env, int x, int y, TowerType type, int deduct_resources);
void place_tower_full(mazing_contest* env, int x, int y, TowerType type, int deduct_resources, int is_random_obstacle);
void check_tower_proximity(mazing_contest* env);
void move_runner(mazing_contest* env);
void update_thunderclap_effects(mazing_contest* env);
void update_thunderclap_towers(mazing_contest* env);
void compute_observations(mazing_contest* env);
void add_log(mazing_contest* env);
void init_round(mazing_contest* env);
void place_random_obstacles(mazing_contest* env);
void place_random_thunderclaps(mazing_contest* env);  // Separate function for thunderclaps (disabled)
void place_goal(mazing_contest* env);                // Place goal with distance constraints
void allocate(mazing_contest* env);
void c_reset(mazing_contest* env);
void c_step(mazing_contest* env);
Client* make_client(mazing_contest* env);
void close_client(Client* client);
void c_render(mazing_contest* env);
void c_close(mazing_contest* env);
void free_allocated(mazing_contest* env);