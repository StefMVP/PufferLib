#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include "raylib.h"
#include "constants.h"

typedef struct GameConfig {
    uint16_t max_monsters;
    uint16_t max_items;
    float monster_spawn_rate;
    float item_drop_rate;
    uint32_t player_start_health;
    uint32_t player_start_mana;
    uint16_t starting_gold;
    float monster_move_speed;
    uint16_t episode_length_limit;
    float completion_reward;
    float monster_kill_reward;
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
    .starting_gold = 500,
    .monster_move_speed = 0.5f,
    .episode_length_limit = MAX_EPISODE_LENGTH,
    .completion_reward = 10.0f,
    .monster_kill_reward = 10.0f,
    .death_penalty = 0.0f,
};

typedef struct InventorySlot {
    uint32_t item_type;
    uint32_t item_quality;
    uint32_t item_level;       // Item level (ilvl)
    uint32_t stack_size;
    uint32_t stat_bonuses[4];
    uint16_t item_value;
} InventorySlot;

typedef struct Player {
    float x, y;
    float prev_x, prev_y;
    float movement_x, movement_y;
    float facing_x, facing_y;
    uint32_t old_inventory[MAX_INVENTORY_SIZE];
    InventorySlot inventory[INVENTORY_SLOTS];
    uint32_t gold;
    uint32_t health, max_health;
    uint32_t mana, max_mana;
    uint32_t damage;
    uint32_t dodge_chance;
    uint32_t alive;
    uint32_t blizzard_cooldown;
    uint32_t health_potion_cooldown;
    uint32_t mana_potion_cooldown;
    uint32_t mana_regen_timer;
    uint32_t low_health_penalty_cooldown;
    uint32_t inventory_count;
    uint32_t selected_inventory_slot;
} Player;

typedef struct Monster {
    float x, y;
    float target_x, target_y;
    float speed;
    float attack_range;
    uint16_t id;
    uint32_t health, max_health;
    uint32_t damage;
    uint32_t type;
    uint32_t attack_type;
    uint32_t alive;
    uint32_t move_cooldown;
    uint32_t attack_cooldown;
    uint32_t attack_animation_timer;
} Monster;

typedef struct Boss {
    float x, y;
    uint32_t health, max_health;
    uint32_t damage;
    uint32_t type;
    uint32_t alive;
    uint32_t attack_cooldown;
    uint32_t special_attack_cooldown;
} Boss;

typedef struct Item {
    float x, y;
    uint32_t type;
    uint32_t value;
    uint32_t active;
    uint16_t id;
} Item;

typedef struct Projectile {
    float x, y;
    float vel_x, vel_y;
    float target_x, target_y; // For homing projectiles
    uint32_t type;
    uint32_t damage;
    uint32_t lifetime;
    uint32_t active;
    uint32_t homing; // Is this a homing projectile?
} Projectile;

typedef struct BlizzardArea {
    float x, y;
    uint32_t duration;
    uint32_t damage_timer;
    uint32_t active;
} BlizzardArea;

typedef struct Vendor {
    float x, y;
    uint32_t items_for_sale[10];
    uint32_t prices[10];
    uint32_t stock[10];
} Vendor;

typedef struct HeroStats {
    uint32_t level;
    uint32_t experience;
    uint32_t experience_to_next;
    uint32_t stat_points_available;
    
    uint32_t strength;
    uint32_t dexterity;
    uint32_t intelligence;
    uint32_t vitality;
    
    uint32_t total_strength;
    uint32_t total_dexterity;
    uint32_t total_intelligence;
    uint32_t total_vitality;
    uint32_t average_item_level;
} HeroStats;

typedef struct Equipment {
    // LEFT COLUMN - From top to bottom
    uint32_t shoulders_type;
    uint32_t shoulders_quality;
    uint32_t shoulders_level;
    uint32_t shoulders_stat_bonuses[4];
    
    uint32_t gloves_type;
    uint32_t gloves_quality;
    uint32_t gloves_level;
    uint32_t gloves_stat_bonuses[4];
    
    uint32_t ring_left_type;
    uint32_t ring_left_quality;
    uint32_t ring_left_level;
    uint32_t ring_left_stat_bonuses[4];
    
    uint32_t weapon_type;
    uint32_t weapon_quality;
    uint32_t weapon_level;
    uint32_t weapon_stat_bonuses[4];
    
    // MIDDLE COLUMN - From top to bottom
    uint32_t helmet_type;
    uint32_t helmet_quality;
    uint32_t helmet_level;
    uint32_t helmet_stat_bonuses[4];
    
    uint32_t armor_type;
    uint32_t armor_quality;
    uint32_t armor_level;
    uint32_t armor_stat_bonuses[4];
    
    uint32_t belt_type;
    uint32_t belt_quality;
    uint32_t belt_level;
    uint32_t belt_stat_bonuses[4];
    
    uint32_t pants_type;
    uint32_t pants_quality;
    uint32_t pants_level;
    uint32_t pants_stat_bonuses[4];
    
    uint32_t boots_type;
    uint32_t boots_quality;
    uint32_t boots_level;
    uint32_t boots_stat_bonuses[4];
    
    // RIGHT COLUMN - From top to bottom
    uint32_t amulet_type;
    uint32_t amulet_quality;
    uint32_t amulet_level;
    uint32_t amulet_stat_bonuses[4];
    
    uint32_t bracers_type;
    uint32_t bracers_quality;
    uint32_t bracers_level;
    uint32_t bracers_stat_bonuses[4];
    
    uint32_t ring_right_type;
    uint32_t ring_right_quality;
    uint32_t ring_right_level;
    uint32_t ring_right_stat_bonuses[4];
    
    uint32_t offhand_type;
    uint32_t offhand_quality;
    uint32_t offhand_level;
    uint32_t offhand_stat_bonuses[4];
} Equipment;

typedef struct ShopItem {
    uint32_t item_type;
    uint32_t item_quality;
    uint32_t item_level;
    uint32_t stat_bonuses[4];
    uint16_t price;
    uint32_t available;
} ShopItem;

typedef struct TownInterface {
    uint16_t frames_remaining;
    uint32_t warning_phase;
    uint32_t current_tab;           // TOWN_TAB_CHARACTER or TOWN_TAB_SHOP
    uint32_t character_mode;        // CHARACTER_MODE_EQUIPMENT or CHARACTER_MODE_INVENTORY
    uint32_t selected_item_index;   // Currently selected item in current view
    uint32_t equipment_slot;        // Selected equipment slot (0=weapon, 1=armor, 2=accessory)
    uint32_t input_cooldown;        // Cooldown for WASD navigation to prevent rapid input
    int16_t shop_scroll_offset;    // Scroll offset for shop (negative values scroll up)
    uint32_t current_visit_purchases;  // Number of purchases made during this town visit
} TownInterface;

typedef struct Log {
    float episode_return;
    float episode_length;
    float monsters_killed;
    float boss_kills;
    float gold_earned;
    float damage_dealt;
    float damage_taken;
    float rift_completions;
    float vendor_transactions;
    float blizzards_cast;
    float deaths;
    float completion_rewards;
    float monster_kill_rewards;
    float death_penalties;
    float hero_level;
    float total_experience;
    float shop_purchases;
    float town_time_efficiency;
    float no_purchase_penalties;
    float current_gold;
    float total_strength;
    float total_dexterity;
    float total_intelligence;
    float total_vitality;
    float average_item_level;
    float n;
} Log;

typedef struct SpriteSystem {
    Texture2D tileset;
    Texture2D hero_idle;
    Texture2D hero_walk;
    Texture2D hero_cast;
    Texture2D monsters[5];
    Texture2D boss_texture;
    Texture2D items[3];
    Texture2D projectiles[2];
    Texture2D effects[3];
    int tile_size;
    int hero_frame;
    int hero_animation_timer;
    int hero_animation_state;
} SpriteSystem;

typedef struct Client {
    float cell_size;
    int width;
    int height;
    Camera2D camera;
    SpriteSystem sprites;
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
    
    HeroStats hero_stats;
    Equipment equipment;
    ShopItem shop_items[SHOP_ITEMS_COUNT];
    TownInterface town_interface;
    
    GameConfig config;
    uint32_t tick;
    uint32_t current_phase;
    uint32_t current_rift_level;
    
    uint16_t monsters_spawned;
    uint16_t monsters_killed;
    uint32_t boss_spawned;
    uint32_t rift_completed;
    
    uint32_t elites_spawned;
    uint32_t max_elites;
    
    uint16_t next_monster_id;
    uint16_t next_item_id;
    
    uint16_t episode_length;
    float episode_return;
    float step_reward;
    
    uint16_t episode_monsters_killed;
    uint32_t episode_boss_kills;
    uint16_t episode_gold_earned;
    uint16_t episode_damage_dealt;
    uint16_t episode_damage_taken;
    uint32_t episode_rift_completions;
    uint32_t episode_vendor_transactions;
    uint32_t episode_blizzards_cast;
    uint32_t episode_deaths;
    
    float episode_completion_rewards;
    float episode_monster_kill_rewards;
    float episode_death_penalties;
    float episode_no_purchase_penalties;
    
    uint32_t episode_shop_purchases;
    uint16_t episode_town_time_used;
    
    uint32_t human_mode;               // Toggle for human control (1) vs AI (0)
} Rift;

#include "render.h"

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

void init_hero_stats(Rift* env);
void calculate_total_stats(Rift* env);
void add_experience(Rift* env, uint16_t exp);
void level_up_hero(Rift* env);

void generate_shop_inventory(Rift* env);
void handle_shop_interaction(Rift* env);
void handle_inventory_interaction(Rift* env);
void equip_item_from_inventory(Rift* env, uint32_t slot);
void buy_shop_item(Rift* env, uint32_t shop_slot);

void transition_to_town(Rift* env);
void transition_to_rift(Rift* env);
void update_town_timer(Rift* env);
void handle_town_navigation(Rift* env, int action);
void handle_town_interaction(Rift* env);

uint32_t is_at_shop_item(Rift* env, uint32_t shop_slot);
uint32_t is_at_inventory_area(Rift* env);
uint32_t is_at_vendor(Rift* env);
uint32_t is_at_rift_portal(Rift* env);

// ============================================================================
// GAME LOGIC FUNCTIONS
// ============================================================================

void spawn_monsters(Rift* env);
void spawn_diverse_pack(Rift* env, float center_x, float center_y, uint32_t pack_size);
uint32_t get_random_monster_type(Rift* env, uint32_t allow_elite);
void update_monsters(Rift* env);
void update_player(Rift* env);
void handle_blizzard(Rift* env);
void handle_blizzard_with_direction(Rift* env, float facing_x, float facing_y);
void update_blizzard_areas(Rift* env);
void handle_vendor_interaction(Rift* env);
void spawn_boss(Rift* env);

void spawn_projectile(Rift* env, float start_x, float start_y, float target_x, float target_y, uint32_t type, uint32_t damage);
void execute_monster_attack(Rift* env, Monster* monster);
void update_projectiles(Rift* env);

// Scaling functions
float GetScaledMonsterHealth(float base_health, uint32_t rift_level);
float GetScaledMonsterDamage(float base_damage, uint32_t rift_level);
float GetScaledMonsterSpeed(float base_speed, uint32_t rift_level);
uint32_t GetScaledAttackCooldown(uint32_t base_cooldown, uint32_t rift_level);

void compute_observations(Rift* env);

void c_close_client(Client* client);

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

static inline uint32_t clamp_uint32(int value, uint32_t min_val, uint32_t max_val) {
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return (uint32_t)value;
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
    env->current_rift_level = 1; // Start at Rift 1
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
    
    env->vendor.x = VENDOR_POSITION_X;
    env->vendor.y = VENDOR_POSITION_Y;
    
    env->vendor.items_for_sale[0] = ITEM_HEALTH_POTION;
    env->vendor.prices[0] = VENDOR_HEALTH_POTION_PRICE;
    env->vendor.stock[0] = 10;
    
    env->vendor.items_for_sale[1] = ITEM_MANA_POTION;
    env->vendor.prices[1] = VENDOR_MANA_POTION_PRICE;
    env->vendor.stock[1] = VENDOR_STOCK_AMOUNT;
}

void spawn_monster_pack(Rift* env, float center_x, float center_y, uint32_t pack_size, uint32_t monster_type) {
    for (uint32_t p = 0; p < pack_size; p++) {
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
                        env->monsters[i].health = (uint32_t)GetScaledMonsterHealth(MAGE_HEALTH, env->current_rift_level);
                        env->monsters[i].max_health = env->monsters[i].health;
                        env->monsters[i].damage = (uint32_t)GetScaledMonsterDamage(MAGE_DAMAGE, env->current_rift_level);
                        env->monsters[i].speed = GetScaledMonsterSpeed(MAGE_SPEED, env->current_rift_level);
                        env->monsters[i].attack_range = MAGE_RANGE;
                        env->monsters[i].attack_type = ATTACK_TYPE_HOMING_PROJECTILE;
                        break;
                    case MONSTER_HEAVY_MELEE:
                        env->monsters[i].health = (uint32_t)GetScaledMonsterHealth(HEAVY_HEALTH, env->current_rift_level);
                        env->monsters[i].max_health = env->monsters[i].health;
                        env->monsters[i].damage = (uint32_t)GetScaledMonsterDamage(HEAVY_DAMAGE, env->current_rift_level);
                        env->monsters[i].speed = GetScaledMonsterSpeed(HEAVY_SPEED, env->current_rift_level);
                        env->monsters[i].attack_range = HEAVY_RANGE;
                        env->monsters[i].attack_type = ATTACK_TYPE_CONE_SLAM;
                        break;
                    case MONSTER_LIGHT:
                        env->monsters[i].health = (uint32_t)GetScaledMonsterHealth(LIGHT_HEALTH, env->current_rift_level);
                        env->monsters[i].max_health = env->monsters[i].health;
                        env->monsters[i].damage = (uint32_t)GetScaledMonsterDamage(LIGHT_DAMAGE, env->current_rift_level);
                        env->monsters[i].speed = GetScaledMonsterSpeed(LIGHT_SPEED, env->current_rift_level);
                        env->monsters[i].attack_range = LIGHT_RANGE;
                        env->monsters[i].attack_type = ATTACK_TYPE_FAST_PROJECTILE;
                        break;
                    case MONSTER_ELITE:
                        env->monsters[i].health = (uint32_t)GetScaledMonsterHealth(ELITE_HEALTH, env->current_rift_level);
                        env->monsters[i].max_health = env->monsters[i].health;
                        env->monsters[i].damage = (uint32_t)GetScaledMonsterDamage(ELITE_DAMAGE, env->current_rift_level);
                        env->monsters[i].speed = GetScaledMonsterSpeed(ELITE_SPEED, env->current_rift_level);
                        env->monsters[i].attack_range = ELITE_RANGE;
                        env->monsters[i].attack_type = ATTACK_TYPE_PROJECTILE;
                        break;
                    default: // MONSTER_ZOMBIE
                        env->monsters[i].health = (uint32_t)GetScaledMonsterHealth(MONSTER_BASE_HEALTH, env->current_rift_level);
                        env->monsters[i].max_health = env->monsters[i].health;
                        env->monsters[i].damage = (uint32_t)GetScaledMonsterDamage(MONSTER_BASE_DAMAGE, env->current_rift_level);
                        env->monsters[i].speed = GetScaledMonsterSpeed(0.5f, env->current_rift_level);
                        env->monsters[i].attack_range = 1.5f; // Melee range
                        env->monsters[i].attack_type = ATTACK_TYPE_MELEE;
                        break;
                }
                
                env->monsters_spawned++;
                break;
            }
        }
    }
}

uint32_t get_random_monster_type(Rift* env, uint32_t allow_elite) {
    uint32_t type_roll = rand() % 100;
    
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

void spawn_diverse_pack(Rift* env, float center_x, float center_y, uint32_t pack_size) {
    for (uint32_t p = 0; p < pack_size; p++) {
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
                
                uint32_t monster_type = get_random_monster_type(env, 1);
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
                        env->monsters[i].health = (uint32_t)GetScaledMonsterHealth(MAGE_HEALTH, env->current_rift_level);
                        env->monsters[i].max_health = env->monsters[i].health;
                        env->monsters[i].damage = (uint32_t)(GetScaledMonsterDamage(MAGE_DAMAGE, env->current_rift_level) * 0.75f);
                        env->monsters[i].speed = GetScaledMonsterSpeed(MAGE_SPEED, env->current_rift_level);
                        env->monsters[i].attack_range = MAGE_RANGE;
                        env->monsters[i].attack_type = ATTACK_TYPE_HOMING_PROJECTILE;
                        break;
                    case MONSTER_HEAVY_MELEE:
                        env->monsters[i].health = (uint32_t)GetScaledMonsterHealth(HEAVY_HEALTH, env->current_rift_level);
                        env->monsters[i].max_health = env->monsters[i].health;
                        env->monsters[i].damage = (uint32_t)(GetScaledMonsterDamage(HEAVY_DAMAGE, env->current_rift_level) * 0.75f);
                        env->monsters[i].speed = GetScaledMonsterSpeed(HEAVY_SPEED, env->current_rift_level);
                        env->monsters[i].attack_range = HEAVY_RANGE;
                        env->monsters[i].attack_type = ATTACK_TYPE_CONE_SLAM;
                        break;
                    case MONSTER_LIGHT:
                        env->monsters[i].health = (uint32_t)GetScaledMonsterHealth(LIGHT_HEALTH, env->current_rift_level);
                        env->monsters[i].max_health = env->monsters[i].health;
                        env->monsters[i].damage = (uint32_t)(GetScaledMonsterDamage(LIGHT_DAMAGE, env->current_rift_level) * 0.75f);
                        env->monsters[i].speed = GetScaledMonsterSpeed(LIGHT_SPEED, env->current_rift_level);
                        env->monsters[i].attack_range = LIGHT_RANGE;
                        env->monsters[i].attack_type = ATTACK_TYPE_FAST_PROJECTILE;
                        break;
                    case MONSTER_ELITE:
                        env->monsters[i].health = (uint32_t)GetScaledMonsterHealth(ELITE_HEALTH, env->current_rift_level);
                        env->monsters[i].max_health = env->monsters[i].health;
                        env->monsters[i].damage = (uint32_t)(GetScaledMonsterDamage(ELITE_DAMAGE, env->current_rift_level) * 0.75f);
                        env->monsters[i].speed = GetScaledMonsterSpeed(ELITE_SPEED, env->current_rift_level);
                        env->monsters[i].attack_range = ELITE_RANGE;
                        env->monsters[i].attack_type = ATTACK_TYPE_PROJECTILE;
                        break;
                    default: // MONSTER_ZOMBIE
                        env->monsters[i].health = (uint32_t)GetScaledMonsterHealth(MONSTER_BASE_HEALTH, env->current_rift_level);
                        env->monsters[i].max_health = env->monsters[i].health;
                        env->monsters[i].damage = (uint32_t)(GetScaledMonsterDamage(MONSTER_BASE_DAMAGE, env->current_rift_level) * 0.75f);
                        env->monsters[i].speed = GetScaledMonsterSpeed(0.5f, env->current_rift_level);
                        env->monsters[i].attack_range = 1.5f; // Melee range
                        env->monsters[i].attack_type = ATTACK_TYPE_MELEE;
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
        
        uint32_t pack_size = 2 + rand() % 3;
        
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
                        execute_monster_attack(env, monster);
                        monster->attack_cooldown = GetScaledAttackCooldown(MONSTER_ATTACK_COOLDOWN, env->current_rift_level);
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
    if (env->player.low_health_penalty_cooldown > 0) {
        env->player.low_health_penalty_cooldown--;
    }
    
    // Check for low health penalty (only in rift phase)
    if (env->current_phase == PHASE_RIFT && env->player.alive) {
        float health_percent = (float)env->player.health / (float)env->player.max_health;
        if (health_percent <= LOW_HEALTH_THRESHOLD && 
            env->player.health_potion_cooldown == 0 && 
            env->player.low_health_penalty_cooldown == 0) {
            env->step_reward += LOW_HEALTH_PENALTY;
            env->episode_return += LOW_HEALTH_PENALTY;
            env->player.low_health_penalty_cooldown = LOW_HEALTH_PENALTY_COOLDOWN;
        }
    }
    
    env->player.mana_regen_timer++;
    if (env->player.mana_regen_timer >= MANA_REGEN_RATE) {
        env->player.mana_regen_timer = 0;
        if (env->player.mana < env->player.max_mana) {
            env->player.mana++;
        }
    }
    
    if (env->current_phase == PHASE_TOWN) {
        // Handle navigation (WASD) in town
        if (action == ACTION_MOVE_UP || action == ACTION_MOVE_DOWN || 
            action == ACTION_MOVE_LEFT || action == ACTION_MOVE_RIGHT) {
            handle_town_navigation(env, action);
            return;
        }
        // Handle exit town action (Z key)
        if (action == ACTION_EXIT_TOWN) {
            transition_to_rift(env);
            return;
        }
        // Let ACTION_INTERACT fall through to the switch statement below
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
        case ACTION_USE_HEALTH_POTION:
            if (env->player.alive && env->player.health_potion_cooldown == 0 && env->player.health < env->player.max_health) {
                env->player.health = clamp_uint32(env->player.health + HEALTH_POTION_HEAL, 0, env->player.max_health);
                env->player.health_potion_cooldown = HEALTH_POTION_COOLDOWN;
            }
            break;
        case ACTION_USE_MANA_POTION:
            if (env->player.alive && env->player.mana_potion_cooldown == 0 && env->player.mana < env->player.max_mana) {
                env->player.mana = clamp_uint32(env->player.mana + MANA_POTION_RESTORE, 0, env->player.max_mana);
                env->player.mana_potion_cooldown = MANA_POTION_COOLDOWN;
            }
            break;
        case ACTION_INTERACT:
            if (env->current_phase == PHASE_TOWN) {
                handle_town_interaction(env);
            }
            break;
    }
    
    // Item pickup removed - gold is given directly
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
                uint32_t damage_amount = (env->monsters[j].health <= BLIZZARD_DAMAGE) ? 
                                       env->monsters[j].health : BLIZZARD_DAMAGE;
                if (env->monsters[j].health <= BLIZZARD_DAMAGE) {
                    env->monsters[j].health = 0;
                    env->monsters[j].alive = 0;
                    env->monsters_killed++;
                    env->episode_monsters_killed++;
                    env->step_reward += env->config.monster_kill_reward;
                    env->episode_return += env->config.monster_kill_reward;
                    env->episode_monster_kill_rewards += env->config.monster_kill_reward;
                    add_experience(env, EXP_PER_MONSTER_KILL);
                    
                    // Give gold directly instead of dropping items
                    if ((float)rand() / RAND_MAX < env->config.item_drop_rate) {
                        uint16_t gold_amount = GOLD_DROP_MIN + rand() % GOLD_DROP_RANGE;
                        env->player.gold += gold_amount;
                        env->episode_gold_earned += gold_amount;
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
            uint32_t damage_amount = (env->boss.health <= BLIZZARD_DAMAGE) ? 
                                   env->boss.health : BLIZZARD_DAMAGE;
            if (env->boss.health <= BLIZZARD_DAMAGE) {
                env->boss.health = 0;
                env->boss.alive = 0;
                env->episode_boss_kills++;
                add_experience(env, EXP_PER_BOSS_KILL);
            } else {
                env->boss.health -= damage_amount;
            }
            env->episode_damage_dealt += damage_amount;
        }
    }
    
    // Create the blizzard area for continued DOT
    for (uint32_t i = 0; i < MAX_BLIZZARD_AREAS; i++) {
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
    for (uint32_t i = 0; i < MAX_BLIZZARD_AREAS; i++) {
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
                            uint32_t damage_amount = (env->monsters[j].health <= BLIZZARD_DAMAGE) ? 
                                                   env->monsters[j].health : BLIZZARD_DAMAGE;
                            if (env->monsters[j].health <= BLIZZARD_DAMAGE) {
                                env->monsters[j].health = 0;
                                env->monsters[j].alive = 0;
                                env->monsters_killed++;
                                env->episode_monsters_killed++;
                                env->step_reward += env->config.monster_kill_reward;
                                env->episode_return += env->config.monster_kill_reward;
                                env->episode_monster_kill_rewards += env->config.monster_kill_reward;
                                add_experience(env, EXP_PER_MONSTER_KILL);
                                
                                // Give gold directly instead of dropping items
                                if ((float)rand() / RAND_MAX < env->config.item_drop_rate) {
                                    uint16_t gold_amount = GOLD_DROP_MIN + rand() % GOLD_DROP_RANGE;
                                    env->player.gold += gold_amount;
                                    env->episode_gold_earned += gold_amount;
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
                        uint32_t damage_amount = (env->boss.health <= BLIZZARD_DAMAGE) ? 
                                               env->boss.health : BLIZZARD_DAMAGE;
                        if (env->boss.health <= BLIZZARD_DAMAGE) {
                            env->boss.health = 0;
                            env->boss.alive = 0;
                            env->episode_boss_kills++;
                            add_experience(env, EXP_PER_BOSS_KILL);
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
    env->boss.health = (uint32_t)GetScaledMonsterHealth(BOSS_BASE_HEALTH, env->current_rift_level);
    env->boss.max_health = env->boss.health;
    env->boss.damage = (uint32_t)GetScaledMonsterDamage(BOSS_BASE_DAMAGE, env->current_rift_level);
    env->boss.alive = 1;
    env->boss.attack_cooldown = 0;
    env->boss.special_attack_cooldown = 0;
    env->boss_spawned = 1;
    
    for (uint16_t i = 0; i < MAX_MONSTERS; i++) {
        env->monsters[i].alive = 0;
    }
}



void spawn_projectile(Rift* env, float start_x, float start_y, float target_x, float target_y, uint32_t type, uint32_t damage) {
    for (uint32_t i = 0; i < MAX_PROJECTILES; i++) {
        if (!env->projectiles[i].active) {
            env->projectiles[i].x = start_x;
            env->projectiles[i].y = start_y;
            env->projectiles[i].type = type;
            env->projectiles[i].damage = damage;
            env->projectiles[i].lifetime = PROJECTILE_LIFETIME;
            env->projectiles[i].active = 1;
            env->projectiles[i].homing = 0;
            
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

void spawn_homing_projectile(Rift* env, float start_x, float start_y, float target_x, float target_y, uint32_t type, uint32_t damage) {
    for (uint32_t i = 0; i < MAX_PROJECTILES; i++) {
        if (!env->projectiles[i].active) {
            env->projectiles[i].x = start_x;
            env->projectiles[i].y = start_y;
            env->projectiles[i].target_x = target_x;
            env->projectiles[i].target_y = target_y;
            env->projectiles[i].type = type;
            env->projectiles[i].damage = damage;
            env->projectiles[i].lifetime = PROJECTILE_LIFETIME + 30; // Live longer for homing
            env->projectiles[i].active = 1;
            env->projectiles[i].homing = 1;
            
            // Start with initial velocity toward target
            float dx = target_x - start_x;
            float dy = target_y - start_y;
            float dist = sqrtf(dx * dx + dy * dy);
            
            if (dist > 0) {
                env->projectiles[i].vel_x = (dx / dist) * PROJECTILE_SPEED_HOMING;
                env->projectiles[i].vel_y = (dy / dist) * PROJECTILE_SPEED_HOMING;
            }
            break;
        }
    }
}

void spawn_fast_projectile(Rift* env, float start_x, float start_y, float target_x, float target_y, uint32_t type, uint32_t damage) {
    for (uint32_t i = 0; i < MAX_PROJECTILES; i++) {
        if (!env->projectiles[i].active) {
            env->projectiles[i].x = start_x;
            env->projectiles[i].y = start_y;
            env->projectiles[i].type = type;
            env->projectiles[i].damage = damage;
            env->projectiles[i].lifetime = PROJECTILE_LIFETIME;
            env->projectiles[i].active = 1;
            env->projectiles[i].homing = 0;
            
            float dx = target_x - start_x;
            float dy = target_y - start_y;
            float dist = sqrtf(dx * dx + dy * dy);
            
            if (dist > 0) {
                env->projectiles[i].vel_x = (dx / dist) * PROJECTILE_SPEED_FAST;
                env->projectiles[i].vel_y = (dy / dist) * PROJECTILE_SPEED_FAST;
            }
            break;
        }
    }
}

void execute_monster_attack(Rift* env, Monster* monster) {
    switch (monster->attack_type) {
        case ATTACK_TYPE_MELEE:
            monster->attack_animation_timer = 20; 
            
            if ((rand() % 100) < env->player.dodge_chance) {
            } else {
                if (monster->damage >= env->player.health) {
                    env->player.health = 0;
                    env->player.alive = 0;
                } else {
                    env->player.health -= monster->damage;
                }
            }
            break;
            
        case ATTACK_TYPE_PROJECTILE:
            spawn_projectile(env, monster->x, monster->y, env->player.x, env->player.y, 
                           PROJECTILE_FIREBALL, monster->damage);
            break;
            
        case ATTACK_TYPE_FAST_PROJECTILE:
            spawn_fast_projectile(env, monster->x, monster->y, env->player.x, env->player.y, 
                                PROJECTILE_FIREBALL, monster->damage);
            break;
            
        case ATTACK_TYPE_HOMING_PROJECTILE:
            spawn_homing_projectile(env, monster->x, monster->y, env->player.x, env->player.y, 
                                  PROJECTILE_FIREBALL, monster->damage);
            break;
            
        case ATTACK_TYPE_CONE_SLAM:
            monster->attack_animation_timer = 30; 
            
            float dx = env->player.x - monster->x;
            float dy = env->player.y - monster->y;
            float dist_to_player = sqrtf(dx * dx + dy * dy);
            
            if (dist_to_player <= monster->attack_range + 1.0f) {
                float angle_to_player = atan2f(dy, dx);
                float monster_facing = atan2f(env->player.y - monster->y, env->player.x - monster->x);
                float angle_diff = fabsf(angle_to_player - monster_facing);
                
                if (angle_diff <= PI/3 || angle_diff >= (2*PI - PI/3)) {
                    if ((rand() % 100) < env->player.dodge_chance) {
                    } else {
                        if (monster->damage >= env->player.health) {
                            env->player.health = 0;
                            env->player.alive = 0;
                        } else {
                            env->player.health -= monster->damage;
                        }
                    }
                }
            }
            break;
    }
}

void update_projectiles(Rift* env) {
    for (uint32_t i = 0; i < MAX_PROJECTILES; i++) {
        if (env->projectiles[i].active) {
            Projectile* proj = &env->projectiles[i];
            
            if (proj->homing && env->player.alive) {
                float dx = env->player.x - proj->x;
                float dy = env->player.y - proj->y;
                float dist = distance(proj->x, proj->y, env->player.x, env->player.y);
                
                if (dist > 0.1f) {
                    proj->vel_x = (dx / dist) * PROJECTILE_SPEED_HOMING;
                    proj->vel_y = (dy / dist) * PROJECTILE_SPEED_HOMING;
                }
            }
            
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
                if ((rand() % 100) < env->player.dodge_chance) {
                } else {
                    env->episode_damage_taken += proj->damage;
                    
                    if (proj->damage >= env->player.health) {
                        env->player.health = 0;
                        env->player.alive = 0;
                        env->step_reward += env->config.death_penalty;
                        env->episode_return += env->config.death_penalty;
                        env->episode_death_penalties += env->config.death_penalty;
                        env->episode_deaths++;
                    } else {
                        env->player.health -= proj->damage;
                    }
                }
                
                proj->active = 0;
            }
        }
    }
}



void handle_vendor_interaction(Rift* env) {
    float dist = distance(env->player.x, env->player.y, env->vendor.x, env->vendor.y);
    if (dist <= 2.0f) {
        for (int i = 0; i < 10; i++) {
            if (env->vendor.stock[i] > 0 && env->player.gold >= env->vendor.prices[i]) {
                for (uint32_t j = 0; j < INVENTORY_SLOTS; j++) {
                    if (env->player.inventory[j].item_type == EQUIPMENT_NONE) {
                        env->player.gold -= env->vendor.prices[i];
                        env->vendor.stock[i]--;
                        env->player.inventory[j].item_type = EQUIPMENT_CONSUMABLE;
                        env->player.inventory[j].item_quality = QUALITY_COMMON;
                        env->player.inventory[j].stack_size = 1;
                        env->player.inventory[j].item_value = env->vendor.prices[i];
                        memset(env->player.inventory[j].stat_bonuses, 0, sizeof(env->player.inventory[j].stat_bonuses));
                        env->player.inventory_count++;
                        env->episode_vendor_transactions++;
                        env->town_interface.current_visit_purchases++;
                        break;
                    }
                }
                break;
            }
        }
    }
}


void compute_observations(Rift* env) {
    uint32_t obs_idx = 0;
    
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
    
    float grid[GRID_OBS_SIZE];
    int player_grid_center_x = (int)env->player.x;
    int player_grid_center_y = (int)env->player.y;
    
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
    for (uint32_t i = 0; i < MAX_ITEMS; i++) {
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
    
    // Zero out grid observations in town mode to avoid confusing agent
    for (int i = 0; i < GRID_OBS_SIZE; i++) {
        env->observations[obs_idx++] = (env->current_phase == PHASE_TOWN) ? 0.0f : grid[i];
    }
    
    env->observations[obs_idx++] = (float)env->town_interface.frames_remaining / TOWN_MODE_TIME_LIMIT;
    env->observations[obs_idx++] = env->current_phase == PHASE_TOWN ? 1.0f : 0.0f;
    env->observations[obs_idx++] = (float)env->town_interface.warning_phase / 2.0f;
    
    env->observations[obs_idx++] = (float)env->hero_stats.level / MAX_HERO_LEVEL;
    env->observations[obs_idx++] = (float)env->hero_stats.experience / env->hero_stats.experience_to_next;
    env->observations[obs_idx++] = (float)env->hero_stats.stat_points_available / 10.0f;
    // FIXED SHOP SLOTS (10 slots × 4 properties = 40 observations)
    // Zero out shop observations in rift mode to avoid confusing agent
    for (uint32_t i = 0; i < 10; i++) {
        if (env->current_phase == PHASE_RIFT) {
            // Zero out all shop observations in rift mode
            env->observations[obs_idx++] = 0.0f; // not available
            env->observations[obs_idx++] = 0.0f; // no quality
            env->observations[obs_idx++] = 0.0f; // no price
            env->observations[obs_idx++] = 0.0f; // no stats
        } else if (i < SHOP_ITEMS_COUNT) {
            ShopItem* item = &env->shop_items[i];
            env->observations[obs_idx++] = item->available ? 1.0f : 0.0f;
            env->observations[obs_idx++] = (float)item->item_quality / 3.0f;
            env->observations[obs_idx++] = (float)item->price / 1000.0f;
            uint32_t total_stats = item->stat_bonuses[0] + item->stat_bonuses[1] + 
                                 item->stat_bonuses[2] + item->stat_bonuses[3];
            env->observations[obs_idx++] = (float)total_stats / 20.0f;
        } else {
            // Empty shop slot
            env->observations[obs_idx++] = 0.0f; // not available
            env->observations[obs_idx++] = 0.0f; // no quality
            env->observations[obs_idx++] = 0.0f; // no price
            env->observations[obs_idx++] = 0.0f; // no stats
        }
    }
    
    // FIXED EQUIPMENT SLOTS (13 slots × 4 properties = 52 observations)
    uint32_t equipment_types[] = {
        env->equipment.weapon_type, env->equipment.offhand_type, env->equipment.ring_left_type,
        env->equipment.ring_right_type, env->equipment.amulet_type, env->equipment.boots_type,
        env->equipment.gloves_type, env->equipment.helmet_type, env->equipment.shoulders_type,
        env->equipment.armor_type, env->equipment.belt_type, env->equipment.pants_type, env->equipment.bracers_type
    };
    uint32_t equipment_qualities[] = {
        env->equipment.weapon_quality, env->equipment.offhand_quality, env->equipment.ring_left_quality,
        env->equipment.ring_right_quality, env->equipment.amulet_quality, env->equipment.boots_quality,
        env->equipment.gloves_quality, env->equipment.helmet_quality, env->equipment.shoulders_quality,
        env->equipment.armor_quality, env->equipment.belt_quality, env->equipment.pants_quality, env->equipment.bracers_quality
    };
    uint32_t equipment_levels[] = {
        env->equipment.weapon_level, env->equipment.offhand_level, env->equipment.ring_left_level,
        env->equipment.ring_right_level, env->equipment.amulet_level, env->equipment.boots_level,
        env->equipment.gloves_level, env->equipment.helmet_level, env->equipment.shoulders_level,
        env->equipment.armor_level, env->equipment.belt_level, env->equipment.pants_level, env->equipment.bracers_level
    };
    
    for (uint32_t i = 0; i < 13; i++) {
        env->observations[obs_idx++] = equipment_types[i] > 0 ? 1.0f : 0.0f; // equipped or not
        env->observations[obs_idx++] = (float)equipment_qualities[i] / 3.0f;
        env->observations[obs_idx++] = (float)equipment_levels[i] / 20.0f;
        
        // Calculate total stats for this equipment piece
        uint32_t total_stats = 0;
        switch (i) {
            case 0: total_stats = env->equipment.weapon_stat_bonuses[0] + env->equipment.weapon_stat_bonuses[1] + 
                                 env->equipment.weapon_stat_bonuses[2] + env->equipment.weapon_stat_bonuses[3]; break;
            case 1: total_stats = env->equipment.offhand_stat_bonuses[0] + env->equipment.offhand_stat_bonuses[1] + 
                                 env->equipment.offhand_stat_bonuses[2] + env->equipment.offhand_stat_bonuses[3]; break;
            case 2: total_stats = env->equipment.ring_left_stat_bonuses[0] + env->equipment.ring_left_stat_bonuses[1] + 
                                 env->equipment.ring_left_stat_bonuses[2] + env->equipment.ring_left_stat_bonuses[3]; break;
            case 3: total_stats = env->equipment.ring_right_stat_bonuses[0] + env->equipment.ring_right_stat_bonuses[1] + 
                                 env->equipment.ring_right_stat_bonuses[2] + env->equipment.ring_right_stat_bonuses[3]; break;
            case 4: total_stats = env->equipment.amulet_stat_bonuses[0] + env->equipment.amulet_stat_bonuses[1] + 
                                 env->equipment.amulet_stat_bonuses[2] + env->equipment.amulet_stat_bonuses[3]; break;
            case 5: total_stats = env->equipment.boots_stat_bonuses[0] + env->equipment.boots_stat_bonuses[1] + 
                                 env->equipment.boots_stat_bonuses[2] + env->equipment.boots_stat_bonuses[3]; break;
            case 6: total_stats = env->equipment.gloves_stat_bonuses[0] + env->equipment.gloves_stat_bonuses[1] + 
                                 env->equipment.gloves_stat_bonuses[2] + env->equipment.gloves_stat_bonuses[3]; break;
            case 7: total_stats = env->equipment.helmet_stat_bonuses[0] + env->equipment.helmet_stat_bonuses[1] + 
                                 env->equipment.helmet_stat_bonuses[2] + env->equipment.helmet_stat_bonuses[3]; break;
            case 8: total_stats = env->equipment.shoulders_stat_bonuses[0] + env->equipment.shoulders_stat_bonuses[1] + 
                                 env->equipment.shoulders_stat_bonuses[2] + env->equipment.shoulders_stat_bonuses[3]; break;
            case 9: total_stats = env->equipment.armor_stat_bonuses[0] + env->equipment.armor_stat_bonuses[1] + 
                                 env->equipment.armor_stat_bonuses[2] + env->equipment.armor_stat_bonuses[3]; break;
            case 10: total_stats = env->equipment.belt_stat_bonuses[0] + env->equipment.belt_stat_bonuses[1] + 
                                  env->equipment.belt_stat_bonuses[2] + env->equipment.belt_stat_bonuses[3]; break;
            case 11: total_stats = env->equipment.pants_stat_bonuses[0] + env->equipment.pants_stat_bonuses[1] + 
                                  env->equipment.pants_stat_bonuses[2] + env->equipment.pants_stat_bonuses[3]; break;
            case 12: total_stats = env->equipment.bracers_stat_bonuses[0] + env->equipment.bracers_stat_bonuses[1] + 
                                  env->equipment.bracers_stat_bonuses[2] + env->equipment.bracers_stat_bonuses[3]; break;
        }
        env->observations[obs_idx++] = (float)total_stats / 20.0f;
    }
    
    // TOWN INTERFACE DATA (8 observations)  
    env->observations[obs_idx++] = (float)env->hero_stats.total_strength / 50.0f;
    env->observations[obs_idx++] = (float)env->hero_stats.total_dexterity / 50.0f;
    env->observations[obs_idx++] = (float)env->hero_stats.total_intelligence / 50.0f;
    env->observations[obs_idx++] = (float)env->hero_stats.total_vitality / 50.0f;
    env->observations[obs_idx++] = (float)env->current_rift_level / 20.0f;
    
    uint32_t inventory_items = 0;
    for (uint32_t i = 0; i < INVENTORY_SLOTS; i++) {
        if (env->player.inventory[i].item_type != EQUIPMENT_NONE) {
            inventory_items++;
        }
    }
    env->observations[obs_idx++] = (float)inventory_items / INVENTORY_SLOTS;
    env->observations[obs_idx++] = (float)env->player.selected_inventory_slot / INVENTORY_SLOTS;
    env->observations[obs_idx++] = (float)env->town_interface.current_tab / 1.0f; // Shop vs Character tab
}

// ============================================================================
// LOGGING FUNCTIONS
// ============================================================================

static void add_log(Rift* env) {
    env->log.episode_return += env->episode_return;
    env->log.episode_length += env->episode_length;
    env->log.monsters_killed += env->episode_monsters_killed;
    env->log.boss_kills += env->episode_boss_kills;
    env->log.gold_earned += env->episode_gold_earned;
    env->log.damage_dealt += env->episode_damage_dealt;
    env->log.damage_taken += env->episode_damage_taken;
    env->log.rift_completions += env->episode_rift_completions;
    env->log.vendor_transactions += env->episode_vendor_transactions;
    env->log.blizzards_cast += env->episode_blizzards_cast;
    env->log.deaths += env->episode_deaths;
    env->log.completion_rewards += env->episode_completion_rewards;
    env->log.monster_kill_rewards += env->episode_monster_kill_rewards;
    env->log.death_penalties += env->episode_death_penalties;
    env->log.hero_level += env->hero_stats.level;
    env->log.total_experience += env->hero_stats.experience;
    env->log.shop_purchases += env->episode_shop_purchases;
    env->log.town_time_efficiency += (float)env->episode_town_time_used / TOWN_MODE_TIME_LIMIT;
    env->log.no_purchase_penalties += env->episode_no_purchase_penalties;
    
    env->log.current_gold = env->player.gold;
    env->log.total_strength = env->hero_stats.total_strength;
    env->log.total_dexterity = env->hero_stats.total_dexterity;
    env->log.total_intelligence = env->hero_stats.total_intelligence;
    env->log.total_vitality = env->hero_stats.total_vitality;
    env->log.average_item_level = env->hero_stats.average_item_level;
    
    env->log.n += 1;
}

// ============================================================================
// RIFT PROGRESSION SCALING FUNCTIONS
// ============================================================================

float GetScaledMonsterHealth(float base_health, uint32_t rift_level) {
    return base_health * powf(RIFT_MONSTER_HEALTH_BASE_MULTIPLIER, rift_level - 1);
}

float GetScaledMonsterDamage(float base_damage, uint32_t rift_level) {
    return base_damage * powf(RIFT_MONSTER_DAMAGE_BASE_MULTIPLIER, rift_level - 1);
}

float GetScaledMonsterSpeed(float base_speed, uint32_t rift_level) {
    return base_speed * powf(RIFT_MONSTER_SPEED_BASE_MULTIPLIER, rift_level - 1);
}

uint32_t GetScaledAttackCooldown(uint32_t base_cooldown, uint32_t rift_level) {
    float scaled_cooldown = base_cooldown * powf(RIFT_MONSTER_ATTACK_SPEED_MULTIPLIER, rift_level - 1);
    return (uint32_t)(scaled_cooldown < 5 ? 5 : scaled_cooldown); // Minimum 5 frames between attacks
}

int GetRiftCompletionGold(uint32_t rift_level) {
    return (int)(RIFT_GOLD_REWARD_BASE * powf(RIFT_GOLD_SCALING_MULTIPLIER, rift_level - 1));
}

void GetQualityChances(uint32_t rift_level, int* common, int* rare, int* epic, int* legendary) {
    if (rift_level <= RIFT_TIER_1_MAX) {
        *common = TIER_1_COMMON_CHANCE;
        *rare = TIER_1_RARE_CHANCE;
        *epic = TIER_1_EPIC_CHANCE;
        *legendary = TIER_1_LEGENDARY_CHANCE;
    } else if (rift_level <= RIFT_TIER_2_MAX) {
        *common = TIER_2_COMMON_CHANCE;
        *rare = TIER_2_RARE_CHANCE;
        *epic = TIER_2_EPIC_CHANCE;
        *legendary = TIER_2_LEGENDARY_CHANCE;
    } else if (rift_level <= RIFT_TIER_3_MAX) {
        *common = TIER_3_COMMON_CHANCE;
        *rare = TIER_3_RARE_CHANCE;
        *epic = TIER_3_EPIC_CHANCE;
        *legendary = TIER_3_LEGENDARY_CHANCE;
    } else {
        *common = ENDGAME_COMMON_CHANCE;
        *rare = ENDGAME_RARE_CHANCE;
        *epic = ENDGAME_EPIC_CHANCE;
        *legendary = ENDGAME_LEGENDARY_CHANCE;
    }
}

int GetScaledItemLevel(uint32_t rift_level) {
    int base_ilvl = (int)(rift_level * RIFT_ILVL_BASE_MULTIPLIER);
    int variation = (rand() % (2 * RIFT_ILVL_RANDOM_RANGE + 1)) - RIFT_ILVL_RANDOM_RANGE;
    return base_ilvl + variation;
}

int GetScaledItemPrice(int base_price, uint32_t rift_level, uint32_t quality) {
    float quality_multipliers[] = QUALITY_PRICE_MULTIPLIERS;
    float rift_multiplier = powf(RIFT_PRICE_BASE_MULTIPLIER, rift_level - 1);
    return (int)(base_price * rift_multiplier * quality_multipliers[quality]);
}

// ============================================================================
// TOWN MODE FUNCTIONS
// ============================================================================

void init_hero_stats(Rift* env) {
    env->hero_stats.level = 1;
    env->hero_stats.experience = 0;
    env->hero_stats.experience_to_next = 100;
    env->hero_stats.stat_points_available = 0;
    
    env->hero_stats.strength = 10;
    env->hero_stats.dexterity = 10;
    env->hero_stats.intelligence = 10;
    env->hero_stats.vitality = 10;
    
    // EPIC Starting Equipment - Give hero basic ilvl 1 gear!
    env->equipment.weapon_type = EQUIPMENT_WEAPON;
    env->equipment.weapon_quality = QUALITY_COMMON;
    env->equipment.weapon_level = 1;
    env->equipment.weapon_stat_bonuses[0] = 2; // +2 STR
    env->equipment.weapon_stat_bonuses[1] = 0;
    env->equipment.weapon_stat_bonuses[2] = 0;
    env->equipment.weapon_stat_bonuses[3] = 0;
    
    env->equipment.offhand_type = EQUIPMENT_OFFHAND;
    env->equipment.offhand_quality = QUALITY_COMMON;
    env->equipment.offhand_level = 1;
    env->equipment.offhand_stat_bonuses[0] = 0;
    env->equipment.offhand_stat_bonuses[1] = 0;
    env->equipment.offhand_stat_bonuses[2] = 0;
    env->equipment.offhand_stat_bonuses[3] = 1; // +1 VIT
    
    env->equipment.ring_left_type = EQUIPMENT_RING_LEFT;
    env->equipment.ring_left_quality = QUALITY_COMMON;
    env->equipment.ring_left_level = 1;
    env->equipment.ring_left_stat_bonuses[0] = 1; // +1 STR
    env->equipment.ring_left_stat_bonuses[1] = 0;
    env->equipment.ring_left_stat_bonuses[2] = 0;
    env->equipment.ring_left_stat_bonuses[3] = 0;
    
    env->equipment.ring_right_type = EQUIPMENT_RING_RIGHT;
    env->equipment.ring_right_quality = QUALITY_COMMON;
    env->equipment.ring_right_level = 1;
    env->equipment.ring_right_stat_bonuses[0] = 0;
    env->equipment.ring_right_stat_bonuses[1] = 0;
    env->equipment.ring_right_stat_bonuses[2] = 1; // +1 INT
    env->equipment.ring_right_stat_bonuses[3] = 0;
    
    env->equipment.amulet_type = EQUIPMENT_AMULET;
    env->equipment.amulet_quality = QUALITY_COMMON;
    env->equipment.amulet_level = 1;
    env->equipment.amulet_stat_bonuses[0] = 0;
    env->equipment.amulet_stat_bonuses[1] = 1; // +1 DEX
    env->equipment.amulet_stat_bonuses[2] = 0;
    env->equipment.amulet_stat_bonuses[3] = 0;
    
    env->equipment.boots_type = EQUIPMENT_BOOTS;
    env->equipment.boots_quality = QUALITY_COMMON;
    env->equipment.boots_level = 1;
    env->equipment.boots_stat_bonuses[0] = 0;
    env->equipment.boots_stat_bonuses[1] = 0;
    env->equipment.boots_stat_bonuses[2] = 0;
    env->equipment.boots_stat_bonuses[3] = 1; // +1 VIT
    
    env->equipment.gloves_type = EQUIPMENT_GLOVES;
    env->equipment.gloves_quality = QUALITY_COMMON;
    env->equipment.gloves_level = 1;
    env->equipment.gloves_stat_bonuses[0] = 0;
    env->equipment.gloves_stat_bonuses[1] = 1; // +1 DEX
    env->equipment.gloves_stat_bonuses[2] = 0;
    env->equipment.gloves_stat_bonuses[3] = 0;
    
    env->equipment.helmet_type = EQUIPMENT_HELMET;
    env->equipment.helmet_quality = QUALITY_COMMON;
    env->equipment.helmet_level = 1;
    env->equipment.helmet_stat_bonuses[0] = 1; // +1 STR
    env->equipment.helmet_stat_bonuses[1] = 0;
    env->equipment.helmet_stat_bonuses[2] = 0;
    env->equipment.helmet_stat_bonuses[3] = 0;
    
    env->equipment.shoulders_type = EQUIPMENT_SHOULDERS;
    env->equipment.shoulders_quality = QUALITY_COMMON;
    env->equipment.shoulders_level = 1;
    env->equipment.shoulders_stat_bonuses[0] = 1; // +1 STR
    env->equipment.shoulders_stat_bonuses[1] = 0;
    env->equipment.shoulders_stat_bonuses[2] = 0;
    env->equipment.shoulders_stat_bonuses[3] = 0;
    
    env->equipment.armor_type = EQUIPMENT_ARMOR;
    env->equipment.armor_quality = QUALITY_COMMON;
    env->equipment.armor_level = 1;
    env->equipment.armor_stat_bonuses[0] = 0;
    env->equipment.armor_stat_bonuses[1] = 0;
    env->equipment.armor_stat_bonuses[2] = 0;
    env->equipment.armor_stat_bonuses[3] = 2; // +2 VIT
    
    env->equipment.belt_type = EQUIPMENT_BELT;
    env->equipment.belt_quality = QUALITY_COMMON;
    env->equipment.belt_level = 1;
    env->equipment.belt_stat_bonuses[0] = 0;
    env->equipment.belt_stat_bonuses[1] = 0;
    env->equipment.belt_stat_bonuses[2] = 0;
    env->equipment.belt_stat_bonuses[3] = 1; // +1 VIT
    
    env->equipment.pants_type = EQUIPMENT_PANTS;
    env->equipment.pants_quality = QUALITY_COMMON;
    env->equipment.pants_level = 1;
    env->equipment.pants_stat_bonuses[0] = 0;
    env->equipment.pants_stat_bonuses[1] = 1; // +1 DEX
    env->equipment.pants_stat_bonuses[2] = 0;
    env->equipment.pants_stat_bonuses[3] = 0;
    
    env->equipment.bracers_type = EQUIPMENT_BRACERS;
    env->equipment.bracers_quality = QUALITY_COMMON;
    env->equipment.bracers_level = 1;
    env->equipment.bracers_stat_bonuses[0] = 0;
    env->equipment.bracers_stat_bonuses[1] = 1; // +1 DEX
    env->equipment.bracers_stat_bonuses[2] = 0;
    env->equipment.bracers_stat_bonuses[3] = 0;
    
    calculate_total_stats(env);
    
    memset(env->shop_items, 0, sizeof(env->shop_items));
    
    env->town_interface.frames_remaining = TOWN_MODE_TIME_LIMIT;
    env->town_interface.warning_phase = 0;
    env->town_interface.current_tab = TOWN_TAB_CHARACTER;
    env->town_interface.character_mode = CHARACTER_MODE_EQUIPMENT;
    env->town_interface.selected_item_index = 0;
    env->town_interface.equipment_slot = 0;
    env->town_interface.input_cooldown = 0;
    env->town_interface.shop_scroll_offset = 0;
    env->town_interface.current_visit_purchases = 0;
}

void give_stat_based_rewards(Rift* env, uint32_t old_str, uint32_t old_dex, uint32_t old_int, uint32_t old_vit, uint32_t old_total_ilvl) {
    // Calculate stat differences
    int str_diff = env->hero_stats.total_strength - old_str;
    int dex_diff = env->hero_stats.total_dexterity - old_dex;  
    int int_diff = env->hero_stats.total_intelligence - old_int;
    int vit_diff = env->hero_stats.total_vitality - old_vit;
    
    // Calculate total ilvl from all equipment
    uint32_t current_total_ilvl = env->equipment.weapon_level + env->equipment.offhand_level + 
                                env->equipment.ring_left_level + env->equipment.ring_right_level +
                                env->equipment.amulet_level + env->equipment.boots_level +
                                env->equipment.gloves_level + env->equipment.helmet_level +
                                env->equipment.shoulders_level + env->equipment.armor_level +
                                env->equipment.belt_level + env->equipment.pants_level + env->equipment.bracers_level;
    
    int ilvl_diff = current_total_ilvl - old_total_ilvl;
    
    // Give rewards for stat increases, penalties for decreases
    float stat_reward = (str_diff + dex_diff + int_diff + vit_diff) * 
                       (str_diff + dex_diff + int_diff + vit_diff >= 0 ? TOWN_STAT_INCREASE_REWARD : TOWN_STAT_DECREASE_PENALTY);
    
    // Give rewards for ilvl increases, penalties for decreases  
    float ilvl_reward = ilvl_diff * (ilvl_diff >= 0 ? TOWN_ILVL_INCREASE_REWARD : TOWN_ILVL_DECREASE_PENALTY);
    
    env->step_reward += stat_reward + ilvl_reward;
    env->episode_return += stat_reward + ilvl_reward;
}

void calculate_total_stats(Rift* env) {
    env->hero_stats.total_strength = env->hero_stats.strength + 
                                    env->equipment.weapon_stat_bonuses[0] + 
                                    env->equipment.offhand_stat_bonuses[0] + 
                                    env->equipment.ring_left_stat_bonuses[0] + 
                                    env->equipment.ring_right_stat_bonuses[0] + 
                                    env->equipment.amulet_stat_bonuses[0] + 
                                    env->equipment.boots_stat_bonuses[0] + 
                                    env->equipment.gloves_stat_bonuses[0] + 
                                    env->equipment.helmet_stat_bonuses[0] + 
                                    env->equipment.shoulders_stat_bonuses[0] + 
                                    env->equipment.armor_stat_bonuses[0] + 
                                    env->equipment.belt_stat_bonuses[0] + 
                                    env->equipment.pants_stat_bonuses[0] + 
                                    env->equipment.bracers_stat_bonuses[0];
    
    env->hero_stats.total_dexterity = env->hero_stats.dexterity + 
                                     env->equipment.weapon_stat_bonuses[1] + 
                                     env->equipment.offhand_stat_bonuses[1] + 
                                     env->equipment.ring_left_stat_bonuses[1] + 
                                     env->equipment.ring_right_stat_bonuses[1] + 
                                     env->equipment.amulet_stat_bonuses[1] + 
                                     env->equipment.boots_stat_bonuses[1] + 
                                     env->equipment.gloves_stat_bonuses[1] + 
                                     env->equipment.helmet_stat_bonuses[1] + 
                                     env->equipment.shoulders_stat_bonuses[1] + 
                                     env->equipment.armor_stat_bonuses[1] + 
                                     env->equipment.belt_stat_bonuses[1] + 
                                     env->equipment.pants_stat_bonuses[1] + 
                                     env->equipment.bracers_stat_bonuses[1];
    
    env->hero_stats.total_intelligence = env->hero_stats.intelligence + 
                                        env->equipment.weapon_stat_bonuses[2] + 
                                        env->equipment.offhand_stat_bonuses[2] + 
                                        env->equipment.ring_left_stat_bonuses[2] + 
                                        env->equipment.ring_right_stat_bonuses[2] + 
                                        env->equipment.amulet_stat_bonuses[2] + 
                                        env->equipment.boots_stat_bonuses[2] + 
                                        env->equipment.gloves_stat_bonuses[2] + 
                                        env->equipment.helmet_stat_bonuses[2] + 
                                        env->equipment.shoulders_stat_bonuses[2] + 
                                        env->equipment.armor_stat_bonuses[2] + 
                                        env->equipment.belt_stat_bonuses[2] + 
                                        env->equipment.pants_stat_bonuses[2] + 
                                        env->equipment.bracers_stat_bonuses[2];
    
    env->hero_stats.total_vitality = env->hero_stats.vitality + 
                                    env->equipment.weapon_stat_bonuses[3] + 
                                    env->equipment.offhand_stat_bonuses[3] + 
                                    env->equipment.ring_left_stat_bonuses[3] + 
                                    env->equipment.ring_right_stat_bonuses[3] + 
                                    env->equipment.amulet_stat_bonuses[3] + 
                                    env->equipment.boots_stat_bonuses[3] + 
                                    env->equipment.gloves_stat_bonuses[3] + 
                                    env->equipment.helmet_stat_bonuses[3] + 
                                    env->equipment.shoulders_stat_bonuses[3] + 
                                    env->equipment.armor_stat_bonuses[3] + 
                                    env->equipment.belt_stat_bonuses[3] + 
                                    env->equipment.pants_stat_bonuses[3] + 
                                    env->equipment.bracers_stat_bonuses[3];
    
    uint32_t total_ilvl = env->equipment.weapon_level + env->equipment.offhand_level + 
                         env->equipment.ring_left_level + env->equipment.ring_right_level +
                         env->equipment.amulet_level + env->equipment.boots_level +
                         env->equipment.gloves_level + env->equipment.helmet_level +
                         env->equipment.shoulders_level + env->equipment.armor_level +
                         env->equipment.belt_level + env->equipment.pants_level +
                         env->equipment.bracers_level;
    
    uint32_t equipped_items = 0;
    if (env->equipment.weapon_type != EQUIPMENT_NONE) equipped_items++;
    if (env->equipment.offhand_type != EQUIPMENT_NONE) equipped_items++;
    if (env->equipment.ring_left_type != EQUIPMENT_NONE) equipped_items++;
    if (env->equipment.ring_right_type != EQUIPMENT_NONE) equipped_items++;
    if (env->equipment.amulet_type != EQUIPMENT_NONE) equipped_items++;
    if (env->equipment.boots_type != EQUIPMENT_NONE) equipped_items++;
    if (env->equipment.gloves_type != EQUIPMENT_NONE) equipped_items++;
    if (env->equipment.helmet_type != EQUIPMENT_NONE) equipped_items++;
    if (env->equipment.shoulders_type != EQUIPMENT_NONE) equipped_items++;
    if (env->equipment.armor_type != EQUIPMENT_NONE) equipped_items++;
    if (env->equipment.belt_type != EQUIPMENT_NONE) equipped_items++;
    if (env->equipment.pants_type != EQUIPMENT_NONE) equipped_items++;
    if (env->equipment.bracers_type != EQUIPMENT_NONE) equipped_items++;
    
    env->hero_stats.average_item_level = equipped_items > 0 ? (total_ilvl + equipped_items - 1) / equipped_items : 0; // Round up
    
    env->player.max_health = PLAYER_MAX_HEALTH + (env->hero_stats.total_vitality * 5);
    env->player.max_mana = PLAYER_MAX_MANA + (env->hero_stats.total_intelligence * 2);
    env->player.damage = PLAYER_BASE_DAMAGE + (env->hero_stats.total_strength * 2) + (env->hero_stats.total_intelligence * 1); // Int adds damage too
    env->player.dodge_chance = env->hero_stats.total_dexterity; // 1% dodge per dex point, max 50%
    if (env->player.dodge_chance > 50) env->player.dodge_chance = 50;
}

void add_experience(Rift* env, uint16_t exp) {
    env->hero_stats.experience += exp;
    
    while (env->hero_stats.experience >= env->hero_stats.experience_to_next && 
           env->hero_stats.level < MAX_HERO_LEVEL) {
        level_up_hero(env);
    }
}

void level_up_hero(Rift* env) {
    env->hero_stats.experience -= env->hero_stats.experience_to_next;
    env->hero_stats.level++;
    env->hero_stats.stat_points_available += STAT_POINTS_PER_LEVEL;
    env->hero_stats.experience_to_next = env->hero_stats.level * 50 + 50;
}

void generate_shop_inventory(Rift* env) {
    uint32_t rift_level = env->current_rift_level;
    
    for (uint32_t i = 0; i < SHOP_ITEMS_COUNT; i++) {
        ShopItem* item = &env->shop_items[i];
        
        // Generate all equipment types (1-13, excluding EQUIPMENT_NONE=0 and EQUIPMENT_CONSUMABLE=14)
        item->item_type = (rand() % 13) + 1; // All equipment types from SHOULDERS(1) to OFFHAND(13)
        
        // RIFT-BASED Quality Generation with tier progression
        int common_chance, rare_chance, epic_chance, legendary_chance;
        GetQualityChances(rift_level, &common_chance, &rare_chance, &epic_chance, &legendary_chance);
        
        uint32_t quality_roll = rand() % 100;
        if (quality_roll < legendary_chance) {
            item->item_quality = QUALITY_LEGENDARY;
        } else if (quality_roll < legendary_chance + epic_chance) {
            item->item_quality = QUALITY_EPIC;
        } else if (quality_roll < legendary_chance + epic_chance + rare_chance) {
            item->item_quality = QUALITY_RARE;
        } else {
            item->item_quality = QUALITY_COMMON;
        }
        
        item->item_level = GetScaledItemLevel(rift_level);
        if (item->item_level < 1) item->item_level = 1;
        if (item->item_level > 50) item->item_level = 50; 
        
        uint32_t stat_multiplier;
        switch(item->item_quality) {
            case QUALITY_COMMON: stat_multiplier = 1; break;
            case QUALITY_RARE: stat_multiplier = 2; break;
            case QUALITY_EPIC: stat_multiplier = 3; break;
            case QUALITY_LEGENDARY: stat_multiplier = 5; break;
            default: stat_multiplier = 1; break;
        }
        uint32_t base_stat_budget = item->item_level * stat_multiplier;
        
        for (uint32_t j = 0; j < 4; j++) {
            item->stat_bonuses[j] = 0;
        }
        
        if (base_stat_budget < 1) base_stat_budget = 1;
        
        uint32_t remaining_budget = base_stat_budget;
        
        while (remaining_budget > 0) {
            uint32_t stat_index = rand() % 4;
            uint32_t allocation_amount = 1 + (rand() % 3);
            if (allocation_amount > remaining_budget) allocation_amount = remaining_budget;
            
            item->stat_bonuses[stat_index] += allocation_amount;
            remaining_budget -= allocation_amount;
        }
        
        uint16_t base_price = 50 + (item->item_level * 10);
        uint32_t total_stats = item->stat_bonuses[0] + item->stat_bonuses[1] + 
                             item->stat_bonuses[2] + item->stat_bonuses[3];
        
        item->price = GetScaledItemPrice(base_price, rift_level, item->item_quality) + (total_stats * 5);
        
        item->available = 1;
    }
}

uint32_t is_at_shop_item(Rift* env, uint32_t shop_slot) {
    if (shop_slot >= SHOP_ITEMS_COUNT) return 0;
    
    float shop_positions[SHOP_ITEMS_COUNT][2] = {
        {22, 17}, {28, 17}, {22, 21}, {28, 21}
    };
    
    float dist = distance(env->player.x, env->player.y, 
                         shop_positions[shop_slot][0], shop_positions[shop_slot][1]);
    return dist <= 1.5f;
}

uint32_t is_at_inventory_area(Rift* env) {
    float dist = distance(env->player.x, env->player.y, INVENTORY_AREA_X, INVENTORY_AREA_Y);
    return dist <= 3.0f;
}

uint32_t is_at_vendor(Rift* env) {
    float dist = distance(env->player.x, env->player.y, VENDOR_POSITION_X, VENDOR_POSITION_Y);
    return dist <= 2.0f;
}

uint32_t is_at_rift_portal(Rift* env) {
    float dist = distance(env->player.x, env->player.y, RIFT_PORTAL_X, RIFT_PORTAL_Y);
    return dist <= 2.0f;
}

void buy_shop_item(Rift* env, uint32_t shop_slot) {
    if (shop_slot >= SHOP_ITEMS_COUNT) {
        return;
    }
    
    if (!env->shop_items[shop_slot].available) {
        return;
    }
    
    if (env->player.gold < env->shop_items[shop_slot].price) {
        return;
    }
    
    uint32_t old_str = env->hero_stats.total_strength;
    uint32_t old_dex = env->hero_stats.total_dexterity; 
    uint32_t old_int = env->hero_stats.total_intelligence;
    uint32_t old_vit = env->hero_stats.total_vitality;
    
    uint32_t old_total_ilvl = env->equipment.weapon_level + env->equipment.offhand_level + 
                            env->equipment.ring_left_level + env->equipment.ring_right_level +
                            env->equipment.amulet_level + env->equipment.boots_level +
                            env->equipment.gloves_level + env->equipment.helmet_level +
                            env->equipment.shoulders_level + env->equipment.armor_level +
                            env->equipment.belt_level + env->equipment.pants_level + env->equipment.bracers_level;
    
    ShopItem* shop_item = &env->shop_items[shop_slot];
    
    
    switch (shop_item->item_type) {
        case EQUIPMENT_WEAPON:
            env->equipment.weapon_type = shop_item->item_type;
            env->equipment.weapon_quality = shop_item->item_quality;
            env->equipment.weapon_level = shop_item->item_level;
            for (uint32_t j = 0; j < 4; j++) {
                env->equipment.weapon_stat_bonuses[j] = shop_item->stat_bonuses[j];
            }
            break;
        case EQUIPMENT_OFFHAND:
            env->equipment.offhand_type = shop_item->item_type;
            env->equipment.offhand_quality = shop_item->item_quality;
            env->equipment.offhand_level = shop_item->item_level;
            for (uint32_t j = 0; j < 4; j++) {
                env->equipment.offhand_stat_bonuses[j] = shop_item->stat_bonuses[j];
            }
            break;
        case EQUIPMENT_RING_LEFT:
            env->equipment.ring_left_type = shop_item->item_type;
            env->equipment.ring_left_quality = shop_item->item_quality;
            env->equipment.ring_left_level = shop_item->item_level;
            for (uint32_t j = 0; j < 4; j++) {
                env->equipment.ring_left_stat_bonuses[j] = shop_item->stat_bonuses[j];
            }
            break;
        case EQUIPMENT_RING_RIGHT:
            env->equipment.ring_right_type = shop_item->item_type;
            env->equipment.ring_right_quality = shop_item->item_quality;
            env->equipment.ring_right_level = shop_item->item_level;
            for (uint32_t j = 0; j < 4; j++) {
                env->equipment.ring_right_stat_bonuses[j] = shop_item->stat_bonuses[j];
            }
            break;
        case EQUIPMENT_AMULET:
            env->equipment.amulet_type = shop_item->item_type;
            env->equipment.amulet_quality = shop_item->item_quality;
            env->equipment.amulet_level = shop_item->item_level;
            for (uint32_t j = 0; j < 4; j++) {
                env->equipment.amulet_stat_bonuses[j] = shop_item->stat_bonuses[j];
            }
            break;
        case EQUIPMENT_BOOTS:
            env->equipment.boots_type = shop_item->item_type;
            env->equipment.boots_quality = shop_item->item_quality;
            env->equipment.boots_level = shop_item->item_level;
            for (uint32_t j = 0; j < 4; j++) {
                env->equipment.boots_stat_bonuses[j] = shop_item->stat_bonuses[j];
            }
            break;
        case EQUIPMENT_GLOVES:
            env->equipment.gloves_type = shop_item->item_type;
            env->equipment.gloves_quality = shop_item->item_quality;
            env->equipment.gloves_level = shop_item->item_level;
            for (uint32_t j = 0; j < 4; j++) {
                env->equipment.gloves_stat_bonuses[j] = shop_item->stat_bonuses[j];
            }
            break;
        case EQUIPMENT_HELMET:
            env->equipment.helmet_type = shop_item->item_type;
            env->equipment.helmet_quality = shop_item->item_quality;
            env->equipment.helmet_level = shop_item->item_level;
            for (uint32_t j = 0; j < 4; j++) {
                env->equipment.helmet_stat_bonuses[j] = shop_item->stat_bonuses[j];
            }
            break;
        case EQUIPMENT_SHOULDERS:
            env->equipment.shoulders_type = shop_item->item_type;
            env->equipment.shoulders_quality = shop_item->item_quality;
            env->equipment.shoulders_level = shop_item->item_level;
            for (uint32_t j = 0; j < 4; j++) {
                env->equipment.shoulders_stat_bonuses[j] = shop_item->stat_bonuses[j];
            }
            break;
        case EQUIPMENT_ARMOR:
            env->equipment.armor_type = shop_item->item_type;
            env->equipment.armor_quality = shop_item->item_quality;
            env->equipment.armor_level = shop_item->item_level;
            for (uint32_t j = 0; j < 4; j++) {
                env->equipment.armor_stat_bonuses[j] = shop_item->stat_bonuses[j];
            }
            break;
        case EQUIPMENT_BELT:
            env->equipment.belt_type = shop_item->item_type;
            env->equipment.belt_quality = shop_item->item_quality;
            env->equipment.belt_level = shop_item->item_level;
            for (uint32_t j = 0; j < 4; j++) {
                env->equipment.belt_stat_bonuses[j] = shop_item->stat_bonuses[j];
            }
            break;
        case EQUIPMENT_PANTS:
            env->equipment.pants_type = shop_item->item_type;
            env->equipment.pants_quality = shop_item->item_quality;
            env->equipment.pants_level = shop_item->item_level;
            for (uint32_t j = 0; j < 4; j++) {
                env->equipment.pants_stat_bonuses[j] = shop_item->stat_bonuses[j];
            }
            break;
        case EQUIPMENT_BRACERS:
            env->equipment.bracers_type = shop_item->item_type;
            env->equipment.bracers_quality = shop_item->item_quality;
            env->equipment.bracers_level = shop_item->item_level;
            for (uint32_t j = 0; j < 4; j++) {
                env->equipment.bracers_stat_bonuses[j] = shop_item->stat_bonuses[j];
            }
            break;
        default:
            break;
    }
    
    env->player.gold -= shop_item->price;
    shop_item->available = 0;
    
    calculate_total_stats(env);
    
    give_stat_based_rewards(env, old_str, old_dex, old_int, old_vit, old_total_ilvl);
    
    env->episode_vendor_transactions++;
    env->episode_shop_purchases++;
    env->town_interface.current_visit_purchases++;
    
}

void equip_item_from_inventory(Rift* env, uint32_t slot) {
    if (slot >= INVENTORY_SLOTS) return;
    if (env->player.inventory[slot].item_type == EQUIPMENT_NONE) return;
    
    InventorySlot* item = &env->player.inventory[slot];
    
    if (item->item_type == EQUIPMENT_WEAPON) {
        env->equipment.weapon_type = item->item_type;
        env->equipment.weapon_quality = item->item_quality;
        env->equipment.weapon_level = item->item_level;
        for (uint32_t i = 0; i < 4; i++) {
            env->equipment.weapon_stat_bonuses[i] = item->stat_bonuses[i];
        }
    } else if (item->item_type == EQUIPMENT_OFFHAND) {
        env->equipment.offhand_type = item->item_type;
        env->equipment.offhand_quality = item->item_quality;
        env->equipment.offhand_level = item->item_level;
        for (uint32_t i = 0; i < 4; i++) {
            env->equipment.offhand_stat_bonuses[i] = item->stat_bonuses[i];
        }
    } else if (item->item_type == EQUIPMENT_RING_LEFT) {
        env->equipment.ring_left_type = item->item_type;
        env->equipment.ring_left_quality = item->item_quality;
        env->equipment.ring_left_level = item->item_level;
        for (uint32_t i = 0; i < 4; i++) {
            env->equipment.ring_left_stat_bonuses[i] = item->stat_bonuses[i];
        }
    } else if (item->item_type == EQUIPMENT_RING_RIGHT) {
        env->equipment.ring_right_type = item->item_type;
        env->equipment.ring_right_quality = item->item_quality;
        env->equipment.ring_right_level = item->item_level;
        for (uint32_t i = 0; i < 4; i++) {
            env->equipment.ring_right_stat_bonuses[i] = item->stat_bonuses[i];
        }
    } else if (item->item_type == EQUIPMENT_AMULET) {
        env->equipment.amulet_type = item->item_type;
        env->equipment.amulet_quality = item->item_quality;
        env->equipment.amulet_level = item->item_level;
        for (uint32_t i = 0; i < 4; i++) {
            env->equipment.amulet_stat_bonuses[i] = item->stat_bonuses[i];
        }
    } else if (item->item_type == EQUIPMENT_HELMET) {
        env->equipment.helmet_type = item->item_type;
        env->equipment.helmet_quality = item->item_quality;
        env->equipment.helmet_level = item->item_level;
        for (uint32_t i = 0; i < 4; i++) {
            env->equipment.helmet_stat_bonuses[i] = item->stat_bonuses[i];
        }
    } else if (item->item_type == EQUIPMENT_BOOTS) {
        env->equipment.boots_type = item->item_type;
        env->equipment.boots_quality = item->item_quality;
        env->equipment.boots_level = item->item_level;
        for (uint32_t i = 0; i < 4; i++) {
            env->equipment.boots_stat_bonuses[i] = item->stat_bonuses[i];
        }
    } else if (item->item_type == EQUIPMENT_GLOVES) {
        env->equipment.gloves_type = item->item_type;
        env->equipment.gloves_quality = item->item_quality;
        env->equipment.gloves_level = item->item_level;
        for (uint32_t i = 0; i < 4; i++) {
            env->equipment.gloves_stat_bonuses[i] = item->stat_bonuses[i];
        }
    } else if (item->item_type == EQUIPMENT_SHOULDERS) {
        env->equipment.shoulders_type = item->item_type;
        env->equipment.shoulders_quality = item->item_quality;
        env->equipment.shoulders_level = item->item_level;
        for (uint32_t i = 0; i < 4; i++) {
            env->equipment.shoulders_stat_bonuses[i] = item->stat_bonuses[i];
        }
    } else if (item->item_type == EQUIPMENT_ARMOR) {
        env->equipment.armor_type = item->item_type;
        env->equipment.armor_quality = item->item_quality;
        env->equipment.armor_level = item->item_level;
        for (uint32_t i = 0; i < 4; i++) {
            env->equipment.armor_stat_bonuses[i] = item->stat_bonuses[i];
        }
    } else if (item->item_type == EQUIPMENT_BELT) {
        env->equipment.belt_type = item->item_type;
        env->equipment.belt_quality = item->item_quality;
        env->equipment.belt_level = item->item_level;
        for (uint32_t i = 0; i < 4; i++) {
            env->equipment.belt_stat_bonuses[i] = item->stat_bonuses[i];
        }
    } else if (item->item_type == EQUIPMENT_PANTS) {
        env->equipment.pants_type = item->item_type;
        env->equipment.pants_quality = item->item_quality;
        env->equipment.pants_level = item->item_level;
        for (uint32_t i = 0; i < 4; i++) {
            env->equipment.pants_stat_bonuses[i] = item->stat_bonuses[i];
        }
    } else if (item->item_type == EQUIPMENT_BRACERS) {
        env->equipment.bracers_type = item->item_type;
        env->equipment.bracers_quality = item->item_quality;
        env->equipment.bracers_level = item->item_level;
        for (uint32_t i = 0; i < 4; i++) {
            env->equipment.bracers_stat_bonuses[i] = item->stat_bonuses[i];
        }
    }
    
    memset(item, 0, sizeof(InventorySlot));
    calculate_total_stats(env);
    
}

void handle_shop_interaction(Rift* env) {
    for (uint32_t i = 0; i < SHOP_ITEMS_COUNT; i++) {
        if (is_at_shop_item(env, i)) {
            buy_shop_item(env, i);
            return;
        }
    }
}

void handle_inventory_interaction(Rift* env) {
    if (is_at_inventory_area(env)) {
        env->player.selected_inventory_slot = (env->player.selected_inventory_slot + 1) % INVENTORY_SLOTS;
        
        if (env->player.inventory[env->player.selected_inventory_slot].item_type != EQUIPMENT_NONE) {
            equip_item_from_inventory(env, env->player.selected_inventory_slot);
        }
    }
}

void transition_to_town(Rift* env) {
    env->current_phase = PHASE_TOWN;
    
    env->current_rift_level++;
    
    int rift_gold = GetRiftCompletionGold(env->current_rift_level);
    env->player.gold += rift_gold;
    env->episode_gold_earned += rift_gold;
    
    int rift_exp = EXP_PER_RIFT_COMPLETION + (env->current_rift_level * 25);
    add_experience(env, rift_exp);
    
    float progression_reward = env->current_rift_level * 2.0f;
    env->step_reward += progression_reward;
    env->episode_return += progression_reward;
    
    generate_shop_inventory(env);
    
    env->town_interface.frames_remaining = TOWN_MODE_TIME_LIMIT;
    env->town_interface.warning_phase = 0;
    env->town_interface.current_tab = TOWN_TAB_CHARACTER;
    env->town_interface.character_mode = CHARACTER_MODE_EQUIPMENT;
    env->town_interface.selected_item_index = 0;
    env->town_interface.equipment_slot = 0;
    env->town_interface.input_cooldown = 0;
    env->town_interface.shop_scroll_offset = 0;
    env->town_interface.current_visit_purchases = 0;
}

void transition_to_rift(Rift* env) {
    #if TOWN_TESTING_MODE
        env->current_phase = PHASE_TOWN;
        
        env->current_rift_level++;
        
        int rift_gold = GetRiftCompletionGold(env->current_rift_level);
        env->player.gold += rift_gold;
        env->episode_gold_earned += rift_gold;
        
        int rift_exp = TOWN_TEST_AUTO_EXP + (env->current_rift_level * 25); 
        add_experience(env, rift_exp);
        
        env->episode_rift_completions++;
        env->step_reward += env->config.completion_reward;
        env->episode_return += env->config.completion_reward;
        env->episode_completion_rewards += env->config.completion_reward;
        
        float progression_reward = env->current_rift_level * 2.0f; 
        env->step_reward += progression_reward;
        env->episode_return += progression_reward;
        
        generate_shop_inventory(env);
        
        if (env->town_interface.current_visit_purchases == 0) {
            env->step_reward += TOWN_NO_PURCHASES_PENALTY;
            env->episode_return += TOWN_NO_PURCHASES_PENALTY;
            env->episode_no_purchase_penalties += -TOWN_NO_PURCHASES_PENALTY;
        }
        
        env->town_interface.frames_remaining = TOWN_MODE_TIME_LIMIT;
        env->town_interface.warning_phase = 0;
        env->town_interface.current_visit_purchases = 0;
        
        return;
    #endif
    
    if (env->town_interface.current_visit_purchases == 0) {
        env->step_reward += TOWN_NO_PURCHASES_PENALTY;
        env->episode_return += TOWN_NO_PURCHASES_PENALTY;
        env->episode_no_purchase_penalties += -TOWN_NO_PURCHASES_PENALTY;
    }
    
    env->current_phase = PHASE_RIFT;
    generate_rift_map(env);
    
    env->player.x = MAP_WIDTH / 2;
    env->player.y = MAP_HEIGHT / 2;
    
    env->monsters_spawned = 0;
    env->monsters_killed = 0;
    env->boss_spawned = 0;
    env->rift_completed = 0;
    
    memset(env->monsters, 0, sizeof(env->monsters));
    memset(&env->boss, 0, sizeof(Boss));
    memset(env->items, 0, sizeof(env->items));
    memset(env->projectiles, 0, sizeof(env->projectiles));
    memset(env->blizzard_areas, 0, sizeof(env->blizzard_areas));
}

void update_town_timer(Rift* env) {
    if (env->current_phase != PHASE_TOWN) return;
    
    if (env->town_interface.input_cooldown > 0) {
        env->town_interface.input_cooldown--;
    }
    
    env->town_interface.frames_remaining--;
    env->episode_town_time_used = TOWN_MODE_TIME_LIMIT - env->town_interface.frames_remaining;
    
    if (env->town_interface.frames_remaining <= TOWN_TIMER_WARNING_2) {
        env->town_interface.warning_phase = 2;
    } else if (env->town_interface.frames_remaining <= TOWN_TIMER_WARNING_1) {
        env->town_interface.warning_phase = 1;
    }
    
    if (env->town_interface.frames_remaining <= 0) {
        transition_to_rift(env);
    }
}

void handle_town_navigation(Rift* env, int action) {
    if (env->town_interface.input_cooldown > 0) {
        return; 
    }
    
    switch (action) {
        case ACTION_MOVE_LEFT:
            if (env->town_interface.current_tab == TOWN_TAB_SHOP) {
                env->town_interface.current_tab = TOWN_TAB_CHARACTER;
                env->town_interface.character_mode = CHARACTER_MODE_EQUIPMENT; 
                env->town_interface.selected_item_index = 0;
                env->town_interface.input_cooldown = TOWN_INPUT_COOLDOWN;
            } else {
            }
            break;
        case ACTION_MOVE_RIGHT:
            if (env->town_interface.current_tab == TOWN_TAB_CHARACTER) {
                env->town_interface.current_tab = TOWN_TAB_SHOP;
                env->town_interface.shop_scroll_offset = 0;
                
                int first_available = 0;
                while (first_available < SHOP_ITEMS_COUNT && !env->shop_items[first_available].available) {
                    first_available++;
                }
                
                env->town_interface.selected_item_index = (first_available < SHOP_ITEMS_COUNT) ? first_available : 0;
                env->town_interface.input_cooldown = TOWN_INPUT_COOLDOWN;
                
            } else {
            }
            break;
        case ACTION_MOVE_UP:
            if (env->town_interface.current_tab == TOWN_TAB_CHARACTER && env->town_interface.character_mode == CHARACTER_MODE_EQUIPMENT) {
                if (env->town_interface.selected_item_index > 0) {
                    env->town_interface.selected_item_index--;
                } else {
                    env->town_interface.selected_item_index = 12; 
                }
            } else if (env->town_interface.current_tab == TOWN_TAB_SHOP) {
                int current_idx = env->town_interface.selected_item_index;
                int new_idx = current_idx - 1;
                
                while (new_idx >= 0 && !env->shop_items[new_idx].available) {
                    new_idx--;
                }
                
                if (new_idx < 0) {
                    new_idx = SHOP_ITEMS_COUNT - 1;
                    while (new_idx >= 0 && !env->shop_items[new_idx].available) {
                        new_idx--;
                    }
                }
                
                if (new_idx >= 0 && env->shop_items[new_idx].available) {
                    env->town_interface.selected_item_index = new_idx;
                }
            } else {
                if (env->town_interface.selected_item_index > 0) {
                    env->town_interface.selected_item_index--;
                }
            }
            env->town_interface.input_cooldown = TOWN_INPUT_COOLDOWN;
            break;
        case ACTION_MOVE_DOWN:
            if (env->town_interface.current_tab == TOWN_TAB_CHARACTER && env->town_interface.character_mode == CHARACTER_MODE_EQUIPMENT) {
                if (env->town_interface.selected_item_index < 12) {
                    env->town_interface.selected_item_index++;
                } else {
                    env->town_interface.selected_item_index = 0;
                }
            } else if (env->town_interface.current_tab == TOWN_TAB_SHOP) {
                int current_idx = env->town_interface.selected_item_index;
                int new_idx = current_idx + 1;
                
                while (new_idx < SHOP_ITEMS_COUNT && !env->shop_items[new_idx].available) {
                    new_idx++;
                }
                
                if (new_idx >= SHOP_ITEMS_COUNT) {
                    new_idx = 0;
                    while (new_idx < SHOP_ITEMS_COUNT && !env->shop_items[new_idx].available) {
                        new_idx++;
                    }
                }
                
                if (new_idx < SHOP_ITEMS_COUNT && env->shop_items[new_idx].available) {
                    env->town_interface.selected_item_index = new_idx;
                }
            } else {
                env->town_interface.selected_item_index++;
                
                if (env->town_interface.current_tab == TOWN_TAB_CHARACTER) {
                    if (env->town_interface.selected_item_index >= INVENTORY_SLOTS) env->town_interface.selected_item_index = INVENTORY_SLOTS - 1;
                }
            }
            env->town_interface.input_cooldown = TOWN_INPUT_COOLDOWN;
            break;
    }
}

void handle_town_interaction(Rift* env) {
    
    if (env->town_interface.current_tab == TOWN_TAB_CHARACTER) {
        return;
    } else if (env->town_interface.current_tab == TOWN_TAB_SHOP) {
        uint32_t shop_slot = env->town_interface.selected_item_index;
        if (shop_slot < SHOP_ITEMS_COUNT) {
            buy_shop_item(env, shop_slot);
        } else {
        }
    }
}


void c_reset(Rift* env) {
    init_hero_stats(env);
    
    env->current_rift_level = 1;
    
    #if TOWN_TESTING_MODE
        env->current_phase = PHASE_TOWN;
        generate_town_map(env);
        generate_shop_inventory(env);
    #else
        env->current_phase = PHASE_RIFT;
        generate_rift_map(env);
    #endif
    
    #if TOWN_TESTING_MODE
        env->player.x = 15; 
        env->player.y = 19;
    #else
        env->player.x = MAP_WIDTH / 2; 
        env->player.y = MAP_HEIGHT / 2;
    #endif
    
    env->town_interface.frames_remaining = TOWN_MODE_TIME_LIMIT;
    env->town_interface.warning_phase = 0;
    env->player.prev_x = env->player.x;
    env->player.prev_y = env->player.y;
    env->player.movement_x = 0.0f;
    env->player.movement_y = 0.0f;
    env->player.health = env->player.max_health;
    env->player.mana = env->player.max_mana;
    env->player.gold = env->config.starting_gold;
    env->player.alive = 1;
    env->player.blizzard_cooldown = 0;
    env->player.health_potion_cooldown = 0;
    env->player.mana_potion_cooldown = 0;
    env->player.mana_regen_timer = 0;
    env->player.low_health_penalty_cooldown = 0;
    env->player.inventory_count = 0;
    env->player.selected_inventory_slot = 0;
    env->player.facing_x = 0.0f;
    env->player.facing_y = -1.0f;
    memset(env->player.old_inventory, 0, sizeof(env->player.old_inventory));
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
    env->human_mode = 0;
    
    env->rewards[0] = 0.0f;
    env->terminals[0] = 0;
    
    env->episode_monsters_killed = 0;
    env->episode_boss_kills = 0;
    env->episode_gold_earned = 0;
    env->episode_damage_dealt = 0;
    env->episode_damage_taken = 0;
    env->episode_rift_completions = 0;
    env->episode_vendor_transactions = 0;
    env->episode_blizzards_cast = 0;
    env->episode_deaths = 0;
    env->episode_completion_rewards = 0.0f;
    env->episode_monster_kill_rewards = 0.0f;
    env->episode_death_penalties = 0.0f;
    env->episode_no_purchase_penalties = 0.0f;
    
    env->episode_shop_purchases = 0;
    env->episode_town_time_used = 0;
    
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
    
    env->player.movement_x = env->player.x - env->player.prev_x;
    env->player.movement_y = env->player.y - env->player.prev_y;
    
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
                env->boss.attack_cooldown = GetScaledAttackCooldown(15, env->current_rift_level);
                
                if ((rand() % 100) < env->player.dodge_chance) {
                } else {
                    env->episode_damage_taken += env->boss.damage;
                    
                    if (env->boss.damage >= env->player.health) {
                        env->player.health = 0;
                        env->player.alive = 0;
                        env->step_reward += env->config.death_penalty;
                        env->episode_return += env->config.death_penalty;
                        env->episode_death_penalties += env->config.death_penalty;
                        env->episode_deaths++;
                    } else {
                        env->player.health -= env->boss.damage;
                    }
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
            
            uint16_t boss_gold = (GOLD_DROP_MIN + rand() % GOLD_DROP_RANGE) * 3; 
            env->player.gold += boss_gold;
            env->episode_gold_earned += boss_gold;
            
            transition_to_town(env);
            return;
        }
    } else if (env->current_phase == PHASE_TOWN) {
        update_town_timer(env);
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