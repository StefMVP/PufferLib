#ifndef RIFT_RENDER_RIFT_H
#define RIFT_RENDER_RIFT_H

#include "render_core.h"

static void render_background(Rift* env, float cell_size) {
    if (!env->client || !env->client->sprites.tileset.id) {
        Color bg_color = RIFT_COLORS.background;
        DrawRectangle(0, 0, MAP.width * cell_size, MAP.height * cell_size, bg_color);
        return;
    }
    
    SpriteSystem* sprites = &env->client->sprites;
    
    for (uint32_t y = 0; y < MAP.height; y++) {
        for (uint32_t x = 0; x < MAP.width; x++) {
            uint16_t map_index = y * MAP.width + x;
            uint32_t cell_type = env->map[map_index];
            
            uint32_t tile_id;
            if (cell_type == CELLS.wall) {
                tile_id = TILE_IDS.stone_wall;
            } else if (cell_type == CELLS.door) {
                tile_id = TILE_IDS.stone_door;
            } else if (cell_type == CELLS.vendor) {
                tile_id = (env->current_phase == PHASES.town) ? TILE_IDS.vendor_stall : TILE_IDS.stone_floor;
            } else {
                tile_id = (env->current_phase == PHASES.town) ? TILE_IDS.town_floor : TILE_IDS.stone_floor;
            }
            
            draw_sprite_tile(sprites->tileset, tile_id, x * cell_size, y * cell_size, cell_size);
        }
    }
}

static void render_player(Rift* env, float cell_size) {
    if (!env->client || !env->client->sprites.hero_idle.id) {
        Vector2 screen_pos = grid_to_screen(env->player.x, env->player.y, cell_size);
        float half_cell = cell_size * 0.5f;
        screen_pos.x += half_cell;
        screen_pos.y += half_cell;
        
        Color player_color = RIFT_COLORS.player;
        Color armor_color = RIFT_COLORS.armor;
        
        draw_shadow(screen_pos.x, screen_pos.y, cell_size/3, SHADOW_RENDER.offset, SHADOW_RENDER.alpha);
        DrawCircle(screen_pos.x, screen_pos.y, cell_size/3, player_color);
        DrawCircle(screen_pos.x, screen_pos.y, cell_size/4, armor_color);
        return;
    }
    
    SpriteSystem* sprites = &env->client->sprites;
    Vector2 screen_pos = grid_to_screen(env->player.x, env->player.y, cell_size);
    
    sprites->hero_animation_timer++;
    
    uint32_t is_moving = (env->player.movement_x != 0 || env->player.movement_y != 0);
    uint32_t is_casting = (env->player.blizzard_cooldown > 0);
    
    if (is_casting && sprites->hero_animation_state != HERO_ANIM_CAST) {
        sprites->hero_animation_state = HERO_ANIM_CAST;
        sprites->hero_frame = 0;
        sprites->hero_animation_timer = 0;
    } else if (is_moving && sprites->hero_animation_state != HERO_ANIM_WALK && !is_casting) {
        sprites->hero_animation_state = HERO_ANIM_WALK;
        sprites->hero_frame = 0;
        sprites->hero_animation_timer = 0;
    } else if (!is_moving && !is_casting && sprites->hero_animation_state != HERO_ANIM_IDLE) {
        sprites->hero_animation_state = HERO_ANIM_IDLE;
        sprites->hero_frame = 0;
        sprites->hero_animation_timer = 0;
    }
    
    if (sprites->hero_animation_timer >= HERO_ANIMATION.animation_speed) {
        sprites->hero_animation_timer = 0;
        sprites->hero_frame++;
        
        switch (sprites->hero_animation_state) {
            case HERO_ANIM_IDLE:
                if (sprites->hero_frame >= HERO_ANIMATION.idle_frames) sprites->hero_frame = 0;
                break;
            case HERO_ANIM_WALK:
                if (sprites->hero_frame >= HERO_ANIMATION.walk_frames) sprites->hero_frame = 0;
                break;
            case HERO_ANIM_CAST:
                if (sprites->hero_frame >= HERO_ANIMATION.cast_frames) {
                    sprites->hero_animation_state = HERO_ANIMATION.idle_state;
                    sprites->hero_frame = 0;
                }
                break;
        }
    }
    
    Texture2D current_texture;
    uint32_t max_frames;
    
    switch (sprites->hero_animation_state) {
        case HERO_ANIM_WALK:
            current_texture = sprites->hero_walk;
            max_frames = HERO_ANIMATION.walk_frames;
            break;
        case HERO_ANIM_CAST:
            current_texture = sprites->hero_cast;
            max_frames = HERO_ANIMATION.cast_frames;
            break;
        default:
            current_texture = sprites->hero_idle;
            max_frames = HERO_ANIMATION.idle_frames;
            break;
    }
    
    if (sprites->hero_frame >= max_frames) sprites->hero_frame = 0;
    
    Rectangle source = {
        sprites->hero_frame * SPRITE_SIZE,
        0,
        SPRITE_SIZE,
        SPRITE_SIZE
    };
    
    Rectangle dest = {
        screen_pos.x,
        screen_pos.y,
        cell_size,
        cell_size
    };
    
    draw_shadow(screen_pos.x + cell_size/2, screen_pos.y + cell_size/2, cell_size/4, SHADOW_RENDER.offset, SHADOW_RENDER.alpha);
    
    DrawTexturePro(current_texture, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
}

static void render_monsters(Rift* env, float cell_size) {
    for (uint16_t i = 0; i < MONSTER.max_count; i++) {
        if (!env->monsters[i].alive) continue;
        
        Vector2 screen_pos = grid_to_screen(env->monsters[i].x, env->monsters[i].y, cell_size);
        uint32_t monster_type = env->monsters[i].type;
        float size_multiplier = get_monster_size_multiplier(monster_type);
        
        if (env->client && env->client->sprites.monsters[0].id && monster_type < 5) {
            Rectangle source = {0, 0, SPRITE_SIZE, SPRITE_SIZE};
            Rectangle dest = {
                screen_pos.x,
                screen_pos.y,
                cell_size * size_multiplier,
                cell_size * size_multiplier
            };
            
            draw_shadow(dest.x + dest.width/2, dest.y + dest.height/2, dest.width/4, SHADOW_RENDER.offset, SHADOW_RENDER.alpha);
            
            DrawTexturePro(env->client->sprites.monsters[monster_type], source, dest, 
                          (Vector2){0, 0}, 0.0f, WHITE);
            
            if (monster_type == MONSTERS.elite) {
                float glow = calculate_glow(env->tick, GLOW_EFFECT.speed, 0.0f);
                DrawCircleLines(dest.x + dest.width/2, dest.y + dest.height/2, 
                              dest.width/2 * glow, (Color){255, 255, 255, GLOW_EFFECT.alpha});
            }
        } else {
            screen_pos.x += cell_size * 0.5f;
            screen_pos.y += cell_size * 0.5f;
            draw_simple_monster(screen_pos, cell_size, monster_type, env->tick);
        }
        
        float health_ratio = (float)env->monsters[i].health / env->monsters[i].max_health;
        float half_cell = cell_size * 0.5f;
        float third_cell = cell_size / 3.0f;
        uint32_t bar_width = (uint32_t)half_cell;
        uint32_t half_bar_width = bar_width >> 1;
        int16_t bar_x = screen_pos.x + half_cell - half_bar_width;
        int16_t bar_y = screen_pos.y - third_cell - HEALTH_BAR_RENDER.offset;
        
        draw_health_bar(bar_x, bar_y, bar_width, HEALTH_BAR_RENDER.height, health_ratio);
    }
}

static void render_boss(Rift* env, float cell_size) {
    if (!env->boss.alive) return;
    
    Vector2 screen_pos = grid_to_screen(env->boss.x, env->boss.y, cell_size);
    
    if (env->client && env->client->sprites.boss_texture.id) {
        Rectangle source = {0, 0, SPRITE_SIZE * 2, SPRITE_SIZE * 2};
        Rectangle dest = {
            screen_pos.x - cell_size/2,
            screen_pos.y - cell_size/2,
            cell_size * 2,
            cell_size * 2
        };
        
        float pulse = calculate_pulse(env->tick, GLOW_EFFECT.speed, BOSS_EFFECT.pulse_base, BOSS_EFFECT.pulse_amplitude);
        dest.width *= pulse;
        dest.height *= pulse;
        dest.x -= (dest.width - cell_size * 2) / 2;
        dest.y -= (dest.height - cell_size * 2) / 2;
        
        DrawCircle(dest.x + dest.width/2 + BOSS_EFFECT.shadow_offset, dest.y + dest.height/2 + BOSS_EFFECT.shadow_offset, 
                  dest.width/4, (Color){0, 0, 0, BOSS_EFFECT.shadow_alpha});
        
        DrawTexturePro(env->client->sprites.boss_texture, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
        
        DrawCircleLines(dest.x + dest.width/2, dest.y + dest.height/2, cell_size * BOSS_EFFECT.size_multiplier, 
                       (Color){PLAYER_COLORS.armor_r, 0, PLAYER_COLORS.armor_b, BOSS_EFFECT.aura_alpha});
    } else {
        screen_pos.x += cell_size * 0.5f;
        screen_pos.y += cell_size * 0.5f;
        draw_simple_boss(screen_pos, cell_size, env->tick);
    }
    
    float health_ratio = (float)env->boss.health / env->boss.max_health;
    uint32_t bar_width = (uint32_t)cell_size;
    uint32_t half_bar_width = bar_width >> 1;
    int16_t bar_x = screen_pos.x + cell_size/2 - half_bar_width;
    int16_t bar_y = screen_pos.y - cell_size/2 - BOSS_EFFECT.health_bar_offset;
    
    draw_health_bar(bar_x, bar_y, bar_width, BOSS_EFFECT.health_bar_height, health_ratio);
}

static void render_items(Rift* env, float cell_size) {
    for (uint32_t i = 0; i < POTIONS.max_items; i++) {
        if (!env->items[i].active) continue;
        
        Vector2 screen_pos = grid_to_screen(env->items[i].x, env->items[i].y, cell_size);
        uint32_t item_type = env->items[i].type;
        
        if (env->client && env->client->sprites.items[0].id && item_type < 3) {
            Rectangle source = {0, 0, SPRITE_SIZE, SPRITE_SIZE};
            Rectangle dest = {
                screen_pos.x + cell_size/4,
                screen_pos.y + cell_size/4,
                cell_size/2,
                cell_size/2
            };
            
            float glow = calculate_glow(env->tick, ITEM_EFFECT.glow_speed, i);
            dest.width *= glow;
            dest.height *= glow;
            dest.x -= (dest.width - cell_size/2) / 2;
            dest.y -= (dest.height - cell_size/2) / 2;
            
            Color tint = get_item_tint(item_type);
            
            DrawTexturePro(env->client->sprites.items[item_type], source, dest, (Vector2){0, 0}, 0.0f, tint);
            
            if (glow > 1.0f) {
                Color glow_color = tint;
                glow_color.a = 50;
                DrawCircle(dest.x + dest.width/2, dest.y + dest.height/2, dest.width/2 * 1.5f, glow_color);
            }
        } else {
            screen_pos.x += cell_size * 0.5f;
            screen_pos.y += cell_size * 0.5f;
            draw_simple_item(screen_pos, cell_size, item_type, env->tick, i);
        }
    }
}

static void render_projectiles(Rift* env, float cell_size) {
    float half_cell = cell_size * 0.5f;
    float sixth_cell = cell_size * 0.167f;
    
    for (uint32_t i = 0; i < PROJECTILE.max_projectiles; i++) {
        if (!env->projectiles[i].active) continue;
        
        Vector2 screen_pos = grid_to_screen(env->projectiles[i].x, env->projectiles[i].y, cell_size);
        screen_pos.x += half_cell;
        screen_pos.y += half_cell;
        
        Color projectile_color;
        Color halo_color;
        Color core_color;
        float size_multiplier;
        
        if (env->projectiles[i].type == PROJECTILE_TYPES.fireball) {
            projectile_color = (Color){EFFECT_COLORS_EXTENDED.fireball_r, EFFECT_COLORS_EXTENDED.fireball_g, 0, 255};
            halo_color = (Color){EFFECT_COLORS_EXTENDED.halo_r, EFFECT_COLORS_EXTENDED.halo_g, 0, 255};
            core_color = (Color){EFFECT_COLORS_EXTENDED.core_r, EFFECT_COLORS_EXTENDED.core_g, 0, 255};
            size_multiplier = calculate_pulse(env->tick + i, PROJECTILE_EFFECT.pulse_speed, PROJECTILE_EFFECT.pulse_base, PROJECTILE_EFFECT.pulse_amplitude);
        } else if (env->projectiles[i].type == PROJECTILE_TYPES.ice_shard) {
            projectile_color = (Color){150, 200, 255, 255};
            halo_color = (Color){200, 230, 255, 255};
            core_color = (Color){255, 255, 255, 255};
            size_multiplier = 0.8f;
        } else if (env->projectiles[i].type == PROJECTILE_TYPES.stone_chunk) {
            projectile_color = (Color){MATERIAL_COLORS_EXTENDED.brown_handle_light_r, MATERIAL_COLORS_EXTENDED.brown_handle_light_g, MATERIAL_COLORS_EXTENDED.brown_handle_light_b, 255};
            halo_color = (Color){QUALITY_COLORS.silver, QUALITY_COLORS.silver, QUALITY_COLORS.silver, 255};
            core_color = (Color){MATERIAL_COLORS_EXTENDED.brown_handle_dark_r, MATERIAL_COLORS_EXTENDED.brown_handle_dark_g, MATERIAL_COLORS_EXTENDED.brown_handle_dark_b, 255};
            size_multiplier = 1.2f;
        } else if (env->projectiles[i].type == PROJECTILE_TYPES.energy_bolt) {
            projectile_color = (Color){255, 255, 0, 255};
            halo_color = (Color){255, 215, 0, 255};
            core_color = (Color){255, 255, 200, 255};
            size_multiplier = 0.9f;
        } else if (env->projectiles[i].type == PROJECTILE_TYPES.dark_orb) {
            projectile_color = (Color){QUALITY_COLORS.epic_purple_r, QUALITY_COLORS.epic_purple_g, QUALITY_COLORS.epic_purple_b, 255};
            halo_color = (Color){PLAYER_COLORS.player_r, PLAYER_COLORS.player_g, PLAYER_COLORS.player_b, 255};
            core_color = (Color){PLAYER_COLORS.armor_r, PLAYER_COLORS.armor_g, PLAYER_COLORS.armor_b, 255};
            size_multiplier = 1.1f;
        } else if (env->projectiles[i].type == PROJECTILE_TYPES.melee_strike) {
            projectile_color = (Color){255, 0, 0, 255};
            halo_color = (Color){EFFECT_COLORS_EXTENDED.fireball_r, EFFECT_COLORS_EXTENDED.fireball_g, 0, 255};
            core_color = (Color){EFFECT_COLORS_EXTENDED.trail_r, EFFECT_COLORS_EXTENDED.trail_g, 100, 255};
            size_multiplier = 0.7f;
        } else {
            projectile_color = RED;
            halo_color = (Color){EFFECT_COLORS_EXTENDED.halo_r, EFFECT_COLORS_EXTENDED.halo_g, 0, 255};
            core_color = (Color){EFFECT_COLORS_EXTENDED.core_r, EFFECT_COLORS_EXTENDED.core_g, 0, 255};
            size_multiplier = 1.0f;
        }
        
        float lifetime_ratio = (float)env->projectiles[i].lifetime / PROJECTILE_EFFECT.lifetime;
        uint32_t alpha = alpha_from_ratio(lifetime_ratio);
        projectile_color = fade_color(projectile_color, alpha);
        
        float radius = sixth_cell * size_multiplier;
        float half_radius = radius * 0.5f;
        uint32_t half_alpha = alpha >> 1;
        uint32_t third_alpha = alpha / 3;
        
        halo_color = fade_color(halo_color, half_alpha);
        core_color = fade_color(core_color, alpha);
        
        DrawCircle(screen_pos.x, screen_pos.y, radius + PROJECTILE_EFFECT.halo_offset, halo_color);
        DrawCircle(screen_pos.x, screen_pos.y, radius, projectile_color);
        DrawCircle(screen_pos.x, screen_pos.y, half_radius, core_color);
        
        Vector2 trail_end = {
            screen_pos.x - env->projectiles[i].vel_x * PROJECTILE_EFFECT.trail_length,
            screen_pos.y - env->projectiles[i].vel_y * PROJECTILE_EFFECT.trail_length
        };
        DrawLineEx(screen_pos, trail_end, PROJECTILE_EFFECT.trail_width, (Color){EFFECT_COLORS_EXTENDED.trail_r, EFFECT_COLORS_EXTENDED.trail_g, 0, third_alpha});
    }
}

static void render_blizzard_areas(Rift* env, float cell_size) {
    float half_cell = cell_size * 0.5f;
    
    for (uint32_t i = 0; i < BLIZZARD.max_areas; i++) {
        if (!env->blizzard_areas[i].active) continue;
        
        Vector2 screen_pos = grid_to_screen(env->blizzard_areas[i].x, env->blizzard_areas[i].y, cell_size);
        screen_pos.x += half_cell;
        screen_pos.y += half_cell;
        
        float duration_ratio = (float)env->blizzard_areas[i].duration / BLIZZARD.duration;
        uint32_t alpha = (uint32_t)(BLIZZARD_EFFECT.base_alpha * duration_ratio);
        float radius = BLIZZARD.radius * cell_size;
        uint32_t third_alpha = alpha / 3;
        uint32_t half_alpha = alpha >> 1;
        
        Color base_color = (Color){EFFECT_COLORS_EXTENDED.blizzard_base_r, EFFECT_COLORS_EXTENDED.blizzard_base_g, EFFECT_COLORS_EXTENDED.blizzard_base_b, third_alpha};
        Color ice_shard_color = (Color){255, 255, 255, alpha};
        Color frost_color = (Color){EFFECT_COLORS_EXTENDED.frost_r, EFFECT_COLORS_EXTENDED.frost_g, EFFECT_COLORS_EXTENDED.frost_b, half_alpha};
        
        DrawCircle(screen_pos.x, screen_pos.y, radius, base_color);
        
        uint16_t radius_int = (uint16_t)radius;
        uint16_t radius_double = radius_int << 1;
        
        for (uint32_t j = 0; j < BLIZZARD_EFFECT.ice_shard_count; j++) {
            int16_t random_offset_x = (rand() % radius_double) - radius_int;
            int16_t random_offset_y = (rand() % radius_double) - radius_int;
            float distance_squared = random_offset_x * random_offset_x + random_offset_y * random_offset_y;
            
            if (distance_squared <= radius * radius) {
                float x = screen_pos.x + random_offset_x;
                float y = screen_pos.y + random_offset_y;
                
                uint16_t fall_offset = ((env->tick + j * BLIZZARD_EFFECT.shard_timing_mult) % BLIZZARD_EFFECT.shard_cycle_frames);
                y += fall_offset * BLIZZARD_EFFECT.shard_fall_speed;
                
                uint32_t shard_size = BLIZZARD_EFFECT.shard_size_min + (rand() % BLIZZARD_EFFECT.shard_size_range);
                DrawCircle(x, y, shard_size, ice_shard_color);
                
                if ((rand() & 3) == 0) {
                    DrawLine(x - BLIZZARD_EFFECT.frost_line_size, y - BLIZZARD_EFFECT.frost_line_size, x + BLIZZARD_EFFECT.frost_line_size, y + BLIZZARD_EFFECT.frost_line_size, frost_color);
                    DrawLine(x - BLIZZARD_EFFECT.frost_line_size, y + BLIZZARD_EFFECT.frost_line_size, x + BLIZZARD_EFFECT.frost_line_size, y - BLIZZARD_EFFECT.frost_line_size, frost_color);
                }
            }
        }
        
        DrawCircleLines(screen_pos.x, screen_pos.y, radius, frost_color);
    }
}

void render_rift(Rift* env) {
    float cell_size = env->client->cell_size;
    
    BeginMode2D(env->client->camera);
    
    render_background(env, cell_size);
    render_blizzard_areas(env, cell_size);
    render_items(env, cell_size);
    render_monsters(env, cell_size);
    render_boss(env, cell_size);
    render_projectiles(env, cell_size);
    render_player(env, cell_size);
    
    EndMode2D();
}

#endif