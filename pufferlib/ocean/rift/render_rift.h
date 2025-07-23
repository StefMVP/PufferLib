#ifndef RIFT_RENDER_RIFT_H
#define RIFT_RENDER_RIFT_H

#include "render_core.h"

typedef struct {
    Color dark_stone, darker_stone;
    Color wall_tint, door_tint, vendor_tint, town_tint;
    Color rift_base_tint;
} TileColorScheme;

typedef struct {
    Color mana_aura, hurt_effect;
    Color cast_effect;
    float half_cell_divisor, quarter_cell_divisor, third_cell_divisor;
    float mana_aura_divisor;
} PlayerRenderConfig;

typedef struct {
    Color elite_glow, mage_tint, heavy_tint;
    Color elite_shadow, mage_shadow, heavy_shadow;
    Color health_bg_high, health_bg_med, health_bg_low;
    Color hurt_effect;
    float low_health_threshold, med_health_threshold;
    float hurt_pulse_speed;
} MonsterRenderConfig;

static const TileColorScheme TILE_COLORS = {
    .dark_stone = {25, 22, 18, 255},
    .darker_stone = {15, 13, 10, 255},
    .wall_tint = {95, 85, 75, 255},
    .door_tint = {120, 90, 60, 255},
    .vendor_tint = {88, 80, 70, 255},
    .town_tint = {88, 82, 70, 255},
    .rift_base_tint = {85, 85, 85, 255}
};

static const PlayerRenderConfig PLAYER_RENDER = {
    .mana_aura = {100, 150, 255, 60},
    .hurt_effect = {255, 0, 0, 255},
    .cast_effect = {173, 216, 230, 80},
    .half_cell_divisor = 2.0f,
    .quarter_cell_divisor = 4.0f,
    .third_cell_divisor = 3.0f,
    .mana_aura_divisor = 2.5f
};

static const MonsterRenderConfig MONSTER_RENDER = {
    .elite_glow = {255, 220, 220, 255},
    .mage_tint = {200, 180, 255, 255},
    .heavy_tint = {255, 200, 180, 255},
    .elite_shadow = {60, 20, 20, 140},
    .mage_shadow = {20, 20, 60, 120},
    .heavy_shadow = {40, 25, 10, 130},
    .health_bg_high = {50, 200, 50, 255},
    .health_bg_med = {200, 150, 50, 255},
    .health_bg_low = {200, 50, 50, 255},
    .hurt_effect = {255, 50, 50, 60},
    .low_health_threshold = 0.25f,
    .med_health_threshold = 0.5f,
    .hurt_pulse_speed = 8.0f
};

static void render_background(Rift* env, float cell_size) {
    float map_width = MAP.width * cell_size;
    float map_height = MAP.height * cell_size;
    
    for (int y = 0; y < map_height; y++) {
        float t = (float)y / map_height;
        Color current = {
            (uint32_t)(TILE_COLORS.dark_stone.r * (1-t) + TILE_COLORS.darker_stone.r * t),
            (uint32_t)(TILE_COLORS.dark_stone.g * (1-t) + TILE_COLORS.darker_stone.g * t),
            (uint32_t)(TILE_COLORS.dark_stone.b * (1-t) + TILE_COLORS.darker_stone.b * t),
            255
        };
        DrawRectangle(0, y, map_width, 1, current);
    }
    
    if (!env->client || !env->client->sprites.tileset.id) {
        return;
    }
    
    SpriteSystem* sprites = &env->client->sprites;
    float time_pulse = sinf(GetTime() * 0.3f) * 0.1f + 0.9f;
    
    for (uint32_t y = 0; y < MAP.height; y++) {
        for (uint32_t x = 0; x < MAP.width; x++) {
            uint16_t map_index = y * MAP.width + x;
            uint32_t cell_type = env->map[map_index];
            
            float tile_x = x * cell_size;
            float tile_y = y * cell_size;
            
            uint32_t tile_id;
            Color tint = WHITE;
            
            if (cell_type == CELLS.wall) {
                tile_id = TILE_IDS.stone_wall;
                tint = TILE_COLORS.wall_tint;
                DrawRectangle(tile_x + 1, tile_y + 1, cell_size - 2, cell_size - 2, (Color){10, 8, 6, 100});
            } else if (cell_type == CELLS.door) {
                tile_id = TILE_IDS.stone_door;
                tint = TILE_COLORS.door_tint;
                float door_glow = time_pulse * 0.8f + 0.2f;
                tint.r *= door_glow; tint.g *= door_glow; tint.b *= door_glow;
            } else if (cell_type == CELLS.vendor) {
                tile_id = (env->current_phase == PHASES.town) ? TILE_IDS.vendor_stall : TILE_IDS.stone_floor;
                tint = TILE_COLORS.vendor_tint;
            } else {
                tile_id = (env->current_phase == PHASES.town) ? TILE_IDS.town_floor : TILE_IDS.stone_floor;
                
                if (env->current_phase == PHASES.rift) {
                    float distance_from_top = (float)y / MAP.height;
                    
                    float base_gray = TILE_COLORS.rift_base_tint.r + distance_from_top * 15;
                    float variation = sinf(x * 0.2f + y * 0.15f) * 8;
                    
                    uint8_t gray_value = (uint8_t)(base_gray + variation);
                    tint = (Color){gray_value, gray_value, gray_value, 255};
                    
                } else {
                    tint = TILE_COLORS.town_tint;
                }
            }
            
            draw_sprite_tile(sprites->tileset, tile_id, tile_x, tile_y, cell_size);
            
            Rectangle tile_rect = {tile_x, tile_y, cell_size, cell_size};
            DrawTexturePro(sprites->tileset, 
                          (Rectangle){(tile_id % TILESET_COLS) * SPRITE_SIZE, (tile_id / TILESET_COLS) * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE},
                          tile_rect, (Vector2){0, 0}, 0.0f, tint);
        }
    }
}

static void render_player(Rift* env, float cell_size) {
    if (!env->client || !env->client->sprites.hero_idle.id) {
        Vector2 screen_pos = grid_to_screen(env->player.x, env->player.y, cell_size);
        float half_cell = cell_size / PLAYER_RENDER.half_cell_divisor;
        screen_pos.x += half_cell;
        screen_pos.y += half_cell;
        
        Color player_color = RIFT_COLORS.player;
        Color armor_color = RIFT_COLORS.armor;
        
        float health_ratio = (float)env->player.health / env->player.max_health;
        if (health_ratio < 0.3f) {
            float hurt_pulse = sinf(GetTime() * 6.0f) * 0.4f + 0.6f;
            player_color.r = PLAYER_RENDER.hurt_effect.r;
            player_color.g *= hurt_pulse;
            player_color.b *= hurt_pulse;
        }
        
        float mana_glow = (float)env->player.mana / env->player.max_mana;
        Color mana_aura = {PLAYER_RENDER.mana_aura.r, PLAYER_RENDER.mana_aura.g, PLAYER_RENDER.mana_aura.b, (uint8_t)(PLAYER_RENDER.mana_aura.a * mana_glow)};
        DrawCircle(screen_pos.x, screen_pos.y, cell_size / PLAYER_RENDER.mana_aura_divisor, mana_aura);
        
        draw_shadow(screen_pos.x, screen_pos.y, cell_size / PLAYER_RENDER.third_cell_divisor, SHADOW_RENDER.offset, SHADOW_RENDER.alpha + 20);
        DrawCircle(screen_pos.x, screen_pos.y, cell_size / PLAYER_RENDER.third_cell_divisor, player_color);
        DrawCircle(screen_pos.x, screen_pos.y, cell_size / PLAYER_RENDER.quarter_cell_divisor, armor_color);
        
        if (env->player.blizzard_cooldown > 0) {
            float cast_intensity = (float)env->player.blizzard_cooldown / 20.0f;
            DrawCircle(screen_pos.x, screen_pos.y, cell_size / PLAYER_RENDER.third_cell_divisor + 3, (Color){PLAYER_RENDER.cast_effect.r, PLAYER_RENDER.cast_effect.g, PLAYER_RENDER.cast_effect.b, (uint8_t)(PLAYER_RENDER.cast_effect.a * cast_intensity)});
        }
        
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
                if ((uint32_t)sprites->hero_frame >= HERO_ANIMATION.idle_frames) sprites->hero_frame = 0;
                break;
            case HERO_ANIM_WALK:
                if ((uint32_t)sprites->hero_frame >= HERO_ANIMATION.walk_frames) sprites->hero_frame = 0;
                break;
            case HERO_ANIM_CAST:
                if ((uint32_t)sprites->hero_frame >= HERO_ANIMATION.cast_frames) {
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
    
    if ((uint32_t)sprites->hero_frame >= max_frames) sprites->hero_frame = 0;
    
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
    float pulse = get_ui_pulse(UI_ANIMATION.ui_pulse_med_speed, UI_ANIMATION.ui_pulse_base + 0.1f, UI_ANIMATION.ui_pulse_amplitude - 0.1f);
    
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
            
            Color monster_tint = WHITE;
            Color shadow_color = {0, 0, 0, SHADOW_RENDER.alpha + 20};
            float glow_intensity = 1.0f;
            
            if (monster_type == MONSTERS.elite) {
                glow_intensity = pulse * 1.5f + 0.5f;
                monster_tint = (Color){255, 220 + (uint8_t)(35 * pulse), 220 + (uint8_t)(35 * pulse), 255};
                shadow_color = (Color){60, 20, 20, SHADOW_RENDER.alpha + 40};
                
                float elite_glow = dest.width * 0.6f * glow_intensity;
                DrawCircle(dest.x + dest.width/2, dest.y + dest.height/2, elite_glow, 
                          (Color){MONSTER_COLORS.elite_r, MONSTER_COLORS.elite_g, MONSTER_COLORS.elite_b, 30});
            } else if (monster_type == MONSTERS.mage) {
                monster_tint = (Color){200 + (uint8_t)(55 * pulse), 180 + (uint8_t)(75 * pulse), 255, 255};
                shadow_color = (Color){20, 20, 60, SHADOW_RENDER.alpha + 20};
            } else if (monster_type == MONSTERS.heavy_melee) {
                monster_tint = (Color){255, 200 + (uint8_t)(55 * pulse), 180 + (uint8_t)(75 * pulse), 255};
                shadow_color = (Color){40, 25, 10, SHADOW_RENDER.alpha + 30};
            }
            
            DrawCircle(dest.x + dest.width/2 + SHADOW_RENDER.offset, dest.y + dest.height/2 + SHADOW_RENDER.offset, 
                      dest.width/3, shadow_color);
            
            DrawTexturePro(env->client->sprites.monsters[monster_type], source, dest, 
                          (Vector2){0, 0}, 0.0f, monster_tint);
            
            if (monster_type == MONSTERS.elite) {
                float glow = calculate_glow(env->tick, GLOW_EFFECT.speed, i * 0.5f);
                Color elite_aura = {255, 255, 255, (uint8_t)(GLOW_EFFECT.alpha * glow)};
                DrawCircleLines(dest.x + dest.width/2, dest.y + dest.height/2, 
                              dest.width/2 * glow, elite_aura);
                DrawCircleLines(dest.x + dest.width/2, dest.y + dest.height/2, 
                              dest.width/2 * glow * 0.7f, (Color){255, 200, 200, (uint8_t)(60 * glow)});
            }
            
            if (env->monsters[i].health < env->monsters[i].max_health * 0.3f) {
                float hurt_pulse = sinf(GetTime() * 8.0f) * 0.3f + 0.7f;
                DrawCircle(dest.x + dest.width/2, dest.y + dest.height/2, dest.width/4, 
                          (Color){255, 50, 50, (uint8_t)(60 * hurt_pulse)});
            }
        } else {
            screen_pos.x += cell_size * 0.5f;
            screen_pos.y += cell_size * 0.5f;
            draw_simple_monster(screen_pos, cell_size, monster_type, env->tick);
        }
        
        float health_ratio = (float)env->monsters[i].health / env->monsters[i].max_health;
        float half_cell = cell_size * 0.5f;
        float third_cell = cell_size / 3.0f;
        uint32_t bar_width = (uint32_t)(half_cell * 1.2f);
        uint32_t half_bar_width = bar_width >> 1;
        int16_t bar_x = screen_pos.x + half_cell - half_bar_width;
        int16_t bar_y = screen_pos.y - third_cell - HEALTH_BAR_RENDER.offset - 2;
        
        DrawRectangle(bar_x - 1, bar_y - 1, bar_width + 2, HEALTH_BAR_RENDER.height + 2, 
                      (Color){0, 0, 0, 180});
        
        Color health_bg = (Color){60, 20, 20, 255};
        Color health_fill = health_ratio > 0.5f ? (Color){50, 200, 50, 255} : 
                           health_ratio > 0.25f ? (Color){200, 150, 50, 255} : 
                           (Color){200, 50, 50, 255};
        
        DrawRectangle(bar_x, bar_y, bar_width, HEALTH_BAR_RENDER.height, health_bg);
        DrawRectangle(bar_x, bar_y, (int)(bar_width * health_ratio), HEALTH_BAR_RENDER.height, health_fill);
        DrawRectangleLines(bar_x, bar_y, bar_width, HEALTH_BAR_RENDER.height, (Color){255, 255, 255, 200});
    }
}

static void render_boss(Rift* env, float cell_size) {
    if (!env->boss.alive) return;
    
    Vector2 screen_pos = grid_to_screen(env->boss.x, env->boss.y, cell_size);
    float time_pulse = get_ui_pulse(UI_ANIMATION.ui_pulse_fast_speed, UI_ANIMATION.ui_pulse_base, UI_ANIMATION.ui_pulse_amplitude);
    
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
        
        float danger_aura = dest.width * 1.8f * time_pulse;
        DrawCircle(dest.x + dest.width/2, dest.y + dest.height/2, danger_aura, 
                  (Color){120, 20, 20, 25});
        
        DrawCircle(dest.x + dest.width/2 + BOSS_EFFECT.shadow_offset + 2, dest.y + dest.height/2 + BOSS_EFFECT.shadow_offset + 2, 
                  dest.width/3, (Color){0, 0, 0, BOSS_EFFECT.shadow_alpha + 30});
        
        Color boss_tint = {255, 200 + (uint8_t)(55 * time_pulse), 200 + (uint8_t)(55 * time_pulse), 255};
        DrawTexturePro(env->client->sprites.boss_texture, source, dest, (Vector2){0, 0}, 0.0f, boss_tint);
        
        for (int i = 0; i < 8; i++) {
            float angle = (i * 45 + GetTime() * 30) * DEG2RAD;
            float orbit_radius = cell_size * BOSS_EFFECT.size_multiplier * 1.2f;
            float orbit_x = dest.x + dest.width/2 + cosf(angle) * orbit_radius;
            float orbit_y = dest.y + dest.height/2 + sinf(angle) * orbit_radius;
            DrawCircle(orbit_x, orbit_y, 3, (Color){MONSTER_COLORS.elite_r, MONSTER_COLORS.elite_g, MONSTER_COLORS.elite_b, (uint8_t)(150 * time_pulse)});
        }
        
        DrawCircleLines(dest.x + dest.width/2, dest.y + dest.height/2, cell_size * BOSS_EFFECT.size_multiplier, 
                       (Color){PLAYER_COLORS.armor_r, 0, PLAYER_COLORS.armor_b, BOSS_EFFECT.aura_alpha + (uint8_t)(50 * time_pulse)});
        DrawCircleLines(dest.x + dest.width/2, dest.y + dest.height/2, cell_size * BOSS_EFFECT.size_multiplier * 0.7f, 
                       (Color){255, 100, 100, (uint8_t)(80 * time_pulse)});
    } else {
        screen_pos.x += cell_size * 0.5f;
        screen_pos.y += cell_size * 0.5f;
        draw_simple_boss(screen_pos, cell_size, env->tick);
    }
    
    float health_ratio = (float)env->boss.health / env->boss.max_health;
    uint32_t bar_width = (uint32_t)(cell_size * 1.5f);
    uint32_t half_bar_width = bar_width >> 1;
    int16_t bar_x = screen_pos.x + cell_size/2 - half_bar_width;
    int16_t bar_y = screen_pos.y - cell_size/2 - BOSS_EFFECT.health_bar_offset - 5;
    
    DrawRectangle(bar_x - 2, bar_y - 2, bar_width + 4, BOSS_EFFECT.health_bar_height + 4, 
                  (Color){0, 0, 0, 200});
    
    Color boss_health_bg = {80, 20, 20, 255};
    Color boss_health_fill = health_ratio > 0.3f ? (Color){200, 50, 50, 255} : (Color){255, 20, 20, 255};
    
    DrawRectangle(bar_x, bar_y, bar_width, BOSS_EFFECT.health_bar_height, boss_health_bg);
    DrawRectangle(bar_x, bar_y, (int)(bar_width * health_ratio), BOSS_EFFECT.health_bar_height, boss_health_fill);
    DrawRectangleLines(bar_x, bar_y, bar_width, BOSS_EFFECT.health_bar_height, (Color){255, 200, 200, 255});
    
    char boss_text[32];
    sprintf(boss_text, "BOSS");
    DrawText(boss_text, bar_x + bar_width/2 - 20, bar_y - 18, 12, (Color){255, 100, 100, 255});
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
        Color outer_glow;
        float size_multiplier;
        float pulse_speed = 0.3f;
        
        if (env->projectiles[i].type == PROJECTILE_TYPES.fireball) {
            projectile_color = (Color){EFFECT_COLORS_EXTENDED.fireball_r, EFFECT_COLORS_EXTENDED.fireball_g, 0, 255};
            halo_color = (Color){EFFECT_COLORS_EXTENDED.halo_r, EFFECT_COLORS_EXTENDED.halo_g, 0, 255};
            core_color = (Color){255, 255, 200, 255};
            outer_glow = (Color){255, 100, 0, 40};
            size_multiplier = calculate_pulse(env->tick + i, pulse_speed, PROJECTILE_EFFECT.pulse_base, PROJECTILE_EFFECT.pulse_amplitude);
        } else if (env->projectiles[i].type == PROJECTILE_TYPES.ice_shard) {
            projectile_color = (Color){150, 200, 255, 255};
            halo_color = (Color){200, 230, 255, 255};
            core_color = (Color){255, 255, 255, 255};
            outer_glow = (Color){100, 150, 255, 35};
            size_multiplier = 0.8f + sinf(env->tick * 0.2f + i) * 0.1f;
        } else if (env->projectiles[i].type == PROJECTILE_TYPES.stone_chunk) {
            projectile_color = (Color){MATERIAL_COLORS_EXTENDED.brown_handle_light_r, MATERIAL_COLORS_EXTENDED.brown_handle_light_g, MATERIAL_COLORS_EXTENDED.brown_handle_light_b, 255};
            halo_color = (Color){QUALITY_COLORS.silver, QUALITY_COLORS.silver, QUALITY_COLORS.silver, 255};
            core_color = (Color){MATERIAL_COLORS_EXTENDED.brown_handle_dark_r, MATERIAL_COLORS_EXTENDED.brown_handle_dark_g, MATERIAL_COLORS_EXTENDED.brown_handle_dark_b, 255};
            outer_glow = (Color){100, 80, 60, 30};
            size_multiplier = 1.2f;
        } else if (env->projectiles[i].type == PROJECTILE_TYPES.energy_bolt) {
            projectile_color = (Color){255, 255, 0, 255};
            halo_color = (Color){255, 215, 0, 255};
            core_color = (Color){255, 255, 255, 255};
            outer_glow = (Color){255, 255, 0, 50};
            size_multiplier = 0.9f + sinf(env->tick * 0.5f + i) * 0.2f;
        } else if (env->projectiles[i].type == PROJECTILE_TYPES.dark_orb) {
            projectile_color = (Color){QUALITY_COLORS.epic_purple_r, QUALITY_COLORS.epic_purple_g, QUALITY_COLORS.epic_purple_b, 255};
            halo_color = (Color){PLAYER_COLORS.player_r, PLAYER_COLORS.player_g, PLAYER_COLORS.player_b, 255};
            core_color = (Color){PLAYER_COLORS.armor_r, PLAYER_COLORS.armor_g, PLAYER_COLORS.armor_b, 255};
            outer_glow = (Color){80, 20, 120, 45};
            size_multiplier = 1.1f + sinf(env->tick * 0.4f + i) * 0.15f;
        } else if (env->projectiles[i].type == PROJECTILE_TYPES.melee_strike) {
            projectile_color = (Color){255, 0, 0, 255};
            halo_color = (Color){EFFECT_COLORS_EXTENDED.fireball_r, EFFECT_COLORS_EXTENDED.fireball_g, 0, 255};
            core_color = (Color){255, 200, 200, 255};
            outer_glow = (Color){255, 50, 50, 60};
            size_multiplier = 0.7f;
        } else {
            projectile_color = RED;
            halo_color = (Color){EFFECT_COLORS_EXTENDED.halo_r, EFFECT_COLORS_EXTENDED.halo_g, 0, 255};
            core_color = (Color){EFFECT_COLORS_EXTENDED.core_r, EFFECT_COLORS_EXTENDED.core_g, 0, 255};
            outer_glow = (Color){200, 50, 50, 40};
            size_multiplier = 1.0f;
        }
        
        float lifetime_ratio = (float)env->projectiles[i].lifetime / PROJECTILE_EFFECT.lifetime;
        uint32_t alpha = alpha_from_ratio(lifetime_ratio);
        
        float radius = sixth_cell * size_multiplier;
        float glow_radius = radius * 2.5f;
        float halo_radius = radius + PROJECTILE_EFFECT.halo_offset;
        float core_radius = radius * 0.4f;
        
        uint32_t half_alpha = alpha >> 1;
        uint32_t third_alpha = alpha / 3;
        uint32_t glow_alpha = alpha / 4;
        
        outer_glow = fade_color(outer_glow, glow_alpha);
        projectile_color = fade_color(projectile_color, alpha);
        halo_color = fade_color(halo_color, half_alpha);
        core_color = fade_color(core_color, alpha);
        
        DrawCircle(screen_pos.x, screen_pos.y, glow_radius, outer_glow);
        DrawCircle(screen_pos.x, screen_pos.y, halo_radius, halo_color);
        DrawCircle(screen_pos.x, screen_pos.y, radius, projectile_color);
        DrawCircle(screen_pos.x, screen_pos.y, core_radius, core_color);
        
        if (env->projectiles[i].type == PROJECTILE_TYPES.energy_bolt) {
            float spark_time = GetTime() + i * 0.3f;
            for (int j = 0; j < 4; j++) {
                float angle = (j * 90 + spark_time * 180) * DEG2RAD;
                float spark_x = screen_pos.x + cosf(angle) * radius * 1.5f;
                float spark_y = screen_pos.y + sinf(angle) * radius * 1.5f;
                DrawCircle(spark_x, spark_y, 2, (Color){255, 255, 255, alpha/2});
            }
        }
        
        Vector2 trail_end = {
            screen_pos.x - env->projectiles[i].vel_x * PROJECTILE_EFFECT.trail_length * 1.5f,
            screen_pos.y - env->projectiles[i].vel_y * PROJECTILE_EFFECT.trail_length * 1.5f
        };
        
        Vector2 trail_mid = {
            screen_pos.x - env->projectiles[i].vel_x * PROJECTILE_EFFECT.trail_length * 0.75f,
            screen_pos.y - env->projectiles[i].vel_y * PROJECTILE_EFFECT.trail_length * 0.75f
        };
        
        Color trail_color = fade_color(halo_color, third_alpha);
        Color trail_fade = fade_color(halo_color, third_alpha/2);
        
        DrawLineEx(screen_pos, trail_mid, PROJECTILE_EFFECT.trail_width + 1, trail_color);
        DrawLineEx(trail_mid, trail_end, PROJECTILE_EFFECT.trail_width, trail_fade);
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

static void render_environmental_effects(Rift* env, float cell_size) {
    float time = GetTime();
    uint32_t map_width = MAP.width * cell_size;
    uint32_t map_height = MAP.height * cell_size;
    
    for (int i = 0; i < 20; i++) {
        float particle_time = time + i * 0.3f;
        float particle_x = (sinf(particle_time * 0.2f) * 0.5f + 0.5f) * map_width;
        float particle_y = (sinf(particle_time * 0.15f + 1.0f) * 0.5f + 0.5f) * map_height;
        
        float particle_size = 1 + sinf(particle_time * 0.8f) * 0.5f;
        uint8_t alpha = (uint8_t)(30 + 20 * sinf(particle_time * 1.2f));
        
        DrawCircle(particle_x, particle_y, particle_size, (Color){100, 120, 140, alpha});
    }
    
    if (env->boss_spawned && env->boss.alive) {
        for (int i = 0; i < 15; i++) {
            float danger_time = time * 2 + i * 0.4f;
            float danger_x = (sinf(danger_time * 0.3f) * 0.5f + 0.5f) * map_width;
            float danger_y = (sinf(danger_time * 0.25f + 2.0f) * 0.5f + 0.5f) * map_height;
            
            float danger_size = 2 + sinf(danger_time * 1.5f) * 1.0f;
            uint8_t danger_alpha = (uint8_t)(20 + 15 * sinf(danger_time * 0.8f));
            
            DrawCircle(danger_x, danger_y, danger_size, (Color){200, 80, 80, danger_alpha});
        }
    }
    
    for (int edge = 0; edge < 4; edge++) {
        for (int j = 0; j < 8; j++) {
            float edge_time = time * 0.5f + j * 0.8f + edge * 1.5f;
            float edge_intensity = sinf(edge_time) * 0.3f + 0.7f;
            uint8_t edge_alpha = (uint8_t)(25 * edge_intensity);
            
            if (edge == 0) {
                DrawRectangle(j * (map_width/8), 0, map_width/8, 3, (Color){60, 80, 100, edge_alpha});
            } else if (edge == 1) {
                DrawRectangle(j * (map_width/8), map_height - 3, map_width/8, 3, (Color){60, 80, 100, edge_alpha});
            } else if (edge == 2) {
                DrawRectangle(0, j * (map_height/8), 3, map_height/8, (Color){60, 80, 100, edge_alpha});
            } else {
                DrawRectangle(map_width - 3, j * (map_height/8), 3, map_height/8, (Color){60, 80, 100, edge_alpha});
            }
        }
    }
}

void render_rift(Rift* env) {
    float cell_size = env->client->cell_size;
    
    BeginMode2D(env->client->camera);
    
    render_background(env, cell_size);
    render_environmental_effects(env, cell_size);
    render_blizzard_areas(env, cell_size);
    render_items(env, cell_size);
    render_monsters(env, cell_size);
    render_boss(env, cell_size);
    render_projectiles(env, cell_size);
    render_player(env, cell_size);
    
    EndMode2D();
}

#endif