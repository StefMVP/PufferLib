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
#define ACTION_EXIT_TOWN 13

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
#define BOSS_BASE_DAMAGE 15
#define BOSS_BASE_HEALTH 100

// Player mechanics
#define PLAYER_BASE_DAMAGE 10
#define PLAYER_MAX_HEALTH 100
#define PLAYER_MAX_MANA 50

// Monster mechanics
#define MAX_MONSTERS 200
#define MONSTER_ATTACK_COOLDOWN 30
#define MONSTER_BASE_DAMAGE 5
#define MONSTER_BASE_HEALTH 15
#define MONSTER_DETECTION_RANGE 8.0f
#define MONSTER_MOVEMENT_COOLDOWN 5
#define MONSTER_WANDER_CHANCE 20
#define MONSTER_WANDER_COOLDOWN 10
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
#define MAX_INVENTORY_SIZE 20
#define HEALTH_POTION_COOLDOWN 80
#define HEALTH_POTION_HEAL 1000
#define MANA_POTION_COOLDOWN 80
#define MANA_POTION_RESTORE 1000
#define MANA_REGEN_RATE 15

// Gold and vendor
#define GOLD_DROP_MIN 5
#define GOLD_DROP_RANGE 15
#define VENDOR_HEALTH_POTION_PRICE 20
#define VENDOR_MANA_POTION_PRICE 15
#define VENDOR_STOCK_AMOUNT 10

// Attack types
#define ATTACK_TYPE_MELEE 0
#define ATTACK_TYPE_PROJECTILE 1
#define ATTACK_TYPE_CONE_SLAM 2
#define ATTACK_TYPE_FAST_PROJECTILE 3
#define ATTACK_TYPE_HOMING_PROJECTILE 4

// Projectiles
#define MAX_PROJECTILES 50
#define PROJECTILE_SPEED 4.0f
#define PROJECTILE_SPEED_FAST 6.0f
#define PROJECTILE_SPEED_HOMING 3.0f

// Map generation
#define SPAWN_CHECK_DISTANCE 8.0f

// Episode and observation
#define MAX_EPISODE_LENGTH 3125
#define GRID_SIZE 10
#define GRID_OBS_SIZE (GRID_SIZE * GRID_SIZE)
#define PLAYER_OBS_SIZE 17
#define SHOP_SLOTS_OBS 40          // 10 shop slots × 4 properties (available, quality, price, total_stats)
#define EQUIPMENT_SLOTS_OBS 52     // 13 equipment slots × 4 properties (type, quality, level, total_stats)
#define TOWN_INTERFACE_OBS 9       // Hero stats, rift level, inventory, navigation, current tab
#define TOWN_OBS_SIZE (SHOP_SLOTS_OBS + EQUIPMENT_SLOTS_OBS + TOWN_INTERFACE_OBS)
#define OBS_SIZE (PLAYER_OBS_SIZE + GRID_OBS_SIZE + TOWN_OBS_SIZE)
#define RIFT_COMPLETION_THRESHOLD 0.8f

// Normalization constants
#define DISTANCE_NORMALIZATION 50.0f
#define GOLD_NORMALIZATION 1000.0f
#define FULL_CIRCLE_MULTIPLIER 2.0f

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
#define TOWN_CONTAINER_Y TOP_UI_HEIGHT + 20  // 80
#define TOWN_CONTAINER_WIDTH 920
#define TOWN_CONTAINER_HEIGHT 520

// Content areas within town container
#define CONTENT_MARGIN 20
#define CONTENT_GAP 20
#define CONTENT_AREA_WIDTH 380
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
#define MANUAL_CONTROL_HEIGHT 25


// ============================================================================
// RIFT PROGRESSION SYSTEM
// ============================================================================

// Easy-to-tune scaling factors for game balance
#define RIFT_MONSTER_HEALTH_BASE_MULTIPLIER 1.25f    // Health scales by 25% per rift level
#define RIFT_MONSTER_DAMAGE_BASE_MULTIPLIER 1.20f    // Damage scales by 20% per rift level
#define RIFT_MONSTER_SPEED_BASE_MULTIPLIER 1.05f     // Speed scales by 5% per rift level (subtle)

#define RIFT_GOLD_REWARD_BASE 50                     // Base gold for completing Rift 1  
#define RIFT_GOLD_SCALING_MULTIPLIER 1.05f           // Gold scales by 5% per rift level (much more reasonable)

// Shop item quality chances by rift tier (percentages)
#define RIFT_TIER_1_MAX 3        // Rifts 1-3: Early game
#define RIFT_TIER_2_MAX 6        // Rifts 4-6: Mid game  
#define RIFT_TIER_3_MAX 10       // Rifts 7-10: Late game
// Rift 11+: End game

// Tier 1 (Rifts 1-3): Learning phase
#define TIER_1_COMMON_CHANCE 75
#define TIER_1_RARE_CHANCE 25
#define TIER_1_EPIC_CHANCE 0
#define TIER_1_LEGENDARY_CHANCE 0

// Tier 2 (Rifts 4-6): Progression phase  
#define TIER_2_COMMON_CHANCE 50
#define TIER_2_RARE_CHANCE 45
#define TIER_2_EPIC_CHANCE 5
#define TIER_2_LEGENDARY_CHANCE 0

// Tier 3 (Rifts 7-10): Advanced phase
#define TIER_3_COMMON_CHANCE 30
#define TIER_3_RARE_CHANCE 50
#define TIER_3_EPIC_CHANCE 18
#define TIER_3_LEGENDARY_CHANCE 2

// End game (Rift 11+): Expert phase
#define ENDGAME_COMMON_CHANCE 20
#define ENDGAME_RARE_CHANCE 40
#define ENDGAME_EPIC_CHANCE 35
#define ENDGAME_LEGENDARY_CHANCE 5

// Item level scaling
#define RIFT_ILVL_BASE_MULTIPLIER 1.0f              // ilvl closely tracks rift level
#define RIFT_ILVL_RANDOM_RANGE 2                    // ±2 ilvl variation

// Price scaling (items get more expensive in higher rifts)
#define RIFT_PRICE_BASE_MULTIPLIER 1.15f            // Prices scale by 15% per rift level
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

// Town interface modes
#define TOWN_TAB_CHARACTER 0
#define TOWN_TAB_SHOP 1

#define CHARACTER_MODE_EQUIPMENT 0
#define CHARACTER_MODE_INVENTORY 1

// Equipment types - Diablo 3 warrior layout
#define EQUIPMENT_NONE 0
#define EQUIPMENT_SHOULDERS 1    // Left column top
#define EQUIPMENT_GLOVES 2       // Left column 
#define EQUIPMENT_RING_LEFT 3    // Left column
#define EQUIPMENT_WEAPON 4       // Left column bottom
#define EQUIPMENT_HELMET 5       // Middle column top  
#define EQUIPMENT_ARMOR 6        // Middle column (body armor)
#define EQUIPMENT_BELT 7         // Middle column
#define EQUIPMENT_PANTS 8        // Middle column  
#define EQUIPMENT_BOOTS 9        // Middle column bottom
#define EQUIPMENT_AMULET 10      // Right column top
#define EQUIPMENT_BRACERS 11     // Right column (wrists)
#define EQUIPMENT_RING_RIGHT 12  // Right column
#define EQUIPMENT_OFFHAND 13     // Right column bottom
#define EQUIPMENT_CONSUMABLE 14

// Legacy compatibility constants
#define EQUIPMENT_RING EQUIPMENT_RING_LEFT  // For backward compatibility

// Equipment quality
#define QUALITY_COMMON 0
#define QUALITY_RARE 1
#define QUALITY_EPIC 2
#define QUALITY_LEGENDARY 3

// Shop constants
#define SHOP_ITEMS_COUNT 4
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