#ifndef RIFT_TOWN_H
#define RIFT_TOWN_H

void give_stat_based_rewards(Rift* env, uint32_t old_str, uint32_t old_dex, uint32_t old_int, uint32_t old_vit, uint32_t old_total_ilvl);
void calculate_total_stats(Rift* env);
void level_up_hero(Rift* env);
int GetScaledItemLevel(uint32_t rift_level);
int GetScaledItemPrice(int base_price, uint32_t rift_level, uint32_t quality);
void GetQualityChances(uint32_t rift_level, int* common, int* rare, int* epic, int* legendary);

void init_hero_stats(Rift* env) {
    env->hero_stats.level = 1;
    env->hero_stats.experience = 0;
    env->hero_stats.experience_to_next = 100;
    env->hero_stats.stat_points_available = 0;
    
    env->hero_stats.strength = 10;
    env->hero_stats.dexterity = 10;
    env->hero_stats.intelligence = 10;
    env->hero_stats.vitality = 10;
    
    env->equipment.weapon_type = EQUIPMENT_WEAPON;
    env->equipment.weapon_quality = QUALITY_COMMON;
    env->equipment.weapon_level = 1;
    env->equipment.weapon_stat_bonuses[0] = 2;
    env->equipment.weapon_stat_bonuses[1] = 0;
    env->equipment.weapon_stat_bonuses[2] = 0;
    env->equipment.weapon_stat_bonuses[3] = 0;
    
    env->equipment.offhand_type = EQUIPMENT_OFFHAND;
    env->equipment.offhand_quality = QUALITY_COMMON;
    env->equipment.offhand_level = 1;
    env->equipment.offhand_stat_bonuses[0] = 0;
    env->equipment.offhand_stat_bonuses[1] = 0;
    env->equipment.offhand_stat_bonuses[2] = 0;
    env->equipment.offhand_stat_bonuses[3] = 1;
    
    env->equipment.ring_left_type = EQUIPMENT_RING_LEFT;
    env->equipment.ring_left_quality = QUALITY_COMMON;
    env->equipment.ring_left_level = 1;
    env->equipment.ring_left_stat_bonuses[0] = 1;
    env->equipment.ring_left_stat_bonuses[1] = 0;
    env->equipment.ring_left_stat_bonuses[2] = 0;
    env->equipment.ring_left_stat_bonuses[3] = 0;
    
    env->equipment.ring_right_type = EQUIPMENT_RING_RIGHT;
    env->equipment.ring_right_quality = QUALITY_COMMON;
    env->equipment.ring_right_level = 1;
    env->equipment.ring_right_stat_bonuses[0] = 0;
    env->equipment.ring_right_stat_bonuses[1] = 0;
    env->equipment.ring_right_stat_bonuses[2] = 1;
    env->equipment.ring_right_stat_bonuses[3] = 0;
    
    env->equipment.amulet_type = EQUIPMENT_AMULET;
    env->equipment.amulet_quality = QUALITY_COMMON;
    env->equipment.amulet_level = 1;
    env->equipment.amulet_stat_bonuses[0] = 0;
    env->equipment.amulet_stat_bonuses[1] = 1;
    env->equipment.amulet_stat_bonuses[2] = 0;
    env->equipment.amulet_stat_bonuses[3] = 0;
    
    env->equipment.boots_type = EQUIPMENT_BOOTS;
    env->equipment.boots_quality = QUALITY_COMMON;
    env->equipment.boots_level = 1;
    env->equipment.boots_stat_bonuses[0] = 0;
    env->equipment.boots_stat_bonuses[1] = 0;
    env->equipment.boots_stat_bonuses[2] = 0;
    env->equipment.boots_stat_bonuses[3] = 1;
    
    env->equipment.gloves_type = EQUIPMENT_GLOVES;
    env->equipment.gloves_quality = QUALITY_COMMON;
    env->equipment.gloves_level = 1;
    env->equipment.gloves_stat_bonuses[0] = 0;
    env->equipment.gloves_stat_bonuses[1] = 1;
    env->equipment.gloves_stat_bonuses[2] = 0;
    env->equipment.gloves_stat_bonuses[3] = 0;
    
    env->equipment.helmet_type = EQUIPMENT_HELMET;
    env->equipment.helmet_quality = QUALITY_COMMON;
    env->equipment.helmet_level = 1;
    env->equipment.helmet_stat_bonuses[0] = 1;
    env->equipment.helmet_stat_bonuses[1] = 0;
    env->equipment.helmet_stat_bonuses[2] = 0;
    env->equipment.helmet_stat_bonuses[3] = 0;
    
    env->equipment.shoulders_type = EQUIPMENT_SHOULDERS;
    env->equipment.shoulders_quality = QUALITY_COMMON;
    env->equipment.shoulders_level = 1;
    env->equipment.shoulders_stat_bonuses[0] = 1;
    env->equipment.shoulders_stat_bonuses[1] = 0;
    env->equipment.shoulders_stat_bonuses[2] = 0;
    env->equipment.shoulders_stat_bonuses[3] = 0;
    
    env->equipment.armor_type = EQUIPMENT_ARMOR;
    env->equipment.armor_quality = QUALITY_COMMON;
    env->equipment.armor_level = 1;
    env->equipment.armor_stat_bonuses[0] = 0;
    env->equipment.armor_stat_bonuses[1] = 0;
    env->equipment.armor_stat_bonuses[2] = 0;
    env->equipment.armor_stat_bonuses[3] = 2;
    
    env->equipment.belt_type = EQUIPMENT_BELT;
    env->equipment.belt_quality = QUALITY_COMMON;
    env->equipment.belt_level = 1;
    env->equipment.belt_stat_bonuses[0] = 0;
    env->equipment.belt_stat_bonuses[1] = 0;
    env->equipment.belt_stat_bonuses[2] = 0;
    env->equipment.belt_stat_bonuses[3] = 1;
    
    env->equipment.pants_type = EQUIPMENT_PANTS;
    env->equipment.pants_quality = QUALITY_COMMON;
    env->equipment.pants_level = 1;
    env->equipment.pants_stat_bonuses[0] = 0;
    env->equipment.pants_stat_bonuses[1] = 1;
    env->equipment.pants_stat_bonuses[2] = 0;
    env->equipment.pants_stat_bonuses[3] = 0;
    
    env->equipment.bracers_type = EQUIPMENT_BRACERS;
    env->equipment.bracers_quality = QUALITY_COMMON;
    env->equipment.bracers_level = 1;
    env->equipment.bracers_stat_bonuses[0] = 0;
    env->equipment.bracers_stat_bonuses[1] = 1;
    env->equipment.bracers_stat_bonuses[2] = 0;
    env->equipment.bracers_stat_bonuses[3] = 0;
    
    calculate_total_stats(env);
    
    memset(env->shop_items, 0, sizeof(env->shop_items));
    
    env->town_interface.frames_remaining = TOWN_MODE_TIME_LIMIT;
    env->town_interface.warning_phase = 0;
    env->town_interface.current_tab = TOWN_TAB_SHOP;
    env->town_interface.character_mode = CHARACTER_MODE_EQUIPMENT;
    env->town_interface.selected_item_index = 0;
    env->town_interface.equipment_slot = 0;
    env->town_interface.input_cooldown = 0;
    env->town_interface.shop_scroll_offset = 0;
    env->town_interface.current_visit_purchases = 0;
}

void give_stat_based_rewards(Rift* env, uint32_t old_str, uint32_t old_dex, uint32_t old_int, uint32_t old_vit, uint32_t old_total_ilvl) {
    int str_diff = env->hero_stats.total_strength - old_str;
    int dex_diff = env->hero_stats.total_dexterity - old_dex;  
    int int_diff = env->hero_stats.total_intelligence - old_int;
    int vit_diff = env->hero_stats.total_vitality - old_vit;
    int total_stat_diff = str_diff + dex_diff + int_diff + vit_diff;
    
    uint32_t current_total_ilvl = env->equipment.weapon_level + env->equipment.offhand_level + 
                                env->equipment.ring_left_level + env->equipment.ring_right_level +
                                env->equipment.amulet_level + env->equipment.boots_level +
                                env->equipment.gloves_level + env->equipment.helmet_level +
                                env->equipment.shoulders_level + env->equipment.armor_level +
                                env->equipment.belt_level + env->equipment.pants_level + env->equipment.bracers_level;
    
    int ilvl_diff = current_total_ilvl - old_total_ilvl;
    
    float stat_reward = 0.0f;
    
    if (total_stat_diff > 0) {
        stat_reward = total_stat_diff * TOWN_STAT_INCREASE_REWARD;
    } else if (total_stat_diff < 0) {
        stat_reward = total_stat_diff * TOWN_STAT_DECREASE_PENALTY * 2.0f;
    } else if (total_stat_diff == 0) {
        if (ilvl_diff < 0) {
            stat_reward = -3.0f * abs(ilvl_diff);
        } else if (ilvl_diff == 0) {
            stat_reward = -2.0f;
        }
    }
    
    float ilvl_reward = ilvl_diff * (ilvl_diff >= 0 ? TOWN_ILVL_INCREASE_REWARD : TOWN_ILVL_DECREASE_PENALTY);
    
    env->step_reward += stat_reward + ilvl_reward;
    env->episode_return += stat_reward + ilvl_reward;
    env->episode_stat_rewards += stat_reward;
    env->episode_item_level_rewards += ilvl_reward;
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
    
    env->hero_stats.average_item_level = equipped_items > 0 ? (total_ilvl + equipped_items - 1) / equipped_items : 0;
    
    env->player.max_health = PLAYER_MAX_HEALTH + (env->hero_stats.total_vitality * 5) + (env->hero_stats.total_strength * 3);
    env->player.max_mana = PLAYER_MAX_MANA + (env->hero_stats.total_intelligence * 2);
    env->player.damage = PLAYER_BASE_DAMAGE + (env->hero_stats.total_strength * 2) + (env->hero_stats.total_intelligence * 1);
    env->player.dodge_chance = env->hero_stats.total_dexterity;
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
        
        item->item_type = (rand() % 13) + 1;
        
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

void transition_to_town(Rift* env) {
    env->current_phase = PHASE_TOWN;
    generate_shop_inventory(env);
    
    env->town_interface.frames_remaining = TOWN_MODE_TIME_LIMIT;
    env->town_interface.warning_phase = 0;
    env->town_interface.current_tab = TOWN_TAB_SHOP;
    env->town_interface.character_mode = CHARACTER_MODE_EQUIPMENT;
    env->town_interface.selected_item_index = 0;
    env->town_interface.equipment_slot = 0;
    env->town_interface.input_cooldown = 0;
    env->town_interface.shop_scroll_offset = 0;
    
    add_experience(env, EXP_PER_RIFT_COMPLETION);
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
        env->episode_progression_rewards += progression_reward;
        
        generate_shop_inventory(env);
        
        env->town_interface.frames_remaining = TOWN_MODE_TIME_LIMIT;
        env->town_interface.warning_phase = 0;
        
        return;
    #endif
    
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
            // Left/right navigation within tabs only, no tab switching
            env->town_interface.input_cooldown = TOWN_INPUT_COOLDOWN;
            break;
        case ACTION_MOVE_RIGHT:
            // Left/right navigation within tabs only, no tab switching
            env->town_interface.input_cooldown = TOWN_INPUT_COOLDOWN;
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
                    
                    int visible_items = 4;
                    int item_y = new_idx * 90;
                    int current_scroll = -env->town_interface.shop_scroll_offset;
                    int container_height = 400;
                    
                    if (item_y < current_scroll) {
                        env->town_interface.shop_scroll_offset = -item_y;
                    } else if (item_y + 90 > current_scroll + container_height) {
                        env->town_interface.shop_scroll_offset = -(item_y - container_height + 90);
                    }
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
                    
                    int visible_items = 4;
                    int item_y = new_idx * 90;
                    int current_scroll = -env->town_interface.shop_scroll_offset;
                    int container_height = 400;
                    
                    if (item_y < current_scroll) {
                        env->town_interface.shop_scroll_offset = -item_y;
                    } else if (item_y + 90 > current_scroll + container_height) {
                        env->town_interface.shop_scroll_offset = -(item_y - container_height + 90);
                    }
                }
            } else {
                env->town_interface.selected_item_index++;
                
                if (env->town_interface.current_tab == TOWN_TAB_CHARACTER) {
                    if (env->town_interface.selected_item_index >= INVENTORY_SLOTS) env->town_interface.selected_item_index = INVENTORY_SLOTS - 1;
                }
            }
            env->town_interface.input_cooldown = TOWN_INPUT_COOLDOWN;
            break;
        case ACTION_SWITCH_TO_SHOP:
            env->town_interface.current_tab = TOWN_TAB_SHOP;
            env->town_interface.shop_scroll_offset = 0;
            
            int first_available = 0;
            while (first_available < SHOP_ITEMS_COUNT && !env->shop_items[first_available].available) {
                first_available++;
            }
            
            env->town_interface.selected_item_index = (first_available < SHOP_ITEMS_COUNT) ? first_available : 0;
            env->town_interface.input_cooldown = TOWN_INPUT_COOLDOWN;
            break;
        case ACTION_SWITCH_TO_CHARACTER:
            env->town_interface.current_tab = TOWN_TAB_CHARACTER;
            env->town_interface.character_mode = CHARACTER_MODE_EQUIPMENT;
            env->town_interface.selected_item_index = 0;
            env->town_interface.input_cooldown = TOWN_INPUT_COOLDOWN;
            break;
    }
}

void buy_shop_item(Rift* env, uint32_t shop_slot) {
    if (shop_slot >= SHOP_ITEMS_COUNT) return;
    
    ShopItem* item = &env->shop_items[shop_slot];
    if (!item->available) return;
    if (env->player.gold < item->price) return;
    
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
    
    Equipment* eq = &env->equipment;
    uint32_t* type_ptr = NULL;
    uint32_t* quality_ptr = NULL;
    uint32_t* level_ptr = NULL;
    uint32_t* stat_bonuses = NULL;
    
    switch (item->item_type) {
        case EQUIPMENT_SHOULDERS:
            type_ptr = &eq->shoulders_type; quality_ptr = &eq->shoulders_quality; level_ptr = &eq->shoulders_level; stat_bonuses = eq->shoulders_stat_bonuses; break;
        case EQUIPMENT_GLOVES:
            type_ptr = &eq->gloves_type; quality_ptr = &eq->gloves_quality; level_ptr = &eq->gloves_level; stat_bonuses = eq->gloves_stat_bonuses; break;
        case EQUIPMENT_RING_LEFT:
            type_ptr = &eq->ring_left_type; quality_ptr = &eq->ring_left_quality; level_ptr = &eq->ring_left_level; stat_bonuses = eq->ring_left_stat_bonuses; break;
        case EQUIPMENT_WEAPON:
            type_ptr = &eq->weapon_type; quality_ptr = &eq->weapon_quality; level_ptr = &eq->weapon_level; stat_bonuses = eq->weapon_stat_bonuses; break;
        case EQUIPMENT_HELMET:
            type_ptr = &eq->helmet_type; quality_ptr = &eq->helmet_quality; level_ptr = &eq->helmet_level; stat_bonuses = eq->helmet_stat_bonuses; break;
        case EQUIPMENT_ARMOR:
            type_ptr = &eq->armor_type; quality_ptr = &eq->armor_quality; level_ptr = &eq->armor_level; stat_bonuses = eq->armor_stat_bonuses; break;
        case EQUIPMENT_BELT:
            type_ptr = &eq->belt_type; quality_ptr = &eq->belt_quality; level_ptr = &eq->belt_level; stat_bonuses = eq->belt_stat_bonuses; break;
        case EQUIPMENT_PANTS:
            type_ptr = &eq->pants_type; quality_ptr = &eq->pants_quality; level_ptr = &eq->pants_level; stat_bonuses = eq->pants_stat_bonuses; break;
        case EQUIPMENT_BOOTS:
            type_ptr = &eq->boots_type; quality_ptr = &eq->boots_quality; level_ptr = &eq->boots_level; stat_bonuses = eq->boots_stat_bonuses; break;
        case EQUIPMENT_AMULET:
            type_ptr = &eq->amulet_type; quality_ptr = &eq->amulet_quality; level_ptr = &eq->amulet_level; stat_bonuses = eq->amulet_stat_bonuses; break;
        case EQUIPMENT_BRACERS:
            type_ptr = &eq->bracers_type; quality_ptr = &eq->bracers_quality; level_ptr = &eq->bracers_level; stat_bonuses = eq->bracers_stat_bonuses; break;
        case EQUIPMENT_RING_RIGHT:
            type_ptr = &eq->ring_right_type; quality_ptr = &eq->ring_right_quality; level_ptr = &eq->ring_right_level; stat_bonuses = eq->ring_right_stat_bonuses; break;
        case EQUIPMENT_OFFHAND:
            type_ptr = &eq->offhand_type; quality_ptr = &eq->offhand_quality; level_ptr = &eq->offhand_level; stat_bonuses = eq->offhand_stat_bonuses; break;
        default:
            return;
    }
    
    if (type_ptr && quality_ptr && level_ptr && stat_bonuses) {
        *type_ptr = item->item_type;
        *quality_ptr = item->item_quality;
        *level_ptr = item->item_level;
        for (int i = 0; i < 4; i++) {
            stat_bonuses[i] = item->stat_bonuses[i];
        }
        
        env->player.gold -= item->price;
        item->available = 0;
        
        env->episode_shop_purchases++;
        env->town_interface.current_visit_purchases++;
        env->episode_vendor_transactions++;
        
        calculate_total_stats(env);
        give_stat_based_rewards(env, old_str, old_dex, old_int, old_vit, old_total_ilvl);
    }
}

void handle_town_interaction(Rift* env) {
    if (env->town_interface.current_tab == TOWN_TAB_CHARACTER) {
        return;
    } else if (env->town_interface.current_tab == TOWN_TAB_SHOP) {
        uint32_t shop_slot = env->town_interface.selected_item_index;
        if (shop_slot < SHOP_ITEMS_COUNT) {
            buy_shop_item(env, shop_slot);
        }
    }
}

#endif