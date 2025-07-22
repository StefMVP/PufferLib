#ifndef RIFT_RENDER_CORE_H
#define RIFT_RENDER_CORE_H

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "raylib.h"

typedef struct SpriteSystem SpriteSystem;
typedef struct Rift Rift;
typedef struct Client Client;

typedef struct {
    int width, height;
    int x, y;
} UIRect;

typedef struct {
    int tab_width, tab_height;
    int content_width, content_height;
    UIRect character_tab;
    UIRect shop_tab;
    UIRect content_panel;
    UIRect stats_panel;
} TownLayout;

typedef struct {
    int slot_size;
    int column_spacing, row_spacing;
    int equipment_start_x;
    int left_col_x, middle_col_x, right_col_x;
    int base_y;
} EquipmentLayout;

typedef struct {
    Color background_top, background_bottom;
    Color panel_bg, panel_border;
    Color stats_bg, stats_border;
    Color empty_slot, inactive_border;
    Color quality_text, item_level_text;
    Color selected_glow;
} TownColorScheme;

typedef struct {
    const char* name;
    const char* icon;
    int icon_size;
    int label_x_offset;
} SlotDisplayInfo;

typedef struct {
    uint32_t *type_ptr;
    uint32_t *quality_ptr;
    uint32_t equipment_type;
    int index;
} EquipmentSlotConfig;

static const TownLayout TOWN_UI_LAYOUT = {
    .tab_width = 200, .tab_height = 50,
    .content_width = 400, .content_height = 668,
    .character_tab = {50, 20, 200, 50},
    .shop_tab = {270, 20, 200, 50},
    .content_panel = {50, 80, 924, 668},
    .stats_panel = {470, 80, 454, 668}
};

static const EquipmentLayout EQUIPMENT_UI_LAYOUT = {
    .slot_size = 55,
    .column_spacing = 65, .row_spacing = 75,
    .equipment_start_x = 70,  // CONTENT_AREA_X
    .left_col_x = 85,         // CONTENT_AREA_X + 15 margin
    .middle_col_x = 205,      // 85 + 55 + 65 = 205  
    .right_col_x = 325,       // 205 + 55 + 65 = 325 (total width: 325+55-85 = 295 < 380)
    .base_y = 200
};

typedef struct {
    int slot_size;
    int slots_per_row;
    int slot_spacing_x, slot_spacing_y;
    int start_x, start_y;
} InventoryLayout;

typedef struct {
    int item_height;
    int item_spacing;
    int scroll_container_height;
} ShopLayout;

static const InventoryLayout INVENTORY_UI_LAYOUT = {
    .slot_size = 80,
    .slots_per_row = 4,
    .slot_spacing_x = 90,
    .slot_spacing_y = 90,
    .start_x = 85,
    .start_y = 180
};

static const ShopLayout SHOP_UI_LAYOUT = {
    .item_height = 80,
    .item_spacing = 90,       // item_height + margin
    .scroll_container_height = 400
};

static const TownColorScheme TOWN_COLORS = {
    .background_top = {15, 20, 35, 255},
    .background_bottom = {5, 10, 20, 255},
    .panel_bg = {25, 30, 45, 240},
    .panel_border = {100, 120, 150, 255},
    .stats_bg = {25, 20, 15, 240},
    .stats_border = {150, 120, 80, 255},
    .empty_slot = {80, 80, 80, 255},
    .inactive_border = {80, 80, 80, 255},
    .quality_text = {255, 215, 0, 255},
    .item_level_text = {200, 200, 200, 255},
    .selected_glow = {255, 255, 0, 255}
};

typedef struct {
    Color label, stat_color, divider;
    Color empty_text, disabled_text;
    Color slot_bg, slot_border;
    Color character_active, character_glow;
    Color shop_active, shop_glow;
    Color inactive_tab;
} ExtendedTownColors;

static const ExtendedTownColors EXTENDED_TOWN_COLORS = {
    .label = {180, 180, 180, 255},
    .stat_color = {160, 160, 160, 255},
    .divider = {100, 80, 50, 255},
    .empty_text = {100, 100, 100, 255},
    .disabled_text = {150, 150, 150, 255},
    .slot_bg = {30, 35, 50, 220},
    .slot_border = {100, 120, 150, 255},
    .character_active = {20, 80, 40, 220},
    .character_glow = {0, 255, 100, 255},
    .shop_active = {20, 40, 80, 220},
    .shop_glow = {100, 150, 255, 255},
    .inactive_tab = {40, 40, 40, 180}
};

typedef struct {
    Color background;
    Color player, armor;
    Color zombie, mage, heavy_melee, light, elite;
    Color maroon_default;
} RiftColorScheme;

typedef struct {
    Color core, fireball_base, halo, trail;
    Color blizzard_base, frost;
} EffectColorScheme;

typedef struct {
    Color common_gray, rare_blue, epic_purple, legendary_orange;
    Color light_silver, silver;
    Color brown_handle_light, brown_handle_dark;
    Color brown_leather;
} MaterialColorScheme;

static const RiftColorScheme RIFT_COLORS = {
    .background = {40, 35, 30, 255},
    .player = {138, 43, 226, 255},
    .armor = {75, 0, 130, 255},
    .zombie = {34, 139, 34, 255},
    .mage = {138, 43, 226, 255},
    .heavy_melee = {139, 69, 19, 255},
    .light = {255, 215, 0, 255},
    .elite = {220, 20, 60, 255},
    .maroon_default = {128, 0, 0, 255}
};

static const EffectColorScheme EFFECT_COLORS = {
    .core = {255, 255, 255, 255},
    .fireball_base = {255, 69, 0, 255},
    .halo = {255, 140, 0, 255},
    .trail = {255, 100, 0, 255},
    .blizzard_base = {200, 230, 255, 255},
    .frost = {173, 216, 230, 255}
};

static const MaterialColorScheme MATERIAL_COLORS = {
    .common_gray = {128, 128, 128, 255},
    .rare_blue = {100, 149, 237, 255},
    .epic_purple = {163, 53, 238, 255},
    .legendary_orange = {255, 128, 0, 255},
    .light_silver = {192, 192, 192, 255},
    .silver = {169, 169, 169, 255},
    .brown_handle_light = {139, 115, 85, 255},
    .brown_handle_dark = {101, 67, 33, 255},
    .brown_leather = {139, 69, 19, 255}
};

typedef struct {
    int base_damage, max_health, max_mana;
} PlayerStats;

typedef struct {
    int damage, health;
    float range, speed;
} MonsterTypeStats;

typedef struct {
    int max_count, attack_cooldown, movement_cooldown;
    int base_damage, base_health;
    float detection_range;
    int wander_chance, wander_cooldown;
    MonsterTypeStats zombie, mage, heavy_melee, light, elite;
} MonsterConfig;

typedef struct {
    int health_cooldown, health_heal;
    int mana_cooldown, mana_restore;
    int max_inventory_size;
} ItemConfig;

static const PlayerStats PLAYER_CONFIG = {
    .base_damage = 10,
    .max_health = 100,
    .max_mana = 50
};

static const MonsterConfig MONSTER_CONFIG = {
    .max_count = 200,
    .attack_cooldown = 30,
    .movement_cooldown = 5,
    .base_damage = 5,
    .base_health = 15,
    .detection_range = 8.0f,
    .wander_chance = 20,
    .wander_cooldown = 10,
    .zombie = {5, 15, 1.0f, 0.3f},
    .mage = {8, 10, 6.0f, 0.3f},
    .heavy_melee = {15, 15, 1.5f, 0.2f},
    .light = {3, 8, 2.0f, 0.8f},
    .elite = {20, 15, 4.0f, 0.4f}
};

static const ItemConfig ITEM_CONFIG = {
    .health_cooldown = 80,
    .health_heal = 1000,
    .mana_cooldown = 80,
    .mana_restore = 50,
    .max_inventory_size = 20
};

typedef struct {
    float size_multiplier;
    int health_bar_height, health_bar_offset;
    float pulse_base, pulse_amplitude;
    int shadow_alpha, shadow_offset;
    int core_alpha;
} BossRenderConfig;

typedef struct {
    int lifetime;
    float pulse_base, pulse_amplitude, pulse_speed;
    int fade_speed, halo_offset;
    int trail_width, trail_length;
} ProjectileRenderConfig;

typedef struct {
    float glow_speed;
    int base_alpha;
} ItemRenderConfig;

static const BossRenderConfig BOSS_RENDER = {
    .size_multiplier = 1.5f,
    .health_bar_height = 6,
    .health_bar_offset = 15,
    .pulse_base = 0.9f,
    .pulse_amplitude = 0.1f,
    .shadow_alpha = 150,
    .shadow_offset = 3,
    .core_alpha = 200
};

static const ProjectileRenderConfig PROJECTILE_RENDER = {
    .lifetime = 60,
    .pulse_base = 0.8f,
    .pulse_amplitude = 0.3f,
    .pulse_speed = 0.3f,
    .fade_speed = 8,
    .halo_offset = 3,
    .trail_width = 3,
    .trail_length = 10
};

static const ItemRenderConfig ITEM_RENDER = {
    .glow_speed = 0.2f,
    .base_alpha = 150
};

static const SlotDisplayInfo EQUIPMENT_SLOT_INFO[13] = {
    {"SHOULDERS", "SHLD", 25, -2},
    {"GLOVES", "GLVS", 25, 5},
    {"RING L", "RNGL", 20, 8},
    {"WEAPON", "WEAP", 25, 5},
    {"HELMET", "HELM", 25, 5},
    {"ARMOR", "ARMR", 25, 8},
    {"BELT", "BELT", 25, 18},
    {"PANTS", "PNTS", 25, 8},
    {"BOOTS", "BOOT", 25, 8},
    {"AMULET", "AMUL", 20, 2},
    {"BRACERS", "BRAC", 25, -2},
    {"RING R", "RNGR", 20, 5},
    {"OFFHAND", "OFFH", 25, -2}
};

typedef struct {
    int col, row;
    uint32_t equipment_type;
} SlotPosition;

static const SlotPosition EQUIPMENT_SLOT_POSITIONS[13] = {
    {0, 0, EQUIPMENT_SHOULDERS}, {0, 1, EQUIPMENT_GLOVES}, {0, 2, EQUIPMENT_RING_LEFT}, {0, 3, EQUIPMENT_WEAPON},
    {1, 0, EQUIPMENT_HELMET}, {1, 1, EQUIPMENT_ARMOR}, {1, 2, EQUIPMENT_BELT}, {1, 3, EQUIPMENT_PANTS}, {1, 4, EQUIPMENT_BOOTS},
    {2, 0, EQUIPMENT_AMULET}, {2, 1, EQUIPMENT_BRACERS}, {2, 2, EQUIPMENT_RING_RIGHT}, {2, 3, EQUIPMENT_OFFHAND}
};

static inline EquipmentSlotConfig get_equipment_slot_config(Rift* env, int slot_index) {
    uint32_t* equipment_base = (uint32_t*)&env->equipment;
    uint32_t slot_offset = slot_index * 7;
    
    return (EquipmentSlotConfig){
        .type_ptr = &equipment_base[slot_offset],
        .quality_ptr = &equipment_base[slot_offset + 1],
        .equipment_type = EQUIPMENT_SLOT_POSITIONS[slot_index].equipment_type,
        .index = slot_index
    };
}

static inline Color darken_color(Color color, int divisor) {
    return (Color){color.r/divisor, color.g/divisor, color.b/divisor, color.a};
}

static inline Color brighten_color(Color color, int amount) {
    return (Color){
        color.r + amount > 255 ? 255 : color.r + amount,
        color.g + amount > 255 ? 255 : color.g + amount,
        color.b + amount > 255 ? 255 : color.b + amount,
        color.a
    };
}

static inline Color fade_color(Color color, uint8_t alpha) {
    return (Color){color.r, color.g, color.b, alpha};
}

static inline Color GetQualityColor(uint32_t quality) {
    switch (quality) {
        case QUALITY_COMMON: return MATERIAL_COLORS.common_gray;
        case QUALITY_RARE: return MATERIAL_COLORS.rare_blue;
        case QUALITY_EPIC: return MATERIAL_COLORS.epic_purple;
        case QUALITY_LEGENDARY: return MATERIAL_COLORS.legendary_orange;
        default: return (Color){64, 64, 64, 255};
    }
}

static inline void draw_gradient_rect(int x, int y, int width, int height, Color color) {
    DrawRectangleGradientV(x, y, width, height, color, darken_color(color, 2));
}

static inline void draw_gem(int x, int y, int size, Color color) {
    DrawCircle(x, y, size, color);
    DrawCircle(x, y, size - 2, (Color){COLOR_WHITE, COLOR_WHITE, COLOR_WHITE, COLOR_WHITE_TRANSLUCENT});
}

static inline void draw_shadow(float x, float y, float radius, int offset, int alpha) {
    DrawCircle(x + offset, y + offset, radius, (Color){0, 0, 0, alpha});
}

static inline void draw_health_bar(int x, int y, int width, int height, float health_ratio) {
    DrawRectangle(x, y, width, height, RED);
    DrawRectangle(x, y, (int)(width * health_ratio), height, GREEN);
    DrawRectangleLines(x, y, width, height, WHITE);
}

static inline float calculate_glow(int tick, float speed, float offset) {
    return GLOW_BASE + GLOW_AMPLITUDE * sinf(tick * speed + offset);
}

static inline float calculate_pulse(int tick, float speed, float base, float amplitude) {
    return base + amplitude * sinf(tick * speed);
}

static inline void draw_sprite_tile(Texture2D texture, uint32_t tile_id, float dest_x, float dest_y, float dest_size) {
    uint32_t tile_x = tile_id % TILESET_COLS;
    uint32_t tile_y = tile_id / TILESET_COLS;
    
    Rectangle source = {tile_x * SPRITE_SIZE, tile_y * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE};
    Rectangle dest = {dest_x, dest_y, dest_size, dest_size};
    
    DrawTexturePro(texture, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
}

static inline float get_monster_size_multiplier(uint32_t monster_type) {
    switch (monster_type) {
        case MONSTER_MAGE: return MAGE_SIZE_MULT;
        case MONSTER_HEAVY_MELEE: return HEAVY_SIZE_MULT;
        case MONSTER_LIGHT: return LIGHT_SIZE_MULT;
        case MONSTER_ELITE: return ELITE_SIZE_MULT;
        default: return 1.0f;
    }
}

static inline Color get_monster_color(uint32_t monster_type) {
    switch (monster_type) {
        case MONSTER_ZOMBIE: return RIFT_COLORS.zombie;
        case MONSTER_MAGE: return RIFT_COLORS.mage;
        case MONSTER_HEAVY_MELEE: return RIFT_COLORS.heavy_melee;
        case MONSTER_LIGHT: return RIFT_COLORS.light;
        case MONSTER_ELITE: return RIFT_COLORS.elite;
        default: return RIFT_COLORS.maroon_default;
    }
}

static inline Color get_item_color(uint32_t item_type) {
    switch (item_type) {
        case ITEM_GOLD: return GOLD;
        case ITEM_HEALTH_POTION: return RED;
        case ITEM_MANA_POTION: return BLUE;
        default: return WHITE;
    }
}

static inline Color get_item_tint(uint32_t item_type) {
    switch (item_type) {
        case ITEM_GOLD: return (Color){255, 255, 180, 255};
        case ITEM_HEALTH_POTION: return (Color){255, 200, 200, 255};
        case ITEM_MANA_POTION: return (Color){200, 200, 255, 255};
        default: return WHITE;
    }
}

static inline void draw_simple_monster(Vector2 pos, float size, uint32_t monster_type, int tick) {
    float size_mult = get_monster_size_multiplier(monster_type);
    Color color = get_monster_color(monster_type);
    
    float radius = size * 0.25f * size_mult;
    float inner_radius = size * 0.167f * size_mult;
    
    draw_shadow(pos.x, pos.y, radius, 1, SHADOW_ALPHA);
    DrawCircle(pos.x, pos.y, radius, color);
    DrawCircle(pos.x, pos.y, inner_radius, (Color){255, 0, 0, 150});
    
    if (monster_type == MONSTER_ELITE) {
        float glow = calculate_glow(tick, GLOW_SPEED, 0.0f);
        DrawCircleLines(pos.x, pos.y, size * 0.33f * size_mult * glow, (Color){255, 255, 255, GLOW_ALPHA});
    }
}

static inline void draw_simple_boss(Vector2 pos, float size, int tick) {
    float radius = size * 0.5f * BOSS_RENDER.size_multiplier;
    float inner_radius = size * 0.33f * BOSS_RENDER.size_multiplier;
    
    draw_shadow(pos.x, pos.y, radius, BOSS_RENDER.shadow_offset, BOSS_RENDER.shadow_alpha);
    
    float pulse = calculate_pulse(tick, GLOW_SPEED, BOSS_RENDER.pulse_base, BOSS_RENDER.pulse_amplitude);
    DrawCircle(pos.x, pos.y, radius * pulse, RIFT_COLORS.armor);
    DrawCircle(pos.x, pos.y, inner_radius, RIFT_COLORS.player);
    DrawCircle(pos.x, pos.y, size / 6.0f, (Color){255, 255, 255, BOSS_RENDER.core_alpha});
    
    DrawCircleLines(pos.x, pos.y, size * BOSS_RENDER.size_multiplier, fade_color(RIFT_COLORS.armor, 100));
}

static inline void draw_simple_item(Vector2 pos, float size, uint32_t item_type, int tick, int index) {
    Color color = get_item_color(item_type);
    float glow = calculate_glow(tick, ITEM_RENDER.glow_speed, index);
    
    DrawCircle(pos.x, pos.y, size * 0.125f * glow, color);
    DrawCircle(pos.x, pos.y, size * 0.083f, WHITE);
}

static inline void DrawTextRelative(const char* text, int16_t container_x, int16_t container_y, int16_t offset_x, int16_t offset_y, uint8_t font_size, Color color) {
    DrawText(text, container_x + offset_x, container_y + offset_y, font_size, color);
}

static inline void draw_weapon(int16_t x, int16_t y, uint8_t size, Color color) {
    uint8_t blade_width = size / 4;
    uint8_t blade_height = size * 3 / 4;
    uint8_t handle_height = size / 3;
    
    draw_gradient_rect(x + size/2 - blade_width/2, y + 2, blade_width, blade_height, color);
    DrawRectangle(x + size/2 - 1, y + 2, 2, blade_height, (Color){255, 255, 255, 64});
    DrawRectangle(x + size/4, y + blade_height, size/2, 2, color);
    DrawRectangleGradientV(x + size/2 - 3, y + blade_height + 2, 6, handle_height - 2, 
                          MATERIAL_COLORS.brown_handle_light, MATERIAL_COLORS.brown_handle_dark);
    DrawCircle(x + size/2, y + size - 4, 2, color);
}

static inline void draw_ring(int16_t x, int16_t y, uint8_t size, Color color) {
    uint8_t ring_radius = size / 3;
    uint8_t gem_size = size / 6;
    
    DrawCircleLines(x + size/2, y + size/2, ring_radius, color);
    DrawCircleLines(x + size/2, y + size/2, ring_radius - 2, color);
    draw_gem(x + size/2, y + size/2 - ring_radius + 4, gem_size, color);
}

static inline void draw_amulet(int16_t x, int16_t y, uint8_t size, Color color) {
    int16_t chain_y = y + 4;
    uint8_t pendant_size = size / 3;
    
    for (uint8_t i = 0; i < size - 8; i += 3) {
        DrawCircle(x + size/2, chain_y + i, 1, MATERIAL_COLORS.silver);
    }
    draw_gem(x + size/2, y + size - pendant_size/2 - 4, pendant_size/2, color);
}

static inline void draw_shield(int16_t x, int16_t y, uint8_t size, Color color) {
    uint8_t shield_radius = size / 3;
    
    DrawCircle(x + size/2, y + size/2, shield_radius, color);
    DrawCircle(x + size/2, y + size/2, shield_radius - 2, darken_color(color, 2));
    draw_gem(x + size/2, y + size/2, shield_radius/3, color);
}

static inline void draw_armor_piece(int16_t x, int16_t y, uint8_t size, Color color) {
    int16_t width = size * 4 / 5;
    int16_t height = size * 3 / 4;
    int16_t start_x = x + (size - width) / 2;
    int16_t start_y = y + size / 6;
    
    draw_gradient_rect(start_x, start_y, width, height, color);
    DrawRectangle(x + size/2 - 2, start_y, 4, height, brighten_color(color, 50));
    
    DrawRectangle(start_x - 1, start_y + height / 4, 2, height / 2, darken_color(color, 2));
    DrawRectangle(start_x + width - 1, start_y + height / 4, 2, height / 2, darken_color(color, 2));
    
    for (uint8_t i = 0; i < 3; i++) {
        DrawCircle(start_x + 4, start_y + 8 + i * 6, 1, brighten_color(color, 80));
        DrawCircle(start_x + width - 4, start_y + 8 + i * 6, 1, brighten_color(color, 80));
    }
}

static inline void draw_boots(int16_t x, int16_t y, uint8_t size, Color color) {
    int16_t boot_width = size * 3 / 4;
    int16_t boot_height = size * 2 / 3;
    int16_t boot_x = x + (size - boot_width) / 2;
    int16_t boot_y = y + size / 4;
    
    draw_gradient_rect(boot_x, boot_y, boot_width, boot_height, color);
    DrawRectangleGradientV(boot_x + 3, boot_y, boot_width - 6, boot_height / 2, 
                          brighten_color(color, 60), brighten_color(color, 30));
    DrawRectangle(boot_x, boot_y + boot_height / 2, boot_width, boot_height / 3, 
                  darken_color(color, 3));
}

static inline void draw_helmet(int16_t x, int16_t y, uint8_t size, Color color) {
    int16_t helm_width = size * 4 / 5;
    int16_t helm_height = size * 2 / 3;
    int16_t helm_x = x + (size - helm_width) / 2;
    int16_t helm_y = y + size / 6;
    
    DrawCircleGradient(x + size/2, helm_y + helm_height / 3, helm_width / 2, 
                       brighten_color(color, 40), color);
    DrawRectangleGradientV(helm_x, helm_y + helm_height / 3, helm_width, helm_height / 2, 
                           color, darken_color(color, 2));
    DrawRectangle(helm_x + 6, helm_y + helm_height / 2, helm_width - 12, 4, 
                  darken_color(color, 4));
    
    for (uint8_t i = 0; i < 3; i++) {
        DrawRectangle(helm_x + 8 + i * 6, helm_y + helm_height / 2 + 2, 2, 2, 
                      (Color){0, 0, 0, 255});
    }
}

static inline void draw_belt(int16_t x, int16_t y, uint8_t size, Color color) {
    int16_t belt_width = size * 4 / 5;
    uint8_t belt_height = size / 4;
    int16_t belt_x = x + (size - belt_width) / 2;
    int16_t belt_y = y + size / 3;
    
    DrawRectangleGradientV(belt_x, belt_y, belt_width, belt_height, 
                           MATERIAL_COLORS.brown_leather, MATERIAL_COLORS.brown_handle_dark);
    
    uint8_t buckle_size = 6;
    DrawRectangle(x + size/2 - buckle_size/2, belt_y + (belt_height - buckle_size)/2, 
                  buckle_size, buckle_size, MATERIAL_COLORS.silver);
}

static inline void draw_equipment_simple(int16_t x, int16_t y, uint8_t size, Color color, uint32_t equipment_type) {
    switch (equipment_type) {
        case EQUIPMENT_WEAPON: draw_weapon(x, y, size, color); break;
        case EQUIPMENT_RING_LEFT:
        case EQUIPMENT_RING_RIGHT: draw_ring(x, y, size, color); break;
        case EQUIPMENT_AMULET: draw_amulet(x, y, size, color); break;
        case EQUIPMENT_OFFHAND: draw_shield(x, y, size, color); break;
        case EQUIPMENT_HELMET: draw_helmet(x, y, size, color); break;
        case EQUIPMENT_BOOTS: draw_boots(x, y, size, color); break;
        case EQUIPMENT_BELT: draw_belt(x, y, size, color); break;
        case EQUIPMENT_ARMOR:
        case EQUIPMENT_SHOULDERS:
        case EQUIPMENT_GLOVES:
        case EQUIPMENT_BRACERS:
        case EQUIPMENT_PANTS: draw_armor_piece(x, y, size, color); break;
        default: DrawRectangle(x + size/4, y + size/4, size/2, size/2, color); break;
    }
}

static inline float get_ui_pulse(float speed, float base, float amplitude) {
    return base + amplitude * sinf(GetTime() * speed);
}

static inline void draw_ui_tab(int x, int y, int width, int height, Color tab_color, Color border_color, Color glow_color, bool active, float pulse) {
    if (active) {
        DrawRectangle(x - 5, y - 5, width + 10, height + 10, glow_color);
    }
    draw_gradient_rect(x, y, width, height, tab_color);
    DrawRectangleLines(x, y, width, height, border_color);
    DrawRectangleLines(x - 1, y - 1, width + 2, height + 2, border_color);
}

static inline void draw_content_panel(int x, int y, int width, int height, Color content_bg) {
    Color shadow = {0, 0, 0, 150};
    DrawRectangle(x + 5, y + 5, width, height, shadow);
    DrawRectangle(x + 4, y + 4, width, height, shadow);
    draw_gradient_rect(x, y, width, height, content_bg);
    
    Color border_main = {100, 120, 150, 255};
    DrawRectangleLines(x, y, width, height, border_main);
    DrawRectangleLines(x - 1, y - 1, width + 2, height + 2, darken_color(border_main, 2));
    DrawRectangleLines(x + 1, y + 1, width - 2, height - 2, brighten_color(border_main, 50));
}

static inline void draw_stats_panel(int x, int y, int width, int height) {
    draw_gradient_rect(x, y, width, height, (Color){25, 20, 15, 240});
    DrawRectangleLines(x, y, width, height, (Color){150, 120, 80, 255});
}

static inline void draw_item_icon(int x, int y, int size, Color quality_color) {
    draw_gradient_rect(x, y, size, size, (Color){40, 35, 30, 200});
    DrawRectangleLines(x, y, size, size, quality_color);
}

static inline void DrawStatsText(const char* text, int16_t stats_y, int16_t line_offset, uint8_t font_size, Color color, uint8_t indent) {
    int16_t x = STATS_PANEL_X + (indent ? STATS_CONTENT_MARGIN_INDENT : STATS_CONTENT_MARGIN_LEFT);
    int16_t y = stats_y + STATS_CONTENT_MARGIN_LEFT + line_offset;
    DrawText(text, x, y, font_size, color);
}

static inline Vector2 grid_to_screen(float grid_x, float grid_y, float cell_size) {
    return (Vector2){
        grid_x * cell_size,
        grid_y * cell_size
    };
}

static inline uint32_t alpha_from_ratio(float ratio) {
    return (uint32_t)(255.0f * ratio);
}

static inline void draw_tile_base(Image* img, Rectangle tile_rect, Color base_color) {
    ImageDrawRectangleRec(img, tile_rect, base_color);
}

static inline void draw_stone_grid(Image* img, uint32_t tile_x, uint32_t tile_y, uint32_t grid_size, uint32_t stone_size, Color highlight, Color grout, int crack_chance) {
    for (uint32_t gx = 0; gx < grid_size; gx++) {
        for (uint32_t gy = 0; gy < grid_size; gy++) {
            uint16_t stone_x = tile_x * SPRITE_SIZE + gx * stone_size;
            uint16_t stone_y = tile_y * SPRITE_SIZE + gy * stone_size;
            
            ImageDrawRectangle(img, stone_x + 1, stone_y + 1, stone_size - 2, stone_size - 2, highlight);
            Rectangle stone_rect = {stone_x, stone_y, stone_size, stone_size};
            ImageDrawRectangleLines(img, stone_rect, 1, grout);
            
            if (rand() % crack_chance == 0) {
                uint16_t crack_x = stone_x + (rand() % (stone_size - 2)) + 1;
                uint16_t crack_y = stone_y + (rand() % (stone_size - 2)) + 1;
                ImageDrawPixel(img, crack_x, crack_y, grout);
            }
        }
    }
}

static inline void draw_brick_pattern(Image* img, uint32_t tile_x, uint32_t tile_y, Color brick_color, Color mortar_color) {
    for (uint32_t bx = 0; bx < 2; bx++) {
        for (uint32_t by = 0; by < 4; by++) {
            uint16_t brick_x = tile_x * SPRITE_SIZE + bx * 16 + (by % 2) * 8;
            uint16_t brick_y = tile_y * SPRITE_SIZE + by * 8;
            
            if ((uint32_t)(brick_x + 12) <= (uint32_t)(tile_x * SPRITE_SIZE + SPRITE_SIZE)) {
                ImageDrawRectangle(img, brick_x, brick_y, 12, 6, brick_color);
                Rectangle brick_rect = {brick_x, brick_y, 12, 6};
                ImageDrawRectangleLines(img, brick_rect, 1, mortar_color);
            }
        }
    }
}

static inline void draw_random_cobbles(Image* img, uint32_t tile_x, uint32_t tile_y, uint32_t count, Color stone_color, Color mortar_color) {
    for (uint32_t i = 0; i < count; i++) {
        uint32_t stone_size = 6 + rand() % 4;
        uint16_t sx = tile_x * SPRITE_SIZE + (rand() % (SPRITE_SIZE - stone_size));
        uint16_t sy = tile_y * SPRITE_SIZE + (rand() % (SPRITE_SIZE - stone_size));
        
        ImageDrawRectangle(img, sx, sy, stone_size, stone_size, stone_color);
        Rectangle cobble_rect = {sx, sy, stone_size, stone_size};
        ImageDrawRectangleLines(img, cobble_rect, 1, mortar_color);
    }
}

static Client* make_client(Rift* env) {
    Client* client = (Client*)calloc(1, sizeof(Client));
    client->cell_size = CLIENT_CELL_SIZE;
    client->width = CLIENT_WIDTH;
    client->height = CLIENT_HEIGHT;
    
    InitWindow(client->width, client->height, "PufferLib Rift");
    SetTargetFPS(CLIENT_FPS);
    
    client->camera.target = (Vector2){MAP_WIDTH * client->cell_size / 2, MAP_HEIGHT * client->cell_size / 2};
    client->camera.offset = (Vector2){client->width >> 1, client->height >> 1};
    client->camera.rotation = 0;
    client->camera.zoom = 1.0f;
    
    load_sprites(&client->sprites);
    
    return client;
}

void load_sprites(SpriteSystem* sprites) {
    sprites->tile_size = SPRITE_SIZE;
    sprites->hero_frame = 0;
    sprites->hero_animation_timer = 0;
    sprites->hero_animation_state = HERO_ANIM_IDLE;
    
    Image tileset_img = GenImageColor(TILESET_COLS * SPRITE_SIZE, TILESET_ROWS * SPRITE_SIZE, (Color){0, 0, 0, 255});
    
    for (uint32_t y = 0; y < TILESET_ROWS; y++) {
        for (uint32_t x = 0; x < TILESET_COLS; x++) {
            Rectangle tile_rect = {x * SPRITE_SIZE, y * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE};
            uint32_t tile_type = y * TILESET_COLS + x;
            
            switch (tile_type) {
                case TILE_STONE_FLOOR: {
                    Color base_color = (Color){72, 61, 50, 255};
                    Color grout_color = (Color){56, 47, 38, 255};
                    Color highlight_color = (Color){88, 75, 62, 255};
                    
                    draw_tile_base(&tileset_img, tile_rect, base_color);
                    draw_stone_grid(&tileset_img, x, y, 4, 8, highlight_color, grout_color, 4);
                    break;
                }
                case TILE_STONE_WALL: {
                    Color wall_color = (Color){65, 55, 45, 255};
                    Color mortar_color = (Color){45, 35, 25, 255};
                    Color highlight_color = (Color){85, 75, 65, 255};
                    
                    draw_tile_base(&tileset_img, tile_rect, wall_color);
                    draw_brick_pattern(&tileset_img, x, y, highlight_color, mortar_color);
                    ImageDrawRectangleLines(&tileset_img, tile_rect, 1, mortar_color);
                    break;
                }
                case TILE_STONE_DOOR: {
                    Color door_color = (Color){101, 67, 33, 255};
                    Color frame_color = (Color){85, 75, 65, 255};
                    Color handle_color = (Color){139, 115, 85, 255};
                    
                    ImageDrawRectangleRec(&tileset_img, tile_rect, frame_color);
                    
                    Rectangle door_panel = {x * SPRITE_SIZE + 4, y * SPRITE_SIZE + 2, 24, 28};
                    ImageDrawRectangleRec(&tileset_img, door_panel, door_color);
                    
                    ImageDrawRectangle(&tileset_img, x * SPRITE_SIZE + 6, y * SPRITE_SIZE + 4, 20, 4, handle_color);
                    ImageDrawRectangle(&tileset_img, x * SPRITE_SIZE + 6, y * SPRITE_SIZE + 12, 20, 4, handle_color);
                    ImageDrawRectangle(&tileset_img, x * SPRITE_SIZE + 6, y * SPRITE_SIZE + 20, 20, 4, handle_color);
                    
                    ImageDrawCircle(&tileset_img, x * SPRITE_SIZE + 22, y * SPRITE_SIZE + 16, 2, handle_color);
                    break;
                }
                case TILE_TOWN_FLOOR: {
                    Color cobble_color = (Color){88, 82, 70, 255};
                    Color mortar_color = (Color){65, 60, 50, 255};
                    Color wear_color = (Color){95, 88, 75, 255};
                    
                    draw_tile_base(&tileset_img, tile_rect, cobble_color);
                    draw_random_cobbles(&tileset_img, x, y, 6, wear_color, mortar_color);
                    break;
                }
                case TILE_VENDOR_STALL: {
                    Color wood_color = (Color){101, 67, 33, 255};
                    Color dark_wood = (Color){85, 55, 25, 255};
                    Color cloth_color = (Color){139, 115, 85, 255};
                    
                    ImageDrawRectangleRec(&tileset_img, tile_rect, wood_color);
                    
                    for (uint32_t plank = 0; plank < 4; plank++) {
                        uint16_t plank_y = y * SPRITE_SIZE + plank * 8;
                        ImageDrawRectangle(&tileset_img, x * SPRITE_SIZE, plank_y, SPRITE_SIZE, 6, 
                                         (plank % 2) ? wood_color : dark_wood);
                        ImageDrawLine(&tileset_img, x * SPRITE_SIZE, plank_y + 6, 
                                    x * SPRITE_SIZE + SPRITE_SIZE, plank_y + 6, dark_wood);
                    }
                    
                    ImageDrawRectangle(&tileset_img, x * SPRITE_SIZE + 4, y * SPRITE_SIZE + 4, 24, 8, cloth_color);
                    break;
                }
                default: {
                    Color dirt_color = (Color){62, 52, 42, 255};
                    Color rock_color = (Color){75, 65, 55, 255};
                    
                    ImageDrawRectangleRec(&tileset_img, tile_rect, dirt_color);
                    
                    for (uint32_t i = 0; i < 12; i++) {
                        uint16_t rx = x * SPRITE_SIZE + (rand() % SPRITE_SIZE);
                        uint16_t ry = y * SPRITE_SIZE + (rand() % SPRITE_SIZE);
                        uint32_t size = 1 + rand() % 3;
                        ImageDrawRectangle(&tileset_img, rx, ry, size, size, rock_color);
                    }
                    break;
                }
            }
        }
    }
    
    sprites->tileset = LoadTextureFromImage(tileset_img);
    UnloadImage(tileset_img);
    
    Image hero_img = GenImageColor(SPRITE_SIZE * 4, SPRITE_SIZE, (Color){0, 0, 0, 0});
    for (uint32_t frame = 0; frame < 4; frame++) {
        int x = frame * SPRITE_SIZE;
        ImageDrawCircle(&hero_img, x + SPRITE_SIZE/2, SPRITE_SIZE/2, 12, (Color){138, 43, 226, 255});
        ImageDrawCircle(&hero_img, x + SPRITE_SIZE/2, SPRITE_SIZE/2, 8, (Color){75, 0, 130, 255});
        ImageDrawCircle(&hero_img, x + SPRITE_SIZE/2, SPRITE_SIZE/2 - 4, 4, (Color){255, 220, 177, 255});
        
        float staff_offset = sinf(frame * 0.3f) * 2;
        ImageDrawLine(&hero_img, x + SPRITE_SIZE/2 - 6, SPRITE_SIZE/2 + 4, 
                     x + SPRITE_SIZE/2 - 8, SPRITE_SIZE - 4 + staff_offset, (Color){139, 69, 19, 255});
        ImageDrawCircle(&hero_img, x + SPRITE_SIZE/2 - 8, SPRITE_SIZE - 6 + staff_offset, 2, (Color){0, 0, 255, 255});
    }
    sprites->hero_idle = LoadTextureFromImage(hero_img);
    UnloadImage(hero_img);
    
    Image walk_img = GenImageColor(SPRITE_SIZE * 8, SPRITE_SIZE, (Color){0, 0, 0, 0});
    for (uint32_t frame = 0; frame < 8; frame++) {
        int x = frame * SPRITE_SIZE;
        float walk_bob = sinf(frame * 0.8f) * 2;
        ImageDrawCircle(&walk_img, x + SPRITE_SIZE/2, SPRITE_SIZE/2 + walk_bob, 12, (Color){138, 43, 226, 255});
        ImageDrawCircle(&walk_img, x + SPRITE_SIZE/2, SPRITE_SIZE/2 + walk_bob, 8, (Color){75, 0, 130, 255});
        ImageDrawCircle(&walk_img, x + SPRITE_SIZE/2, SPRITE_SIZE/2 - 4 + walk_bob, 4, (Color){255, 220, 177, 255});
    }
    sprites->hero_walk = LoadTextureFromImage(walk_img);
    UnloadImage(walk_img);
    
    Image cast_img = GenImageColor(SPRITE_SIZE * 6, SPRITE_SIZE, (Color){0, 0, 0, 0});
    for (uint32_t frame = 0; frame < 6; frame++) {
        int x = frame * SPRITE_SIZE;
        float glow = 1.0f + sinf(frame * 1.2f) * 0.3f;
        ImageDrawCircle(&cast_img, x + SPRITE_SIZE/2, SPRITE_SIZE/2, 12 * glow, (Color){138, 43, 226, 255});
        ImageDrawCircle(&cast_img, x + SPRITE_SIZE/2, SPRITE_SIZE/2, 8, (Color){75, 0, 130, 255});
        ImageDrawCircle(&cast_img, x + SPRITE_SIZE/2, SPRITE_SIZE/2 - 4, 4, (Color){255, 220, 177, 255});
        
        if (frame > 2) {
            for (uint32_t i = 0; i < 6; i++) {
                float angle = i * 60 * DEG2RAD;
                int spark_x = x + SPRITE_SIZE/2 + cosf(angle) * 16;
                int spark_y = SPRITE_SIZE/2 + sinf(angle) * 16;
                ImageDrawCircle(&cast_img, spark_x, spark_y, 2, (Color){173, 216, 230, 255});
            }
        }
    }
    sprites->hero_cast = LoadTextureFromImage(cast_img);
    UnloadImage(cast_img);
    
    for (uint32_t i = 0; i < 5; i++) {
        Image monster_img = GenImageColor(SPRITE_SIZE, SPRITE_SIZE, (Color){0, 0, 0, 0});
        Color monster_colors[] = {
            {34, 139, 34, 255},
            {138, 43, 226, 255},
            {139, 69, 19, 255},
            {255, 215, 0, 255},
            {220, 20, 60, 255} 
        };
        
        ImageDrawCircle(&monster_img, SPRITE_SIZE/2, SPRITE_SIZE/2, 10, monster_colors[i]);
        ImageDrawCircle(&monster_img, SPRITE_SIZE/2, SPRITE_SIZE/2, 6, (Color){0, 0, 0, 255});
        ImageDrawCircle(&monster_img, SPRITE_SIZE/2 - 2, SPRITE_SIZE/2 - 3, 2, (Color){255, 0, 0, 255});
        ImageDrawCircle(&monster_img, SPRITE_SIZE/2 + 2, SPRITE_SIZE/2 - 3, 2, (Color){255, 0, 0, 255});
        
        sprites->monsters[i] = LoadTextureFromImage(monster_img);
        UnloadImage(monster_img);
    }
    
    Image boss_img = GenImageColor(SPRITE_SIZE * 2, SPRITE_SIZE * 2, (Color){0, 0, 0, 0});
    ImageDrawCircle(&boss_img, SPRITE_SIZE, SPRITE_SIZE, 28, (Color){75, 0, 130, 255});
    ImageDrawCircle(&boss_img, SPRITE_SIZE, SPRITE_SIZE, 20, (Color){128, 0, 0, 255});
    ImageDrawCircle(&boss_img, SPRITE_SIZE - 8, SPRITE_SIZE - 8, 4, (Color){255, 0, 0, 255});
    ImageDrawCircle(&boss_img, SPRITE_SIZE + 8, SPRITE_SIZE - 8, 4, (Color){255, 0, 0, 255});
    sprites->boss_texture = LoadTextureFromImage(boss_img);
    UnloadImage(boss_img);
    
    Color item_colors[] = {
        {255, 215, 0, 255},
        {255, 0, 0, 255},
        {0, 0, 255, 255}
    };
    
    for (uint32_t i = 0; i < 3; i++) {
        Image item_img = GenImageColor(SPRITE_SIZE, SPRITE_SIZE, (Color){0, 0, 0, 0});
        
        if (i == 0) {
            for (uint32_t coin = 0; coin < 3; coin++) {
                ImageDrawCircle(&item_img, SPRITE_SIZE/2 + coin * 4 - 4, SPRITE_SIZE/2, 6, item_colors[i]);
                ImageDrawCircle(&item_img, SPRITE_SIZE/2 + coin * 4 - 4, SPRITE_SIZE/2, 4, (Color){255, 255, 0, 255});
            }
        } else {
            ImageDrawRectangle(&item_img, SPRITE_SIZE/2 - 6, SPRITE_SIZE/2 - 8, 12, 16, item_colors[i]);
            ImageDrawRectangle(&item_img, SPRITE_SIZE/2 - 4, SPRITE_SIZE/2 - 10, 8, 4, (Color){160, 160, 160, 255});
            ImageDrawCircle(&item_img, SPRITE_SIZE/2, SPRITE_SIZE/2, 3, (Color){255, 255, 255, 100});
        }
        
        sprites->items[i] = LoadTextureFromImage(item_img);
        UnloadImage(item_img);
    }
}

void unload_sprites(SpriteSystem* sprites) {
    UnloadTexture(sprites->tileset);
    UnloadTexture(sprites->hero_idle);
    UnloadTexture(sprites->hero_walk);
    UnloadTexture(sprites->hero_cast);
    
    for (uint32_t i = 0; i < 5; i++) {
        UnloadTexture(sprites->monsters[i]);
    }
    
    UnloadTexture(sprites->boss_texture);
    
    for (uint32_t i = 0; i < 3; i++) {
        UnloadTexture(sprites->items[i]);
    }
}

void render_ui(Rift* env, uint16_t screen_width, uint16_t screen_height) {
    uint16_t ui_bottom = screen_height - UI_HEIGHT;
    float time = GetTime();
    float pulse = sinf(time * 2.0f) * 0.2f + 0.8f;
    
    Color ui_top = {15, 20, 35, 250};
    Color ui_bot = {5, 10, 20, 250};
    DrawRectangleGradientV(0, ui_bottom, screen_width, UI_HEIGHT, ui_top, ui_bot);
    
    DrawRectangle(0, ui_bottom - 3, screen_width, 3, (Color){100, 120, 150, 200});
    DrawRectangle(0, ui_bottom - 2, screen_width, 2, (Color){150, 180, 220, 150});
    DrawRectangle(0, ui_bottom - 1, screen_width, 1, (Color){200, 220, 255, 100});
    
    float health_ratio = (float)env->player.health / env->player.max_health;
    uint16_t health_globe_x = GLOBE_RADIUS + GLOBE_MARGIN + 10;
    uint16_t health_globe_y = screen_height - GLOBE_RADIUS - GLOBE_MARGIN;
    
    Color health_glow = {255, 50, 50, (uint32_t)(80 * pulse)};
    DrawCircle(health_globe_x, health_globe_y, GLOBE_RADIUS + 8, health_glow);
    DrawCircle(health_globe_x, health_globe_y, GLOBE_RADIUS + 5, (Color){100, 20, 20, 150});
    
    DrawCircle(health_globe_x, health_globe_y, GLOBE_RADIUS, (Color){60, 15, 15, 255});
    
    if (health_ratio > 0) {
        uint16_t fill_height = (uint16_t)(GLOBE_RADIUS * 2 * health_ratio);
        uint16_t fill_y = health_globe_y + GLOBE_RADIUS - fill_height;
        uint16_t globe_radius_sq = GLOBE_RADIUS * GLOBE_RADIUS;
        
        for (uint16_t y = 0; y < fill_height; y++) {
            uint16_t current_y = fill_y + y;
            int16_t dy = current_y - health_globe_y;
            uint16_t dy_sq = dy * dy;
            if (dy_sq <= globe_radius_sq) {
                uint16_t half_width = (uint16_t)sqrt(globe_radius_sq - dy_sq);
                uint16_t full_width = half_width << 1;
                Color health_fill = {255, 0, 0, 255};
                DrawRectangle(health_globe_x - half_width, current_y, full_width, 1, health_fill);
            }
        }
    }
    
    DrawCircleLines(health_globe_x, health_globe_y, GLOBE_RADIUS, (Color){255, 100, 100, 255});
    DrawCircleLines(health_globe_x, health_globe_y, GLOBE_RADIUS + 1, (Color){255, 150, 150, 150});
    DrawText("HP", health_globe_x - 10, health_globe_y - 25, 12, (Color){255, 100, 100, 255});
    DrawText(TextFormat("%d", env->player.health), health_globe_x - 15, health_globe_y - 8, TEXT_SIZE_16, WHITE);
    
    float mana_ratio = (float)env->player.mana / env->player.max_mana;
    uint16_t mana_globe_x = screen_width - GLOBE_RADIUS - GLOBE_MARGIN - 10;
    uint16_t mana_globe_y = screen_height - GLOBE_RADIUS - GLOBE_MARGIN;
    
    Color mana_glow = {50, 100, 255, (uint32_t)(80 * pulse)};
    DrawCircle(mana_globe_x, mana_globe_y, GLOBE_RADIUS + 8, mana_glow);
    DrawCircle(mana_globe_x, mana_globe_y, GLOBE_RADIUS + 5, (Color){20, 40, 100, 150});
    
    DrawCircle(mana_globe_x, mana_globe_y, GLOBE_RADIUS, (Color){15, 30, 60, 255});
    
    if (mana_ratio > 0) {
        uint16_t fill_height = (uint16_t)(GLOBE_RADIUS * 2 * mana_ratio);
        uint16_t fill_y = mana_globe_y + GLOBE_RADIUS - fill_height;
        uint16_t globe_radius_sq = GLOBE_RADIUS * GLOBE_RADIUS;
        
        for (uint16_t y = 0; y < fill_height; y++) {
            uint16_t current_y = fill_y + y;
            int16_t dy = current_y - mana_globe_y;
            uint16_t dy_sq = dy * dy;
            if (dy_sq <= globe_radius_sq) {
                uint16_t half_width = (uint16_t)sqrt(globe_radius_sq - dy_sq);
                uint16_t full_width = half_width << 1;
                Color mana_fill = {50, (uint32_t)(100 + 100 * mana_ratio), 255, 255};
                DrawRectangle(mana_globe_x - half_width, current_y, full_width, 1, mana_fill);
            }
        }
    }
    
    DrawCircleLines(mana_globe_x, mana_globe_y, GLOBE_RADIUS, (Color){100, 150, 255, 255});
    DrawCircleLines(mana_globe_x, mana_globe_y, GLOBE_RADIUS + 1, (Color){150, 180, 255, 150});
    DrawText("MP", mana_globe_x - 10, mana_globe_y - 25, 12, (Color){100, 150, 255, 255});
    DrawText(TextFormat("%d", env->player.mana), mana_globe_x - 15, mana_globe_y - 8, TEXT_SIZE_16, WHITE);
    
    uint16_t center_panel_x = health_globe_x + 80;
    uint16_t center_panel_w = mana_globe_x - center_panel_x - 80;
    uint16_t center_panel_y = ui_bottom + 10;
    
    Color center_bg_top = {30, 35, 50, 200};
    Color center_bg_bot = {15, 20, 30, 200};
    DrawRectangleGradientV(center_panel_x, center_panel_y, center_panel_w, UI_HEIGHT - 20, center_bg_top, center_bg_bot);
    DrawRectangleLines(center_panel_x, center_panel_y, center_panel_w, UI_HEIGHT - 20, (Color){100, 120, 150, 255});
    DrawRectangleLines(center_panel_x - 1, center_panel_y - 1, center_panel_w + 2, UI_HEIGHT - 18, (Color){150, 180, 220, 150});
    
    char gold_text[32];
    sprintf(gold_text, "%d Gold", env->player.gold);
    Color gold_sparkle = {255, 215, 0, (uint32_t)(200 + 55 * sinf(time * 6.0f))};
    DrawText(gold_text, center_panel_x + 20, center_panel_y + 8, 18, gold_sparkle);
    
    int avg_ilvl = 0;
    int item_count = 0;
    if (env->equipment.weapon_type != EQUIPMENT_NONE) { avg_ilvl += env->equipment.weapon_level; item_count++; }
    if (env->equipment.offhand_type != EQUIPMENT_NONE) { avg_ilvl += env->equipment.offhand_level; item_count++; }
    if (env->equipment.helmet_type != EQUIPMENT_NONE) { avg_ilvl += env->equipment.helmet_level; item_count++; }
    if (env->equipment.armor_type != EQUIPMENT_NONE) { avg_ilvl += env->equipment.armor_level; item_count++; }
    if (env->equipment.boots_type != EQUIPMENT_NONE) { avg_ilvl += env->equipment.boots_level; item_count++; }
    if (env->equipment.gloves_type != EQUIPMENT_NONE) { avg_ilvl += env->equipment.gloves_level; item_count++; }
    if (env->equipment.ring_left_type != EQUIPMENT_NONE) { avg_ilvl += env->equipment.ring_left_level; item_count++; }
    if (env->equipment.ring_right_type != EQUIPMENT_NONE) { avg_ilvl += env->equipment.ring_right_level; item_count++; }
    if (env->equipment.amulet_type != EQUIPMENT_NONE) { avg_ilvl += env->equipment.amulet_level; item_count++; }
    if (env->equipment.shoulders_type != EQUIPMENT_NONE) { avg_ilvl += env->equipment.shoulders_level; item_count++; }
    if (env->equipment.belt_type != EQUIPMENT_NONE) { avg_ilvl += env->equipment.belt_level; item_count++; }
    if (env->equipment.pants_type != EQUIPMENT_NONE) { avg_ilvl += env->equipment.pants_level; item_count++; }
    if (env->equipment.bracers_type != EQUIPMENT_NONE) { avg_ilvl += env->equipment.bracers_level; item_count++; }
    
    if (item_count > 0) avg_ilvl /= item_count;
    
    char stats_text[128];
    sprintf(stats_text, "iLvl %d | STR %d | DEX %d | INT %d | VIT %d", 
            avg_ilvl, env->hero_stats.total_strength, env->hero_stats.total_dexterity,
            env->hero_stats.total_intelligence, env->hero_stats.total_vitality);
    DrawText(stats_text, center_panel_x + 20, center_panel_y + 32, 14, (Color){200, 220, 255, 255});
    
    if (env->current_phase == PHASE_RIFT) {
        char rift_text[64];
        sprintf(rift_text, "Rift %d", env->current_rift_level);
        Color rift_color = {100, 255, 100, (uint32_t)(200 + 55 * pulse)};
        DrawText(rift_text, center_panel_x + 20, center_panel_y + 50, 14, rift_color);
        
        if (env->boss_spawned) {
            Color boss_warning = {255, 100, 100, (uint32_t)(150 + 105 * pulse)};
            DrawText(">>> BOSS BATTLE! <<<", center_panel_x + 200, center_panel_y + 50, 14, boss_warning);
        }
    } else {
        DrawText(">>> Town Mode <<<", center_panel_x + 20, center_panel_y + 50, 14, (Color){255, 215, 0, 255});
    }
    
    uint16_t hp_x = center_panel_x + center_panel_w - 120;
    uint16_t hp_y = screen_height - 35;
    Color hp_glow = {255, 100, 100, (uint32_t)(60 * pulse)};
    Color hp_color = (env->player.health_potion_cooldown > 0) ? (Color){100, 50, 50, 255} : (Color){200, 50, 50, 255};
    
    DrawCircle(hp_x, hp_y, POTION_SIZE + 3, hp_glow);
    DrawCircle(hp_x, hp_y, POTION_SIZE, hp_color);
    DrawCircleLines(hp_x, hp_y, POTION_SIZE, (Color){255, 150, 150, 255});
    DrawText("Q", hp_x - 5, hp_y - 8, TEXT_SIZE_16, WHITE);
    
    if (env->player.health_potion_cooldown > 0) {
        float cooldown_ratio = (float)env->player.health_potion_cooldown / HEALTH_POTION_COOLDOWN;
        Vector2 hp_center = {hp_x, hp_y};
        DrawCircleSector(hp_center, POTION_SIZE, 0, 360 * cooldown_ratio, 32, (Color){0, 0, 0, 180});
    }
    
    uint16_t mp_x = center_panel_x + center_panel_w - 60;
    uint16_t mp_y = screen_height - 35;
    Color mp_glow = {100, 150, 255, (uint32_t)(60 * pulse)};
    Color mp_color = (env->player.mana_potion_cooldown > 0) ? (Color){50, 75, 100, 255} : (Color){50, 100, 200, 255};
    
    DrawCircle(mp_x, mp_y, POTION_SIZE + 3, mp_glow);
    DrawCircle(mp_x, mp_y, POTION_SIZE, mp_color);
    DrawCircleLines(mp_x, mp_y, POTION_SIZE, (Color){150, 180, 255, 255});
    DrawText("E", mp_x - 5, mp_y - 8, TEXT_SIZE_16, WHITE);
    
    if (env->player.mana_potion_cooldown > 0) {
        float cooldown_ratio = (float)env->player.mana_potion_cooldown / MANA_POTION_COOLDOWN;
        Vector2 mp_center = {mp_x, mp_y};
        DrawCircleSector(mp_center, POTION_SIZE, 0, 360 * cooldown_ratio, 32, (Color){0, 0, 0, 180});
    }
    
    Rectangle help_icon = {5, 5, 20, 20};
    Vector2 mouse_pos = GetMousePosition();
    bool hovering_help = CheckCollisionPointRec(mouse_pos, help_icon);
    
    Color help_color = hovering_help ? (Color){255, 255, 255, 255} : (Color){180, 180, 180, 200};
    DrawCircle(help_icon.x + 10, help_icon.y + 10, 10, (Color){40, 40, 40, 180});
    DrawCircleLines(help_icon.x + 10, help_icon.y + 10, 10, help_color);
    DrawText("?", help_icon.x + 6, help_icon.y + 3, 16, help_color);
    
    if (hovering_help) {
        int panel_width = 400;
        int panel_height = 120;
        int panel_x = 15;
        int panel_y = 95;
        
        DrawRectangle(panel_x, panel_y, panel_width, panel_height, (Color){0, 0, 0, 220});
        DrawRectangleLines(panel_x, panel_y, panel_width, panel_height, (Color){180, 180, 180, 255});
        
        DrawText("CONTROLS:", panel_x + 10, panel_y + 10, 14, (Color){255, 215, 0, 255});
        DrawText("WASD: Move character", panel_x + 10, panel_y + 30, 12, (Color){255, 255, 255, 255});
        DrawText("Space: Cast Blizzard spell", panel_x + 10, panel_y + 45, 12, (Color){255, 255, 255, 255});
        DrawText("Q/E: Use Health/Mana potions", panel_x + 10, panel_y + 60, 12, (Color){255, 255, 255, 255});
        DrawText("F: Interact (buy items, enter rift)", panel_x + 10, panel_y + 75, 12, (Color){255, 255, 255, 255});
        DrawText("Z: Exit town and go to next rift", panel_x + 10, panel_y + 90, 12, (Color){255, 215, 0, 255});
        DrawText("Hold SHIFT: Manual control (vs AI)", panel_x + 10, panel_y + 110, 12, (Color){100, 255, 100, 255});
    }
    
    if (!(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
        DrawRectangleGradientV(CLIENT_WIDTH - 45, 5, 40, MANUAL_CONTROL_HEIGHT, 
                              (Color){40, 40, 40, 200}, (Color){20, 20, 20, 200});
        DrawRectangleLines(CLIENT_WIDTH - 45, 5, 40, MANUAL_CONTROL_HEIGHT, (Color){100, 100, 100, 255});
        DrawText("AI", CLIENT_WIDTH - 35, 12, TEXT_SIZE_14, (Color){150, 150, 150, 255});
    }
}

#endif