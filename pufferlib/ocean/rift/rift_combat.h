#ifndef RIFT_COMBAT_H
#define RIFT_COMBAT_H

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
    
    for (uint16_t i = 0; i < MONSTER.max_count; i++) {
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
            env->projectiles[i].lifetime = PROJECTILE_LIFETIME + 30;
            env->projectiles[i].active = 1;
            env->projectiles[i].homing = 1;
            
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
        case ATTACK_TYPE_MELEE_PROJECTILE:
            monster->attack_animation_timer = 20;
            if (monster->damage >= env->player.health) {
                env->player.health = 0;
                env->player.alive = 0;
            } else {
                env->player.health -= monster->damage;
            }
            env->episode_damage_taken += monster->damage;
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
                    if (monster->damage >= env->player.health) {
                        env->player.health = 0;
                        env->player.alive = 0;
                    } else {
                        env->player.health -= monster->damage;
                    }
                    env->episode_damage_taken += monster->damage;
                }
            }
            break;
    }
}

void execute_boss_attack(Rift* env) {
    if (!env->boss.alive || !env->player.alive) return;
    
    float dist = distance(env->player.x, env->player.y, env->boss.x, env->boss.y);
    
    if (dist <= BOSS_ATTACK_RANGE && env->boss.attack_cooldown == 0) {
        env->episode_damage_taken += env->boss.damage;
        env->boss.attack_cooldown = GetScaledAttackCooldown(BOSS_ATTACK_COOLDOWN, env->current_rift_level);
        
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
            if (dist_to_player <= PROJECTILE_HIT_RADIUS && env->player.alive) {
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
                
                proj->active = 0;
            }
        }
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
                
                for (uint16_t j = 0; j < MONSTER.max_count; j++) {
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
                                
                                if ((float)rand() / RAND_MAX < env->config.item_drop_rate) {
                                    uint16_t gold_amount = VENDOR.drop_min + rand() % VENDOR.drop_range;
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

#endif