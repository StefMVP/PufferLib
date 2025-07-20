#ifndef RIFT_CONSTANTS_H
#define RIFT_CONSTANTS_H

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
#define ACTION_BLIZZARD 8
#define ACTION_USE_HEALTH_POTION 9
#define ACTION_USE_MANA_POTION 10
#define ACTION_INTERACT 11
#define ACTION_NOOP 12
#define ACTION_BLIZZARD_UP 13
#define ACTION_BLIZZARD_DOWN 14
#define ACTION_BLIZZARD_LEFT 15
#define ACTION_BLIZZARD_RIGHT 16
#define ACTION_BLIZZARD_UP_LEFT 17
#define ACTION_BLIZZARD_UP_RIGHT 18
#define ACTION_BLIZZARD_DOWN_LEFT 19
#define ACTION_BLIZZARD_DOWN_RIGHT 20

// ============================================================================
// MAP AND CELL CONSTANTS
// ============================================================================
#define MAP_WIDTH 50
#define MAP_HEIGHT 38
#define MAP_SIZE (MAP_WIDTH * MAP_HEIGHT)
#define MAP_BORDER_SIZE 5

#define CELL_EMPTY 0
#define CELL_WALL 1
#define CELL_FLOOR 2
#define CELL_DOOR 3
#define CELL_VENDOR 4

// ============================================================================
// PHASE CONSTANTS
// ============================================================================
#define PHASE_RIFT 0
#define PHASE_TOWN 1

// ============================================================================
// MONSTER CONSTANTS
// ============================================================================
#define MONSTER_ZOMBIE 0
#define MONSTER_MAGE 1
#define MONSTER_HEAVY_MELEE 2
#define MONSTER_LIGHT 3
#define MONSTER_ELITE 4

// ============================================================================
// BOSS CONSTANTS
// ============================================================================
#define BOSS_RIFT_GUARDIAN 0

// ============================================================================
// ITEM CONSTANTS
// ============================================================================
#define ITEM_GOLD 0
#define ITEM_HEALTH_POTION 1
#define ITEM_MANA_POTION 2

// ============================================================================
// GAME MECHANICS CONSTANTS
// ============================================================================

// Blizzard mechanics
#define BLIZZARD_ACTIVATION_COOLDOWN 20
#define BLIZZARD_DAMAGE 15
#define BLIZZARD_DAMAGE_INTERVAL 15
#define BLIZZARD_DURATION 60
#define BLIZZARD_MANA_COST 15
#define BLIZZARD_RADIUS 2.0f
#define MAX_BLIZZARD_AREAS 10

// Boss mechanics
#define BOSS_ATTACK_COOLDOWN 15
#define BOSS_ATTACK_DISTANCE 2.0f
#define BOSS_BASE_DAMAGE 15
#define BOSS_BASE_HEALTH 100

// Player mechanics
#define PLAYER_BASE_DAMAGE 10
#define PLAYER_MAX_HEALTH 100
#define PLAYER_MAX_MANA 50
#define MELEE_RANGE 1.0f
#define MOVEMENT_STEP 1.0f

// Monster mechanics
#define MAX_MONSTERS 200
#define MONSTER_ATTACK_COOLDOWN 30
#define MONSTER_BASE_DAMAGE 5
#define MONSTER_BASE_HEALTH 15
#define MONSTER_DETECTION_RANGE 8.0f
#define MONSTER_MOVEMENT_COOLDOWN 5
#define MONSTER_WANDER_CHANCE 20
#define MONSTER_WANDER_COOLDOWN 10
#define MONSTER_ANIM_SPEED 12
#define MONSTERS_TO_SPAWN ((int)(MAP_WIDTH * MAP_HEIGHT * 0.02f))

// Monster type stats
#define ELITE_DAMAGE 20
#define ELITE_HEALTH 15
#define ELITE_RANGE 4.0f
#define ELITE_SPEED 0.4f
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

// Items and potions
#define MAX_ITEMS 100
#define ITEM_PICKUP_DISTANCE 1.0f
#define ITEM_SCAN_DISTANCE 5.0f
#define MAX_INVENTORY_SIZE 20
#define HEALTH_POTION_COOLDOWN 60
#define HEALTH_POTION_HEAL 1000
#define HEALTH_POTION_DROP_CHANCE 70
#define MANA_POTION_COOLDOWN 60
#define MANA_POTION_RESTORE 1000
#define MANA_REGEN_RATE 15

// Gold and vendor
#define GOLD_DROP_CHANCE 40
#define GOLD_DROP_MIN 5
#define GOLD_DROP_RANGE 15
#define VENDOR_HEALTH_POTION_PRICE 20
#define VENDOR_INTERACTION_DISTANCE 2.0f
#define VENDOR_MANA_POTION_PRICE 15
#define VENDOR_STOCK_AMOUNT 10

// Projectiles
#define MAX_PROJECTILES 50
#define PROJECTILE_SPEED 3.0f

// Map generation
#define MAP_SPAWN_BORDER 10
#define MAP_SPAWN_BORDER_OFFSET 20
#define SAFE_SPAWN_RADIUS 15.0f
#define SPAWN_AREA_MARGIN 20
#define SPAWN_BORDER_SIZE 10
#define SPAWN_CHECK_DISTANCE 8.0f
#define WALL_CLUSTER_COUNT 20
#define WALL_CLUSTER_DENSITY 3
#define WALL_CLUSTER_SIZE 2

// Episode and observation
#define MAX_EPISODE_LENGTH 2500
#define GRID_SIZE 10
#define GRID_OBS_SIZE (GRID_SIZE * GRID_SIZE)
#define PLAYER_OBS_SIZE 17
#define OBS_SIZE (PLAYER_OBS_SIZE + GRID_OBS_SIZE)
#define RIFT_COMPLETION_THRESHOLD 0.8f

// Normalization constants
#define COOLDOWN_NORMALIZATION 100.0f
#define DISTANCE_NORMALIZATION 50.0f
#define GOLD_NORMALIZATION 1000.0f
#define FULL_CIRCLE_MULTIPLIER 2.0f
#define TWO_PI_MULTIPLIER 2.0f

// ============================================================================
// RENDERING CONSTANTS
// ============================================================================

// Sprite and tileset
#define SPRITE_SIZE 32
#define TILESET_COLS 8
#define TILESET_ROWS 8

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

// Tile constants
#define TILE_STONE_FLOOR 0
#define TILE_STONE_WALL 1
#define TILE_STONE_DOOR 2
#define TILE_TOWN_FLOOR 3
#define TILE_VENDOR_STALL 4

// Background and border colors
#define BG_COLOR_R 40
#define BG_COLOR_G 35
#define BG_COLOR_B 30
#define BORDER_COLOR_RGB 80
#define CLEAR_BG_R 20
#define CLEAR_BG_G 20
#define CLEAR_BG_B 25

// Player rendering
#define PLAYER_COLOR_R 138
#define PLAYER_COLOR_G 43
#define PLAYER_COLOR_B 226
#define ARMOR_COLOR_R 75
#define ARMOR_COLOR_B 130

// Shadow rendering
#define SHADOW_ALPHA 100
#define SHADOW_OFFSET 2

// Monster colors and sizes
#define ELITE_COLOR_R 220
#define ELITE_COLOR_G 20
#define ELITE_COLOR_B 60
#define ELITE_SIZE_MULT 1.4f
#define HEAVY_COLOR_R 139
#define HEAVY_COLOR_G 69
#define HEAVY_COLOR_B 19
#define HEAVY_SIZE_MULT 1.3f
#define LIGHT_COLOR_R 255
#define LIGHT_COLOR_G 215
#define LIGHT_SIZE_MULT 1.2f
#define MAGE_SIZE_MULT 1.1f
#define ZOMBIE_COLOR_R 34
#define ZOMBIE_COLOR_G 139
#define ZOMBIE_COLOR_B 34
#define MAROON_COLOR_R 128

// Glow effects
#define GLOW_ALPHA 100
#define GLOW_AMPLITUDE 0.2f
#define GLOW_BASE 0.8f
#define GLOW_SPEED 0.1f

// Health bars
#define HEALTH_BAR_HEIGHT 2
#define HEALTH_BAR_OFFSET 5

// Boss rendering
#define BOSS_AURA_ALPHA 100
#define BOSS_CORE_ALPHA 200
#define BOSS_HEALTH_BAR_HEIGHT 6
#define BOSS_HEALTH_BAR_OFFSET 15
#define BOSS_PULSE_AMPLITUDE 0.1f
#define BOSS_PULSE_BASE 0.9f
#define BOSS_SHADOW_ALPHA 150
#define BOSS_SHADOW_OFFSET 3
#define BOSS_SIZE_MULT 1.5f

// Item rendering
#define ITEM_GLOW_SPEED 0.2f
#define ITEM_SIZE_EIGHTH 0.125f
#define ITEM_SIZE_TWELFTH 0.0833f

// Projectile rendering
#define PROJECTILE_FIREBALL 0
#define PROJECTILE_LIFETIME 60
#define PROJECTILE_SIXTH 0.167f
#define CORE_COLOR_R 255
#define CORE_COLOR_G 255
#define FIREBALL_COLOR_R 255
#define FIREBALL_COLOR_G 69
#define FIREBALL_PULSE_AMPLITUDE 0.3f
#define FIREBALL_PULSE_BASE 0.8f
#define FIREBALL_PULSE_SPEED 0.3f
#define HALO_COLOR_R 255
#define HALO_COLOR_G 140
#define PROJECTILE_FADE_SPEED 8
#define PROJECTILE_HALO_OFFSET 3
#define TRAIL_COLOR_R 255
#define TRAIL_COLOR_G 100
#define TRAIL_WIDTH 3
#define TRAIL_LENGTH 10

// Blizzard rendering
#define BLIZZARD_BASE_COLOR_R 200
#define BLIZZARD_BASE_COLOR_G 230
#define BLIZZARD_BASE_COLOR_B 255
#define BLIZZARD_BASE_ALPHA 150
#define FROST_COLOR_R 173
#define FROST_COLOR_G 216
#define FROST_COLOR_B 230
#define FROST_LINE_SIZE 3
#define ICE_SHARD_COUNT 12
#define SHARD_TIMING_MULT 5
#define SHARD_CYCLE_FRAMES 60
#define SHARD_FALL_SPEED 2
#define SHARD_SIZE_MIN 1
#define SHARD_SIZE_RANGE 3

// UI rendering
#define UI_BG_ALPHA 200
#define UI_BG_COLOR_RGB 20
#define UI_HEIGHT 100
#define UI_LINE_COLOR_RGB 60
#define UI_STATS_OFFSET 45
#define UI_STATS_LINE_HEIGHT 18
#define POTION_SIZE 15
#define POTION_SPACING 30
#define GLOBE_RADIUS 25
#define GLOBE_MARGIN 10
#define HEALTH_GLOBE_BG_R 128

// Text sizes
#define TEXT_SIZE_12 12
#define TEXT_SIZE_14 14
#define TEXT_SIZE_16 16
#define TEXT_SIZE_18 18
#define TEXT_SIZE_20 20

// Control display
#define MANUAL_CONTROL_WIDTH 180
#define MANUAL_CONTROL_HEIGHT 25
#define MANUAL_CONTROL_OFFSET 185
#define AI_CONTROL_WIDTH 100

// Vendor rendering
#define VENDOR_OUTER_SIZE 0.4f
#define VENDOR_INNER_SIZE 0.25f
#define VENDOR_TEXT_OFFSET_X 25
#define VENDOR_TEXT_OFFSET_Y 5

// ============================================================================
// MATH CONSTANTS
// ============================================================================
#ifndef PI
#define PI 3.14159265358979323846f
#endif

#define ZERO_VALUE 0.0f

#endif // RIFT_CONSTANTS_H