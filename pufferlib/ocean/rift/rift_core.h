#ifndef RIFT_CORE_H
#define RIFT_CORE_H

static void add_log(Rift* env);
void init_hero_stats(Rift* env);
void generate_rift_map(Rift* env);
void generate_town_map(Rift* env);
void generate_shop_inventory(Rift* env);
void spawn_monsters(Rift* env);
void update_monsters(Rift* env);
void update_player(Rift* env);
void update_projectiles(Rift* env);
void update_blizzard_areas(Rift* env);
void execute_boss_attack(Rift* env);
void transition_to_town(Rift* env);
void update_town_timer(Rift* env);

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

void init(Rift* env) {
    env->tick = 0;
    env->config = DEFAULT_CONFIG;
    env->current_phase = PHASE_RIFT;
    env->current_rift_level = 1;
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
    env->max_elites = 2 + env->current_rift_level / 3;
    
    memset(env->items, 0, sizeof(env->items));
    memset(env->projectiles, 0, sizeof(env->projectiles));
    memset(env->blizzard_areas, 0, sizeof(env->blizzard_areas));
    
    memset(&env->boss, 0, sizeof(Boss));
    env->boss_spawned = 0;
    env->rift_completed = 0;
    
    env->episode_length = 0;
    env->episode_return = 0.0f;
    env->step_reward = 0.0f;
    
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
    env->episode_low_health_penalties = 0.0f;
    env->episode_stat_rewards = 0.0f;
    env->episode_item_level_rewards = 0.0f;
    env->episode_progression_rewards = 0.0f;
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
        } else if (IsKeyDown(KEY_ONE)) {
            env->actions[0] = ACTION_SWITCH_TO_SHOP;
        } else if (IsKeyDown(KEY_TWO)) {
            env->actions[0] = ACTION_SWITCH_TO_CHARACTER;
        } else if (IsKeyDown(KEY_R)) {
            env->actions[0] = ACTION_REROLL_SHOP;
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
            execute_boss_attack(env);
            if (env->boss.attack_cooldown > 0) {
                env->boss.attack_cooldown--;
            }
            if (env->boss.special_attack_cooldown > 0) {
                env->boss.special_attack_cooldown--;
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
    
    memset(grid, 0, sizeof(grid));
    
    int half_grid = GRID_SIZE / 2;
    
    if (env->current_phase == PHASE_RIFT) {
        for (uint16_t i = 0; i < MAX_MONSTERS; i++) {
            if (env->monsters[i].alive) {
                int monster_x = (int)env->monsters[i].x;
                int monster_y = (int)env->monsters[i].y;
                int gx = monster_x - player_grid_center_x + half_grid;
                int gy = monster_y - player_grid_center_y + half_grid;
                
                if (gx >= 0 && gx < GRID_SIZE && gy >= 0 && gy < GRID_SIZE) {
                    switch (env->monsters[i].type) {
                        case MONSTER_ZOMBIE: grid[gy * GRID_SIZE + gx] = 0.2f; break;
                        case MONSTER_MAGE: grid[gy * GRID_SIZE + gx] = 0.4f; break;
                        case MONSTER_HEAVY_MELEE: grid[gy * GRID_SIZE + gx] = 0.6f; break;
                        case MONSTER_LIGHT: grid[gy * GRID_SIZE + gx] = 0.3f; break;
                        case MONSTER_ELITE: grid[gy * GRID_SIZE + gx] = 0.8f; break;
                    }
                }
            }
        }
        
        if (env->boss.alive) {
            int boss_x = (int)env->boss.x;
            int boss_y = (int)env->boss.y;
            int gx = boss_x - player_grid_center_x + half_grid;
            int gy = boss_y - player_grid_center_y + half_grid;
            
            if (gx >= 0 && gx < GRID_SIZE && gy >= 0 && gy < GRID_SIZE) {
                grid[gy * GRID_SIZE + gx] = 1.0f;
            }
        }
        
        for (uint16_t i = 0; i < MAX_ITEMS; i++) {
            if (env->items[i].active) {
                int item_x = (int)env->items[i].x;
                int item_y = (int)env->items[i].y;
                int gx = item_x - player_grid_center_x + half_grid;
                int gy = item_y - player_grid_center_y + half_grid;
                
                if (gx >= 0 && gx < GRID_SIZE && gy >= 0 && gy < GRID_SIZE && grid[gy * GRID_SIZE + gx] == 0.0f) {
                    grid[gy * GRID_SIZE + gx] = -0.3f;
                }
            }
        }
    } else if (env->current_phase == PHASE_TOWN) {
        int vendor_gx = VENDOR_POSITION_X - player_grid_center_x + half_grid;
        int vendor_gy = VENDOR_POSITION_Y - player_grid_center_y + half_grid;
        if (vendor_gx >= 0 && vendor_gx < GRID_SIZE && vendor_gy >= 0 && vendor_gy < GRID_SIZE) {
            grid[vendor_gy * GRID_SIZE + vendor_gx] = 0.5f;
        }
        
        int portal_gx = RIFT_PORTAL_X - player_grid_center_x + half_grid;
        int portal_gy = RIFT_PORTAL_Y - player_grid_center_y + half_grid;
        if (portal_gx >= 0 && portal_gx < GRID_SIZE && portal_gy >= 0 && portal_gy < GRID_SIZE) {
            grid[portal_gy * GRID_SIZE + portal_gx] = 0.7f;
        }
    }
    
    for (int gy = 0; gy < GRID_SIZE; gy++) {
        for (int gx = 0; gx < GRID_SIZE; gx++) {
            int world_x = player_grid_center_x + (gx - half_grid);
            int world_y = player_grid_center_y + (gy - half_grid);
            
            if (world_x < 0 || world_x >= MAP_WIDTH || world_y < 0 || world_y >= MAP_HEIGHT) {
                grid[gy * GRID_SIZE + gx] = -1.0f;
            }
        }
    }
    
    for (int i = 0; i < GRID_OBS_SIZE; i++) {
        env->observations[obs_idx++] = grid[i];
    }
    
    if (env->current_phase == PHASE_TOWN) {
        for (int i = 0; i < SHOP_ITEMS_COUNT; i++) {
            ShopItem* item = &env->shop_items[i];
            env->observations[obs_idx++] = item->available ? 1.0f : 0.0f;
            env->observations[obs_idx++] = (float)item->item_quality / 3.0f;
            env->observations[obs_idx++] = (float)item->price / 500.0f;
            env->observations[obs_idx++] = (float)(item->stat_bonuses[0] + item->stat_bonuses[1] + 
                                                  item->stat_bonuses[2] + item->stat_bonuses[3]) / 20.0f;
        }
        
        uint32_t slot_info[13][4];
        for (int slot = 0; slot < 13; slot++) {
            slot_info[slot][0] = 0; slot_info[slot][1] = 0; slot_info[slot][2] = 0; slot_info[slot][3] = 0;
        }
        
        for (int i = 0; i < INVENTORY_SLOTS; i++) {
            InventorySlot* inv_item = &env->player.inventory[i];
            if (inv_item->item_type > 0 && inv_item->item_type <= 13) {
                int slot_idx = inv_item->item_type - 1;
                slot_info[slot_idx][0] = 1;
                slot_info[slot_idx][1] = inv_item->item_quality;
                slot_info[slot_idx][2] = inv_item->item_level;
                slot_info[slot_idx][3] = inv_item->stat_bonuses[0] + inv_item->stat_bonuses[1] + 
                                        inv_item->stat_bonuses[2] + inv_item->stat_bonuses[3];
            }
        }
        
        for (int slot = 0; slot < 13; slot++) {
            env->observations[obs_idx++] = (float)slot_info[slot][0];
            env->observations[obs_idx++] = (float)slot_info[slot][1] / 3.0f;
            env->observations[obs_idx++] = (float)slot_info[slot][2] / 20.0f;
            env->observations[obs_idx++] = (float)slot_info[slot][3] / 40.0f;
        }
        
        env->observations[obs_idx++] = (float)env->hero_stats.level / MAX_HERO_LEVEL;
        env->observations[obs_idx++] = (float)env->hero_stats.stat_points_available / 20.0f;
        env->observations[obs_idx++] = (float)env->player.inventory_count / INVENTORY_SLOTS;
        env->observations[obs_idx++] = env->town_interface.current_tab == TOWN_TAB_SHOP ? 1.0f : 0.0f;
        env->observations[obs_idx++] = env->town_interface.character_mode == CHARACTER_MODE_EQUIPMENT ? 1.0f : 0.0f;
        env->observations[obs_idx++] = (float)env->current_rift_level / 15.0f;
        env->observations[obs_idx++] = (float)env->player.gold / GOLD_NORMALIZATION;
        env->observations[obs_idx++] = (float)env->town_interface.frames_remaining / TOWN_MODE_TIME_LIMIT;
        env->observations[obs_idx++] = (float)env->hero_stats.average_item_level / 200.0f;
    } else {
        for (int i = 0; i < TOWN_OBS_SIZE; i++) {
            env->observations[obs_idx++] = 0.0f;
        }
    }
}

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
    env->log.low_health_penalties += env->episode_low_health_penalties;
    env->log.stat_rewards += env->episode_stat_rewards;
    env->log.item_level_rewards += env->episode_item_level_rewards;
    env->log.progression_rewards += env->episode_progression_rewards;
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

#endif