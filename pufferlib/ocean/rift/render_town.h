#ifndef RIFT_RENDER_TOWN_H
#define RIFT_RENDER_TOWN_H

#include "render_core.h"



static inline void DrawItem(int16_t x, int16_t y, uint8_t size, Color color, uint8_t equipment_type);

static const char* EQUIPMENT_SLOT_NAMES[13] = {
    "SHOULDERS", "GLOVES", "RING (LEFT)", "WEAPON", "HELMET",
    "CHEST ARMOR", "BELT", "PANTS", "BOOTS", "AMULET",
    "BRACERS", "RING (RIGHT)", "OFFHAND"
};

static inline void get_equipment_slot_info(Rift* env, int slot_index, uint32_t* item_type, uint32_t* item_quality, uint32_t* item_level, uint32_t stat_bonuses[4], const char** slot_name) {
    *slot_name = (slot_index >= 0 && slot_index < 13) ? EQUIPMENT_SLOT_NAMES[slot_index] : "NONE";
    
    uint32_t* equipment_base = (uint32_t*)&env->equipment;
    uint32_t slot_offset = slot_index * 7;
    
    *item_type = equipment_base[slot_offset];
    *item_quality = equipment_base[slot_offset + 1];
    *item_level = equipment_base[slot_offset + 2];
    
    for (int i = 0; i < 4; i++) {
        stat_bonuses[i] = equipment_base[slot_offset + 3 + i];
    }
}

static void render_equipment_slot(Rift* env, int x, int y, int slot_size, EquipmentSlotConfig config, float pulse) {
    Color quality_color = (*config.type_ptr != EQUIPMENT_NONE) ? 
                          GetQualityColor(*config.quality_ptr) : TOWN_COLORS.empty_slot;
    
    if ((int)env->town_interface.selected_item_index == config.index) {
        DrawRectangle(x - 6, y - 6, slot_size + 12, slot_size + 12, 
                      fade_color(quality_color, (uint32_t)(120 * pulse)));
    }
    
    DrawRectangleGradientV(x, y, slot_size, slot_size, 
                           darken_color(quality_color, 4), darken_color(quality_color, 8));
    DrawRectangleLines(x, y, slot_size, slot_size, quality_color);
    
    SlotDisplayInfo display = EQUIPMENT_SLOT_INFO[config.index];
    if (*config.type_ptr != EQUIPMENT_NONE) {
        DrawItem(x, y, slot_size, GetQualityColor(*config.quality_ptr), config.equipment_type);
    } else {
        DrawText(display.icon, x + 15, y + 10, display.icon_size, TOWN_COLORS.empty_slot);
    }
    DrawText(display.name, x + display.label_x_offset, y + slot_size + 5, 8, EXTENDED_TOWN_COLORS.label);
}

static inline void draw_armor_plates(int x, int y, int width, int count, int start_y, int spacing, int height, Color color) {
    for (uint8_t i = 0; i < count; i++) {
        int plate_y = y + start_y + i * spacing;
        DrawRectangle(x, plate_y, width, height, brighten_color(color, 40));
        DrawRectangleLines(x, plate_y, width, height, (Color){COLOR_LIGHT_SILVER, COLOR_LIGHT_SILVER, COLOR_LIGHT_SILVER_STRONG, COLOR_WHITE});
    }
}

static inline void DrawItem(int16_t x, int16_t y, uint8_t size, Color color, uint8_t equipment_type) {
     draw_equipment_simple(x, y, size, color, equipment_type);
}


void render_town(Rift* env) {
    for (int y = 0; y < GetScreenHeight(); y++) {
        float t = (float)y / GetScreenHeight();
        Color current = {
            (uint32_t)(TOWN_COLORS.background_top.r * (1-t) + TOWN_COLORS.background_bottom.r * t),
            (uint32_t)(TOWN_COLORS.background_top.g * (1-t) + TOWN_COLORS.background_bottom.g * t),
            (uint32_t)(TOWN_COLORS.background_top.b * (1-t) + TOWN_COLORS.background_bottom.b * t),
            255
        };
        DrawRectangle(0, y, GetScreenWidth(), 1, current);
    }
    
    float glow_pulse = get_ui_pulse(UI_ANIMATION.ui_pulse_slow_speed, UI_ANIMATION.ui_pulse_base, UI_ANIMATION.ui_pulse_amplitude);
    
    bool shop_active = (env->town_interface.current_tab == TOWN_TAB_SHOP);
    Color shop_color = shop_active ? WHITE : (Color){150, 150, 150, 255};
    DrawText("SHOP (1)", 100, 30, 20, shop_color);
    if (shop_active) {
        DrawRectangle(100, 55, 100, 3, (Color){100, 150, 255, 255});
        DrawRectangle(98, 56, 104, 1, (Color){100, 150, 255, 150});
    }
    
    bool char_active = (env->town_interface.current_tab == TOWN_TAB_CHARACTER);
    Color char_color = char_active ? WHITE : (Color){150, 150, 150, 255};
    DrawText("CHARACTER (2)", 250, 30, 20, char_color);
    if (char_active) {
        DrawRectangle(250, 55, 160, 3, (Color){0, 255, 100, 255});
        DrawRectangle(248, 56, 164, 1, (Color){0, 255, 100, 150});
    }
    
    
    
    if (env->town_interface.current_tab == TOWN_TAB_CHARACTER) {
        render_character_tab(env);
    } else if (env->town_interface.current_tab == TOWN_TAB_SHOP) {
        render_shop_tab(env);
    }
    
    Color timer_color = GREEN;
    if (env->town_interface.frames_remaining < 120) timer_color = RED;
    else if (env->town_interface.frames_remaining < 200) timer_color = ORANGE;
    
    float time_remaining = (float)env->town_interface.frames_remaining / 60.0f;
    char timer_text[32];
    sprintf(timer_text, "Time: %.1fs", time_remaining);
    DrawText(timer_text, TOP_UI_X + TOP_UI_WIDTH - 180, TOP_UI_Y + 15, 18, timer_color);
    
    char rift_text[32];
    sprintf(rift_text, "Rift %d", env->current_rift_level);
    DrawText(rift_text, TOP_UI_X + 40, TOP_UI_Y + 15, 20, TOWN_COLORS.quality_text);
}

void render_character_tab(Rift* env) {
    
    int start_y = 120;
    float pulse = get_ui_pulse(UI_ANIMATION.ui_pulse_med_speed, UI_ANIMATION.ui_pulse_base + 0.1f, UI_ANIMATION.ui_pulse_amplitude - 0.1f);
    
    if (env->town_interface.character_mode == CHARACTER_MODE_EQUIPMENT) {
        DrawText("HERO EQUIPMENT", CONTENT_AREA_X, start_y, 24, TOWN_COLORS.quality_text);
        
        EquipmentLayout layout = EQUIPMENT_UI_LAYOUT;
        int cols_x[] = {layout.left_col_x, layout.middle_col_x, layout.right_col_x};
        
        for (int i = 0; i < 13; i++) {
            SlotPosition pos = EQUIPMENT_SLOT_POSITIONS[i];
            int x = cols_x[pos.col];
            int y = layout.base_y + pos.row * layout.row_spacing;
            
            EquipmentSlotConfig config = get_equipment_slot_config(env, i);
            render_equipment_slot(env, x, y, layout.slot_size, config, pulse);
        }
        
        int stats_panel_y = layout.base_y;
        int available_height = TOWN_CONTAINER_HEIGHT - (stats_panel_y - TOWN_CONTAINER_Y) - 20;
        int stats_panel_height = available_height * 0.75;
        
        draw_stats_panel(STATS_PANEL_X, stats_panel_y, STATS_PANEL_WIDTH, stats_panel_height);
        
        uint32_t selected_slot = env->town_interface.selected_item_index;
        uint32_t item_type = EQUIPMENT_NONE;
        uint32_t item_quality = QUALITY_COMMON;
        uint32_t item_level = 0;
        uint32_t stat_bonuses[4] = {0, 0, 0, 0};
        const char* slot_name = "NONE";
        
        get_equipment_slot_info(env, selected_slot, &item_type, &item_quality, &item_level, stat_bonuses, &slot_name);
        
        if (item_type != EQUIPMENT_NONE) {
            Color quality_color = GetQualityColor(item_quality);
            DrawText(slot_name, STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT, 14, quality_color);
            
            char ilvl_text[32];
            sprintf(ilvl_text, "Item Level %d", item_level);
            DrawText(ilvl_text, STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT + STATS_LINE_HEIGHT, 12, TOWN_COLORS.item_level_text);
            
            char* quality_names[] = {"COMMON", "RARE", "EPIC", "LEGENDARY"};
            char* quality_name = (item_quality < 4) ? quality_names[item_quality] : "UNKNOWN";
            DrawText(quality_name, STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT + 2 * STATS_LINE_HEIGHT, 12, quality_color);
            
            int icon_x = STATS_PANEL_X + STATS_PANEL_WIDTH - 70;
            int icon_y = stats_panel_y + 15;
            int icon_size = 50;
            draw_item_icon(icon_x, icon_y, icon_size, quality_color);
            
            uint8_t slot_to_equipment[] = {EQUIPMENT_SHOULDERS, EQUIPMENT_GLOVES, EQUIPMENT_RING_LEFT, EQUIPMENT_WEAPON, 
                                         EQUIPMENT_HELMET, EQUIPMENT_ARMOR, EQUIPMENT_BELT, EQUIPMENT_PANTS, 
                                         EQUIPMENT_BOOTS, EQUIPMENT_AMULET, EQUIPMENT_BRACERS, EQUIPMENT_RING_RIGHT, EQUIPMENT_OFFHAND};
            if (selected_slot < 13) {
                DrawItem(icon_x, icon_y, icon_size, quality_color, slot_to_equipment[selected_slot]);
            }
            
            DrawText("STATS:", STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT + 2 * STATS_LINE_HEIGHT + STATS_SECTION_SPACING, 12, TOWN_COLORS.quality_text);
            int stat_y = stats_panel_y + 110;
            char* stat_names[] = {"Strength", "Dexterity", "Intelligence", "Vitality"};
            Color stat_colors[] = {
                EXTENDED_TOWN_COLORS.stat_color,
                EXTENDED_TOWN_COLORS.stat_color, 
                EXTENDED_TOWN_COLORS.stat_color,
                EXTENDED_TOWN_COLORS.stat_color 
            };
            
            for (int i = 0; i < 4; i++) {
                if (stat_bonuses[i] > 0) {
                    char stat_text[64];
                    sprintf(stat_text, "+%d %s", stat_bonuses[i], stat_names[i]);
                    DrawText(stat_text, STATS_PANEL_X + STATS_CONTENT_MARGIN_INDENT, stat_y, 11, stat_colors[i]);
                    stat_y += 18;
                }
            }
            
            DrawRectangle(STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stat_y + 5, STATS_PANEL_WIDTH - 2 * STATS_CONTENT_MARGIN_LEFT, 1, EXTENDED_TOWN_COLORS.divider);
            
        } else {
            DrawText("EMPTY SLOT", STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT, 14, EXTENDED_TOWN_COLORS.empty_text);
            DrawText("No equipment equipped", STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT + STATS_LINE_HEIGHT, 11, EXTENDED_TOWN_COLORS.disabled_text);
        }
        
    } else {
        DrawText("HERO INVENTORY", CONTENT_AREA_X, start_y, 24, TOWN_COLORS.quality_text);
        
        InventoryLayout inv_layout = INVENTORY_UI_LAYOUT;
        
        for (int i = 0; i < INVENTORY_SLOTS; i++) {
            int row = i / inv_layout.slots_per_row;
            int col = i % inv_layout.slots_per_row;
            int x = inv_layout.start_x + col * inv_layout.slot_spacing_x;
            int y = inv_layout.start_y + row * inv_layout.slot_spacing_y;
            
            Color slot_bg = EXTENDED_TOWN_COLORS.slot_bg;
            Color slot_border = EXTENDED_TOWN_COLORS.slot_border;
            
            if ((uint32_t)env->town_interface.selected_item_index == (uint32_t)i) {
                int selection_margin = 8;
                int selection_size = inv_layout.slot_size + 2 * selection_margin;
                DrawRectangle(x - selection_margin, y - selection_margin, selection_size, selection_size, 
                              fade_color(TOWN_COLORS.selected_glow, (uint32_t)(150 * pulse)));
            }
            
            draw_gradient_rect(x, y, inv_layout.slot_size, inv_layout.slot_size, slot_bg);
            
            if (env->player.inventory[i].item_type != EQUIPMENT_NONE) {
                Color item_quality_color = GetQualityColor(env->player.inventory[i].item_quality);
                int quality_margin = 2;
                int quality_size = inv_layout.slot_size + 2 * quality_margin;
                DrawRectangle(x - quality_margin, y - quality_margin, quality_size, quality_size, 
                              fade_color(item_quality_color, 80));
                
                DrawItem(x, y, inv_layout.slot_size - 20, item_quality_color, env->player.inventory[i].item_type);
                
                char ilvl_text[8];
                sprintf(ilvl_text, "ilvl%d", env->player.inventory[i].item_level);
                DrawText(ilvl_text, x + 5, y + inv_layout.slot_size - 15, 10, (Color){255, 255, 255, 200});
            } else {
                int icon_offset = inv_layout.slot_size / 2 - 15;
                DrawText("[ ]", x + icon_offset, y + icon_offset - 5, 30, TOWN_COLORS.empty_slot);
            }
            
            DrawRectangleLines(x, y, inv_layout.slot_size, inv_layout.slot_size, slot_border);
            DrawRectangleLines(x - 1, y - 1, inv_layout.slot_size + 2, inv_layout.slot_size + 2, 
                               brighten_color(slot_border, 30));
        }
    }
}

bool get_equipped_item_stats(Rift* env, uint32_t item_type, uint32_t* equipped_stat_bonuses) {
    Equipment* eq = &env->equipment;
    
    const struct {
        uint32_t equipment_type;
        uint32_t* type_ptr;
        uint32_t* stat_bonuses;
    } equipment_map[] = {
        {EQUIPMENT_SHOULDERS, &eq->shoulders_type, eq->shoulders_stat_bonuses},
        {EQUIPMENT_GLOVES, &eq->gloves_type, eq->gloves_stat_bonuses},
        {EQUIPMENT_RING_LEFT, &eq->ring_left_type, eq->ring_left_stat_bonuses},
        {EQUIPMENT_WEAPON, &eq->weapon_type, eq->weapon_stat_bonuses},
        {EQUIPMENT_HELMET, &eq->helmet_type, eq->helmet_stat_bonuses},
        {EQUIPMENT_ARMOR, &eq->armor_type, eq->armor_stat_bonuses},
        {EQUIPMENT_BELT, &eq->belt_type, eq->belt_stat_bonuses},
        {EQUIPMENT_PANTS, &eq->pants_type, eq->pants_stat_bonuses},
        {EQUIPMENT_BOOTS, &eq->boots_type, eq->boots_stat_bonuses},
        {EQUIPMENT_AMULET, &eq->amulet_type, eq->amulet_stat_bonuses},
        {EQUIPMENT_BRACERS, &eq->bracers_type, eq->bracers_stat_bonuses},
        {EQUIPMENT_RING_RIGHT, &eq->ring_right_type, eq->ring_right_stat_bonuses},
        {EQUIPMENT_OFFHAND, &eq->offhand_type, eq->offhand_stat_bonuses}
    };
    
    for (int i = 0; i < 13; i++) {
        if (equipment_map[i].equipment_type == item_type) {
            if (*(equipment_map[i].type_ptr) != EQUIPMENT_NONE) {
                for (int j = 0; j < 4; j++) {
                    equipped_stat_bonuses[j] = equipment_map[i].stat_bonuses[j];
                }
                return true;
            }
            return false;
        }
    }
    return false;
}

void render_shop_tab(Rift* env) {
    
    int start_y = 120;
    float pulse = get_ui_pulse(UI_ANIMATION.ui_pulse_fast_speed, UI_ANIMATION.ui_pulse_base, UI_ANIMATION.ui_pulse_amplitude);
    
    DrawText("SHOP", CONTENT_AREA_X, start_y, 24, TOWN_COLORS.quality_text);
    
    // Reroll button
    uint32_t reroll_cost = SHOP_REROLL_BASE_COST * powf(SHOP_REROLL_SCALING, env->current_rift_level - 1);
    char reroll_text[64];
    sprintf(reroll_text, "REROLL (R) - %d Gold", reroll_cost);
    
    Color reroll_color = (env->player.gold >= reroll_cost) ? 
        (Color){0, 255, 100, 255} : (Color){255, 100, 100, 255};
    DrawText(reroll_text, CONTENT_AREA_X + 300, start_y, 16, reroll_color);
    
    ShopLayout shop_layout = SHOP_UI_LAYOUT;
    int container_x = CONTENT_AREA_X;
    int container_y = start_y + 50; 
    int container_width = CONTENT_AREA_WIDTH - 40;
    int container_height = shop_layout.scroll_container_height;
    
    BeginScissorMode(container_x, container_y, container_width, container_height);
    
    for (int i = 0; i < SHOP_ITEMS_COUNT; i++) {
        int x = container_x + 20;
        int y = container_y + 10 + i * shop_layout.item_spacing + env->town_interface.shop_scroll_offset;
        
        Color case_bg = {20, 25, 40, 240};
        Color case_border = {100, 120, 160, 255};
        Color item_glow;
        
        if (env->shop_items[i].available) {
            Color item_quality_color = GetQualityColor(env->shop_items[i].item_quality);
            item_glow = (Color){item_quality_color.r, item_quality_color.g, item_quality_color.b, (uint32_t)(100 * pulse)};
            
            if ((uint32_t)env->town_interface.selected_item_index == (uint32_t)i) {
                int selection_margin = 12;
                int selection_width = 300 + 2 * selection_margin;
                int selection_height = shop_layout.item_height + 2 * selection_margin;
                DrawRectangle(x - selection_margin, y - selection_margin, selection_width, selection_height,
                              fade_color(TOWN_COLORS.selected_glow, (uint32_t)(120 * pulse)));
                DrawRectangle(x - 8, y - 8, 316, shop_layout.item_height + 16, item_glow);
            }
            
            draw_gradient_rect(x, y, 300, shop_layout.item_height, case_bg);
            DrawRectangleLines(x, y, 300, shop_layout.item_height, case_border);
            DrawRectangleLines(x - 1, y - 1, 302, 82, brighten_color(case_border, 30));
            
            int icon_x = x + 10, icon_y = y + 10;
            draw_gradient_rect(icon_x, icon_y, 60, 60, (Color){40, 45, 60, 200});
            DrawRectangleLines(icon_x, icon_y, 60, 60, item_quality_color);
            DrawRectangleLines(icon_x - 1, icon_y - 1, 62, 62, item_quality_color);
            
            DrawItem(icon_x, icon_y, 60, item_quality_color, env->shop_items[i].item_type);
            
            char* type_names[] = {"None", "Shoulders", "Gloves", "Ring L", "Weapon", "Helmet", "Armor", "Belt", "Pants", "Boots", "Amulet", "Bracers", "Ring R", "Offhand", "Consumable"};
            char* quality_names[] = {"Common", "Rare", "Epic", "Legendary"};
            
            DrawText(type_names[env->shop_items[i].item_type], x + 80, y + 10, 16, item_quality_color);
            
            char quality_text[32];
            sprintf(quality_text, "%s Quality", quality_names[env->shop_items[i].item_quality]);
            DrawText(quality_text, x + 80, y + 30, 12, item_quality_color);
            
            char ilvl_text[16];
            sprintf(ilvl_text, "Item Level %d", env->shop_items[i].item_level);
            DrawText(ilvl_text, x + 80, y + 45, 12, (Color){180, 180, 180, 255});
            
            char price_text[16];
            sprintf(price_text, "%d Gold", env->shop_items[i].price);
            Color price_color = (env->player.gold >= env->shop_items[i].price) ? 
                (Color){0, 255, 100, 255} : (Color){255, 100, 100, 255};
            DrawText(price_text, x + 190, y + 25, 16, price_color);
            
            if (env->player.gold >= env->shop_items[i].price) {
                DrawText("Can Buy", x + 190, y + 50, 12, (Color){0, 255, 100, 200});
            } else {
                DrawText("Too Expensive", x + 190, y + 50, 12, (Color){255, 100, 100, 200});
            }
            
        } else {
            draw_gradient_rect(x, y, 300, shop_layout.item_height, (Color){30, 30, 30, 180});
            DrawRectangleLines(x, y, 300, shop_layout.item_height, (Color){60, 60, 60, 255});
            DrawText("SOLD OUT", x + 100, y + shop_layout.item_height/2 - 9, 18, (Color){120, 120, 120, 255});
        }
    }
    
    EndScissorMode();
    
    int stats_panel_y = container_y;
    int available_height = TOWN_CONTAINER_HEIGHT - (stats_panel_y - TOWN_CONTAINER_Y) - 20;
    int stats_panel_height = available_height * 0.75;
    
    draw_stats_panel(STATS_PANEL_X, stats_panel_y, STATS_PANEL_WIDTH, stats_panel_height);
    
    int selected_idx = env->town_interface.selected_item_index;
    if (selected_idx >= 0 && selected_idx < SHOP_ITEMS_COUNT && env->shop_items[selected_idx].available) {
        ShopItem* shop_item = &env->shop_items[selected_idx];
        
        Color quality_color = GetQualityColor(shop_item->item_quality);
        char* type_names[] = {"None", "Shoulders", "Gloves", "Ring L", "Weapon", "Helmet", "Armor", "Belt", "Pants", "Boots", "Amulet", "Bracers", "Ring R", "Offhand", "Consumable"};
        DrawText(type_names[shop_item->item_type], STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT, 14, quality_color);
        
        char ilvl_text[32];
        sprintf(ilvl_text, "Item Level %d", shop_item->item_level);
        DrawText(ilvl_text, STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT + STATS_LINE_HEIGHT, 12, TOWN_COLORS.item_level_text);
        
        char* quality_names[] = {"COMMON", "RARE", "EPIC", "LEGENDARY"};
        char* quality_name = (shop_item->item_quality < 4) ? quality_names[shop_item->item_quality] : "UNKNOWN";
        DrawText(quality_name, STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT + 2 * STATS_LINE_HEIGHT, 12, quality_color);
        
        char price_text[32];
        sprintf(price_text, "%d Gold", shop_item->price);
        Color price_color = (env->player.gold >= shop_item->price) ? (Color){0, 255, 100, 255} : (Color){255, 100, 100, 255};
        DrawText(price_text, STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT + 3 * STATS_LINE_HEIGHT, 12, price_color);
        
        int icon_x = STATS_PANEL_X + STATS_PANEL_WIDTH - 70;
        int icon_y = stats_panel_y + 15;
        int icon_size = 50;
        draw_item_icon(icon_x, icon_y, icon_size, quality_color);
        
        DrawItem(icon_x, icon_y, icon_size, quality_color, shop_item->item_type);
        
        DrawText("STATS:", STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT + 3 * STATS_LINE_HEIGHT + STATS_SECTION_SPACING, 12, TOWN_COLORS.quality_text);
        int stat_y = stats_panel_y + 130;
        char* stat_names[] = {"Strength", "Dexterity", "Intelligence", "Vitality"};
        Color stat_colors[] = {
            {160, 160, 160, 255},
            {160, 160, 160, 255},
            {160, 160, 160, 255},
            {160, 160, 160, 255} 
        };
        
        DrawText("ITEM BONUSES:", STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stat_y - 5, 12, TOWN_COLORS.quality_text);
        stat_y += 20;
        bool has_stats = false;
        for (int i = 0; i < 4; i++) {
            if (shop_item->stat_bonuses[i] > 0) {
                char stat_text[64];
                sprintf(stat_text, "+%d %s", shop_item->stat_bonuses[i], stat_names[i]);
                DrawText(stat_text, STATS_PANEL_X + STATS_CONTENT_MARGIN_INDENT, stat_y, 11, stat_colors[i]);
                stat_y += 16;
                has_stats = true;
            }
        }
        if (!has_stats) {
            DrawText("No stat bonuses", STATS_PANEL_X + STATS_CONTENT_MARGIN_INDENT, stat_y, 10, (Color){120, 120, 120, 255});
            stat_y += 16;
        }
        stat_y += 10;
        
        uint32_t equipped_stat_bonuses[4] = {0, 0, 0, 0};
        bool has_equipped = get_equipped_item_stats(env, shop_item->item_type, equipped_stat_bonuses);
        
        DrawText("NET EFFECT ON HERO:", STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stat_y, 12, (Color){100, 255, 255, 255});
        stat_y += 20;
        
        bool has_net_changes = false;
        for (int i = 0; i < 4; i++) {
            int stat_diff = shop_item->stat_bonuses[i] - equipped_stat_bonuses[i];
            if (stat_diff != 0) {
                char net_effect_text[64];
                Color effect_color;
                if (stat_diff > 0) {
                    sprintf(net_effect_text, "%s: +%d", stat_names[i], stat_diff);
                    effect_color = (Color){100, 200, 100, 255};
                } else {
                    sprintf(net_effect_text, "%s: %d", stat_names[i], stat_diff);
                    effect_color = (Color){200, 100, 100, 255};
                }
                DrawText(net_effect_text, STATS_PANEL_X + STATS_CONTENT_MARGIN_INDENT, stat_y, 11, effect_color);
                stat_y += 16;
                has_net_changes = true;
            }
        }
        
        if (!has_net_changes) {
            if (has_equipped) {
                DrawText("No net change", STATS_PANEL_X + STATS_CONTENT_MARGIN_INDENT, stat_y, 10, (Color){160, 160, 160, 255});
            } else {
                DrawText("Pure gain (no item equipped)", STATS_PANEL_X + STATS_CONTENT_MARGIN_INDENT, stat_y, 10, (Color){100, 200, 100, 255});
            }
            stat_y += 16;
        }
        
        stat_y += 10;
        
        if (has_equipped) {
            DrawText("CURRENTLY EQUIPPED:", STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stat_y, 12, (Color){180, 180, 180, 255});
            stat_y += 20;
            
            bool has_equipped_stats = false;
            for (int i = 0; i < 4; i++) {
                if (equipped_stat_bonuses[i] > 0) {
                    char equipped_text[64];
                    sprintf(equipped_text, "+%d %s", equipped_stat_bonuses[i], stat_names[i]);
                    DrawText(equipped_text, STATS_PANEL_X + STATS_CONTENT_MARGIN_INDENT, stat_y, 10, (Color){120, 120, 120, 255});
                    stat_y += 14;
                    has_equipped_stats = true;
                }
            }
            if (!has_equipped_stats) {
                DrawText("No stat bonuses", STATS_PANEL_X + STATS_CONTENT_MARGIN_INDENT, stat_y, 10, (Color){100, 100, 100, 255});
            }
        } else {
            DrawText("NO ITEM EQUIPPED", STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stat_y, 12, (Color){180, 180, 180, 255});
            DrawText("This would be pure gain!", STATS_PANEL_X + STATS_CONTENT_MARGIN_INDENT, stat_y + STATS_LINE_HEIGHT, 10, (Color){100, 200, 100, 255});
        }
        
    } else {
        DrawText("SELECT ITEM", STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT, 14, (Color){100, 100, 100, 255});
        DrawText("Choose an item to view stats", STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT + STATS_LINE_HEIGHT, 11, (Color){150, 150, 150, 255});
    }
}


#endif
