#ifndef RIFT_WORLD_H
#define RIFT_WORLD_H

float GetScaledMonsterHealth(float base_health, uint32_t rift_level);
float GetScaledMonsterDamage(float base_damage, uint32_t rift_level);
float GetScaledMonsterSpeed(float base_speed, uint32_t rift_level);
uint32_t GetScaledAttackCooldown(uint32_t base_cooldown, uint32_t rift_level);
void execute_monster_attack(Rift* env, Monster* monster);
void spawn_boss(Rift* env);
void add_experience(Rift* env, uint16_t exp);
void handle_blizzard_with_direction(Rift* env, float facing_x, float facing_y);
void handle_town_navigation(Rift* env, int action);
void handle_town_interaction(Rift* env);
void transition_to_rift(Rift* env);

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

uint32_t get_random_monster_type(Rift* env, uint32_t allow_elite) {
    uint32_t type_roll = rand() % 100;
    
    if (allow_elite && type_roll < 10 && env->elites_spawned < env->max_elites) {
        return MONSTER_ELITE;
    } else if (type_roll < 30) {
        return MONSTER_HEAVY_MELEE;
    } else if (type_roll < 55) {
        return MONSTER_MAGE;
    } else if (type_roll < 80) {
        return MONSTER_LIGHT;
    } else {
        return MONSTER_ZOMBIE;
    }
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
                    default:
                        env->monsters[i].health = (uint32_t)GetScaledMonsterHealth(MONSTER_BASE_HEALTH, env->current_rift_level);
                        env->monsters[i].max_health = env->monsters[i].health;
                        env->monsters[i].damage = (uint32_t)GetScaledMonsterDamage(MONSTER_BASE_DAMAGE, env->current_rift_level);
                        env->monsters[i].speed = GetScaledMonsterSpeed(0.5f, env->current_rift_level);
                        env->monsters[i].attack_range = 2.5f;
                        env->monsters[i].attack_type = ATTACK_TYPE_MELEE_PROJECTILE;
                        break;
                }
                
                env->monsters_spawned++;
                break;
            }
        }
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
                    default:
                        env->monsters[i].health = (uint32_t)GetScaledMonsterHealth(MONSTER_BASE_HEALTH, env->current_rift_level);
                        env->monsters[i].max_health = env->monsters[i].health;
                        env->monsters[i].damage = (uint32_t)(GetScaledMonsterDamage(MONSTER_BASE_DAMAGE, env->current_rift_level) * 0.75f);
                        env->monsters[i].speed = GetScaledMonsterSpeed(0.5f, env->current_rift_level);
                        env->monsters[i].attack_range = 2.5f;
                        env->monsters[i].attack_type = ATTACK_TYPE_MELEE_PROJECTILE;
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

uint32_t GetScaledAttackCooldown(uint32_t base_cooldown, uint32_t rift_level) {
    float scaled_cooldown = base_cooldown * powf(RIFT_MONSTER_ATTACK_SPEED_MULTIPLIER, rift_level - 1);
    return (uint32_t)(scaled_cooldown < 5 ? 5 : scaled_cooldown);
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
    
    if (env->current_phase == PHASE_RIFT && env->player.alive) {
        float health_percent = (float)env->player.health / (float)env->player.max_health;
        if (health_percent <= LOW_HEALTH_THRESHOLD && 
            env->player.health_potion_cooldown == 0 && 
            env->player.low_health_penalty_cooldown == 0) {
            env->step_reward += LOW_HEALTH_PENALTY;
            env->episode_return += LOW_HEALTH_PENALTY;
            env->episode_low_health_penalties += LOW_HEALTH_PENALTY;
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
        if (action == ACTION_MOVE_UP || action == ACTION_MOVE_DOWN || 
            action == ACTION_MOVE_LEFT || action == ACTION_MOVE_RIGHT) {
            handle_town_navigation(env, action);
            return;
        }
        if (action == ACTION_SWITCH_TO_SHOP || action == ACTION_SWITCH_TO_CHARACTER) {
            handle_town_navigation(env, action);
            return;
        }
        if (action == ACTION_EXIT_TOWN) {
            transition_to_rift(env);
            return;
        }
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
                uint32_t heal_amount = env->player.max_health * HEALTH_POTION_HEAL_PERCENT;
                env->player.health = clamp_uint32(env->player.health + heal_amount, 0, env->player.max_health);
                env->player.health_potion_cooldown = HEALTH_POTION_COOLDOWN;
            }
            break;
        case ACTION_USE_MANA_POTION:
            if (env->player.alive && env->player.mana_potion_cooldown == 0 && env->player.mana < env->player.max_mana) {
                uint32_t mana_amount = env->player.max_mana * MANA_POTION_RESTORE_PERCENT;
                env->player.mana = clamp_uint32(env->player.mana + mana_amount, 0, env->player.max_mana);
                env->player.mana_potion_cooldown = MANA_POTION_COOLDOWN;
            }
            break;
        case ACTION_INTERACT:
            if (env->current_phase == PHASE_TOWN) {
                handle_town_interaction(env);
            }
            break;
    }
}

void handle_blizzard(Rift* env) {
    handle_blizzard_with_direction(env, env->player.facing_x, env->player.facing_y);
}

void handle_blizzard_with_direction(Rift* env, float facing_x, float facing_y) {
    if (env->current_phase != PHASE_RIFT) return;
    
    env->player.mana -= BLIZZARD_MANA_COST;
    env->player.blizzard_cooldown = BLIZZARD_ACTIVATION_COOLDOWN;
    env->episode_blizzards_cast++;
    
    env->player.facing_x = facing_x;
    env->player.facing_y = facing_y;
    
    float blizzard_x = env->player.x + facing_x * 2.5f;
    float blizzard_y = env->player.y + facing_y * 2.5f;
    
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
    
    float completion = (float)env->monsters_killed / MONSTERS_TO_SPAWN;
    if (completion >= RIFT_COMPLETION_THRESHOLD && !env->boss_spawned) {
        spawn_boss(env);
    }
}

float GetScaledMonsterHealth(float base_health, uint32_t rift_level) {
    return base_health * powf(RIFT_MONSTER_HEALTH_BASE_MULTIPLIER, rift_level - 1);
}

float GetScaledMonsterDamage(float base_damage, uint32_t rift_level) {
    return base_damage * powf(RIFT_MONSTER_DAMAGE_BASE_MULTIPLIER, rift_level - 1);
}

float GetScaledMonsterSpeed(float base_speed, uint32_t rift_level) {
    return base_speed * powf(RIFT_MONSTER_SPEED_BASE_MULTIPLIER, rift_level - 1);
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
    return base_ilvl + variation + 2;
}

int GetScaledItemPrice(int base_price, uint32_t rift_level, uint32_t quality) {
    float quality_multipliers[] = QUALITY_PRICE_MULTIPLIERS;
    float rift_multiplier = powf(RIFT_PRICE_BASE_MULTIPLIER, rift_level - 1);
    return (int)(base_price * rift_multiplier * quality_multipliers[quality]);
}

#endif