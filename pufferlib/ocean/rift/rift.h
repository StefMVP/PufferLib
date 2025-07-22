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
    .max_monsters = MONSTER.max_count,
    .max_items = POTIONS.max_items,
    .monster_spawn_rate = 0.8f,
    .item_drop_rate = 0.7f,  // Increased from 0.3f for more gold drops
    .player_start_health = PLAYER.max_health,
    .player_start_mana = PLAYER.max_mana,
    .starting_gold = 500,
    .monster_move_speed = 0.5f,
    .episode_length_limit = OBSERVATION.max_episode_length,
    .completion_reward = 10.0f,
    .monster_kill_reward = 10.0f,
    .death_penalty = 0.0f,
};

typedef struct InventorySlot {
    uint32_t item_type;
    uint32_t item_quality;
    uint32_t item_level;
    uint32_t stack_size;
    uint32_t stat_bonuses[4];
    uint16_t item_value;
} InventorySlot;

typedef struct Player {
    float x, y;
    float prev_x, prev_y;
    float movement_x, movement_y;
    float facing_x, facing_y;
    uint32_t old_inventory[20];
    InventorySlot inventory[12];
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
    uint32_t stuck_timer;           // Anti-stuck mechanism
    float last_position_x, last_position_y;
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
    float target_x, target_y;
    uint32_t type;
    uint32_t damage;
    uint32_t lifetime;
    uint32_t active;
    uint32_t homing;
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
    float low_health_penalties;
    float stat_rewards;
    float item_level_rewards;
    float progression_rewards;
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
    
    unsigned char map[1900];
    
    Player player;
    Monster monsters[200];
    Boss boss;
    Item items[100];
    Projectile projectiles[50];
    BlizzardArea blizzard_areas[10];
    Vendor vendor;
    
    HeroStats hero_stats;
    Equipment equipment;
    ShopItem shop_items[10];
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
    float episode_low_health_penalties;
    float episode_stat_rewards;
    float episode_item_level_rewards;
    float episode_progression_rewards;
    float episode_no_purchase_penalties;
    
    uint32_t episode_shop_purchases;
    uint16_t episode_town_time_used;
    
    uint32_t human_mode;               // Toggle for human control (1) vs AI (0)
} Rift;

#include "render.h"
#include "rift_core.h"
#include "rift_world.h" 
#include "rift_town.h"
#include "rift_combat.h"

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
void execute_boss_attack(Rift* env);
void update_projectiles(Rift* env);

// Scaling functions
float GetScaledMonsterHealth(float base_health, uint32_t rift_level);
float GetScaledMonsterDamage(float base_damage, uint32_t rift_level);
float GetScaledMonsterSpeed(float base_speed, uint32_t rift_level);
uint32_t GetScaledAttackCooldown(uint32_t base_cooldown, uint32_t rift_level);

void compute_observations(Rift* env);

void c_close_client(Client* client);

static void add_log(Rift* env);

