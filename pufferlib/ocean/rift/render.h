#ifndef RIFT_RENDER_H
#define RIFT_RENDER_H

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include "raylib.h"
#include "constants.h"

typedef struct SpriteSystem SpriteSystem;
typedef struct Rift Rift;
typedef struct Client Client;

// ============================================================================
// RENDERING FUNCTION DECLARATIONS
// ============================================================================

void load_sprites(SpriteSystem* sprites);
void unload_sprites(SpriteSystem* sprites);
void c_render(Rift* env);
void render_rift(Rift* env);
void render_town(Rift* env);

static void render_background(Rift* env, float cell_size);
static void render_player(Rift* env, float cell_size);
static void render_monsters(Rift* env, float cell_size);
static void render_boss(Rift* env, float cell_size);
static void render_items(Rift* env, float cell_size);
static void render_projectiles(Rift* env, float cell_size);
static void render_blizzard_areas(Rift* env, float cell_size);
static void render_ui(Rift* env, uint16_t screen_width, uint16_t screen_height);

// ============================================================================
// UTILITY FUNCTIONS  
// ============================================================================

static inline Vector2 grid_to_screen(float grid_x, float grid_y, float cell_size) {
    return (Vector2){ grid_x * cell_size, grid_y * cell_size };
}

static Client* make_client(Rift* env) {
    Client* client = (Client*)calloc(1, sizeof(Client));
    client->cell_size = CLIENT_CELL_SIZE;
    client->width = CLIENT_WIDTH;
    client->height = CLIENT_HEIGHT;
    
    InitWindow(client->width, client->height, "PufferLib Rift");
    SetTargetFPS(CLIENT_FPS);
    
    client->camera.target = (Vector2){ MAP_WIDTH * client->cell_size / 2, MAP_HEIGHT * client->cell_size / 2 };
    client->camera.offset = (Vector2){ client->width / 2, client->height / 2 };
    client->camera.rotation = ZERO_VALUE;
    client->camera.zoom = 1.0f;
    
    load_sprites(&client->sprites);
    
    return client;
}

// ============================================================================
// SPRITE LOADING FUNCTIONS
// ============================================================================

void load_sprites(SpriteSystem* sprites) {
    sprites->tile_size = SPRITE_SIZE;
    sprites->hero_frame = 0;
    sprites->hero_animation_timer = 0;
    sprites->hero_animation_state = HERO_ANIM_IDLE;
    
    Image tileset_img = GenImageColor(TILESET_COLS * SPRITE_SIZE, TILESET_ROWS * SPRITE_SIZE, (Color){0, 0, 0, 255});
    
    for (int y = 0; y < TILESET_ROWS; y++) {
        for (int x = 0; x < TILESET_COLS; x++) {
            Rectangle tile_rect = {x * SPRITE_SIZE, y * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE};
            int tile_type = y * TILESET_COLS + x;
            
            switch (tile_type) {
                case TILE_STONE_FLOOR: {
                    Color base_color = (Color){72, 61, 50, 255};
                    Color grout_color = (Color){56, 47, 38, 255};
                    Color highlight_color = (Color){88, 75, 62, 255};
                    
                    ImageDrawRectangleRec(&tileset_img, tile_rect, base_color);
                    
                    for (int tx = 0; tx < 4; tx++) {
                        for (int ty = 0; ty < 4; ty++) {
                            int stone_x = x * SPRITE_SIZE + tx * 8;
                            int stone_y = y * SPRITE_SIZE + ty * 8;
                            
                            ImageDrawRectangle(&tileset_img, stone_x + 1, stone_y + 1, 6, 6, highlight_color);
                            Rectangle stone_rect = {stone_x, stone_y, 8, 8};
                            ImageDrawRectangleLines(&tileset_img, stone_rect, 1, grout_color);
                            
                            if (rand() % 4 == 0) {
                                int crack_x = stone_x + (rand() % 6) + 1;
                                int crack_y = stone_y + (rand() % 6) + 1;
                                ImageDrawPixel(&tileset_img, crack_x, crack_y, grout_color);
                            }
                        }
                    }
                    break;
                }
                case TILE_STONE_WALL: {
                    Color wall_color = (Color){65, 55, 45, 255};
                    Color mortar_color = (Color){45, 35, 25, 255};
                    Color highlight_color = (Color){85, 75, 65, 255};
                    
                    ImageDrawRectangleRec(&tileset_img, tile_rect, wall_color);
                    
                    for (int bx = 0; bx < 2; bx++) {
                        for (int by = 0; by < 4; by++) {
                            int brick_x = x * SPRITE_SIZE + bx * 16 + (by % 2) * 8;
                            int brick_y = y * SPRITE_SIZE + by * 8;
                            
                            if (brick_x + 12 <= x * SPRITE_SIZE + SPRITE_SIZE) {
                                ImageDrawRectangle(&tileset_img, brick_x, brick_y, 12, 6, highlight_color);
                                Rectangle brick_rect = {brick_x, brick_y, 12, 6};
                                ImageDrawRectangleLines(&tileset_img, brick_rect, 1, mortar_color);
                            }
                        }
                    }
                    
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
                    
                    ImageDrawRectangleRec(&tileset_img, tile_rect, cobble_color);
                    
                    for (int i = 0; i < 6; i++) {
                        int stone_size = 6 + rand() % 4;
                        int sx = x * SPRITE_SIZE + (rand() % (SPRITE_SIZE - stone_size));
                        int sy = y * SPRITE_SIZE + (rand() % (SPRITE_SIZE - stone_size));
                        
                        ImageDrawRectangle(&tileset_img, sx, sy, stone_size, stone_size, wear_color);
                        Rectangle cobble_rect = {sx, sy, stone_size, stone_size};
                        ImageDrawRectangleLines(&tileset_img, cobble_rect, 1, mortar_color);
                    }
                    break;
                }
                case TILE_VENDOR_STALL: {
                    Color wood_color = (Color){101, 67, 33, 255};
                    Color dark_wood = (Color){85, 55, 25, 255};
                    Color cloth_color = (Color){139, 115, 85, 255};
                    
                    ImageDrawRectangleRec(&tileset_img, tile_rect, wood_color);
                    
                    for (int plank = 0; plank < 4; plank++) {
                        int plank_y = y * SPRITE_SIZE + plank * 8;
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
                    
                    for (int i = 0; i < 12; i++) {
                        int rx = x * SPRITE_SIZE + (rand() % SPRITE_SIZE);
                        int ry = y * SPRITE_SIZE + (rand() % SPRITE_SIZE);
                        int size = 1 + rand() % 3;
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
    for (int frame = 0; frame < 4; frame++) {
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
    for (int frame = 0; frame < 8; frame++) {
        int x = frame * SPRITE_SIZE;
        float walk_bob = sinf(frame * 0.8f) * 2;
        ImageDrawCircle(&walk_img, x + SPRITE_SIZE/2, SPRITE_SIZE/2 + walk_bob, 12, (Color){138, 43, 226, 255});
        ImageDrawCircle(&walk_img, x + SPRITE_SIZE/2, SPRITE_SIZE/2 + walk_bob, 8, (Color){75, 0, 130, 255});
        ImageDrawCircle(&walk_img, x + SPRITE_SIZE/2, SPRITE_SIZE/2 - 4 + walk_bob, 4, (Color){255, 220, 177, 255});
    }
    sprites->hero_walk = LoadTextureFromImage(walk_img);
    UnloadImage(walk_img);
    
    Image cast_img = GenImageColor(SPRITE_SIZE * 6, SPRITE_SIZE, (Color){0, 0, 0, 0});
    for (int frame = 0; frame < 6; frame++) {
        int x = frame * SPRITE_SIZE;
        float glow = 1.0f + sinf(frame * 1.2f) * 0.3f;
        ImageDrawCircle(&cast_img, x + SPRITE_SIZE/2, SPRITE_SIZE/2, 12 * glow, (Color){138, 43, 226, 255});
        ImageDrawCircle(&cast_img, x + SPRITE_SIZE/2, SPRITE_SIZE/2, 8, (Color){75, 0, 130, 255});
        ImageDrawCircle(&cast_img, x + SPRITE_SIZE/2, SPRITE_SIZE/2 - 4, 4, (Color){255, 220, 177, 255});
        
        if (frame > 2) {
            for (int i = 0; i < 6; i++) {
                float angle = i * 60 * DEG2RAD;
                int spark_x = x + SPRITE_SIZE/2 + cosf(angle) * 16;
                int spark_y = SPRITE_SIZE/2 + sinf(angle) * 16;
                ImageDrawCircle(&cast_img, spark_x, spark_y, 2, (Color){173, 216, 230, 255});
            }
        }
    }
    sprites->hero_cast = LoadTextureFromImage(cast_img);
    UnloadImage(cast_img);
    
    for (int i = 0; i < 5; i++) {
        Image monster_img = GenImageColor(SPRITE_SIZE, SPRITE_SIZE, (Color){0, 0, 0, 0});
        Color monster_colors[] = {
            {34, 139, 34, 255},    // Zombie - green
            {138, 43, 226, 255},   // Mage - purple
            {139, 69, 19, 255},    // Heavy - brown
            {255, 215, 0, 255},    // Light - gold
            {220, 20, 60, 255}     // Elite - red
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
        {255, 215, 0, 255},    // Gold
        {255, 0, 0, 255},      // Health potion
        {0, 0, 255, 255}       // Mana potion
    };
    
    for (int i = 0; i < 3; i++) {
        Image item_img = GenImageColor(SPRITE_SIZE, SPRITE_SIZE, (Color){0, 0, 0, 0});
        
        if (i == 0) {
            for (int coin = 0; coin < 3; coin++) {
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
    
    for (int i = 0; i < 5; i++) {
        UnloadTexture(sprites->monsters[i]);
    }
    
    UnloadTexture(sprites->boss_texture);
    
    for (int i = 0; i < 3; i++) {
        UnloadTexture(sprites->items[i]);
    }
}


// ============================================================================
// RENDERING FUNCTION IMPLEMENTATIONS
// ============================================================================

static void render_background(Rift* env, float cell_size) {
    if (!env->client || !env->client->sprites.tileset.id) {
        Color bg_color = (Color){BG_COLOR_R, BG_COLOR_G, BG_COLOR_B, 255};
        DrawRectangle(0, 0, MAP_WIDTH * cell_size, MAP_HEIGHT * cell_size, bg_color);
        return;
    }
    
    SpriteSystem* sprites = &env->client->sprites;
    
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            int map_index = y * MAP_WIDTH + x;
            uint8_t cell_type = env->map[map_index];
            
            int tile_id;
            switch (cell_type) {
                case CELL_WALL:
                    tile_id = TILE_STONE_WALL;
                    break;
                case CELL_DOOR:
                    tile_id = TILE_STONE_DOOR;
                    break;
                case CELL_VENDOR:
                    tile_id = (env->current_phase == PHASE_TOWN) ? TILE_VENDOR_STALL : TILE_STONE_FLOOR;
                    break;
                default:
                    tile_id = (env->current_phase == PHASE_TOWN) ? TILE_TOWN_FLOOR : TILE_STONE_FLOOR;
                    break;
            }
            
            int tile_x = tile_id % TILESET_COLS;
            int tile_y = tile_id / TILESET_COLS;
            
            Rectangle source = {
                tile_x * SPRITE_SIZE,
                tile_y * SPRITE_SIZE,
                SPRITE_SIZE,
                SPRITE_SIZE
            };
            
            Rectangle dest = {
                x * cell_size,
                y * cell_size,
                cell_size,
                cell_size
            };
            
            DrawTexturePro(sprites->tileset, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
        }
    }
}

static void render_player(Rift* env, float cell_size) {
    if (!env->client || !env->client->sprites.hero_idle.id) {
        Vector2 screen_pos = grid_to_screen(env->player.x, env->player.y, cell_size);
        float half_cell = cell_size * 0.5f;
        screen_pos.x += half_cell;
        screen_pos.y += half_cell;
        
        Color player_color = (Color){PLAYER_COLOR_R, PLAYER_COLOR_G, PLAYER_COLOR_B, 255};
        Color armor_color = (Color){ARMOR_COLOR_R, 0, ARMOR_COLOR_B, 255};
        
        DrawCircle(screen_pos.x + SHADOW_OFFSET, screen_pos.y + SHADOW_OFFSET, cell_size/3, (Color){0, 0, 0, SHADOW_ALPHA});
        DrawCircle(screen_pos.x, screen_pos.y, cell_size/3, player_color);
        DrawCircle(screen_pos.x, screen_pos.y, cell_size/4, armor_color);
        return;
    }
    
    SpriteSystem* sprites = &env->client->sprites;
    Vector2 screen_pos = grid_to_screen(env->player.x, env->player.y, cell_size);
    
    sprites->hero_animation_timer++;
    
    bool is_moving = (env->player.movement_x != 0 || env->player.movement_y != 0);
    bool is_casting = (env->player.blizzard_cooldown > 0);
    
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
    
    if (sprites->hero_animation_timer >= HERO_ANIM_SPEED) {
        sprites->hero_animation_timer = 0;
        sprites->hero_frame++;
        
        switch (sprites->hero_animation_state) {
            case HERO_ANIM_IDLE:
                if (sprites->hero_frame >= HERO_FRAMES_IDLE) sprites->hero_frame = 0;
                break;
            case HERO_ANIM_WALK:
                if (sprites->hero_frame >= HERO_FRAMES_WALK) sprites->hero_frame = 0;
                break;
            case HERO_ANIM_CAST:
                if (sprites->hero_frame >= HERO_FRAMES_CAST) {
                    sprites->hero_animation_state = HERO_ANIM_IDLE;
                    sprites->hero_frame = 0;
                }
                break;
        }
    }
    
    Texture2D current_texture;
    int max_frames;
    
    switch (sprites->hero_animation_state) {
        case HERO_ANIM_WALK:
            current_texture = sprites->hero_walk;
            max_frames = HERO_FRAMES_WALK;
            break;
        case HERO_ANIM_CAST:
            current_texture = sprites->hero_cast;
            max_frames = HERO_FRAMES_CAST;
            break;
        default:
            current_texture = sprites->hero_idle;
            max_frames = HERO_FRAMES_IDLE;
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
    
    DrawCircle(screen_pos.x + cell_size/2 + SHADOW_OFFSET, screen_pos.y + cell_size/2 + SHADOW_OFFSET, 
               cell_size/4, (Color){0, 0, 0, SHADOW_ALPHA});
    
    DrawTexturePro(current_texture, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
}

static void render_monsters(Rift* env, float cell_size) {
    for (uint16_t i = 0; i < MAX_MONSTERS; i++) {
        if (!env->monsters[i].alive) continue;
        
        Vector2 screen_pos = grid_to_screen(env->monsters[i].x, env->monsters[i].y, cell_size);
        uint8_t monster_type = env->monsters[i].type;
        float size_multiplier = 1.0f;
        
        switch (monster_type) {
            case MONSTER_MAGE:
                size_multiplier = MAGE_SIZE_MULT;
                break;
            case MONSTER_HEAVY_MELEE:
                size_multiplier = HEAVY_SIZE_MULT;
                break;
            case MONSTER_LIGHT:
                size_multiplier = LIGHT_SIZE_MULT;
                break;
            case MONSTER_ELITE:
                size_multiplier = ELITE_SIZE_MULT;
                break;
            default:
                size_multiplier = 1.0f;
                break;
        }
        
        if (env->client && env->client->sprites.monsters[0].id && monster_type < 5) {
            Rectangle source = {0, 0, SPRITE_SIZE, SPRITE_SIZE};
            Rectangle dest = {
                screen_pos.x,
                screen_pos.y,
                cell_size * size_multiplier,
                cell_size * size_multiplier
            };
            
            DrawCircle(dest.x + dest.width/2 + SHADOW_OFFSET, dest.y + dest.height/2 + SHADOW_OFFSET, 
                      dest.width/4, (Color){0, 0, 0, SHADOW_ALPHA});
            
            DrawTexturePro(env->client->sprites.monsters[monster_type], source, dest, 
                          (Vector2){0, 0}, 0.0f, WHITE);
            
            if (monster_type == MONSTER_ELITE) {
                float glow = GLOW_BASE + GLOW_AMPLITUDE * sinf(env->tick * GLOW_SPEED);
                DrawCircleLines(dest.x + dest.width/2, dest.y + dest.height/2, 
                              dest.width/2 * glow, (Color){255, 255, 255, GLOW_ALPHA});
            }
        } else {
            float half_cell = cell_size * 0.5f;
            float quarter_cell = cell_size * 0.25f;
            float sixth_cell = cell_size / 6.0f;
            float third_cell = cell_size / 3.0f;
            
            screen_pos.x += half_cell;
            screen_pos.y += half_cell;
            
            Color monster_color;
            
            if (monster_type == MONSTER_ZOMBIE) {
                monster_color = (Color){ZOMBIE_COLOR_R, ZOMBIE_COLOR_G, ZOMBIE_COLOR_B, 255};
            } else if (monster_type == MONSTER_MAGE) {
                monster_color = (Color){PLAYER_COLOR_R, PLAYER_COLOR_G, PLAYER_COLOR_B, 255};
            } else if (monster_type == MONSTER_HEAVY_MELEE) {
                monster_color = (Color){HEAVY_COLOR_R, HEAVY_COLOR_G, HEAVY_COLOR_B, 255};
            } else if (monster_type == MONSTER_LIGHT) {
                monster_color = (Color){LIGHT_COLOR_R, LIGHT_COLOR_G, 0, 255};
            } else if (monster_type == MONSTER_ELITE) {
                monster_color = (Color){ELITE_COLOR_R, ELITE_COLOR_G, ELITE_COLOR_B, 255};
            } else {
                monster_color = (Color){MAROON_COLOR_R, 0, 0, 255};
            }
            
            float scaled_quarter = quarter_cell * size_multiplier;
            float scaled_sixth = sixth_cell * size_multiplier;
            
            DrawCircle(screen_pos.x + 1, screen_pos.y + 1, scaled_quarter, (Color){0, 0, 0, SHADOW_ALPHA});
            DrawCircle(screen_pos.x, screen_pos.y, scaled_quarter, monster_color);
            DrawCircle(screen_pos.x, screen_pos.y, scaled_sixth, (Color){255, 0, 0, 150});
            
            if (monster_type == MONSTER_ELITE) {
                float glow = GLOW_BASE + GLOW_AMPLITUDE * sinf(env->tick * GLOW_SPEED);
                DrawCircleLines(screen_pos.x, screen_pos.y, third_cell * size_multiplier * glow, (Color){255, 255, 255, GLOW_ALPHA});
            }
        }
        
        float health_ratio = (float)env->monsters[i].health / env->monsters[i].max_health;
        float half_cell = cell_size * 0.5f;
        float third_cell = cell_size / 3.0f;
        uint8_t bar_width = (uint8_t)half_cell;
        uint8_t half_bar_width = bar_width >> 1;
        int16_t bar_x = screen_pos.x + half_cell - half_bar_width;
        int16_t bar_y = screen_pos.y - third_cell - HEALTH_BAR_OFFSET;
        
        DrawRectangle(bar_x, bar_y, bar_width, HEALTH_BAR_HEIGHT, RED);
        DrawRectangle(bar_x, bar_y, (uint8_t)(bar_width * health_ratio), HEALTH_BAR_HEIGHT, GREEN);
        DrawRectangleLines(bar_x, bar_y, bar_width, HEALTH_BAR_HEIGHT, WHITE);
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
        
        float pulse = BOSS_PULSE_BASE + BOSS_PULSE_AMPLITUDE * sinf(env->tick * GLOW_SPEED);
        dest.width *= pulse;
        dest.height *= pulse;
        dest.x -= (dest.width - cell_size * 2) / 2;
        dest.y -= (dest.height - cell_size * 2) / 2;
        
        DrawCircle(dest.x + dest.width/2 + BOSS_SHADOW_OFFSET, dest.y + dest.height/2 + BOSS_SHADOW_OFFSET, 
                  dest.width/4, (Color){0, 0, 0, BOSS_SHADOW_ALPHA});
        
        DrawTexturePro(env->client->sprites.boss_texture, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
        
        DrawCircleLines(dest.x + dest.width/2, dest.y + dest.height/2, cell_size * BOSS_SIZE_MULT, 
                       (Color){ARMOR_COLOR_R, 0, ARMOR_COLOR_B, BOSS_AURA_ALPHA});
    } else {
        float half_cell = cell_size * 0.5f;
        screen_pos.x += half_cell;
        screen_pos.y += half_cell;
        
        Color boss_color = (Color){ARMOR_COLOR_R, 0, ARMOR_COLOR_B, 255};
        float scaled_half = half_cell * BOSS_SIZE_MULT;
        float scaled_third = (cell_size / 3.0f) * BOSS_SIZE_MULT;
        
        DrawCircle(screen_pos.x + BOSS_SHADOW_OFFSET, screen_pos.y + BOSS_SHADOW_OFFSET, scaled_half, (Color){0, 0, 0, BOSS_SHADOW_ALPHA});
        
        float pulse = BOSS_PULSE_BASE + BOSS_PULSE_AMPLITUDE * sinf(env->tick * GLOW_SPEED);
        DrawCircle(screen_pos.x, screen_pos.y, scaled_half * pulse, boss_color);
        DrawCircle(screen_pos.x, screen_pos.y, scaled_third, (Color){PLAYER_COLOR_R, PLAYER_COLOR_G, PLAYER_COLOR_B, 255});
        DrawCircle(screen_pos.x, screen_pos.y, cell_size / 6.0f, (Color){255, 255, 255, BOSS_CORE_ALPHA});
        
        DrawCircleLines(screen_pos.x, screen_pos.y, cell_size * BOSS_SIZE_MULT, (Color){ARMOR_COLOR_R, 0, ARMOR_COLOR_B, BOSS_AURA_ALPHA});
    }
    
    float health_ratio = (float)env->boss.health / env->boss.max_health;
    uint8_t bar_width = (uint8_t)cell_size;
    uint8_t half_bar_width = bar_width >> 1;
    int16_t bar_x = screen_pos.x + cell_size/2 - half_bar_width;
    int16_t bar_y = screen_pos.y - cell_size/2 - BOSS_HEALTH_BAR_OFFSET;
    
    DrawRectangle(bar_x, bar_y, bar_width, BOSS_HEALTH_BAR_HEIGHT, RED);
    DrawRectangle(bar_x, bar_y, (uint8_t)(bar_width * health_ratio), BOSS_HEALTH_BAR_HEIGHT, GREEN);
    DrawRectangleLines(bar_x, bar_y, bar_width, BOSS_HEALTH_BAR_HEIGHT, WHITE);
}

static void render_items(Rift* env, float cell_size) {
    for (uint8_t i = 0; i < MAX_ITEMS; i++) {
        if (!env->items[i].active) continue;
        
        Vector2 screen_pos = grid_to_screen(env->items[i].x, env->items[i].y, cell_size);
        uint8_t item_type = env->items[i].type;
        
        if (env->client && env->client->sprites.items[0].id && item_type < 3) {
            Rectangle source = {0, 0, SPRITE_SIZE, SPRITE_SIZE};
            Rectangle dest = {
                screen_pos.x + cell_size/4,
                screen_pos.y + cell_size/4,
                cell_size/2,
                cell_size/2
            };
            
            float glow = GLOW_BASE + GLOW_AMPLITUDE * sinf(env->tick * ITEM_GLOW_SPEED + i);
            dest.width *= glow;
            dest.height *= glow;
            dest.x -= (dest.width - cell_size/2) / 2;
            dest.y -= (dest.height - cell_size/2) / 2;
            
            Color tint = WHITE;
            if (item_type == ITEM_GOLD) {
                tint = (Color){255, 255, 180, 255};
            } else if (item_type == ITEM_HEALTH_POTION) {
                tint = (Color){255, 200, 200, 255};
            } else if (item_type == ITEM_MANA_POTION) {
                tint = (Color){200, 200, 255, 255};
            }
            
            DrawTexturePro(env->client->sprites.items[item_type], source, dest, (Vector2){0, 0}, 0.0f, tint);
            
            if (glow > 1.0f) {
                Color glow_color = tint;
                glow_color.a = 50;
                DrawCircle(dest.x + dest.width/2, dest.y + dest.height/2, dest.width/2 * 1.5f, glow_color);
            }
        } else {
            float half_cell = cell_size * 0.5f;
            float eighth_cell = cell_size * ITEM_SIZE_EIGHTH;
            float twelfth_cell = cell_size * ITEM_SIZE_TWELFTH;
            
            screen_pos.x += half_cell;
            screen_pos.y += half_cell;
            
            Color item_color;
            
            if (item_type == ITEM_GOLD) {
                item_color = GOLD;
            } else if (item_type == ITEM_HEALTH_POTION) {
                item_color = RED;
            } else if (item_type == ITEM_MANA_POTION) {
                item_color = BLUE;
            } else {
                item_color = WHITE;
            }
            
            float glow = GLOW_BASE + GLOW_AMPLITUDE * sinf(env->tick * ITEM_GLOW_SPEED + i);
            DrawCircle(screen_pos.x, screen_pos.y, eighth_cell * glow, item_color);
            DrawCircle(screen_pos.x, screen_pos.y, twelfth_cell, WHITE);
        }
    }
}

static void render_projectiles(Rift* env, float cell_size) {
    float half_cell = cell_size * 0.5f;
    float sixth_cell = cell_size * PROJECTILE_SIXTH;
    
    for (uint8_t i = 0; i < MAX_PROJECTILES; i++) {
        if (!env->projectiles[i].active) continue;
        
        Vector2 screen_pos = grid_to_screen(env->projectiles[i].x, env->projectiles[i].y, cell_size);
        screen_pos.x += half_cell;
        screen_pos.y += half_cell;
        
        Color projectile_color;
        float size_multiplier;
        
        if (env->projectiles[i].type == PROJECTILE_FIREBALL) {
            projectile_color = (Color){FIREBALL_COLOR_R, FIREBALL_COLOR_G, 0, 255};
            size_multiplier = FIREBALL_PULSE_BASE + FIREBALL_PULSE_AMPLITUDE * sinf(env->tick * FIREBALL_PULSE_SPEED + i);
        } else {
            projectile_color = RED;
            size_multiplier = 1.0f;
        }
        
        float lifetime_ratio = (float)env->projectiles[i].lifetime / PROJECTILE_LIFETIME;
        uint8_t alpha = (uint8_t)(255 * lifetime_ratio);
        projectile_color.a = alpha;
        
        float radius = sixth_cell * size_multiplier;
        float half_radius = radius * 0.5f;
        uint8_t half_alpha = alpha >> 1;
        uint8_t third_alpha = alpha / 3;
        
        DrawCircle(screen_pos.x, screen_pos.y, radius + PROJECTILE_HALO_OFFSET, (Color){HALO_COLOR_R, HALO_COLOR_G, 0, half_alpha});
        DrawCircle(screen_pos.x, screen_pos.y, radius, projectile_color);
        DrawCircle(screen_pos.x, screen_pos.y, half_radius, (Color){CORE_COLOR_R, CORE_COLOR_G, 0, alpha});
        
        Vector2 trail_end = {
            screen_pos.x - env->projectiles[i].vel_x * TRAIL_LENGTH,
            screen_pos.y - env->projectiles[i].vel_y * TRAIL_LENGTH
        };
        DrawLineEx(screen_pos, trail_end, TRAIL_WIDTH, (Color){TRAIL_COLOR_R, TRAIL_COLOR_G, 0, third_alpha});
    }
}

static void render_blizzard_areas(Rift* env, float cell_size) {
    float half_cell = cell_size * 0.5f;
    
    for (uint8_t i = 0; i < MAX_BLIZZARD_AREAS; i++) {
        if (!env->blizzard_areas[i].active) continue;
        
        Vector2 screen_pos = grid_to_screen(env->blizzard_areas[i].x, env->blizzard_areas[i].y, cell_size);
        screen_pos.x += half_cell;
        screen_pos.y += half_cell;
        
        float duration_ratio = (float)env->blizzard_areas[i].duration / BLIZZARD_DURATION;
        uint8_t alpha = (uint8_t)(BLIZZARD_BASE_ALPHA * duration_ratio);
        float radius = BLIZZARD_RADIUS * cell_size;
        uint8_t third_alpha = alpha / 3;
        uint8_t half_alpha = alpha >> 1;
        
        Color base_color = (Color){BLIZZARD_BASE_COLOR_R, BLIZZARD_BASE_COLOR_G, BLIZZARD_BASE_COLOR_B, third_alpha};
        Color ice_shard_color = (Color){255, 255, 255, alpha};
        Color frost_color = (Color){FROST_COLOR_R, FROST_COLOR_G, FROST_COLOR_B, half_alpha};
        
        DrawCircle(screen_pos.x, screen_pos.y, radius, base_color);
        
        uint16_t radius_int = (uint16_t)radius;
        uint16_t radius_double = radius_int << 1;
        
        for (uint8_t j = 0; j < ICE_SHARD_COUNT; j++) {
            int16_t random_offset_x = (rand() % radius_double) - radius_int;
            int16_t random_offset_y = (rand() % radius_double) - radius_int;
            float distance_squared = random_offset_x * random_offset_x + random_offset_y * random_offset_y;
            
            if (distance_squared <= radius * radius) {
                float x = screen_pos.x + random_offset_x;
                float y = screen_pos.y + random_offset_y;
                
                uint16_t fall_offset = ((env->tick + j * SHARD_TIMING_MULT) % SHARD_CYCLE_FRAMES);
                y += fall_offset * SHARD_FALL_SPEED;
                
                uint8_t shard_size = SHARD_SIZE_MIN + (rand() % SHARD_SIZE_RANGE);
                DrawCircle(x, y, shard_size, ice_shard_color);
                
                if ((rand() & 3) == 0) {
                    DrawLine(x - FROST_LINE_SIZE, y - FROST_LINE_SIZE, x + FROST_LINE_SIZE, y + FROST_LINE_SIZE, frost_color);
                    DrawLine(x - FROST_LINE_SIZE, y + FROST_LINE_SIZE, x + FROST_LINE_SIZE, y - FROST_LINE_SIZE, frost_color);
                }
            }
        }
        
        DrawCircleLines(screen_pos.x, screen_pos.y, radius, frost_color);
    }
}

static void render_ui(Rift* env, uint16_t screen_width, uint16_t screen_height) {
    uint16_t ui_bottom = screen_height - UI_HEIGHT;
    DrawRectangle(0, ui_bottom, screen_width, UI_HEIGHT, (Color){UI_BG_COLOR_RGB, UI_BG_COLOR_RGB, UI_BG_COLOR_RGB, UI_BG_ALPHA});
    DrawLine(0, ui_bottom, screen_width, ui_bottom, (Color){UI_LINE_COLOR_RGB, UI_LINE_COLOR_RGB, UI_LINE_COLOR_RGB, 255});
    
    float health_ratio = (float)env->player.health / env->player.max_health;
    uint16_t health_globe_x = GLOBE_RADIUS + GLOBE_MARGIN;
    uint16_t health_globe_y = screen_height - GLOBE_RADIUS - GLOBE_MARGIN;
    
    DrawCircle(health_globe_x, health_globe_y, GLOBE_RADIUS, (Color){HEALTH_GLOBE_BG_R, 0, 0, 255});
    
    if (health_ratio > ZERO_VALUE) {
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
                DrawRectangle(health_globe_x - half_width, current_y, full_width, 1, RED);
            }
        }
    }
    
    DrawCircleLines(health_globe_x, health_globe_y, GLOBE_RADIUS, WHITE);
    DrawText(TextFormat("%d", env->player.health), health_globe_x - GLOBE_MARGIN, health_globe_y - 5, TEXT_SIZE_16, WHITE);
    
    float mana_ratio = (float)env->player.mana / env->player.max_mana;
    uint16_t mana_globe_x = screen_width - GLOBE_RADIUS - GLOBE_MARGIN;
    uint16_t mana_globe_y = screen_height - GLOBE_RADIUS - GLOBE_MARGIN;
    
    DrawCircle(mana_globe_x, mana_globe_y, GLOBE_RADIUS, (Color){0, 0, HEALTH_GLOBE_BG_R, 255});
    
    if (mana_ratio > ZERO_VALUE) {
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
                DrawRectangle(mana_globe_x - half_width, current_y, full_width, 1, BLUE);
            }
        }
    }
    
    DrawCircleLines(mana_globe_x, mana_globe_y, GLOBE_RADIUS, WHITE);
    DrawText(TextFormat("%d", env->player.mana), mana_globe_x - GLOBE_MARGIN, mana_globe_y - 5, TEXT_SIZE_16, WHITE);
    uint16_t screen_center = screen_width >> 1;
    DrawText(TextFormat("Gold: %d", env->player.gold), screen_center - 30, 60, TEXT_SIZE_16, GOLD);
    
    if (env->current_phase == PHASE_RIFT) {
        float progress = (float)env->monsters_killed / MONSTERS_TO_SPAWN;
        DrawText(TextFormat("Rift Progress: %.1f%%", progress * 100), screen_center - 80, 10, TEXT_SIZE_20, GREEN);
        
        if (env->boss_spawned) {
            DrawText("BOSS SPAWNED!", screen_center - 60, 35, TEXT_SIZE_18, RED);
        }
    }
    
    uint16_t hp_x = screen_center - POTION_SPACING;
    uint16_t hp_y = screen_height - POTION_SPACING;
    Color hp_color = (env->player.health_potion_cooldown > 0) ? GRAY : RED;
    DrawCircle(hp_x, hp_y, POTION_SIZE, hp_color);
    DrawCircleLines(hp_x, hp_y, POTION_SIZE, WHITE);
    DrawText("Q", hp_x - 5, hp_y - 8, TEXT_SIZE_16, WHITE);
    
    if (env->player.health_potion_cooldown > 0) {
        float cooldown_ratio = (float)env->player.health_potion_cooldown / HEALTH_POTION_COOLDOWN;
        Vector2 hp_center = {hp_x, hp_y};
        DrawCircleSector(hp_center, POTION_SIZE, 0, 360 * cooldown_ratio, 32, (Color){0, 0, 0, 150});
    }
    
    uint16_t mp_x = screen_center + POTION_SPACING;
    uint16_t mp_y = screen_height - POTION_SPACING;
    Color mp_color = (env->player.mana_potion_cooldown > 0) ? GRAY : BLUE;
    DrawCircle(mp_x, mp_y, POTION_SIZE, mp_color);
    DrawCircleLines(mp_x, mp_y, POTION_SIZE, WHITE);
    DrawText("E", mp_x - 5, mp_y - 8, TEXT_SIZE_16, WHITE);
    
    if (env->player.mana_potion_cooldown > 0) {
        float cooldown_ratio = (float)env->player.mana_potion_cooldown / MANA_POTION_COOLDOWN;
        Vector2 mp_center = {mp_x, mp_y};
        DrawCircleSector(mp_center, POTION_SIZE, 0, 360 * cooldown_ratio, 32, (Color){0, 0, 0, 150});
    }
    
    DrawText("Hold SHIFT for manual control: WASD=Move Space=Blizzard Q/E=Potions (F=Interact in town)", 
             GLOBE_MARGIN, screen_height - 15, TEXT_SIZE_12, YELLOW);
    
    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        DrawText("MANUAL CONTROL ACTIVE", screen_width - MANUAL_CONTROL_WIDTH, GLOBE_MARGIN, TEXT_SIZE_16, GREEN);
        DrawRectangle(screen_width - MANUAL_CONTROL_OFFSET, 5, MANUAL_CONTROL_WIDTH, MANUAL_CONTROL_HEIGHT, (Color){0, 255, 0, 50});
    } else {
        DrawText("AI CONTROL", screen_width - AI_CONTROL_WIDTH, GLOBE_MARGIN, TEXT_SIZE_16, GRAY);
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

void render_town(Rift* env) {
    float cell_size = env->client->cell_size;
    
    BeginMode2D(env->client->camera);
    
    render_background(env, cell_size);
    render_player(env, cell_size);
    
    Vector2 vendor_pos = grid_to_screen(env->vendor.x, env->vendor.y, cell_size);
    float half_cell = cell_size * 0.5f;
    vendor_pos.x += half_cell;
    vendor_pos.y += half_cell;
    DrawCircle(vendor_pos.x, vendor_pos.y, cell_size * VENDOR_OUTER_SIZE, GOLD);
    DrawCircle(vendor_pos.x, vendor_pos.y, cell_size * VENDOR_INNER_SIZE, YELLOW);
    DrawText("VENDOR", vendor_pos.x - VENDOR_TEXT_OFFSET_X, vendor_pos.y - VENDOR_TEXT_OFFSET_Y, TEXT_SIZE_12, BLACK);
    
    EndMode2D();
}

void c_render(Rift* env) {
    if (env->client == NULL) {
        env->client = make_client(env);
    }
    
    if (IsKeyDown(KEY_ESCAPE)) {
        exit(0);
    }
    
    Vector2 player_screen = grid_to_screen(env->player.x, env->player.y, env->client->cell_size);
    env->client->camera.target = player_screen;
    
    BeginDrawing();
    ClearBackground((Color){CLEAR_BG_R, CLEAR_BG_G, CLEAR_BG_B, 255});
    
    if (env->current_phase == PHASE_RIFT) {
        render_rift(env);
    } else {
        render_town(env);
    }
    
    render_ui(env, env->client->width, env->client->height);
    
    EndDrawing();
}

#endif
