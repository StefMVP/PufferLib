#include "mazing_contest.h"

// Default configuration
const GameConfig DEFAULT_CONFIG = {
    .gold = 50,  // Scale up for 10x10 grid
    .lumber = 1,
    .phase = PHASE_BUILD,
    .phase_timer = 0,
    .round_number = 1,
    .runner_time = 0.0f,
    .best_time = 0.0f,
    .maze_completed = 0,
    .build_time_limit = 800,  // 
    .max_moves = 3000000,    // More moves for larger maze
    .max_rounds = 1,     // Keep same number of rounds
    .min_gold = 5,        // Scale up for 10x10 grid  
    .max_gold = 25,       // Scale up for 10x10 grid
    .min_lumber = 2,        // Enable lumber for thunderclap towers
    .max_lumber = 4,        // Scale up for 10x10 grid
    .num_thunderclap_towers = 5,  // Allow more thunderclap towers
    
    // Thunderclap settings
    .thunderclap_slowdown_factor = 0.3f,  // 30% speed (70% slower)
    .thunderclap_slowdown_duration = 60,  // 1 seconds at 60 FPS
    .thunderclap_range = 50.0f,           // activation range in pixels (slightly larger)
    .thunderclap_cooldown = 60,           // 1.0 second cooldown (very fast for multiple hits)
    
    // Runner settings
    .runner_base_speed = 6.0f,             // base movement speed
    
    // Reward values - FOCUSED ON PATH LENGTH (IMMEDIATE REWARDS)
    .path_length_reward_multiplier = 1.4f, // MAIN REWARD: 1.4 points per step increase in path length
    // tower_utilization_reward removed - no longer used
    .wall_touch_base_reward = 0.1f,   // base reward for wall touches (exponential per same wall)
    .thunderclap_activation_reward = 0.0f, // no reward for activation - only for actual slowdown
    .thunderclap_slowdown_reward = 0.06f,  // reward per tick of slowdown (slight decrease)
    .thunderclap_unused_penalty = 3.0f,    // penalty for thunderclap towers never activated (increased)
    .runner_time_reward_multiplier = 0.0f, // DISABLED - using path length instead
    
    // Grid and world constants
    .grid_cell_size = 32.0f,               // size of each grid cell in pixels
    .grid_cell_center_offset = 16.0f,      // offset to center of grid cell
    
    // Observation normalization constants
    .gold_normalization = 50.0f,           // divisor for gold normalization (scaled for 10x10 grid)
    .lumber_normalization = 5.0f,          // divisor for lumber normalization
    .phase_normalization = 2.0f,           // divisor for phase normalization
    .build_time_normalization = 600.0f,    // divisor for build time normalization (scaled for 10x10 grid)
    .runner_time_normalization = 30.0f,    // divisor for runner time normalization (scaled for 10x10 grid)
    
    // Timing constants
    .fps = 60.0f,                          // frames per second (for time conversion)
    .stuck_teleport_ticks = 120,           // ticks before teleporting stuck runner (2 seconds)
    .tower_touch_cooldown_ticks = 60,      // ticks before tower can be touched again (1 second)
    
    // Rendering constants
    .cell_size_render = 50.0f,              // cell size for rendering
    
    // Random obstacle generation
    .min_random_walls = 5,                  // minimum number of random walls to place (scaled for 10x10)
    .max_random_walls = 15,                 // maximum number of random walls to place (scaled for 10x10)
    .min_random_thunderclaps = 0,           // minimum number of random thunderclaps to place (DISABLED)
    .max_random_thunderclaps = 0,           // maximum number of random thunderclaps to place (DISABLED)
    
    // Goal system
    .goal_enabled = 1                       // disabled by default
};

// Utility functions
float distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

int get_tower_cost(TowerType type) {
    switch (type) {
        case TOWER_WALL: return 1;  // 1 gold
        case TOWER_THUNDERCLAP: return 0;  // 0 gold (lumber only!)
        default: return 1;
    }
}

int get_tower_lumber_cost(TowerType type) {
    switch (type) {
        case TOWER_WALL: return 0;
        case TOWER_THUNDERCLAP: return 1;
        default: return 0;
    }
}

void grid_to_world(int grid_x, int grid_y, float* world_x, float* world_y, GameConfig* config) {
    *world_x = grid_x * config->grid_cell_size + config->grid_cell_center_offset;
    *world_y = grid_y * config->grid_cell_size + config->grid_cell_center_offset;
}

void world_to_grid(float world_x, float world_y, int* grid_x, int* grid_y, GameConfig* config) {
    *grid_x = (int)(world_x / config->grid_cell_size);
    *grid_y = (int)(world_y / config->grid_cell_size);
}

int is_valid_position(int x, int y) {
    return x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT;
}

int can_place_tower(MazingContest* env, int x, int y, TowerType type) {
    if (!is_valid_position(x, y)) return 0;
    if (env->grid[x][y] != 0) {
        // Already occupied (includes random obstacles)
        return 0;
    }
    
    // Can't block entrance or exit
    if ((x == env->entrance_x && y == env->entrance_y) ||
        (x == env->exit_x && y == env->exit_y)) {
        return 0;
    }
    
    // Can't block goal (if goal system is enabled)
    if (env->config.goal_enabled && 
        (x == env->goal_x && y == env->goal_y)) {
        return 0;
    }
    
    // Check resources
    if (env->config.gold < get_tower_cost(type)) return 0;
    if (env->config.lumber < get_tower_lumber_cost(type)) return 0;
    
    return 1;
}

// A* pathfinding implementation
typedef struct {
    int x, y;
    int g_cost;  // Distance from start
    int h_cost;  // Distance to goal (heuristic)
    int f_cost;  // g_cost + h_cost
    int parent_x, parent_y;
    int in_open_set;
    int in_closed_set;
} AStarNode;

// Manhattan distance heuristic
int heuristic(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

// A* pathfinding - returns 1 if path found, 0 if no path
int find_next_step(MazingContest* env, int start_x, int start_y, int end_x, int end_y, int* next_x, int* next_y) {
    if (start_x == end_x && start_y == end_y) {
        *next_x = start_x;
        *next_y = start_y;
        return 1;
    }
    
    static AStarNode nodes[GRID_WIDTH][GRID_HEIGHT];
    static int open_set[GRID_WIDTH * GRID_HEIGHT][2];
    int open_count = 0;
    
    // Initialize all nodes
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            nodes[x][y].x = x;
            nodes[x][y].y = y;
            nodes[x][y].g_cost = 999999;
            nodes[x][y].h_cost = heuristic(x, y, end_x, end_y);
            nodes[x][y].f_cost = 999999;
            nodes[x][y].parent_x = -1;
            nodes[x][y].parent_y = -1;
            nodes[x][y].in_open_set = 0;
            nodes[x][y].in_closed_set = 0;
        }
    }
    
    // Initialize start node
    nodes[start_x][start_y].g_cost = 0;
    nodes[start_x][start_y].f_cost = nodes[start_x][start_y].h_cost;
    
    // Add start node to open set
    open_set[open_count][0] = start_x;
    open_set[open_count][1] = start_y;
    nodes[start_x][start_y].in_open_set = 1;
    open_count++;
    
    int dx[] = {-1, 1, 0, 0};
    int dy[] = {0, 0, -1, 1};
    
    while (open_count > 0) {
        // Find node with lowest f_cost in open set
        int current_idx = 0;
        for (int i = 1; i < open_count; i++) {
            int curr_x = open_set[i][0];
            int curr_y = open_set[i][1];
            int best_x = open_set[current_idx][0];
            int best_y = open_set[current_idx][1];
            
            if (nodes[curr_x][curr_y].f_cost < nodes[best_x][best_y].f_cost) {
                current_idx = i;
            }
        }
        
        int current_x = open_set[current_idx][0];
        int current_y = open_set[current_idx][1];
        
        // Remove current node from open set
        for (int i = current_idx; i < open_count - 1; i++) {
            open_set[i][0] = open_set[i + 1][0];
            open_set[i][1] = open_set[i + 1][1];
        }
        open_count--;
        nodes[current_x][current_y].in_open_set = 0;
        nodes[current_x][current_y].in_closed_set = 1;
        
        // Check if we reached the goal
        if (current_x == end_x && current_y == end_y) {
            // Reconstruct path to find first step
            int path_x = end_x, path_y = end_y;
            int prev_x = path_x, prev_y = path_y;
            
            // Trace back the complete path
            while (nodes[path_x][path_y].parent_x != -1 && nodes[path_x][path_y].parent_y != -1) {
                prev_x = path_x;
                prev_y = path_y;
                int temp_x = nodes[path_x][path_y].parent_x;
                int temp_y = nodes[path_x][path_y].parent_y;
                path_x = temp_x;
                path_y = temp_y;
                
                // If we've reached the start, prev_x/prev_y is our first step
                if (path_x == start_x && path_y == start_y) {
                    *next_x = prev_x;
                    *next_y = prev_y;
                    return 1;
                }
            }
            
            // Fallback - should not happen
            *next_x = start_x;
            *next_y = start_y;
            return 1;
        }
        
        // Check all neighbors
        for (int i = 0; i < 4; i++) {
            int neighbor_x = current_x + dx[i];
            int neighbor_y = current_y + dy[i];
            
            // Check bounds
            if (neighbor_x < 0 || neighbor_x >= GRID_WIDTH || 
                neighbor_y < 0 || neighbor_y >= GRID_HEIGHT) {
                continue;
            }
            
            // Check if neighbor is walkable
            if (env->grid[neighbor_x][neighbor_y] != 0 && 
                !(neighbor_x == end_x && neighbor_y == end_y)) {
                continue;
            }
            
            // Skip if in closed set
            if (nodes[neighbor_x][neighbor_y].in_closed_set) {
                continue;
            }
            
            int tentative_g_cost = nodes[current_x][current_y].g_cost + 1;
            
            // If this path to neighbor is better than any previous one
            if (tentative_g_cost < nodes[neighbor_x][neighbor_y].g_cost) {
                nodes[neighbor_x][neighbor_y].parent_x = current_x;
                nodes[neighbor_x][neighbor_y].parent_y = current_y;
                nodes[neighbor_x][neighbor_y].g_cost = tentative_g_cost;
                nodes[neighbor_x][neighbor_y].f_cost = tentative_g_cost + nodes[neighbor_x][neighbor_y].h_cost;
                
                // Add to open set if not already there
                if (!nodes[neighbor_x][neighbor_y].in_open_set) {
                    open_set[open_count][0] = neighbor_x;
                    open_set[open_count][1] = neighbor_y;
                    nodes[neighbor_x][neighbor_y].in_open_set = 1;
                    open_count++;
                }
            }
        }
    }
    
    // No path found
    return 0;
}

// Simple path existence check
int find_path(MazingContest* env, int start_x, int start_y, int end_x, int end_y) {
    int dummy_x, dummy_y;
    return find_next_step(env, start_x, start_y, end_x, end_y, &dummy_x, &dummy_y);
}

// Calculate path length using BFS - returns number of steps in optimal path
int calculate_path_length(MazingContest* env, int start_x, int start_y, int end_x, int end_y) {
    if (start_x == end_x && start_y == end_y) {
        return 0;
    }
    
    // Use same BFS logic as find_next_step but return path length
    static int visited[GRID_WIDTH][GRID_HEIGHT];
    static int parent_x[GRID_WIDTH][GRID_HEIGHT];
    static int parent_y[GRID_WIDTH][GRID_HEIGHT];
    static int queue_x[GRID_WIDTH * GRID_HEIGHT];
    static int queue_y[GRID_WIDTH * GRID_HEIGHT];
    
    memset(visited, 0, sizeof(visited));
    
    int queue_front = 0, queue_back = 0;
    
    queue_x[queue_back] = start_x;
    queue_y[queue_back] = start_y;
    queue_back++;
    visited[start_x][start_y] = 1;
    parent_x[start_x][start_y] = -1;
    parent_y[start_x][start_y] = -1;
    
    const int dx[4] = {0, 0, 1, -1};
    const int dy[4] = {1, -1, 0, 0};
    
    while (queue_front < queue_back) {
        int x = queue_x[queue_front];
        int y = queue_y[queue_front];
        queue_front++;
        
        if (x == end_x && y == end_y) {
            // Count path length by backtracking
            int path_length = 0;
            int curr_x = x, curr_y = y;
            
            while (curr_x != start_x || curr_y != start_y) {
                path_length++;
                if (parent_x[curr_x][curr_y] == -1) break;
                int temp_x = parent_x[curr_x][curr_y];
                int temp_y = parent_y[curr_x][curr_y];
                curr_x = temp_x;
                curr_y = temp_y;
            }
            
            return path_length;
        }
        
        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            
            if (nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT && 
                !visited[nx][ny] && 
                (env->grid[nx][ny] == 0 || (nx == end_x && ny == end_y))) {
                visited[nx][ny] = 1;
                parent_x[nx][ny] = x;
                parent_y[nx][ny] = y;
                queue_x[queue_back] = nx;
                queue_y[queue_back] = ny;
                queue_back++;
            }
        }
    }
    
    return -1; // No path found
}

void place_tower(MazingContest* env, int x, int y, TowerType type) {
    place_tower_full(env, x, y, type, 1, 0);  // Default: deduct resources, not random
}

void place_tower_with_cost(MazingContest* env, int x, int y, TowerType type, int deduct_resources) {
    place_tower_full(env, x, y, type, deduct_resources, 0);  // Not random obstacle
}

void place_tower_full(MazingContest* env, int x, int y, TowerType type, int deduct_resources, int is_random_obstacle) {
    // ALWAYS check basic validity first
    if (!is_valid_position(x, y)) return;
    if (env->grid[x][y] != 0) return;  // Already occupied by ANYTHING
    
    // Can't block entrance or exit
    if ((x == env->entrance_x && y == env->entrance_y) ||
        (x == env->exit_x && y == env->exit_y)) {
        return;
    }
    
    // For player towers, check resources
    if (deduct_resources) {
        if (env->config.gold < get_tower_cost(type)) return;
        if (env->config.lumber < get_tower_lumber_cost(type)) return;
    }
    
    // CRITICAL FIX: Always check path validation before placing ANY tower
    // This ensures no tower (random or user) can block the path to exit
    // Temporarily place tower to test path (includes both random and user towers)
    env->grid[x][y] = (type == TOWER_WALL) ? 1 : 2;
    
    // Path cache removed
    
    // Check if path still exists with COMPLETE grid state (random + user towers)
    int path_valid = 0;
    if (env->config.goal_enabled) {
        // With goal: check both entrance -> goal and goal -> exit paths
        path_valid = find_path(env, env->entrance_x, env->entrance_y, env->goal_x, env->goal_y) &&
                     find_path(env, env->goal_x, env->goal_y, env->exit_x, env->exit_y);
    } else {
        // Without goal: check direct entrance -> exit path
        path_valid = find_path(env, env->entrance_x, env->entrance_y, env->exit_x, env->exit_y);
    }
    
    if (!path_valid) {
        // Path blocked by combined towers - remove tower and return
        env->grid[x][y] = 0;
        return;
    }
    
    // Path exists - place tower permanently
    Tower* tower = &env->towers[env->num_towers];
    tower->active = 1;
    tower->type = type;
    tower->grid_x = x;
    tower->grid_y = y;
    grid_to_world(x, y, &tower->x, &tower->y, &env->config);
    tower->cooldown = 0;
    tower->caused_new_slow = 0;  // Initialize to 0 - will be set to 1 if it causes a NEW slow
    tower->last_activated_tick = -1;
    tower->touched_by_runner = 0;
    tower->touch_count = 0;
    tower->is_random_obstacle = is_random_obstacle;
    
    // Initialize edge touch tracking
    for (int i = 0; i < 9; i++) {
        tower->currently_touching_edges[i] = 0;
        tower->previously_touching_edges[i] = 0;
        tower->edge_touch_cooldown[i] = 0;
    }
    
    // Update tower lookup grid for O(1) tower finding
    env->tower_grid[x][y] = env->num_towers;
    
    env->num_towers++;
    
    // IMMEDIATE PATH LENGTH REWARD - only for player-placed towers during build phase
    if (!is_random_obstacle && env->config.phase == PHASE_BUILD) {
        int current_path_length;
        if (env->config.goal_enabled) {
            // With goal: path length is entrance -> goal -> exit
            int entrance_to_goal = calculate_path_length(env, env->entrance_x, env->entrance_y, env->goal_x, env->goal_y);
            int goal_to_exit = calculate_path_length(env, env->goal_x, env->goal_y, env->exit_x, env->exit_y);
            current_path_length = entrance_to_goal + goal_to_exit;
        } else {
            // Without goal: direct path entrance -> exit
            current_path_length = calculate_path_length(env, env->entrance_x, env->entrance_y, env->exit_x, env->exit_y);
        }
        
        if (current_path_length > env->previous_path_length) {
            int path_increase = current_path_length - env->previous_path_length;
            float reward = path_increase * env->config.path_length_reward_multiplier;
            env->rewards[0] += reward;
            env->total_path_length_rewards += reward;  // Track total path length rewards
        }
        env->previous_path_length = current_path_length;
    }
    
    // Path cache removed
    
    // Deduct resources only if requested
    if (deduct_resources) {
        env->config.gold -= get_tower_cost(type);
        env->config.lumber -= get_tower_lumber_cost(type);
    }
}

void check_tower_proximity(MazingContest* env) {
    // Get runner's current grid position
    int runner_x, runner_y;
    world_to_grid(env->runner.x, env->runner.y, &runner_x, &runner_y, &env->config);
    
    // STEP 1: Copy current touching state to previous, then clear current
    for (int t = 0; t < env->num_towers; t++) {
        for (int e = 0; e < 9; e++) {
            env->towers[t].previously_touching_edges[e] = env->towers[t].currently_touching_edges[e];
            env->towers[t].currently_touching_edges[e] = 0;
        }
    }
    
    // STEP 2: Mark all edges that runner is currently touching
    // Only count the 4 cardinal directions (N, S, E, W) as edges of the square
    const int offsets[4][2] = {{-1,0}, {1,0}, {0,-1}, {0,1}};  // W, E, N, S
    
    for (int i = 0; i < 4; i++) {
        int check_x = runner_x + offsets[i][0];
        int check_y = runner_y + offsets[i][1];
        
        if (check_x < 0 || check_x >= GRID_WIDTH || check_y < 0 || check_y >= GRID_HEIGHT) continue;
        
        int tower_idx = env->tower_grid[check_x][check_y];
        if (tower_idx == -1) continue;
        
        Tower* tower = &env->towers[tower_idx];
        if (!tower->active) continue;
        
        // Calculate which edge this represents (runner position relative to tower)
        int edge_index = i;
        
        // Check if we were already touching this SPECIFIC edge last frame
        int was_touching_this_edge = tower->previously_touching_edges[edge_index];
        
        // Mark as currently touching
        tower->currently_touching_edges[edge_index] = 1;
        
        // If we weren't touching this specific edge before, increment touch count
        if (!was_touching_this_edge) {
            // First touch on this tower ever?
            if (!tower->touched_by_runner) {
                tower->touched_by_runner = 1;
                tower->touch_count = 1;
                
                // NEW: No reward for first touch (already handled by tower utilization)
                if (tower->type == TOWER_WALL && !tower->is_random_obstacle) {
                    // First touch: no reward (tower utilization handles this)
                }
            } else {
                // Additional touch on this tower (new edge contact)
                tower->touch_count++;
            }
            
            // NEW: Exponential wall touch rewards - for 2nd+ touches on SAME wall (ALL walls + thunderclap!)
            if ((tower->type == TOWER_WALL || tower->type == TOWER_THUNDERCLAP) && tower->touch_count >= 2) {
                // EXTREMELY AGGRESSIVE exponential reward: base_reward * (4.0^(touch_count-2)) - starts at 2nd touch
                // touch_count=2: base_reward * 1 = 200.0 reward
                // touch_count=3: base_reward * 4 = 800.0 reward  
                // touch_count=4: base_reward * 16 = 3200.0 reward (great looping!)
                // touch_count=5: base_reward * 64 = 12800.0 reward (excellent looping!)
                // touch_count=6: base_reward * 256 = 51200.0 reward (amazing looping!)
                // touch_count=7: base_reward * 1024 = 204800.0 reward (incredible!)
                // NO CAP - unlimited exponential growth for maximum looping incentive!
                float exponential_multiplier = powf(4.0f, (float)(tower->touch_count - 2));
                float base_reward = env->config.wall_touch_base_reward;
                
                // THUNDERCLAP MULTIPLIER: 2x rewards for thunderclap towers (they're more expensive!)
                if (tower->type == TOWER_THUNDERCLAP) {
                    base_reward *= 2.0f;
                }
                
                float reward = base_reward * exponential_multiplier;
                env->rewards[0] += reward;
                env->total_wall_touch_rewards += reward;
            }
        }
    }
}

void move_runner(MazingContest* env) {
    Runner* runner = &env->runner;
    
    // Get current grid position
    int current_x, current_y;
    world_to_grid(runner->x, runner->y, &current_x, &current_y, &env->config);
    
    
    // Check if reached goal (if goal system is enabled)
    if (env->config.goal_enabled && !env->goal_visited &&
        current_x == env->goal_x && current_y == env->goal_y) {
        env->goal_visited = 1;
        env->log.goal_visits += 1.0f;  // Track goal visits
        // Path cache removed
        return;
    }
    
    // Check if reached exit (only if goal is visited or goal system is disabled)
    if ((!env->config.goal_enabled || env->goal_visited) &&
        current_x == env->exit_x && current_y == env->exit_y) {
        env->config.maze_completed = 1;
        return;
    }
    
    // Use BFS to find next step in optimal path
    int next_x, next_y;
    int target_x, target_y;
    
    // Determine target based on goal system and goal status
    if (env->config.goal_enabled && !env->goal_visited) {
        // Goal system enabled and goal not visited - go to goal first
        target_x = env->goal_x;
        target_y = env->goal_y;
        
        
        // DEBUG: Ensure we're definitely targeting the goal
        if (target_x == env->exit_x && target_y == env->exit_y) {
            // This should never happen! Goal and exit are at same position
            // Force different goal placement
            target_x = env->goal_x;
            target_y = env->goal_y;
        }
    } else {
        // Goal system disabled or goal already visited - go to exit
        target_x = env->exit_x;
        target_y = env->exit_y;
        
    }
    
    if (find_next_step(env, current_x, current_y, target_x, target_y, &next_x, &next_y)) {
        
        // Calculate target world position
        float target_world_x, target_world_y;
        grid_to_world(next_x, next_y, &target_world_x, &target_world_y, &env->config);
        
        // Calculate speed (with thunderclap slowdown)
        float speed = runner->speed;
        if (env->tick < runner->slowed_until_tick) {
            speed *= env->config.thunderclap_slowdown_factor;  // Configurable slowdown
            
            // Track slowdown time and reward for it
            env->rewards[0] += env->config.thunderclap_slowdown_reward;  // Reward for each tick of slowdown
            env->total_thunderclap_rewards += env->config.thunderclap_slowdown_reward;
            
            // Track total slowdown time (convert ticks to seconds)
            env->log.thunderclap_slowdown_time += 1.0f / env->config.fps;  // Add 1 tick worth of time
        }
        
        // Move towards target
        float dx = target_world_x - runner->x;
        float dy = target_world_y - runner->y;
        float dist = sqrtf(dx * dx + dy * dy);
        
        if (dist > speed) {
            // Move towards target
            runner->x += (dx / dist) * speed;
            runner->y += (dy / dist) * speed;
        } else {
            // Snap to grid center when close enough to reach in one frame
            runner->x = target_world_x;
            runner->y = target_world_y;
        }
        
        runner->stuck_counter = 0;
    } else {
        // No path found - increment stuck counter
        runner->stuck_counter++;
        if (runner->stuck_counter > env->config.stuck_teleport_ticks) {  // 2 seconds stuck - teleport to exit
            grid_to_world(env->exit_x, env->exit_y, &runner->x, &runner->y, &env->config);
            env->config.maze_completed = 1;
        }
    }
}

void update_thunderclap_effects(MazingContest* env) {
    // Update existing thunderclap effects
    for (int i = 0; i < env->num_thunderclap_effects; i++) {
        ThunderclapEffect* effect = &env->thunderclap_effects[i];
        if (!effect->active) continue;
        
        effect->duration--;
        if (effect->duration <= 0) {
            effect->active = 0;
            // Remove by swapping with last effect
            *effect = env->thunderclap_effects[env->num_thunderclap_effects - 1];
            env->num_thunderclap_effects--;
            i--; // Check this index again
        }
    }
}

void update_thunderclap_towers(MazingContest* env) {
    // ULTRA-OPTIMIZED: Pre-compute runner position and range check
    float runner_x = env->runner.x;
    float runner_y = env->runner.y;
    float range_sq = env->config.thunderclap_range * env->config.thunderclap_range; // Avoid sqrt
    
    // Track if runner was already slowed at the start of this tick
    int was_slowed_at_start = (env->tick < env->runner.slowed_until_tick);
    int new_slow_credit_given = 0;  // Only give credit to first thunderclap that activates
    
    // OPTIMIZED: Process towers in reverse order for cache efficiency
    for (int i = env->num_towers - 1; i >= 0; i--) {
        Tower* tower = &env->towers[i];
        if (!tower->active || tower->type != TOWER_THUNDERCLAP) continue;
        
        // Fast cooldown decrement
        if (tower->cooldown > 0) {
            tower->cooldown--;
            continue;
        }
        
        // OPTIMIZED: Distance check using squared distance (avoid sqrt)
        float dx = tower->x - runner_x;
        float dy = tower->y - runner_y;
        float dist_sq = dx * dx + dy * dy;
        
        if (dist_sq <= range_sq) {
            // ACTIVATE THUNDERCLAP SKILL!
            
            // Set the slowdown timer
            env->runner.slowed_until_tick = env->tick + env->config.thunderclap_slowdown_duration;
            tower->cooldown = env->config.thunderclap_cooldown;
            tower->last_activated_tick = env->tick;
            
            // Track if this thunderclap caused a NEW slow (not when already slowed)
            // Only give credit if runner wasn't slowed at start AND no other thunderclap got credit yet
            if (!was_slowed_at_start && !new_slow_credit_given) {
                tower->caused_new_slow = 1;  // This thunderclap caused a new slow
                new_slow_credit_given = 1;   // Prevent other thunderclaps from getting credit
            } else if (was_slowed_at_start) {
                // Penalty for hitting already slowed runner - wasted activation
                float penalty = 5.0f;  // -5 points for hitting already slowed runner
                env->rewards[0] -= penalty;
                env->total_thunderclap_rewards -= penalty;  // Track as negative thunderclap reward
            }
            
            // Create visual effect with bounds check
            if (env->num_thunderclap_effects < MAX_TOWERS) {
                ThunderclapEffect* effect = &env->thunderclap_effects[env->num_thunderclap_effects];
                effect->active = 1;
                effect->x = tower->x;
                effect->y = tower->y;
                effect->duration = 30;
                effect->max_duration = 30;
                env->num_thunderclap_effects++;
            }
            
            // No activation reward - only reward for actual slowdown effect
            // env->rewards[0] += env->config.thunderclap_activation_reward;
            // env->total_thunderclap_rewards += env->config.thunderclap_activation_reward;
        }
    }
}

void compute_observations(MazingContest* env) {
    float* obs = env->observations;
    int idx = 0;
    
    // Grid state (10x10 = 100 values)
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            obs[idx++] = env->grid[x][y] / 2.0f;  // Normalize to [0, 1]
        }
    }
    
    // Resources (2 values)
    obs[idx++] = (float)env->config.gold / env->config.gold_normalization;  // Normalize
    obs[idx++] = (float)env->config.lumber / env->config.lumber_normalization;  // Normalize
    
    // Phase info (3 values)
    obs[idx++] = env->config.phase / env->config.phase_normalization;  // Normalize
    obs[idx++] = env->config.phase_timer / env->config.build_time_normalization;  // Normalize to build time
    obs[idx++] = env->config.runner_time / env->config.runner_time_normalization;  // Normalize to reasonable time
    
    // Goal system info (3 values)
    obs[idx++] = env->config.goal_enabled ? 1.0f : 0.0f;  // Goal system enabled flag
    obs[idx++] = env->config.goal_enabled ? (float)env->goal_x / (float)GRID_WIDTH : 0.0f;  // Goal X position (normalized)
    obs[idx++] = env->config.goal_enabled ? (float)env->goal_y / (float)GRID_HEIGHT : 0.0f; // Goal Y position (normalized)
}

void add_log(MazingContest* env) {
    env->log.episode_return += env->episode_return;
    env->log.episode_length += env->tick;  // Like drone_race
    env->log.n += 1;
    
    // Update stats (only count player-placed towers)
    int player_towers = 0;
    for (int i = 0; i < env->num_towers; i++) {
        if (!env->towers[i].is_random_obstacle) {
            player_towers++;
        }
    }
    env->log.towers_built += player_towers;
    env->log.total_gold_spent += (env->config.max_gold - env->config.gold);
    env->log.best_time = env->config.best_time;  // Keep current best
    env->log.rounds_completed += env->rounds_completed;
    
    // PATH LENGTH STATS - MAIN METRICS (entrance → goal + goal → exit)
    int entrance_to_goal_length = calculate_path_length(env, env->entrance_x, env->entrance_y, env->goal_x, env->goal_y);
    int goal_to_exit_length = calculate_path_length(env, env->goal_x, env->goal_y, env->exit_x, env->exit_y);
    env->log.final_path_length = entrance_to_goal_length + goal_to_exit_length;
    env->log.path_length_rewards += env->total_path_length_rewards;
    
    // Track cumulative path length for averaging
    env->total_path_length += env->log.final_path_length;
    if (env->rounds_completed > 0) {
        env->log.average_path_length = env->total_path_length / env->rounds_completed;
    }
    
    // Count tower types (only player-placed towers)
    for (int i = 0; i < env->num_towers; i++) {
        if (!env->towers[i].is_random_obstacle) {  // Only count player-placed towers
            if (env->towers[i].type == TOWER_WALL) {
                env->log.walls_built += 1;
            }
        }
    }
    
    // Wall touch stats (new exponential system)
    env->log.wall_touch_rewards += env->total_wall_touch_rewards;
    
    // Efficiency stats (percentage-based system)
    env->log.efficiency_rewards += env->total_efficiency_rewards;
    if (env->rounds_completed > 0) {
        env->log.efficiency_percentage = (env->total_efficiency_percentage / env->rounds_completed) * 100.0f;  // Convert to percentage (0-100+)
    }
    
    // Detailed wall touch breakdown - count walls by their touch count AND total touches (ALL walls + thunderclap!)
    for (int i = 0; i < env->num_towers; i++) {
        Tower* tower = &env->towers[i];
        if (tower->active && (tower->type == TOWER_WALL || tower->type == TOWER_THUNDERCLAP)) {
            // Count total touches across all walls
            env->log.wall_touches += (float)tower->touch_count;
            
            // Count walls by their touch count
            if (tower->touch_count == 1) {
                env->log.wall_touch_1 += 1.0f;
            } else if (tower->touch_count == 2) {
                env->log.wall_touch_2 += 1.0f;
            } else if (tower->touch_count == 3) {
                env->log.wall_touch_3 += 1.0f;
            } else if (tower->touch_count == 4) {
                env->log.wall_touch_4 += 1.0f;
            } else if (tower->touch_count == 5) {
                env->log.wall_touch_5 += 1.0f;
            } else if (tower->touch_count >= 6) {
                env->log.wall_touch_6 += 1.0f;
            }
        }
    }
    
    // Thunderclap stats
    int thunderclap_towers = 0;
    for (int i = 0; i < env->num_towers; i++) {
        if (!env->towers[i].is_random_obstacle && env->towers[i].type == TOWER_THUNDERCLAP) {
            thunderclap_towers++;
        }
    }
    env->log.thunderclap_towers_built += thunderclap_towers;
    env->log.thunderclap_rewards += env->total_thunderclap_rewards;
    
    // Maze completion rate removed - all mazes complete, it's about delay time
}

void init_round(MazingContest* env) {
    // Reset grid
    memset(env->grid, 0, sizeof(env->grid));
    
    // Reset towers
    env->num_towers = 0;
    memset(env->towers, 0, sizeof(env->towers));
    
    // Reset tower lookup grid
    memset(env->tower_grid, -1, sizeof(env->tower_grid));
    
    // Path cache removed
    
    // Reset effects
    env->num_thunderclap_effects = 0;
    memset(env->thunderclap_effects, 0, sizeof(env->thunderclap_effects));
    
    // Set entrance and exit
    env->entrance_x = 0;
    env->entrance_y = GRID_HEIGHT / 2;  // Should be 3 for 6x6 grid
    env->exit_x = GRID_WIDTH - 1;      // Should be 5 for 6x6 grid
    env->exit_y = GRID_HEIGHT / 2;     // Should be 3 for 6x6 grid
    
    // Randomize resources for each round
    int gold_range = env->config.max_gold - env->config.min_gold + 1;
    int lumber_range = env->config.max_lumber - env->config.min_lumber + 1;
    
    // Reset resources each round
    env->config.gold = env->config.min_gold + (rand() % gold_range);
    env->config.lumber = env->config.min_lumber + (rand() % lumber_range);
    
    // Reset phase
    env->config.phase = PHASE_BUILD;
    env->config.phase_timer = 0;
    env->config.runner_time = 0.0f;
    env->config.maze_completed = 0;
    
    //env->moves_left = env->config.max_moves;
    
    // Reset runner
    grid_to_world(env->entrance_x, env->entrance_y, &env->runner.x, &env->runner.y, &env->config);
    env->runner.speed = env->config.runner_base_speed;
    env->runner.slowed_until_tick = 0;
    env->runner.stuck_counter = 0;
    
    // Place random obstacles to vary the challenge
    place_random_obstacles(env);
    
    // Place goal if enabled
    if (env->config.goal_enabled) {
        place_goal(env);
        env->goal_visited = 0;  // Reset goal visited status
        // Path cache removed
    }
    
    // Initialize runner target based on goal system (AFTER goal is placed)
    if (env->config.goal_enabled) {
        env->runner.target_x = env->goal_x;
        env->runner.target_y = env->goal_y;
    } else {
        env->runner.target_x = env->exit_x;
        env->runner.target_y = env->exit_y;
    }
    
    // Initialize path length for immediate rewards AFTER placing random obstacles and goal
    if (env->config.goal_enabled) {
        // With goal: path length is entrance -> goal -> exit
        int entrance_to_goal = calculate_path_length(env, env->entrance_x, env->entrance_y, env->goal_x, env->goal_y);
        int goal_to_exit = calculate_path_length(env, env->goal_x, env->goal_y, env->exit_x, env->exit_y);
        env->previous_path_length = entrance_to_goal + goal_to_exit;
    } else {
        // Without goal: direct path entrance -> exit
        env->previous_path_length = calculate_path_length(env, env->entrance_x, env->entrance_y, env->exit_x, env->exit_y);
    }
}

void allocate(MazingContest* env) {
    // Don't reset config here - it should be set by binding
    env->tick = 0;
    env->episode_return = 0.0f;
    env->total_path_length = 0.0f;
    // total_tower_utilization_rewards removed
    env->total_thunderclap_rewards = 0.0f;
    env->total_wall_touch_rewards = 0.0f;
    env->total_efficiency_rewards = 0.0f;
    env->total_efficiency_percentage = 0.0f;
    env->rounds_completed = 0;
    env->client = NULL;
    
    // Initialize performance optimizations
    memset(env->tower_grid, -1, sizeof(env->tower_grid));
    // Path cache removed
    
    // Initialize random seed
    srand(time(NULL));
    
    // Only allocate buffers if they haven't been set by Python
    if (env->observations == NULL) {
        env->observations = (float*)calloc(OBS_SIZE, sizeof(float));
    }
    if (env->actions == NULL) {
        env->actions = (int*)calloc(1, sizeof(int));
    }
    if (env->rewards == NULL) {
        env->rewards = (float*)calloc(1, sizeof(float));
    }
    if (env->terminals == NULL) {
        env->terminals = (int*)calloc(1, sizeof(int));
    }
    
    // Clear log (including thunderclap_slowdown_time)
    memset(&env->log, 0, sizeof(Log));
    
    // Initialize moves_left from config
    env->moves_left = env->config.max_moves;
    
    // Resources will be set in init_round() for each round
    
    init_round(env);
}

void place_random_obstacles(MazingContest* env) {
    // ULTRA-OPTIMIZED: Pre-generate valid positions to avoid repeated validation
    static int valid_positions[GRID_WIDTH * GRID_HEIGHT][2];
    int valid_count = 0;
    
    // O(n) pre-computation of valid positions (only empty cells)
    for (int x = 1; x < GRID_WIDTH - 1; x++) {
        for (int y = 1; y < GRID_HEIGHT - 1; y++) {
            if (env->grid[x][y] == 0 && 
                !(x == env->entrance_x && y == env->entrance_y) &&
                !(x == env->exit_x && y == env->exit_y)) {
                valid_positions[valid_count][0] = x;
                valid_positions[valid_count][1] = y;
                valid_count++;
            }
        }
    }
    
    if (valid_count == 0) return; // No valid positions
    
    // CRITICAL: Place random obstacles with path validation
    int num_walls = env->config.min_random_walls;
    if (env->config.max_random_walls > env->config.min_random_walls) {
        num_walls += rand() % (env->config.max_random_walls - env->config.min_random_walls + 1);
    }
    
    // Use Fisher-Yates shuffle for O(n) random selection without replacement
    for (int i = 0; i < num_walls && i < valid_count; i++) {
        int rand_idx = i + rand() % (valid_count - i);
        // Swap positions
        int temp_x = valid_positions[i][0];
        int temp_y = valid_positions[i][1];
        valid_positions[i][0] = valid_positions[rand_idx][0];
        valid_positions[i][1] = valid_positions[rand_idx][1];
        valid_positions[rand_idx][0] = temp_x;
        valid_positions[rand_idx][1] = temp_y;
        
        // CRITICAL: Place wall with path validation (ensures path remains open)
        place_tower_full(env, valid_positions[i][0], valid_positions[i][1], TOWER_WALL, 0, 1);
    }
    
    // THUNDERCLAP PLACEMENT DISABLED - moved to separate function
    // place_random_thunderclaps(env);
}

void place_random_thunderclaps(MazingContest* env) {
    // DISABLED: This function can be re-enabled later by calling it from place_random_obstacles
    // and updating the config values for min_random_thunderclaps and max_random_thunderclaps
    
    // ULTRA-OPTIMIZED: Pre-generate valid positions to avoid repeated validation
    static int valid_positions[GRID_WIDTH * GRID_HEIGHT][2];
    int valid_count = 0;
    
    // O(n) pre-computation of valid positions (only empty cells)
    for (int x = 1; x < GRID_WIDTH - 1; x++) {
        for (int y = 1; y < GRID_HEIGHT - 1; y++) {
            if (env->grid[x][y] == 0 && 
                !(x == env->entrance_x && y == env->entrance_y) &&
                !(x == env->exit_x && y == env->exit_y)) {
                valid_positions[valid_count][0] = x;
                valid_positions[valid_count][1] = y;
                valid_count++;
            }
        }
    }
    
    if (valid_count == 0) return; // No valid positions
    
    // OPTIMIZED: Place thunderclaps from remaining positions
    int num_thunderclaps = env->config.min_random_thunderclaps;
    if (env->config.max_random_thunderclaps > env->config.min_random_thunderclaps) {
        num_thunderclaps += rand() % (env->config.max_random_thunderclaps - env->config.min_random_thunderclaps + 1);
    }
    
    // Use Fisher-Yates shuffle for O(n) random selection without replacement
    for (int i = 0; i < num_thunderclaps && i < valid_count; i++) {
        int rand_idx = i + rand() % (valid_count - i);
        // Swap positions
        int temp_x = valid_positions[i][0];
        int temp_y = valid_positions[i][1];
        valid_positions[i][0] = valid_positions[rand_idx][0];
        valid_positions[i][1] = valid_positions[rand_idx][1];
        valid_positions[rand_idx][0] = temp_x;
        valid_positions[rand_idx][1] = temp_y;
        
        // CRITICAL: Place thunderclap with path validation
        place_tower_full(env, valid_positions[i][0], valid_positions[i][1], TOWER_THUNDERCLAP, 0, 1);
    }
}

void place_goal(MazingContest* env) {
    // Find valid positions for goal: at least 2 squares away from entrance and exit
    static int valid_positions[GRID_WIDTH * GRID_HEIGHT][2];
    int valid_count = 0;
    
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            // Must be empty
            if (env->grid[x][y] != 0) continue;
            
            // Must not be entrance or exit
            if ((x == env->entrance_x && y == env->entrance_y) ||
                (x == env->exit_x && y == env->exit_y)) continue;
            
            // Must be at least 2 squares away from entrance and exit
            int dist_to_entrance = abs(x - env->entrance_x) + abs(y - env->entrance_y);
            int dist_to_exit = abs(x - env->exit_x) + abs(y - env->exit_y);
            
            if (dist_to_entrance >= 2 && dist_to_exit >= 2) {
                valid_positions[valid_count][0] = x;
                valid_positions[valid_count][1] = y;
                valid_count++;
            }
        }
    }
    
    if (valid_count == 0) {
        // Fallback: place goal at center of grid if no valid positions
        env->goal_x = GRID_WIDTH / 2;
        env->goal_y = GRID_HEIGHT / 2;
        return;
    }
    
    // Try to place goal with path validation
    for (int attempts = 0; attempts < valid_count; attempts++) {
        int rand_idx = rand() % valid_count;
        int test_x = valid_positions[rand_idx][0];
        int test_y = valid_positions[rand_idx][1];
        
        // Check if paths exist: entrance -> goal -> exit
        if (find_path(env, env->entrance_x, env->entrance_y, test_x, test_y) &&
            find_path(env, test_x, test_y, env->exit_x, env->exit_y)) {
            env->goal_x = test_x;
            env->goal_y = test_y;
            return;
        }
        
        // Remove this position from consideration
        valid_positions[rand_idx][0] = valid_positions[valid_count - 1][0];
        valid_positions[rand_idx][1] = valid_positions[valid_count - 1][1];
        valid_count--;
    }
    
    // Fallback: place goal at center if no valid path found
    env->goal_x = GRID_WIDTH / 2;
    env->goal_y = GRID_HEIGHT / 2;
    printf("GOAL FALLBACK at (%d,%d), Entrance (%d,%d), Exit (%d,%d)\n", 
           env->goal_x, env->goal_y, env->entrance_x, env->entrance_y, env->exit_x, env->exit_y);
}

void c_reset(MazingContest* env) {
    env->tick = 0;
    env->episode_return = 0.0f;
    env->total_path_length = 0.0f;
    env->rounds_completed = 0;
    env->config.round_number = 1;
    env->config.best_time = 0.0f;
    env->moves_left = env->config.max_moves;  // Like drone_race
    env->total_path_length_rewards = 0.0f;  // Reset path length rewards
    // total_tower_utilization_rewards removed  // Reset tower utilization rewards
    env->total_thunderclap_rewards = 0.0f;  // Reset thunderclap rewards
    env->total_wall_touch_rewards = 0.0f;   // Reset wall touch rewards
    
    init_round(env);
    compute_observations(env);
}

void c_step(MazingContest* env) {
    env->tick++;  // Increment tick at START like drone_race
    env->rewards[0] = 0.0f;
    env->terminals[0] = 0;
    
    int action = env->actions[0];
    
    if (env->config.phase == PHASE_BUILD) {
        env->config.phase_timer++;
        
        // INSTANT building actions - no delays, no animations, no NOOP
        if (action >= ACTION_BUILD_WALL_START && action <= ACTION_BUILD_WALL_END) {
            int pos = action - ACTION_BUILD_WALL_START;
            int x = pos % GRID_WIDTH;
            int y = pos / GRID_WIDTH;
            place_tower(env, x, y, TOWER_WALL);
            // No building reward - focus only on runner time
        } 
        else if (action >= ACTION_BUILD_THUNDERCLAP_START && action <= ACTION_BUILD_THUNDERCLAP_END) {
            int pos = action - ACTION_BUILD_THUNDERCLAP_START;
            int x = pos % GRID_WIDTH;
            int y = pos / GRID_WIDTH;
            place_tower(env, x, y, TOWER_THUNDERCLAP);
            // No building reward - focus on activation and slowdown rewards
        }
        
        // Check if build phase should end (when ALL resources are spent)
        if (env->config.gold == 0 && env->config.lumber == 0) {
            // No resource penalties - lumber is always 0, gold is always 0 here
            
            // CRITICAL: Check if path exists with ALL towers (random + user)
            int path_exists;
            if (env->config.goal_enabled) {
                // With goal: check both entrance -> goal and goal -> exit paths
                path_exists = find_path(env, env->entrance_x, env->entrance_y, env->goal_x, env->goal_y) &&
                             find_path(env, env->goal_x, env->goal_y, env->exit_x, env->exit_y);
            } else {
                // Without goal: check direct entrance -> exit path
                path_exists = find_path(env, env->entrance_x, env->entrance_y, env->exit_x, env->exit_y);
            }
            
            if (path_exists) {
                // Include run phase
                env->config.phase = PHASE_RUN;
                env->config.phase_timer = 0;
            } else {
                // This should never happen due to path validation during tower placement
                // But if it does, just end the round with 0 time and NO rewards/stats
                env->config.runner_time = 0.0f;
                
                // Skip all reward calculations and go directly to next round
                env->rounds_completed++;
                env->config.round_number++;
                
                // Check if we've reached max rounds
                if (env->rounds_completed >= env->config.max_rounds) {
                    // Episode complete - terminate
                    env->terminals[0] = 1;
                    add_log(env);
                    c_reset(env);
                    return;
                }
                
                // Continue with next round (no rewards given)
                init_round(env);
            }
        }
    } else if (env->config.phase == PHASE_RUN) {
        env->config.phase_timer++;
        env->config.runner_time = env->config.phase_timer / env->config.fps;  // Convert to seconds
        
        move_runner(env);
        check_tower_proximity(env);  // Track which towers runner touches
        update_thunderclap_towers(env);  // Check for thunderclap activations
        update_thunderclap_effects(env); // Update visual effects
        
        if (env->config.maze_completed) {
            // RUNNER TIME REWARD DISABLED - using path length rewards instead
            // env->rewards[0] += env->config.runner_time * env->config.runner_time_reward_multiplier;
            
            // Update best time tracking
            if (env->config.runner_time > env->config.best_time) {
                env->config.best_time = env->config.runner_time;
            }
            
            // NEW Efficiency system - percentage-based reward (0-1 scale) - ONCE PER ROUND
            // Step 1: Count total blockades (all towers - walls, thunderclaps, random)
            int total_blockades = 0;
            for (int i = 0; i < env->num_towers; i++) {
                if (env->towers[i].active) {
                    total_blockades++;
                }
            }
            
            // Step 2: Count actual touches using wall touch system (can be >4 per blockade)
            int actual_touches = 0;
            for (int i = 0; i < env->num_towers; i++) {
                Tower* tower = &env->towers[i];
                if (tower->active) {
                    actual_touches += tower->touch_count;  // Use existing touch count system
                }
            }
            
            // Step 3: Calculate efficiency as percentage (0-1 scale, can go >1 for loops)
            if (total_blockades > 0) {
                int possible_edges = total_blockades * 4;  // 4 edges per blockade
                float efficiency_ratio = (float)actual_touches / (float)possible_edges;
                
                // Give reward based on efficiency percentage (0-10 reward range, capped at 100%)
                float capped_ratio = efficiency_ratio > 1.0f ? 1.0f : efficiency_ratio;
                float efficiency_reward = capped_ratio * 10.0f;
                
                env->rewards[0] += efficiency_reward;
                
                // Track efficiency stats using same pattern as wall touch rewards
                env->total_efficiency_rewards += efficiency_reward;
                env->total_efficiency_percentage += efficiency_ratio;  // Don't cap for stats (can show >100%)
            }
            
            // Thunderclap NEW slow penalty - penalize thunderclap towers that never caused a NEW slow
            int ineffective_thunderclaps = 0;
            for (int i = 0; i < env->num_towers; i++) {
                Tower* tower = &env->towers[i];
                if (tower->active && !tower->is_random_obstacle && tower->type == TOWER_THUNDERCLAP) {
                    if (tower->caused_new_slow == 0) {  // Never caused a NEW slow
                        ineffective_thunderclaps++;
                    }
                }
            }
            if (ineffective_thunderclaps > 0) {
                float penalty = ineffective_thunderclaps * 20.0f;  // -20 points per ineffective thunderclap
                env->rewards[0] -= penalty;  // Apply penalty
                env->total_thunderclap_rewards -= penalty;  // Track in thunderclap rewards (as negative)
            }
            
            env->rounds_completed++;
            env->config.round_number++;
            
            // Check if we've reached max rounds
            if (env->rounds_completed >= env->config.max_rounds) {
                // Episode complete - terminate after this round
                env->terminals[0] = 1;
                
                // FINAL SCORING DISABLED - using only path length rewards
                // No penalties or bonuses at episode end - all rewards are immediate
                
                add_log(env);
                c_reset(env);
                return;
            }
            
            // Continue with next round
            init_round(env);
        }
    }
    
    // Keep moves_left for emergency timeout (in case of infinite loops)
    env->moves_left--;
    
    // Emergency termination if stuck in infinite loop (safety fallback)
    if (env->moves_left == 0) {
        env->terminals[0] = 1;
        add_log(env);
        c_reset(env);
        return;
    }
    
    env->episode_return += env->rewards[0];
    compute_observations(env);
}

Client* make_client(MazingContest* env) {
    Client* client = (Client*)calloc(1, sizeof(Client));
    client->cell_size = env->config.cell_size_render;
    client->width = GRID_WIDTH * client->cell_size;
    client->height = GRID_HEIGHT * client->cell_size + 100;  // Extra space for UI
    
    InitWindow(client->width, client->height, "PufferLib Mazing Contest");
    SetTargetFPS(60);
    
    return client;
}

void close_client(Client* client) {
    CloseWindow();
    free(client);
}

void c_render(MazingContest* env) {
    if (env->client == NULL) {
        env->client = make_client(env);
    }
    
    if (IsKeyDown(KEY_ESCAPE)) {
        exit(0);
    }
    
    BeginDrawing();
    
    // EPIC dark gradient background
    DrawRectangleGradientV(0, 0, env->client->width, env->client->height, 
                          (Color){10, 15, 25, 255}, (Color){25, 35, 50, 255});
    
    float cell_size = env->client->cell_size;
    
    // ULTRA-OPTIMIZED: Pre-compute colors and render in optimized order
    const Color base_color1 = (Color){20, 25, 35, 255};
    const Color base_color2 = (Color){25, 30, 40, 255};
    const Color wall_shadow = (Color){0, 0, 0, 80};
    const Color wall_grad1 = (Color){140, 140, 140, 255};
    const Color wall_grad2 = (Color){80, 80, 80, 255};
    const Color wall_line1 = (Color){180, 180, 180, 255};
    const Color wall_line2 = (Color){60, 60, 60, 255};
    const Color thunder_shadow = (Color){0, 0, 0, 100};
    const Color thunder_grad1 = (Color){138, 43, 226, 255};
    const Color thunder_grad2 = (Color){75, 0, 130, 255};
    const Color thunder_line = (Color){186, 85, 211, 255};
    const Color white_marker = (Color){255, 255, 255, 200};
    
    // PERFORMANCE: Draw in row-major order for cache efficiency
    for (int y = 0; y < GRID_HEIGHT; y++) {
        int y_pos = y * cell_size;
        
        for (int x = 0; x < GRID_WIDTH; x++) {
            int x_pos = x * cell_size;
            
            // Fast checkerboard pattern
            Color base_color = ((x + y) & 1) ? base_color2 : base_color1;
            DrawRectangle(x_pos, y_pos, cell_size, cell_size, base_color);
            
            int grid_val = env->grid[x][y];
            if (grid_val == 1) {
                // WALL - Optimized rendering
                DrawRectangle(x_pos + 2, y_pos + 2, cell_size - 2, cell_size - 2, wall_shadow);
                DrawRectangleGradientV(x_pos, y_pos, cell_size, cell_size, wall_grad1, wall_grad2);
                DrawRectangleLines(x_pos, y_pos, cell_size, cell_size, wall_line1);
                DrawRectangleLines(x_pos + 1, y_pos + 1, cell_size - 2, cell_size - 2, wall_line2);
                
                // Fast random obstacle marker
                int tower_idx = env->tower_grid[x][y];
                if (tower_idx != -1 && env->towers[tower_idx].is_random_obstacle) {
                    int quarter = (int)cell_size >> 2; // Divide by 4 using bit shift
                    for (int gx = 0; gx < cell_size; gx += quarter) {
                        DrawLine(x_pos + gx, y_pos, x_pos + gx, y_pos + cell_size, white_marker);
                    }
                    for (int gy = 0; gy < cell_size; gy += quarter) {
                        DrawLine(x_pos, y_pos + gy, x_pos + cell_size, y_pos + gy, white_marker);
                    }
                }
                
                // Display touch count on walls (ALL walls including random obstacles)
                if (tower_idx != -1 && env->towers[tower_idx].touch_count > 0) {
                    char touch_text[4];
                    snprintf(touch_text, sizeof(touch_text), "%d", env->towers[tower_idx].touch_count);
                    
                    // Color based on touch count (more touches = more vibrant)
                    Color touch_color = WHITE;
                    if (env->towers[tower_idx].touch_count >= 6) {
                        touch_color = (Color){255, 255, 0, 255};  // Yellow for 6+ touches
                    } else if (env->towers[tower_idx].touch_count >= 4) {
                        touch_color = (Color){255, 165, 0, 255};  // Orange for 4-5 touches
                    } else if (env->towers[tower_idx].touch_count >= 2) {
                        touch_color = (Color){255, 255, 255, 255}; // White for 2-3 touches
                    }
                    
                    // Center the text in the cell
                    int text_width = MeasureText(touch_text, 10);
                    int text_x = x_pos + (cell_size - text_width) / 2;
                    int text_y = y_pos + (cell_size - 10) / 2;
                    
                    DrawText(touch_text, text_x, text_y, 10, touch_color);
                }
            } else if (grid_val == 2) {
                // THUNDERCLAP - Optimized rendering
                DrawRectangle(x_pos + 2, y_pos + 2, cell_size - 2, cell_size - 2, thunder_shadow);
                DrawRectangleGradientV(x_pos, y_pos, cell_size, cell_size, thunder_grad1, thunder_grad2);
                DrawRectangleLines(x_pos, y_pos, cell_size, cell_size, thunder_line);
                
                int tower_idx = env->tower_grid[x][y];
                if (tower_idx != -1) {
                    Tower* tower = &env->towers[tower_idx];
                    
                    // Fast random obstacle X marker
                    if (tower->is_random_obstacle) {
                        int offset = 4;
                        int end_pos = cell_size - 4;
                        DrawLine(x_pos + offset, y_pos + offset, x_pos + end_pos, y_pos + end_pos, white_marker);
                        DrawLine(x_pos + end_pos, y_pos + offset, x_pos + offset, y_pos + end_pos, white_marker);
                        DrawLine(x_pos + offset + 1, y_pos + offset, x_pos + end_pos - 1, y_pos + end_pos, white_marker);
                        DrawLine(x_pos + end_pos - 1, y_pos + offset, x_pos + offset + 1, y_pos + end_pos, white_marker);
                    }
                    
                    // Fast glow effect for ready towers
                    if (tower->cooldown == 0 && !tower->is_random_obstacle) {
                        DrawRectangleLines(x_pos - 1, y_pos - 1, cell_size + 2, cell_size + 2, 
                                         (Color){255, 255, 255, 150});
                        DrawRectangleLines(x_pos - 2, y_pos - 2, cell_size + 4, cell_size + 4, 
                                         (Color){255, 255, 255, 80});
                    }
                    
                    // Display touch count on thunderclap towers (2x multiplier bonus!)
                    if (tower->touch_count > 0) {
                        char touch_text[4];
                        snprintf(touch_text, sizeof(touch_text), "%d", tower->touch_count);
                        
                        // Color based on touch count (thunderclap towers get special coloring)
                        Color touch_color = (Color){255, 255, 255, 255}; // Default white
                        if (tower->touch_count >= 6) {
                            touch_color = (Color){255, 255, 100, 255};  // Bright yellow for 6+ touches
                        } else if (tower->touch_count >= 4) {
                            touch_color = (Color){255, 200, 100, 255};  // Gold for 4-5 touches  
                        } else if (tower->touch_count >= 2) {
                            touch_color = (Color){200, 255, 255, 255};  // Cyan for 2-3 touches (thunderclap special)
                        }
                        
                        // Center the text in the cell
                        int text_width = MeasureText(touch_text, 10);
                        int text_x = x_pos + (cell_size - text_width) / 2;
                        int text_y = y_pos + (cell_size - 10) / 2;
                        
                        DrawText(touch_text, text_x, text_y, 10, touch_color);
                    }
                }
            }
        }
    }
    
    // Epic cyber grid lines
    for (int x = 0; x <= GRID_WIDTH; x++) {
        DrawLine(x * cell_size, 0, x * cell_size, GRID_HEIGHT * cell_size, (Color){100, 150, 200, 100});
    }
    for (int y = 0; y <= GRID_HEIGHT; y++) {
        DrawLine(0, y * cell_size, GRID_WIDTH * cell_size, y * cell_size, (Color){100, 150, 200, 100});
    }
    
    // EPIC ENTRANCE - Glowing portal
    DrawRectangle(env->entrance_x * cell_size + 3, env->entrance_y * cell_size + 3, 
                 cell_size - 6, cell_size - 6, (Color){0, 255, 100, 200});
    DrawRectangleLines(env->entrance_x * cell_size + 2, env->entrance_y * cell_size + 2, 
                      cell_size - 4, cell_size - 4, (Color){0, 255, 100, 255});
    DrawRectangleLines(env->entrance_x * cell_size + 1, env->entrance_y * cell_size + 1, 
                      cell_size - 2, cell_size - 2, (Color){255, 255, 255, 150});
    DrawText("START", env->entrance_x * cell_size + 5, env->entrance_y * cell_size + 12, 8, WHITE);
    
    // EPIC EXIT - Fiery portal
    DrawRectangle(env->exit_x * cell_size + 3, env->exit_y * cell_size + 3, 
                 cell_size - 6, cell_size - 6, (Color){255, 50, 50, 200});
    DrawRectangleLines(env->exit_x * cell_size + 2, env->exit_y * cell_size + 2, 
                      cell_size - 4, cell_size - 4, (Color){255, 50, 50, 255});
    DrawRectangleLines(env->exit_x * cell_size + 1, env->exit_y * cell_size + 1, 
                      cell_size - 2, cell_size - 2, (Color){255, 255, 255, 150});
    DrawText("EXIT", env->exit_x * cell_size + 8, env->exit_y * cell_size + 12, 8, WHITE);
    
    // EPIC GOAL - Golden star (if goal system is enabled)
    if (env->config.goal_enabled) {
        Color goal_color = env->goal_visited ? (Color){255, 215, 0, 100} : (Color){255, 215, 0, 255};
        DrawRectangle(env->goal_x * cell_size + 3, env->goal_y * cell_size + 3, 
                     cell_size - 6, cell_size - 6, goal_color);
        DrawRectangleLines(env->goal_x * cell_size + 2, env->goal_y * cell_size + 2, 
                          cell_size - 4, cell_size - 4, (Color){255, 215, 0, 255});
        DrawRectangleLines(env->goal_x * cell_size + 1, env->goal_y * cell_size + 1, 
                          cell_size - 2, cell_size - 2, (Color){255, 255, 255, 150});
        DrawText("GOAL", env->goal_x * cell_size + 5, env->goal_y * cell_size + 12, 8, WHITE);
    }
    
    // EPIC THUNDERCLAP EARTHQUAKE EFFECTS
    for (int i = 0; i < env->num_thunderclap_effects; i++) {
        ThunderclapEffect* effect = &env->thunderclap_effects[i];
        if (!effect->active) continue;
        
        // Convert world coordinates to screen coordinates
        int screen_x = (int)(effect->x * cell_size / env->config.grid_cell_size);
        int screen_y = (int)(effect->y * cell_size / env->config.grid_cell_size);
        
        // Calculate effect intensity (stronger at start)
        float intensity = (float)effect->duration / effect->max_duration;
        int alpha = (int)(255 * intensity);
        
        // EPIC EARTHQUAKE SHOCKWAVE
        float radius = (1.0f - intensity) * 60.0f; // Expanding shockwave
        DrawCircleLines(screen_x, screen_y, radius, (Color){255, 255, 0, alpha});
        DrawCircleLines(screen_x, screen_y, radius * 0.8f, (Color){255, 150, 0, alpha});
        DrawCircleLines(screen_x, screen_y, radius * 0.6f, (Color){255, 50, 0, alpha});
        
        // Lightning effect
        DrawCircle(screen_x, screen_y, 8 * intensity, (Color){255, 255, 255, alpha});
        DrawCircle(screen_x, screen_y, 12 * intensity, (Color){255, 255, 0, alpha/2});
        
        // Screen shake lines for earthquake
        if (intensity > 0.5f) {
            for (int j = 0; j < 8; j++) {
                int offset = (int)(intensity * 3);
                DrawLine(screen_x - 20 + (j * 5), screen_y + offset, 
                        screen_x - 15 + (j * 5), screen_y - offset, 
                        (Color){255, 255, 100, alpha/2});
            }
        }
    }
    
    // EPIC RUNNER with glow trail
    if (env->config.phase == PHASE_RUN || env->config.phase == PHASE_BUILD) {
        Color runner_color = (Color){0, 150, 255, 255};
        if (env->tick < env->runner.slowed_until_tick) {
            runner_color = (Color){255, 0, 255, 255};  // Bright magenta when slowed
        }
        
        // During build phase, make runner semi-transparent to show it's waiting
        if (env->config.phase == PHASE_BUILD) {
            runner_color = (Color){runner_color.r, runner_color.g, runner_color.b, 150};
        }
        
        int screen_x = (int)(env->runner.x * cell_size / env->config.grid_cell_size);
        int screen_y = (int)(env->runner.y * cell_size / env->config.grid_cell_size);
        
        // Epic glow trail
        DrawCircle(screen_x, screen_y, 15, (Color){runner_color.r, runner_color.g, runner_color.b, 30});
        DrawCircle(screen_x, screen_y, 10, (Color){runner_color.r, runner_color.g, runner_color.b, 80});
        DrawCircle(screen_x, screen_y, 6, runner_color);
        DrawCircleLines(screen_x, screen_y, 8, (Color){255, 255, 255, 200});
    }
    
    // EPIC FUTURISTIC HUD
    int ui_y = GRID_HEIGHT * cell_size + 15;
    Color ui_bg = (Color){10, 15, 25, 240};
    Color ui_accent = (Color){100, 150, 255, 255};
    
    // Main HUD panel with epic styling
    DrawRectangle(10, ui_y - 5, env->client->width - 20, 110, ui_bg);
    DrawRectangleGradientH(10, ui_y - 5, env->client->width - 20, 3, ui_accent, (Color){50, 100, 200, 255});
    DrawRectangleLines(10, ui_y - 5, env->client->width - 20, 110, ui_accent);
    
    // Epic phase indicator
    const char* phase_text = env->config.phase == PHASE_BUILD ? "⚒ BUILD PHASE ⚒" : 
                             env->config.phase == PHASE_RUN ? "⚡ RUN PHASE ⚡" : "📊 RESULT";
    Color phase_color = env->config.phase == PHASE_BUILD ? (Color){255, 200, 0, 255} : 
                        env->config.phase == PHASE_RUN ? (Color){0, 255, 0, 255} : 
                        (Color){255, 100, 100, 255};
    DrawText(phase_text, 20, ui_y + 5, 18, phase_color);
    
    // Resources with epic styling
    DrawText(TextFormat("💰 Gold: %d", env->config.gold), 20, ui_y + 30, 14, (Color){255, 215, 0, 255});
    DrawText(TextFormat("🌲 Lumber: %d", env->config.lumber), 20, ui_y + 50, 14, (Color){139, 69, 19, 255});
    
    // Game info
    DrawText(TextFormat("Round: %d", env->config.round_number), 200, ui_y + 30, 14, 
             (Color){150, 200, 255, 255});
    DrawText(TextFormat("Score: %.1f", env->episode_return), 200, ui_y + 50, 14, (Color){255, 255, 100, 255});
    
    // Epic timer
    if (env->config.phase == PHASE_BUILD) {
        int time_left = (env->config.build_time_limit - env->config.phase_timer) / 60;
        Color timer_color = time_left > 5 ? (Color){255, 255, 255, 255} : (Color){255, 100, 100, 255};
        DrawText(TextFormat("⏱ Build Time: %ds", time_left), 20, ui_y + 70, 14, timer_color);
    } else if (env->config.phase == PHASE_RUN) {
        DrawText(TextFormat("⏱ Runner Time: %.1fs", env->config.runner_time), 20, ui_y + 70, 14, 
                (Color){100, 255, 100, 255});
    } else {
        DrawText(TextFormat("⏱ Final: %.1fs (Best: %.1fs)", env->config.runner_time, env->config.best_time), 
                20, ui_y + 70, 14, WHITE);
    }
    
    EndDrawing();
}

void c_close(MazingContest* env) {
    if (env->client) {
        close_client(env->client);
        env->client = NULL;
    }
}

void free_allocated(MazingContest* env) {
    free(env->observations);
    free(env->actions);
    free(env->rewards);
    free(env->terminals);
    c_close(env);
}