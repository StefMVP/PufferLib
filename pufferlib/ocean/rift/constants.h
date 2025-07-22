#ifndef RIFT_CONSTANTS_H
#define RIFT_CONSTANTS_H

// ============================================================================
// ACTION CONSTANTS
// ============================================================================
typedef struct {
    int move_up, move_down, move_left, move_right;
    int move_up_left, move_up_right, move_down_left, move_down_right;
    int blizzard, use_health_potion, use_mana_potion;
    int interact, noop, exit_town;
    int switch_to_shop, switch_to_character, reroll_shop;
} ActionTypes;

static const ActionTypes ACTIONS = {
    .move_up = 0, .move_down = 1, .move_left = 2, .move_right = 3,
    .move_up_left = 4, .move_up_right = 5, .move_down_left = 6, .move_down_right = 7,
    .blizzard = 8, .use_health_potion = 9, .use_mana_potion = 10,
    .interact = 11, .noop = 12, .exit_town = 13,
    .switch_to_shop = 14, .switch_to_character = 15, .reroll_shop = 16
};

// ============================================================================
// MAP AND CELL CONSTANTS
// ============================================================================
typedef struct {
    int width, height, size, border_size;
} MapConfig;

typedef struct {
    int empty, wall, floor, door, vendor;
} CellTypes;

static const MapConfig MAP = {
    .width = 50, .height = 38, .size = 1900, .border_size = 5
};

static const CellTypes CELLS = {
    .empty = 0, .wall = 1, .floor = 2, .door = 3, .vendor = 4
};

// ============================================================================
// GAME PHASE CONSTANTS
// ============================================================================
typedef struct {
    int rift, town;
} PhaseTypes;

typedef struct {
    int zombie, mage, heavy_melee, light, elite;
} MonsterTypes;

typedef struct {
    int rift_guardian;
} BossTypes;

typedef struct {
    int gold, health_potion, mana_potion;
} ItemTypes;

static const PhaseTypes PHASES = {
    .rift = 0, .town = 1
};

static const MonsterTypes MONSTERS = {
    .zombie = 0, .mage = 1, .heavy_melee = 2, .light = 3, .elite = 4
};

static const BossTypes BOSSES = {
    .rift_guardian = 0
};

static const ItemTypes ITEMS = {
    .gold = 0, .health_potion = 1, .mana_potion = 2
};

// ============================================================================
// GAME MECHANICS CONSTANTS
// ============================================================================

typedef struct {
    int activation_cooldown, damage, damage_interval, duration;
    int mana_cost, max_areas;
    float radius;
} BlizzardConfig;

static const BlizzardConfig BLIZZARD = {
    .activation_cooldown = 20, .damage = 15, .damage_interval = 15,
    .duration = 60, .mana_cost = 15, .max_areas = 10, .radius = 2.0f
};

typedef struct {
    int base_damage, base_health, attack_cooldown;
    float attack_range;
} BossConfig;

typedef struct {
    int base_damage, max_health, max_mana;
} PlayerConfig;

static const BossConfig BOSS = {
    .base_damage = 15, .base_health = 100, .attack_cooldown = 45, .attack_range = 3.0f
};

static const PlayerConfig PLAYER = {
    .base_damage = 10, .max_health = 100, .max_mana = 50
};

typedef struct {
    int max_count, attack_cooldown, base_damage, base_health;
    int movement_cooldown, wander_chance, wander_cooldown, spawn_count;
    float detection_range;
} MonsterConfig;

static const MonsterConfig MONSTER = {
    .max_count = 200, .attack_cooldown = 30, .base_damage = 5, .base_health = 15,
    .movement_cooldown = 5, .wander_chance = 20, .wander_cooldown = 10,
    .spawn_count = 38, .detection_range = 8.0f
};

typedef struct {
    int damage, health;
    float range, speed;
} MonsterTypeStats;

typedef struct {
    MonsterTypeStats elite, heavy, light, mage;
} MonsterTypeConfig;

static const MonsterTypeConfig MONSTER_STATS = {
    .elite = {.damage = 20, .health = 15, .range = 4.0f, .speed = 0.4f},
    .heavy = {.damage = 15, .health = 15, .range = 1.5f, .speed = 0.2f},
    .light = {.damage = 3, .health = 8, .range = 2.0f, .speed = 0.8f},
    .mage = {.damage = 8, .health = 10, .range = 6.0f, .speed = 0.3f}
};

typedef struct {
    int max_items, max_inventory_size, mana_regen_rate;
    int health_potion_cooldown, mana_potion_cooldown;
    float health_potion_heal_percent, mana_potion_restore_percent;
} PotionConfig;

static const PotionConfig POTIONS = {
    .max_items = 100, .max_inventory_size = 20, .mana_regen_rate = 15,
    .health_potion_cooldown = 80, .mana_potion_cooldown = 68,
    .health_potion_heal_percent = 0.48f, .mana_potion_restore_percent = 0.48f
};

typedef struct {
    int drop_min, drop_range;
    int health_potion_price, mana_potion_price, stock_amount;
    int reroll_base_cost;
    float reroll_scaling;
} VendorConfig;

static const VendorConfig VENDOR = {
    .drop_min = 15, .drop_range = 25,
    .health_potion_price = 20, .mana_potion_price = 15, .stock_amount = 10,
    .reroll_base_cost = 25, .reroll_scaling = 1.5f
};

typedef struct {
    int melee, projectile, cone_slam, fast_projectile;
    int homing_projectile, melee_projectile, boss_ground_slam, boss_cone;
} AttackTypes;

typedef struct {
    int max_projectiles;
    float speed, speed_fast, speed_homing, hit_radius;
} ProjectileConfig;

static const AttackTypes ATTACKS = {
    .melee = 0, .projectile = 1, .cone_slam = 2, .fast_projectile = 3,
    .homing_projectile = 4, .melee_projectile = 5, .boss_ground_slam = 6, .boss_cone = 7
};

static const ProjectileConfig PROJECTILE = {
    .max_projectiles = 50, .speed = 4.0f, .speed_fast = 6.0f,
    .speed_homing = 3.0f, .hit_radius = 1.0f
};

typedef struct {
    float spawn_check_distance;
} MapGenConfig;

typedef struct {
    int max_episode_length, grid_size, grid_obs_size;
    int player_obs_size, shop_slots_obs, equipment_slots_obs;
    int town_interface_obs, town_obs_size, obs_size;
    float rift_completion_threshold;
} ObservationConfig;

typedef struct {
    float distance, gold, full_circle_multiplier;
} NormalizationConfig;

static const MapGenConfig MAPGEN = {
    .spawn_check_distance = 8.0f
};

static const ObservationConfig OBSERVATION = {
    .max_episode_length = 12000, .grid_size = 10, .grid_obs_size = 100,
    .player_obs_size = 17, .shop_slots_obs = 40, .equipment_slots_obs = 52,
    .town_interface_obs = 9, .town_obs_size = 101, .obs_size = 218,
    .rift_completion_threshold = 0.8f
};

static const NormalizationConfig NORMALIZATION = {
    .distance = 50.0f, .gold = 1000.0f, .full_circle_multiplier = 2.0f
};

// ============================================================================
// RENDERING CONSTANTS
// ============================================================================

// PROPER CONTAINER HIERARCHY FOR RESPONSIVE LAYOUT
// Top-level containers relative to screen
#define SCREEN_WIDTH CLIENT_WIDTH   // 1024
#define SCREEN_HEIGHT CLIENT_HEIGHT // 768

#define TOP_UI_HEIGHT 60
#define TOP_UI_X 0
#define TOP_UI_Y 0
#define TOP_UI_WIDTH SCREEN_WIDTH

#define BOTTOM_UI_HEIGHT 100
#define BOTTOM_UI_X 0
#define BOTTOM_UI_Y (SCREEN_HEIGHT - BOTTOM_UI_HEIGHT)  // 668
#define BOTTOM_UI_WIDTH SCREEN_WIDTH

// Main town container (between top and bottom UI)
#define TOWN_CONTAINER_X 50
#define TOWN_CONTAINER_Y TOP_UI_HEIGHT + 20
#define TOWN_CONTAINER_WIDTH SCREEN_WIDTH - 100
#define TOWN_CONTAINER_HEIGHT SCREEN_HEIGHT - TOP_UI_HEIGHT - 40

// Content areas within town container
#define CONTENT_MARGIN 20
#define CONTENT_GAP 20
#define CONTENT_AREA_WIDTH 420
#define STATS_PANEL_WIDTH (TOWN_CONTAINER_WIDTH - (2 * CONTENT_MARGIN) - CONTENT_AREA_WIDTH - CONTENT_GAP)  // 480

#define CONTENT_AREA_X (TOWN_CONTAINER_X + CONTENT_MARGIN)  // 70
#define STATS_PANEL_X (CONTENT_AREA_X + CONTENT_AREA_WIDTH + CONTENT_GAP)  // 470

// Stats panel content positioning (relative to panel)
#define STATS_CONTENT_MARGIN_LEFT 10
#define STATS_CONTENT_MARGIN_INDENT 15  // For indented items
#define STATS_LINE_HEIGHT 20
#define STATS_SECTION_SPACING 30

// Sprite and tileset
#define SPRITE_SIZE 32
#define TILESET_COLS 1
#define TILESET_ROWS 1

// Client/Window constants
#define CLIENT_CELL_SIZE 24
#define CLIENT_FPS 60
#define CLIENT_HEIGHT 768
#define CLIENT_WIDTH 1024

// Hero animation constants
#define HERO_ANIM_IDLE 0
#define HERO_ANIM_WALK 1
#define HERO_ANIM_CAST 2
#define HERO_ANIM_SPEED 8
#define HERO_FRAMES_IDLE 4
#define HERO_FRAMES_WALK 8
#define HERO_FRAMES_CAST 6

// ============================================================================
// ANIMATION AND TIMING SYSTEM STRUCTS
// ============================================================================

typedef struct {
    int idle_frames, walk_frames, cast_frames;
    int animation_speed;
    int idle_state, walk_state, cast_state;
} HeroAnimationConfig;

typedef struct {
    float base, amplitude, speed;
    int alpha;
} GlowEffectConfig;

typedef struct {
    float pulse_base, pulse_amplitude, pulse_speed;
    int fade_speed, halo_offset;
    int trail_width, trail_length;
    int lifetime;
} ProjectileEffectConfig;

typedef struct {
    float size_multiplier;
    int health_bar_height, health_bar_offset;
    float pulse_base, pulse_amplitude;
    int shadow_alpha, shadow_offset;
    int core_alpha, aura_alpha;
} BossEffectConfig;

typedef struct {
    float glow_speed;
    int base_alpha;
    float size_eighth, size_twelfth;
} ItemEffectConfig;

typedef struct {
    int base_alpha, duration;
    int damage_interval;
    int ice_shard_count;
    int shard_timing_mult, shard_cycle_frames;
    int shard_fall_speed, shard_size_min, shard_size_range;
    int frost_line_size;
} BlizzardEffectConfig;

typedef struct {
    float ui_pulse_slow_speed, ui_pulse_med_speed, ui_pulse_fast_speed;
    float ui_pulse_base, ui_pulse_amplitude;
    int selection_pulse_alpha;
} UIAnimationConfig;

static const HeroAnimationConfig HERO_ANIMATION = {
    .idle_frames = 4, .walk_frames = 8, .cast_frames = 6,
    .animation_speed = 8,
    .idle_state = 0, .walk_state = 1, .cast_state = 2
};

static const GlowEffectConfig GLOW_EFFECT = {
    .base = 0.8f, .amplitude = 0.2f, .speed = 0.1f,
    .alpha = 100
};

static const ProjectileEffectConfig PROJECTILE_EFFECT = {
    .pulse_base = 0.8f, .pulse_amplitude = 0.3f, .pulse_speed = 0.3f,
    .fade_speed = 8, .halo_offset = 3,
    .trail_width = 3, .trail_length = 10,
    .lifetime = 60
};

static const BossEffectConfig BOSS_EFFECT = {
    .size_multiplier = 1.5f,
    .health_bar_height = 6, .health_bar_offset = 15,
    .pulse_base = 0.9f, .pulse_amplitude = 0.1f,
    .shadow_alpha = 150, .shadow_offset = 3,
    .core_alpha = 200, .aura_alpha = 100
};

static const ItemEffectConfig ITEM_EFFECT = {
    .glow_speed = 0.2f, .base_alpha = 150,
    .size_eighth = 0.125f, .size_twelfth = 0.0833f
};

static const BlizzardEffectConfig BLIZZARD_EFFECT = {
    .base_alpha = 150, .duration = 60,
    .damage_interval = 15,
    .ice_shard_count = 12,
    .shard_timing_mult = 5, .shard_cycle_frames = 60,
    .shard_fall_speed = 2, .shard_size_min = 1, .shard_size_range = 3,
    .frost_line_size = 3
};

static const UIAnimationConfig UI_ANIMATION = {
    .ui_pulse_slow_speed = 2.0f, .ui_pulse_med_speed = 3.0f, .ui_pulse_fast_speed = 4.0f,
    .ui_pulse_base = 0.7f, .ui_pulse_amplitude = 0.3f,
    .selection_pulse_alpha = 120
};

// ============================================================================
// UI LAYOUT SYSTEM STRUCTS
// ============================================================================

typedef struct {
    int screen_width, screen_height;
    int cell_size, fps;
} ClientConfig;

typedef struct {
    int size_12, size_14, size_16, size_18, size_20;
} TextSizeConfig;

typedef struct {
    int top_height, top_x, top_y, top_width;
    int bottom_height, bottom_x, bottom_y, bottom_width;
    int manual_control_height;
} TopBottomUIConfig;

typedef struct {
    int container_x, container_y, container_width, container_height;
    int content_margin, content_gap;
    int content_area_width, content_area_x;
    int stats_panel_width, stats_panel_x;
} TownContainerConfig;

typedef struct {
    int content_margin_left, content_margin_indent;
    int line_height, section_spacing;
    int stats_offset, stats_line_height;
} StatsLayoutConfig;

typedef struct {
    int radius, margin;
    int potion_size, potion_spacing;
    int health_bg_r;
} GlobeUIConfig;

static const ClientConfig CLIENT_CONFIG = {
    .screen_width = 1024, .screen_height = 768,
    .cell_size = 24, .fps = 60
};

static const TextSizeConfig TEXT_SIZES = {
    .size_12 = 12, .size_14 = 14, .size_16 = 16, .size_18 = 18, .size_20 = 20
};

static const TopBottomUIConfig TOP_BOTTOM_UI = {
    .top_height = 60, .top_x = 0, .top_y = 0, .top_width = 1024,
    .bottom_height = 100, .bottom_x = 0, .bottom_y = 668, .bottom_width = 1024,
    .manual_control_height = 25
};

static const TownContainerConfig TOWN_CONTAINER = {
    .container_x = 50, .container_y = 80, .container_width = 924, .container_height = 688,
    .content_margin = 20, .content_gap = 20,
    .content_area_width = 420, .content_area_x = 70,
    .stats_panel_width = 480, .stats_panel_x = 470
};

static const StatsLayoutConfig STATS_LAYOUT = {
    .content_margin_left = 10, .content_margin_indent = 15,
    .line_height = 20, .section_spacing = 30,
    .stats_offset = 45, .stats_line_height = 18
};

static const GlobeUIConfig GLOBE_UI = {
    .radius = 25, .margin = 10,
    .potion_size = 15, .potion_spacing = 30,
    .health_bg_r = 128
};

// ============================================================================
// VISUAL EFFECTS RENDERING STRUCTS
// ============================================================================

typedef struct {
    int alpha, offset;
} ShadowConfig;

typedef struct {
    int height, offset;
    int boss_height, boss_offset;
} HealthBarConfig;

typedef struct {
    int sprite_size;
    int tileset_cols, tileset_rows;
} SpriteConfig;

typedef struct {
    int bg_alpha, bg_color_rgb;
    int height, line_color_rgb;
} UIRenderConfig;

typedef struct {
    int stone_floor, stone_wall, stone_door;
    int town_floor, vendor_stall;
} TileConfig;

static const ShadowConfig SHADOW_RENDER = {
    .alpha = 100, .offset = 2
};

static const HealthBarConfig HEALTH_BAR_RENDER = {
    .height = 2, .offset = 5,
    .boss_height = 6, .boss_offset = 15
};

static const SpriteConfig SPRITE_RENDER = {
    .sprite_size = 32,
    .tileset_cols = 8, .tileset_rows = 8
};

static const UIRenderConfig UI_RENDER = {
    .bg_alpha = 200, .bg_color_rgb = 20,
    .height = 100, .line_color_rgb = 60
};

static const TileConfig TILE_IDS = {
    .stone_floor = 0, .stone_wall = 1, .stone_door = 2,
    .town_floor = 3, .vendor_stall = 4
};

// ============================================================================
// COLOR SCHEME STRUCTS
// ============================================================================

typedef struct {
    int bg_r, bg_g, bg_b;
    int clear_bg_r, clear_bg_g, clear_bg_b;
    int border_rgb;
} BackgroundColorConfig;

typedef struct {
    int player_r, player_g, player_b;
    int armor_r, armor_g, armor_b;
} PlayerColorConfig;

typedef struct {
    int zombie_r, zombie_g, zombie_b;
    int elite_r, elite_g, elite_b;
    int heavy_r, heavy_g, heavy_b;
    int light_r, light_g, light_b;
    int maroon_r;
} MonsterColorConfig;

typedef struct {
    int core_r, core_g;
    int fireball_r, fireball_g;
    int halo_r, halo_g;
    int trail_r, trail_g;
    int blizzard_base_r, blizzard_base_g, blizzard_base_b;
    int frost_r, frost_g, frost_b;
} EffectColorConfig;

typedef struct {
    int common_gray;
    int white, white_translucent, white_alpha;
    int rare_blue_r, rare_blue_g, rare_blue_b;
    int epic_purple_r, epic_purple_g, epic_purple_b;
    int legendary_orange_r, legendary_orange_g, legendary_orange_b;
    int default_dark;
    int light_silver, light_silver_strong, silver;
} QualityColorConfig;

typedef struct {
    int brown_handle_light_r, brown_handle_light_g, brown_handle_light_b;
    int brown_handle_dark_r, brown_handle_dark_g, brown_handle_dark_b;
    int brown_leather_r, brown_leather_g, brown_leather_b;
} MaterialColorConfig;

static const BackgroundColorConfig BACKGROUND_COLORS = {
    .bg_r = 40, .bg_g = 35, .bg_b = 30,
    .clear_bg_r = 20, .clear_bg_g = 20, .clear_bg_b = 25,
    .border_rgb = 80
};

static const PlayerColorConfig PLAYER_COLORS = {
    .player_r = 138, .player_g = 43, .player_b = 226,
    .armor_r = 75, .armor_g = 0, .armor_b = 130
};

static const MonsterColorConfig MONSTER_COLORS = {
    .zombie_r = 34, .zombie_g = 139, .zombie_b = 34,
    .elite_r = 220, .elite_g = 20, .elite_b = 60,
    .heavy_r = 139, .heavy_g = 69, .heavy_b = 19,
    .light_r = 255, .light_g = 215, .light_b = 0,
    .maroon_r = 128
};

static const EffectColorConfig EFFECT_COLORS_EXTENDED = {
    .core_r = 255, .core_g = 255,
    .fireball_r = 255, .fireball_g = 69,
    .halo_r = 255, .halo_g = 140,
    .trail_r = 255, .trail_g = 100,
    .blizzard_base_r = 200, .blizzard_base_g = 230, .blizzard_base_b = 255,
    .frost_r = 173, .frost_g = 216, .frost_b = 230
};

static const QualityColorConfig QUALITY_COLORS = {
    .common_gray = 128,
    .white = 255, .white_translucent = 128, .white_alpha = 64,
    .rare_blue_r = 100, .rare_blue_g = 149, .rare_blue_b = 237,
    .epic_purple_r = 163, .epic_purple_g = 53, .epic_purple_b = 238,
    .legendary_orange_r = 255, .legendary_orange_g = 128, .legendary_orange_b = 0,
    .default_dark = 64,
    .light_silver = 192, .light_silver_strong = 224, .silver = 169
};

static const MaterialColorConfig MATERIAL_COLORS_EXTENDED = {
    .brown_handle_light_r = 139, .brown_handle_light_g = 115, .brown_handle_light_b = 85,
    .brown_handle_dark_r = 101, .brown_handle_dark_g = 67, .brown_handle_dark_b = 33,
    .brown_leather_r = 139, .brown_leather_g = 69, .brown_leather_b = 19
};

// ============================================================================
// SHOP AND INVENTORY CONFIGURATION STRUCTS
// ============================================================================

typedef struct {
    int items_count, slots_obs;
    int container_y_offset, container_width_margin;
    int item_x_margin, item_y_margin;
    int case_width, selection_margin;
    int icon_size, icon_margin;
    int text_spacing_y;
} ShopConfig;

typedef struct {
    int slots, max_size;
    int area_x, area_y;
    int selection_margin, quality_margin;
    int icon_offset_reduction;
    int ilvl_text_margin;
} InventoryConfig;

typedef struct {
    int case_bg_r, case_bg_g, case_bg_b, case_bg_a;
    int case_border_r, case_border_g, case_border_b;
    int icon_bg_r, icon_bg_g, icon_bg_b, icon_bg_a;
} ShopColorConfig;

static const ShopConfig SHOP_CONFIGURATION = {
    .items_count = 10, .slots_obs = 40,
    .container_y_offset = 50, .container_width_margin = 40,
    .item_x_margin = 20, .item_y_margin = 10,
    .case_width = 300, .selection_margin = 12,
    .icon_size = 60, .icon_margin = 10,
    .text_spacing_y = 20
};

static const InventoryConfig INVENTORY_CONFIGURATION = {
    .slots = 12, .max_size = 20,
    .area_x = 10, .area_y = 10,
    .selection_margin = 8, .quality_margin = 2,
    .icon_offset_reduction = 20, .ilvl_text_margin = 15
};

static const ShopColorConfig SHOP_COLORS = {
    .case_bg_r = 20, .case_bg_g = 25, .case_bg_b = 40, .case_bg_a = 240,
    .case_border_r = 100, .case_border_g = 120, .case_border_b = 160,
    .icon_bg_r = 40, .icon_bg_g = 45, .icon_bg_b = 60, .icon_bg_a = 200
};

// ============================================================================
// CALCULATED GAME CONSTANTS
// ============================================================================

typedef struct {
    int map_size;
    int monsters_to_spawn;
    float spawn_density;
} MapCalculationConfig;

static const MapCalculationConfig MAP_CALCULATIONS = {
    .map_size = 1900,  // MAP_WIDTH * MAP_HEIGHT = 50 * 38
    .monsters_to_spawn = 38,  // (int)(1900 * 0.02f)
    .spawn_density = 0.02f
};




typedef struct {
    int fireball, ice_shard, stone_chunk, energy_bolt, dark_orb, melee_strike;
} ProjectileTypes;

static const ProjectileTypes PROJECTILE_TYPES = {
    .fireball = 0, .ice_shard = 1, .stone_chunk = 2,
    .energy_bolt = 3, .dark_orb = 4, .melee_strike = 5
};


// ============================================================================
// RIFT PROGRESSION SYSTEM
// ============================================================================

// Easy-to-tune scaling factors for game balance
#define RIFT_MONSTER_HEALTH_BASE_MULTIPLIER 1.25f    // Health scales by 25% per rift level 
#define RIFT_MONSTER_DAMAGE_BASE_MULTIPLIER 1.18f    // Damage scales by 18% per rift level
#define RIFT_MONSTER_SPEED_BASE_MULTIPLIER 1.05f     // Speed scales by 5% per rift level (reduced from 8%)
#define RIFT_MONSTER_ATTACK_SPEED_MULTIPLIER 0.95f   // Attack cooldown reduces by 5% per rift level (reduced from 8%)

#define RIFT_GOLD_REWARD_BASE 120                    // Base gold for completing Rift 1 (increased from 90)
#define RIFT_GOLD_SCALING_MULTIPLIER 1.12f           // Gold scales by 12% per rift level (reduced from 15%)

// Shop item quality chances by rift tier (percentages)
#define RIFT_TIER_1_MAX 3        // Rifts 1-3: Early game
#define RIFT_TIER_2_MAX 6        // Rifts 4-6: Mid game  
#define RIFT_TIER_3_MAX 10       // Rifts 7-10: Late game
// Rift 11+: End game

// Tier 1 (Rifts 1-3): Learning phase - now with epic chance
#define TIER_1_COMMON_CHANCE 65
#define TIER_1_RARE_CHANCE 30
#define TIER_1_EPIC_CHANCE 5
#define TIER_1_LEGENDARY_CHANCE 0

// Tier 2 (Rifts 4-6): Progression phase - now with legendary chance
#define TIER_2_COMMON_CHANCE 40
#define TIER_2_RARE_CHANCE 45
#define TIER_2_EPIC_CHANCE 13
#define TIER_2_LEGENDARY_CHANCE 2

// Tier 3 (Rifts 7-10): Advanced phase - increased epic/legendary
#define TIER_3_COMMON_CHANCE 25
#define TIER_3_RARE_CHANCE 45
#define TIER_3_EPIC_CHANCE 25
#define TIER_3_LEGENDARY_CHANCE 5

// End game (Rift 11+): Expert phase
#define ENDGAME_COMMON_CHANCE 20
#define ENDGAME_RARE_CHANCE 40
#define ENDGAME_EPIC_CHANCE 35
#define ENDGAME_LEGENDARY_CHANCE 5

// Item level scaling
#define RIFT_ILVL_BASE_MULTIPLIER 0.75f             // ilvl scales at 3/4 rift level for better progression (increased from 0.5f)
#define RIFT_ILVL_RANDOM_RANGE 1                    // ±1 ilvl variation

// Price scaling (items get more expensive in higher rifts)
#define RIFT_PRICE_BASE_MULTIPLIER 1.08f            // Prices scale by 8% per rift level (reduced from 10%)
#define QUALITY_PRICE_MULTIPLIERS {1.0f, 2.5f, 6.0f, 15.0f}  // Common, Rare, Epic, Legendary

// ============================================================================
// TOWN MODE CONSTANTS
// ============================================================================

// Town testing mode
#define TOWN_TESTING_MODE 0         // Set to 1 for town training, 0 for normal gameplay
#define TOWN_TEST_AUTO_GOLD 1000    // Gold given each rift completion in testing mode
#define TOWN_TEST_AUTO_EXP 200      // Experience given each rift completion in testing mode

// Town mode phases  
#define TOWN_MODE_TIME_LIMIT 1200   // TODO: TESTING - 20 seconds for rapid testing (1200 frames = 20s)
#define TOWN_TIMER_WARNING_1 600    // 10 seconds warning
#define TOWN_TIMER_WARNING_2 300    // 5 seconds warning
#define TOWN_INPUT_COOLDOWN 8       // Frames between WASD inputs (prevents rapid navigation)

typedef struct {
    int shop, character;
} TownTabs;

typedef struct {
    int equipment, inventory;
} CharacterModes;

static const TownTabs TOWN_TAB = {
    .shop = 0, .character = 1
};

static const CharacterModes CHARACTER_MODE = {
    .equipment = 0, .inventory = 1
};

typedef struct {
    int none, shoulders, gloves, ring_left, weapon;
    int helmet, armor, belt, pants, boots;
    int amulet, bracers, ring_right, offhand, consumable;
    int ring;
} EquipmentTypes;

typedef struct {
    int common, rare, epic, legendary;
} QualityTypes;

static const EquipmentTypes EQUIPMENT = {
    .none = 0, .shoulders = 1, .gloves = 2, .ring_left = 3, .weapon = 4,
    .helmet = 5, .armor = 6, .belt = 7, .pants = 8, .boots = 9,
    .amulet = 10, .bracers = 11, .ring_right = 12, .offhand = 13, .consumable = 14,
    .ring = 3
};

static const QualityTypes QUALITY = {
    .common = 0, .rare = 1, .epic = 2, .legendary = 3
};

#define EQUIPMENT_SHOULDERS 1
#define EQUIPMENT_GLOVES 2
#define EQUIPMENT_RING_LEFT 3
#define EQUIPMENT_WEAPON 4
#define EQUIPMENT_HELMET 5
#define EQUIPMENT_ARMOR 6
#define EQUIPMENT_BELT 7
#define EQUIPMENT_PANTS 8
#define EQUIPMENT_BOOTS 9
#define EQUIPMENT_AMULET 10
#define EQUIPMENT_BRACERS 11
#define EQUIPMENT_RING_RIGHT 12
#define EQUIPMENT_OFFHAND 13

#define QUALITY_COMMON 0
#define QUALITY_RARE 1
#define QUALITY_EPIC 2
#define QUALITY_LEGENDARY 3

#define MONSTER_ZOMBIE 0
#define MONSTER_MAGE 1
#define MONSTER_HEAVY_MELEE 2
#define MONSTER_LIGHT 3
#define MONSTER_ELITE 4

// Missing combat and world constants
#define MAP_WIDTH 50
#define MAP_HEIGHT 38
#define BOSS_RIFT_GUARDIAN 0
#define BOSS_BASE_HEALTH 100
#define BOSS_BASE_DAMAGE 15
#define BOSS_ATTACK_RANGE 3.0f
#define BOSS_ATTACK_COOLDOWN 45
#define MAX_PROJECTILES 50
#define PROJECTILE_LIFETIME 60
#define PROJECTILE_SPEED 4.0f
#define PROJECTILE_SPEED_HOMING 3.0f
#define PROJECTILE_SPEED_FAST 6.0f
#define PROJECTILE_HIT_RADIUS 1.0f
#define PROJECTILE_FIREBALL 0
#define BLIZZARD_DURATION 60
#define BLIZZARD_DAMAGE 15
#define BLIZZARD_DAMAGE_INTERVAL 15
#define RIFT_COMPLETION_THRESHOLD 0.8f
#define MAX_BLIZZARD_AREAS 10
#define TOWN_TAB_SHOP 0
#define TOWN_TAB_CHARACTER 1
#define MONSTERS_TO_SPAWN 38

// Attack type constants for switch statements
#define ATTACK_TYPE_MELEE 0
#define ATTACK_TYPE_PROJECTILE 1
#define ATTACK_TYPE_CONE_SLAM 2
#define ATTACK_TYPE_FAST_PROJECTILE 3
#define ATTACK_TYPE_HOMING_PROJECTILE 4
#define ATTACK_TYPE_MELEE_PROJECTILE 5
#define ATTACK_TYPE_BOSS_GROUND_SLAM 6
#define ATTACK_TYPE_BOSS_CONE 7

// Phase constants for legacy code
#define PHASE_RIFT 0
#define PHASE_TOWN 1

// Character mode constants
#define CHARACTER_MODE_EQUIPMENT 0
#define CHARACTER_MODE_INVENTORY 1

#define ITEM_GOLD 0
#define ITEM_HEALTH_POTION 1
#define ITEM_MANA_POTION 2

#define TILE_STONE_FLOOR 0
#define TILE_STONE_WALL 1
#define TILE_STONE_DOOR 2
#define TILE_TOWN_FLOOR 3
#define TILE_VENDOR_STALL 4

// Action constants for legacy code
#define ACTION_MOVE_UP 0
#define ACTION_MOVE_DOWN 1
#define ACTION_MOVE_LEFT 2
#define ACTION_MOVE_RIGHT 3
#define ACTION_MOVE_UP_LEFT 4
#define ACTION_MOVE_UP_RIGHT 5
#define ACTION_MOVE_DOWN_LEFT 6
#define ACTION_MOVE_DOWN_RIGHT 7
#define ACTION_BLIZZARD 8
#define ACTION_USE_HEALTH_POTION 9
#define ACTION_USE_MANA_POTION 10
#define ACTION_INTERACT 11
#define ACTION_NOOP 12
#define ACTION_EXIT_TOWN 13
#define ACTION_SWITCH_TO_SHOP 14
#define ACTION_SWITCH_TO_CHARACTER 15
#define ACTION_REROLL_SHOP 16

// Shop constants
#define SHOP_ITEMS_COUNT 10
#define INVENTORY_SLOTS 12

// Town layout positions
#define VENDOR_POSITION_X 25
#define VENDOR_POSITION_Y 19
#define RIFT_PORTAL_X 45
#define RIFT_PORTAL_Y 19
#define INVENTORY_AREA_X 10
#define INVENTORY_AREA_Y 10

// Hero stats
#define MAX_HERO_LEVEL 99
#define STAT_POINTS_PER_LEVEL 2

// Experience constants
#define EXP_PER_MONSTER_KILL 5
#define EXP_PER_BOSS_KILL 50
#define EXP_PER_RIFT_COMPLETION 100

// Town mode rewards - stat-based system
#define TOWN_STAT_INCREASE_REWARD 1.0f      // +1.0 per stat point gained
#define TOWN_ILVL_INCREASE_REWARD 2.0f      // +2.0 per ilvl gained (double stat reward)
#define TOWN_STAT_DECREASE_PENALTY -1.0f    // -1.0 per stat point lost
#define TOWN_ILVL_DECREASE_PENALTY -2.0f    // -2.0 per ilvl lost
#define TOWN_INEFFICIENT_ACTION_PENALTY -1.0f
#define TOWN_NO_PURCHASES_PENALTY -5.0f     // Penalty for exiting town without buying anything

// Health management penalties
#define LOW_HEALTH_THRESHOLD 0.25f           // 25% health threshold
#define LOW_HEALTH_PENALTY -2.0f             // Penalty for not using potion when low health
#define LOW_HEALTH_PENALTY_COOLDOWN 120      // Frames between health penalties (2 seconds)

// ============================================================================
// MATH CONSTANTS
// ============================================================================
#ifndef PI
#define PI 3.14159265358979323846f
#endif

#define ZERO_VALUE 0.0f

#endif // RIFT_CONSTANTS_H