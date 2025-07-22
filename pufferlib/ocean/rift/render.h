#ifndef RIFT_RENDER_H
#define RIFT_RENDER_H

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

// ============================================================================
// FUNCTION DECLARATIONS
// ============================================================================

void load_sprites(SpriteSystem* sprites);
void unload_sprites(SpriteSystem* sprites);
void c_render(Rift* env);
void render_rift(Rift* env);
void render_town(Rift* env);
void render_character_tab(Rift* env);
void render_shop_tab(Rift* env);

Color GetQualityColor(uint32_t quality) {
    switch (quality) {
        case 0: return (Color){180, 180, 180, 255}; // Common - Light Gray
        case 1: return (Color){100, 150, 255, 255}; // Rare - Blue  
        case 2: return (Color){200, 100, 255, 255}; // Epic - Purple
        case 3: return (Color){255, 150, 50, 255};  // Legendary - Orange
        default: return (Color){100, 100, 100, 255};
    }
}

// Helper function for consistent relative positioning within containers
void DrawTextRelative(const char* text, int container_x, int container_y, int offset_x, int offset_y, int font_size, Color color) {
    DrawText(text, container_x + offset_x, container_y + offset_y, font_size, color);
}

// Helper function for positioning within stats panel
void DrawStatsText(const char* text, int stats_y, int line_offset, int font_size, Color color, int indent) {
    int x = STATS_PANEL_X + (indent ? STATS_CONTENT_MARGIN_INDENT : STATS_CONTENT_MARGIN_LEFT);
    int y = stats_y + STATS_CONTENT_MARGIN_LEFT + line_offset;
    DrawText(text, x, y, font_size, color);
}


void DrawEpicWeapon(int x, int y, int size, Color color) {
    // Epic sword design
    int blade_width = size / 8;
    int blade_height = size * 3 / 4;
    int handle_height = size / 4;
    
    // Draw blade with gradient effect
    DrawRectangleGradientV(x + size/2 - blade_width/2, y + 5, blade_width, blade_height, color, (Color){color.r/2, color.g/2, color.b/2, color.a});
    // Blade edge highlight
    DrawRectangle(x + size/2 - 1, y + 5, 2, blade_height, (Color){255, 255, 255, 150});
    
    // Draw crossguard
    DrawRectangle(x + size/4, y + blade_height, size/2, 4, color);
    
    // Draw handle
    DrawRectangleGradientV(x + size/2 - 3, y + blade_height + 4, 6, handle_height - 4, (Color){139, 69, 19, 255}, (Color){69, 39, 19, 255});
    
    // Draw pommel
    DrawCircle(x + size/2, y + size - 4, 4, color);
}

void DrawEpicRing(int x, int y, int size, Color color) {
    // Epic ring design
    int ring_radius = size / 4;
    int gem_size = size / 8;
    
    // Draw ring band
    DrawCircleLines(x + size/2, y + size/2, ring_radius, color);
    DrawCircleLines(x + size/2, y + size/2, ring_radius - 1, color);
    
    // Draw gem on top
    DrawCircle(x + size/2, y + size/2 - ring_radius + 2, gem_size, color);
    DrawCircle(x + size/2, y + size/2 - ring_radius + 2, gem_size - 2, (Color){255, 255, 255, 200});
}

void DrawEpicAmulet(int x, int y, int size, Color color) {
    // Epic amulet design
    int chain_y = y + 8;
    int pendant_size = size / 3;
    
    // Draw chain
    for (int i = 0; i < size - 16; i += 4) {
        DrawCircle(x + size/2, chain_y + i, 1, (Color){200, 200, 200, 255});
    }
    
    // Draw pendant
    DrawCircle(x + size/2, y + size - pendant_size/2 - 4, pendant_size/2, color);
    DrawCircle(x + size/2, y + size - pendant_size/2 - 4, pendant_size/2 - 2, (Color){255, 255, 255, 150});
}

void DrawEpicBoots(int x, int y, int size, Color color) {
    // KNIGHT ARMORED BOOTS - Medieval fantasy style
    int boot_x = x + size / 6;
    int boot_y = y + size / 8;
    int boot_width = size * 2 / 3;
    int boot_height = size * 3 / 4;
    
    // Main boot body - tall medieval style
    DrawRectangleGradientV(boot_x, boot_y, boot_width, boot_height, color, (Color){color.r/2, color.g/2, color.b/2, color.a});
    
    // Armored shin guard overlay
    int shin_width = boot_width - 8;
    int shin_height = boot_height * 2 / 3;
    DrawRectangleGradientV(boot_x + 4, boot_y, shin_width, shin_height, 
                          (Color){color.r + 40, color.g + 40, color.b + 40, color.a},
                          (Color){color.r + 20, color.g + 20, color.b + 20, color.a});
    
    // Foot armored section
    int foot_height = boot_height / 3;
    DrawRectangleGradientH(boot_x, boot_y + shin_height, boot_width, foot_height,
                          color, (Color){color.r - 30, color.g - 30, color.b - 30, color.a});
    
    // Metallic plates on shin (3 segments)
    for (int i = 0; i < 3; i++) {
        int plate_y = boot_y + 8 + i * (shin_height / 4);
        int plate_height = 6;
        DrawRectangle(boot_x + 6, plate_y, shin_width - 4, plate_height, (Color){180, 180, 190, 255});
        DrawRectangleLines(boot_x + 6, plate_y, shin_width - 4, plate_height, (Color){220, 220, 230, 255});
    }
    
    // Knee cap protection
    int knee_y = boot_y + shin_height / 4;
    DrawCircle(boot_x + boot_width/2, knee_y, 8, (Color){160, 160, 170, 255});
    DrawCircleLines(boot_x + boot_width/2, knee_y, 8, (Color){200, 200, 210, 255});
    
    // Armored toe cap
    int toe_size = boot_width / 3;
    DrawCircle(boot_x + toe_size/2, boot_y + boot_height - toe_size/2, toe_size/2, (Color){140, 140, 150, 255});
    DrawCircleLines(boot_x + toe_size/2, boot_y + boot_height - toe_size/2, toe_size/2, (Color){180, 180, 190, 255});
    
    // Leather straps
    for (int i = 0; i < 4; i++) {
        int strap_y = boot_y + 12 + i * 12;
        DrawRectangle(boot_x + 2, strap_y, boot_width - 4, 3, (Color){101, 67, 33, 255}); // Brown leather
        // Buckles
        DrawRectangle(boot_x + boot_width - 8, strap_y - 1, 4, 5, (Color){160, 160, 170, 255});
    }
    
    // Heel spur (knight detail)
    DrawTriangle((Vector2){boot_x + boot_width, boot_y + boot_height - 15},
                (Vector2){boot_x + boot_width + 6, boot_y + boot_height - 12},
                (Vector2){boot_x + boot_width, boot_y + boot_height - 9},
                (Color){140, 140, 150, 255});
    
    // Heavy sole
    DrawRectangle(boot_x - 2, boot_y + boot_height - 4, boot_width + 4, 6, (Color){40, 40, 40, 255});
    
    // Metallic border/trim
    DrawRectangleLines(boot_x, boot_y, boot_width, boot_height, (Color){180, 180, 190, 200});
    DrawRectangleLines(boot_x - 1, boot_y - 1, boot_width + 2, boot_height + 2, (Color){220, 220, 230, 150});
}

void DrawEpicGloves(int x, int y, int size, Color color) {
    // Epic gloves design
    int glove_width = size / 2;
    int glove_height = size * 2 / 3;
    
    // Draw glove body
    DrawRectangleGradientV(x + size/4, y + size/4, glove_width, glove_height, color, (Color){color.r/2, color.g/2, color.b/2, color.a});
    
    // Draw fingers
    for (int i = 0; i < 4; i++) {
        DrawRectangle(x + size/4 + i * (glove_width/4), y + size/6, glove_width/5, size/3, color);
    }
    
    // Draw thumb
    DrawRectangle(x + size/4 - 4, y + size/2, glove_width/4, size/4, color);
}

void DrawEpicOffhand(int x, int y, int size, Color color) {
    // Epic shield design
    int shield_radius = size / 3;
    
    // Draw shield rim
    DrawCircle(x + size/2, y + size/2, shield_radius, color);
    DrawCircle(x + size/2, y + size/2, shield_radius - 4, (Color){color.r/2, color.g/2, color.b/2, color.a});
    
    // Draw boss (center)
    DrawCircle(x + size/2, y + size/2, shield_radius/3, color);
    DrawCircle(x + size/2, y + size/2, shield_radius/3 - 2, (Color){255, 255, 255, 150});
}

void DrawEpicHelmet(int x, int y, int size, Color color) {
    // KNIGHT HELMET - Medieval great helm style
    int helm_width = size * 3 / 4;
    int helm_height = size * 4 / 5;
    int helm_x = x + (size - helm_width) / 2;
    int helm_y = y + size / 10;
    
    // Main helmet dome - rounded top
    DrawRectangleGradientV(helm_x, helm_y + helm_height / 4, helm_width, helm_height * 3 / 4, color, (Color){color.r/2, color.g/2, color.b/2, color.a});
    
    // Helmet crown (rounded top section)
    DrawCircleGradient(x + size/2, helm_y + helm_height / 4, helm_width / 2, (Color){color.r + 20, color.g + 20, color.b + 20, color.a}, color);
    
    // Face plate with breathing holes
    int faceplate_y = helm_y + helm_height / 2;
    DrawRectangle(helm_x + 5, faceplate_y, helm_width - 10, helm_height / 3, (Color){color.r/3, color.g/3, color.b/3, color.a});
    
    // Visor slit (eye opening)
    int slit_width = helm_width / 2;
    int slit_height = 3;
    DrawRectangle(helm_x + (helm_width - slit_width) / 2, faceplate_y + 5, slit_width, slit_height, BLACK);
    
    // Breathing holes (small circles)
    for (int i = 0; i < 3; i++) {
        int hole_x = helm_x + helm_width / 4 + i * (helm_width / 6);
        int hole_y = faceplate_y + helm_height / 6;
        DrawCircle(hole_x, hole_y, 2, BLACK);
    }
    
    // Helmet plume holder (top ornament)
    DrawRectangle(x + size/2 - 2, helm_y - 3, 4, 8, (Color){color.r + 60, color.g + 60, color.b + 60, color.a});
    
    // Side protection (cheek guards)
    DrawRectangle(helm_x - 2, faceplate_y + 5, 4, helm_height / 4, (Color){color.r/2, color.g/2, color.b/2, color.a});
    DrawRectangle(helm_x + helm_width - 2, faceplate_y + 5, 4, helm_height / 4, (Color){color.r/2, color.g/2, color.b/2, color.a});
    
    // Neck guard (aventail suggestion)
    DrawRectangle(helm_x + 3, helm_y + helm_height - 3, helm_width - 6, 5, (Color){color.r/3, color.g/3, color.b/3, color.a});
}

void DrawEpicShoulders(int x, int y, int size, Color color) {
    // KNIGHT SHOULDER GUARDS - Medieval pauldrons
    int pauldron_width = size * 3 / 4;
    int pauldron_height = size / 2;
    int pauldron_x = x + (size - pauldron_width) / 2;
    int pauldron_y = y + size / 6;
    
    // Main shoulder plate (curved design)
    DrawRectangleGradientV(pauldron_x, pauldron_y, pauldron_width, pauldron_height, color, (Color){color.r/2, color.g/2, color.b/2, color.a});
    
    // Articulated plates (segments)
    for (int i = 0; i < 3; i++) {
        int segment_y = pauldron_y + i * (pauldron_height / 4);
        DrawRectangle(pauldron_x + 2, segment_y, pauldron_width - 4, 2, (Color){color.r + 40, color.g + 40, color.b + 40, color.a});
    }
    
    // Spike decoration on top
    DrawRectangle(x + size/2 - 1, pauldron_y - 4, 2, 8, (Color){color.r + 60, color.g + 60, color.b + 60, color.a});
}

void DrawEpicArmor(int x, int y, int size, Color color) {
    // KNIGHT CHEST ARMOR - Medieval breastplate
    int armor_width = size * 2 / 3;
    int armor_height = size * 3 / 4;
    int armor_x = x + (size - armor_width) / 2;
    int armor_y = y + size / 8;
    
    // Main breastplate
    DrawRectangleGradientV(armor_x, armor_y, armor_width, armor_height, color, (Color){color.r/2, color.g/2, color.b/2, color.a});
    
    // Central ridge (medieval style)
    int ridge_width = 4;
    DrawRectangle(x + size/2 - ridge_width/2, armor_y, ridge_width, armor_height, (Color){color.r + 40, color.g + 40, color.b + 40, color.a});
    
    // Side plates
    DrawRectangle(armor_x - 2, armor_y + armor_height / 4, 4, armor_height / 2, (Color){color.r/2, color.g/2, color.b/2, color.a});
    DrawRectangle(armor_x + armor_width - 2, armor_y + armor_height / 4, 4, armor_height / 2, (Color){color.r/2, color.g/2, color.b/2, color.a});
    
    // Decorative rivets
    for (int i = 0; i < 4; i++) {
        DrawCircle(armor_x + 5, armor_y + 10 + i * 12, 1, (Color){color.r + 80, color.g + 80, color.b + 80, color.a});
        DrawCircle(armor_x + armor_width - 5, armor_y + 10 + i * 12, 1, (Color){color.r + 80, color.g + 80, color.b + 80, color.a});
    }
}

void DrawEpicBelt(int x, int y, int size, Color color) {
    // MEDIEVAL BELT with buckle and pouches
    int belt_width = size * 4 / 5;
    int belt_height = size / 6;
    int belt_x = x + (size - belt_width) / 2;
    int belt_y = y + size * 2 / 5;
    
    // Main belt strap
    DrawRectangleGradientV(belt_x, belt_y, belt_width, belt_height, color, (Color){color.r/2, color.g/2, color.b/2, color.a});
    
    // Belt buckle (square metal)
    int buckle_size = belt_height + 4;
    int buckle_x = belt_x + belt_width / 3;
    DrawRectangle(buckle_x, belt_y - 2, buckle_size, buckle_size, (Color){150, 150, 150, 255});
    DrawRectangle(buckle_x + 2, belt_y, buckle_size - 4, buckle_size - 4, (Color){80, 80, 80, 255});
    
    // Belt pouches
    int pouch_width = belt_width / 6;
    int pouch_height = belt_height + 6;
    DrawRectangle(belt_x + belt_width - pouch_width - 5, belt_y + belt_height, pouch_width, pouch_height, (Color){color.r/3, color.g/3, color.b/3, color.a});
    DrawRectangle(belt_x + 5, belt_y + belt_height, pouch_width, pouch_height, (Color){color.r/3, color.g/3, color.b/3, color.a});
}

void DrawEpicPants(int x, int y, int size, Color color) {
    // MEDIEVAL TROUSERS/LEGGINGS
    int leg_width = size / 3;
    int leg_height = size * 3 / 4;
    int left_leg_x = x + size / 6;
    int right_leg_x = x + size / 2;
    int legs_y = y + size / 8;
    
    // Left leg
    DrawRectangleGradientV(left_leg_x, legs_y, leg_width, leg_height, color, (Color){color.r/2, color.g/2, color.b/2, color.a});
    // Right leg
    DrawRectangleGradientV(right_leg_x, legs_y, leg_width, leg_height, color, (Color){color.r/2, color.g/2, color.b/2, color.a});
    
    // Knee protection
    DrawCircle(left_leg_x + leg_width/2, legs_y + leg_height/2, 4, (Color){color.r + 40, color.g + 40, color.b + 40, color.a});
    DrawCircle(right_leg_x + leg_width/2, legs_y + leg_height/2, 4, (Color){color.r + 40, color.g + 40, color.b + 40, color.a});
    
    // Belt line at top
    DrawRectangle(x + size/8, legs_y, size * 3/4, 3, (Color){color.r/3, color.g/3, color.b/3, color.a});
}

void DrawEpicBracers(int x, int y, int size, Color color) {
    // MEDIEVAL WRIST GUARDS/VAMBRACES
    int bracer_width = size * 2 / 3;
    int bracer_height = size / 2;
    int bracer_x = x + (size - bracer_width) / 2;
    int bracer_y = y + size / 4;
    
    // Main bracer body
    DrawRectangleGradientV(bracer_x, bracer_y, bracer_width, bracer_height, color, (Color){color.r/2, color.g/2, color.b/2, color.a});
    
    // Articulated segments
    for (int i = 0; i < 3; i++) {
        int segment_y = bracer_y + i * (bracer_height / 4);
        DrawRectangle(bracer_x + 2, segment_y, bracer_width - 4, 2, (Color){color.r + 40, color.g + 40, color.b + 40, color.a});
    }
    
    // Side straps
    DrawRectangle(bracer_x - 1, bracer_y + 5, 2, bracer_height - 10, (Color){139, 69, 19, 255}); // leather strap
    DrawRectangle(bracer_x + bracer_width - 1, bracer_y + 5, 2, bracer_height - 10, (Color){139, 69, 19, 255}); // leather strap
    
    // Metal studs
    DrawCircle(bracer_x + bracer_width/2, bracer_y + 5, 1, (Color){200, 200, 200, 255});
    DrawCircle(bracer_x + bracer_width/2, bracer_y + bracer_height - 5, 1, (Color){200, 200, 200, 255});
}

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
    return (Vector2){grid_x * cell_size, grid_y * cell_size};
}

static inline Color fade_color(Color color, uint32_t alpha) {
    color.a = alpha;
    return color;
}

static inline uint32_t alpha_from_ratio(float ratio) {
    return (uint32_t)(255.0f * ratio);
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

// ============================================================================
// SPRITE SYSTEM
// ============================================================================

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
                    
                    ImageDrawRectangleRec(&tileset_img, tile_rect, base_color);
                    
                    for (uint32_t tx = 0; tx < 4; tx++) {
                        for (uint32_t ty = 0; ty < 4; ty++) {
                            uint16_t stone_x = x * SPRITE_SIZE + tx * 8;
                            uint16_t stone_y = y * SPRITE_SIZE + ty * 8;
                            
                            ImageDrawRectangle(&tileset_img, stone_x + 1, stone_y + 1, 6, 6, highlight_color);
                            Rectangle stone_rect = {stone_x, stone_y, 8, 8};
                            ImageDrawRectangleLines(&tileset_img, stone_rect, 1, grout_color);
                            
                            if (rand() % 4 == 0) {
                                uint16_t crack_x = stone_x + (rand() % 6) + 1;
                                uint16_t crack_y = stone_y + (rand() % 6) + 1;
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
                    
                    for (uint32_t bx = 0; bx < 2; bx++) {
                        for (uint32_t by = 0; by < 4; by++) {
                            uint16_t brick_x = x * SPRITE_SIZE + bx * 16 + (by % 2) * 8;
                            uint16_t brick_y = y * SPRITE_SIZE + by * 8;
                            
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
                    
                    for (uint32_t i = 0; i < 6; i++) {
                        uint32_t stone_size = 6 + rand() % 4;
                        uint16_t sx = x * SPRITE_SIZE + (rand() % (SPRITE_SIZE - stone_size));
                        uint16_t sy = y * SPRITE_SIZE + (rand() % (SPRITE_SIZE - stone_size));
                        
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


// ============================================================================
// RENDER IMPLEMENTATIONS
// ============================================================================

static void render_background(Rift* env, float cell_size) {
    if (!env->client || !env->client->sprites.tileset.id) {
        Color bg_color = (Color){BG_COLOR_R, BG_COLOR_G, BG_COLOR_B, 255};
        DrawRectangle(0, 0, MAP_WIDTH * cell_size, MAP_HEIGHT * cell_size, bg_color);
        return;
    }
    
    SpriteSystem* sprites = &env->client->sprites;
    
    for (uint32_t y = 0; y < MAP_HEIGHT; y++) {
        for (uint32_t x = 0; x < MAP_WIDTH; x++) {
            uint16_t map_index = y * MAP_WIDTH + x;
            uint32_t cell_type = env->map[map_index];
            
            uint32_t tile_id;
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
            
            uint32_t tile_x = tile_id % TILESET_COLS;
            uint32_t tile_y = tile_id / TILESET_COLS;
            
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
    uint32_t max_frames;
    
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
        uint32_t monster_type = env->monsters[i].type;
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
        uint32_t bar_width = (uint32_t)half_cell;
        uint32_t half_bar_width = bar_width >> 1;
        int16_t bar_x = screen_pos.x + half_cell - half_bar_width;
        int16_t bar_y = screen_pos.y - third_cell - HEALTH_BAR_OFFSET;
        
        DrawRectangle(bar_x, bar_y, bar_width, HEALTH_BAR_HEIGHT, RED);
        DrawRectangle(bar_x, bar_y, (uint32_t)(bar_width * health_ratio), HEALTH_BAR_HEIGHT, GREEN);
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
    uint32_t bar_width = (uint32_t)cell_size;
    uint32_t half_bar_width = bar_width >> 1;
    int16_t bar_x = screen_pos.x + cell_size/2 - half_bar_width;
    int16_t bar_y = screen_pos.y - cell_size/2 - BOSS_HEALTH_BAR_OFFSET;
    
    DrawRectangle(bar_x, bar_y, bar_width, BOSS_HEALTH_BAR_HEIGHT, RED);
    DrawRectangle(bar_x, bar_y, (uint32_t)(bar_width * health_ratio), BOSS_HEALTH_BAR_HEIGHT, GREEN);
    DrawRectangleLines(bar_x, bar_y, bar_width, BOSS_HEALTH_BAR_HEIGHT, WHITE);
}

static void render_items(Rift* env, float cell_size) {
    for (uint32_t i = 0; i < MAX_ITEMS; i++) {
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
    
    for (uint32_t i = 0; i < MAX_PROJECTILES; i++) {
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
        uint32_t alpha = alpha_from_ratio(lifetime_ratio);
        projectile_color = fade_color(projectile_color, alpha);
        
        float radius = sixth_cell * size_multiplier;
        float half_radius = radius * 0.5f;
        uint32_t half_alpha = alpha >> 1;
        uint32_t third_alpha = alpha / 3;
        
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
    
    for (uint32_t i = 0; i < MAX_BLIZZARD_AREAS; i++) {
        if (!env->blizzard_areas[i].active) continue;
        
        Vector2 screen_pos = grid_to_screen(env->blizzard_areas[i].x, env->blizzard_areas[i].y, cell_size);
        screen_pos.x += half_cell;
        screen_pos.y += half_cell;
        
        float duration_ratio = (float)env->blizzard_areas[i].duration / BLIZZARD_DURATION;
        uint32_t alpha = (uint32_t)(BLIZZARD_BASE_ALPHA * duration_ratio);
        float radius = BLIZZARD_RADIUS * cell_size;
        uint32_t third_alpha = alpha / 3;
        uint32_t half_alpha = alpha >> 1;
        
        Color base_color = (Color){BLIZZARD_BASE_COLOR_R, BLIZZARD_BASE_COLOR_G, BLIZZARD_BASE_COLOR_B, third_alpha};
        Color ice_shard_color = (Color){255, 255, 255, alpha};
        Color frost_color = (Color){FROST_COLOR_R, FROST_COLOR_G, FROST_COLOR_B, half_alpha};
        
        DrawCircle(screen_pos.x, screen_pos.y, radius, base_color);
        
        uint16_t radius_int = (uint16_t)radius;
        uint16_t radius_double = radius_int << 1;
        
        for (uint32_t j = 0; j < ICE_SHARD_COUNT; j++) {
            int16_t random_offset_x = (rand() % radius_double) - radius_int;
            int16_t random_offset_y = (rand() % radius_double) - radius_int;
            float distance_squared = random_offset_x * random_offset_x + random_offset_y * random_offset_y;
            
            if (distance_squared <= radius * radius) {
                float x = screen_pos.x + random_offset_x;
                float y = screen_pos.y + random_offset_y;
                
                uint16_t fall_offset = ((env->tick + j * SHARD_TIMING_MULT) % SHARD_CYCLE_FRAMES);
                y += fall_offset * SHARD_FALL_SPEED;
                
                uint32_t shard_size = SHARD_SIZE_MIN + (rand() % SHARD_SIZE_RANGE);
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
    float time = GetTime();
    float pulse = sinf(time * 2.0f) * 0.2f + 0.8f;
    
    // EPIC bottom panel with gradient and shadows
    Color ui_top = {15, 20, 35, 250};
    Color ui_bot = {5, 10, 20, 250};
    DrawRectangleGradientV(0, ui_bottom, screen_width, UI_HEIGHT, ui_top, ui_bot);
    
    // Epic border effects
    DrawRectangle(0, ui_bottom - 3, screen_width, 3, (Color){100, 120, 150, 200});
    DrawRectangle(0, ui_bottom - 2, screen_width, 2, (Color){150, 180, 220, 150});
    DrawRectangle(0, ui_bottom - 1, screen_width, 1, (Color){200, 220, 255, 100});
    
    // EPIC Health Globe - Left side
    float health_ratio = (float)env->player.health / env->player.max_health;
    uint16_t health_globe_x = GLOBE_RADIUS + GLOBE_MARGIN + 10;
    uint16_t health_globe_y = screen_height - GLOBE_RADIUS - GLOBE_MARGIN;
    
    // Health globe epic glow effect
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
                Color health_fill = {255, 0, 0, 255}; // Pure blood red
                DrawRectangle(health_globe_x - half_width, current_y, full_width, 1, health_fill);
            }
        }
    }
    
    DrawCircleLines(health_globe_x, health_globe_y, GLOBE_RADIUS, (Color){255, 100, 100, 255});
    DrawCircleLines(health_globe_x, health_globe_y, GLOBE_RADIUS + 1, (Color){255, 150, 150, 150});
    DrawText("HP", health_globe_x - 10, health_globe_y - 25, 12, (Color){255, 100, 100, 255});
    DrawText(TextFormat("%d", env->player.health), health_globe_x - 15, health_globe_y - 8, TEXT_SIZE_16, WHITE);
    
    // EPIC Mana Globe - Right side
    float mana_ratio = (float)env->player.mana / env->player.max_mana;
    uint16_t mana_globe_x = screen_width - GLOBE_RADIUS - GLOBE_MARGIN - 10;
    uint16_t mana_globe_y = screen_height - GLOBE_RADIUS - GLOBE_MARGIN;
    
    // Mana globe epic glow effect
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
    
    // EPIC CENTER PANEL - Hero Stats and Gold
    uint16_t center_panel_x = health_globe_x + 80;
    uint16_t center_panel_w = mana_globe_x - center_panel_x - 80;
    uint16_t center_panel_y = ui_bottom + 10;
    
    // Center panel background with epic gradient
    Color center_bg_top = {30, 35, 50, 200};
    Color center_bg_bot = {15, 20, 30, 200};
    DrawRectangleGradientV(center_panel_x, center_panel_y, center_panel_w, UI_HEIGHT - 20, center_bg_top, center_bg_bot);
    DrawRectangleLines(center_panel_x, center_panel_y, center_panel_w, UI_HEIGHT - 20, (Color){100, 120, 150, 255});
    DrawRectangleLines(center_panel_x - 1, center_panel_y - 1, center_panel_w + 2, UI_HEIGHT - 18, (Color){150, 180, 220, 150});
    
    // EPIC Gold Display with sparkle
    char gold_text[32];
    sprintf(gold_text, "%d Gold", env->player.gold);
    Color gold_sparkle = {255, 215, 0, (uint32_t)(200 + 55 * sinf(time * 6.0f))};
    DrawText(gold_text, center_panel_x + 20, center_panel_y + 8, 18, gold_sparkle);
    
    // EPIC Hero Stats Display - Clean format with ilvl
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
    
    // Phase-specific info
    if (env->current_phase == PHASE_RIFT) {
        char rift_text[64];
        sprintf(rift_text, "Rift %d", env->current_rift_level);
        Color rift_color = {100, 255, 100, (uint32_t)(200 + 55 * pulse)};
        DrawText(rift_text, center_panel_x + 20, center_panel_y + 50, 14, rift_color);
        
        if (env->boss_spawned) {
            Color boss_warning = {255, 100, 100, (uint32_t)(150 + 105 * pulse)};
            DrawText("💀 BOSS BATTLE! 💀", center_panel_x + 200, center_panel_y + 50, 14, boss_warning);
        }
    } else {
        DrawText("🏪 Town Mode", center_panel_x + 20, center_panel_y + 50, 14, (Color){255, 215, 0, 255});
    }
    
    // EPIC Potion slots
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
    
    // Help icon with hover instructions (very top-left corner)
    Rectangle help_icon = {5, 5, 20, 20};
    Vector2 mouse_pos = GetMousePosition();
    bool hovering_help = CheckCollisionPointRec(mouse_pos, help_icon);
    
    // Draw help question mark icon
    Color help_color = hovering_help ? (Color){255, 255, 255, 255} : (Color){180, 180, 180, 200};
    DrawCircle(help_icon.x + 10, help_icon.y + 10, 10, (Color){40, 40, 40, 180});
    DrawCircleLines(help_icon.x + 10, help_icon.y + 10, 10, help_color);
    DrawText("?", help_icon.x + 6, help_icon.y + 3, 16, help_color);
    
    // Show instructions on hover
    if (hovering_help) {
        int panel_width = 400;
        int panel_height = 120;
        int panel_x = 15;
        int panel_y = 95;
        
        // Semi-transparent background panel
        DrawRectangle(panel_x, panel_y, panel_width, panel_height, (Color){0, 0, 0, 220});
        DrawRectangleLines(panel_x, panel_y, panel_width, panel_height, (Color){180, 180, 180, 255});
        
        // Help text
        DrawText("CONTROLS:", panel_x + 10, panel_y + 10, 14, (Color){255, 215, 0, 255});
        DrawText("WASD: Move character", panel_x + 10, panel_y + 30, 12, (Color){255, 255, 255, 255});
        DrawText("Space: Cast Blizzard spell", panel_x + 10, panel_y + 45, 12, (Color){255, 255, 255, 255});
        DrawText("Q/E: Use Health/Mana potions", panel_x + 10, panel_y + 60, 12, (Color){255, 255, 255, 255});
        DrawText("F: Interact (buy items, enter rift)", panel_x + 10, panel_y + 75, 12, (Color){255, 255, 255, 255});
        DrawText("Z: Exit town and go to next rift", panel_x + 10, panel_y + 90, 12, (Color){255, 215, 0, 255});
        DrawText("Hold SHIFT: Manual control (vs AI)", panel_x + 10, panel_y + 110, 12, (Color){100, 255, 100, 255});
    }
    
    // Simple AI indicator - only show when AI is active
    if (!(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))) {
        DrawRectangleGradientV(CLIENT_WIDTH - 45, 5, 40, MANUAL_CONTROL_HEIGHT, 
                              (Color){40, 40, 40, 200}, (Color){20, 20, 20, 200});
        DrawRectangleLines(CLIENT_WIDTH - 45, 5, 40, MANUAL_CONTROL_HEIGHT, (Color){100, 100, 100, 255});
        DrawText("AI", CLIENT_WIDTH - 35, 12, TEXT_SIZE_14, (Color){150, 150, 150, 255});
    }
    // Manual mode: no indicator needed - clean UI
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
    // Epic dark gradient background
    Color bg_top = {15, 20, 35, 255};
    Color bg_bottom = {5, 10, 20, 255};
    for (int y = 0; y < GetScreenHeight(); y++) {
        float t = (float)y / GetScreenHeight();
        Color current = {
            (uint32_t)(bg_top.r * (1-t) + bg_bottom.r * t),
            (uint32_t)(bg_top.g * (1-t) + bg_bottom.g * t),
            (uint32_t)(bg_top.b * (1-t) + bg_bottom.b * t),
            255
        };
        DrawRectangle(0, y, GetScreenWidth(), 1, current);
    }
    
    // EPIC Tab headers with glow effects
    int tab_width = 200;
    int tab_height = 50;
    float glow_pulse = sinf(GetTime() * 2.0f) * 0.3f + 0.7f;
    
    // Character Tab - Emerald theme
    Color char_glow = {0, 255, 100, (uint32_t)(100 * glow_pulse)};
    Color char_tab_color = (env->town_interface.current_tab == TOWN_TAB_CHARACTER) ? 
        (Color){20, 80, 40, 220} : (Color){40, 40, 40, 180};
    Color char_border = (env->town_interface.current_tab == TOWN_TAB_CHARACTER) ? 
        (Color){0, 255, 100, 255} : (Color){80, 80, 80, 255};
    
    if (env->town_interface.current_tab == TOWN_TAB_CHARACTER) {
        DrawRectangle(45, 15, tab_width + 10, tab_height + 10, char_glow);
    }
    DrawRectangleGradientV(50, 20, tab_width, tab_height, char_tab_color, 
                          (Color){char_tab_color.r/2, char_tab_color.g/2, char_tab_color.b/2, char_tab_color.a});
    DrawRectangleLines(50, 20, tab_width, tab_height, char_border);
    DrawRectangleLines(49, 19, tab_width + 2, tab_height + 2, char_border);
    DrawText("⚔ CHARACTER", TOP_UI_X + 70, TOP_UI_Y + 38, 18, (Color){255, 255, 255, 255});
    
    // Shop Tab - Sapphire theme  
    Color shop_glow = {100, 150, 255, (uint32_t)(100 * glow_pulse)};
    Color shop_tab_color = (env->town_interface.current_tab == TOWN_TAB_SHOP) ? 
        (Color){20, 40, 80, 220} : (Color){40, 40, 40, 180};
    Color shop_border = (env->town_interface.current_tab == TOWN_TAB_SHOP) ? 
        (Color){100, 150, 255, 255} : (Color){80, 80, 80, 255};
    
    if (env->town_interface.current_tab == TOWN_TAB_SHOP) {
        DrawRectangle(265, 15, tab_width + 10, tab_height + 10, shop_glow);
    }
    DrawRectangleGradientV(270, 20, tab_width, tab_height, shop_tab_color,
                          (Color){shop_tab_color.r/2, shop_tab_color.g/2, shop_tab_color.b/2, shop_tab_color.a});
    DrawRectangleLines(270, 20, tab_width, tab_height, shop_border);
    DrawRectangleLines(269, 19, tab_width + 2, tab_height + 2, shop_border);
    DrawText("⚜ SHOP", TOP_UI_X + 320, TOP_UI_Y + 38, 18, (Color){255, 255, 255, 255});
    
    
    // EPIC Main content area with deep shadows and glow
    Color content_bg = {25, 30, 45, 240};
    Color content_shadow = {0, 0, 0, 150};
    
    // Drop shadow
    DrawRectangle(55, 85, 920, 520, content_shadow);
    DrawRectangle(54, 84, 920, 520, content_shadow);
    
    // Main panel with gradient
    DrawRectangleGradientV(50, 80, 920, 520, content_bg, 
                          (Color){content_bg.r/2, content_bg.g/2, content_bg.b/2, content_bg.a});
    
    // Epic border effects
    Color border_main = {100, 120, 150, 255};
    DrawRectangleLines(50, 80, 920, 520, border_main);
    DrawRectangleLines(49, 79, 922, 522, (Color){border_main.r/2, border_main.g/2, border_main.b/2, 180});
    DrawRectangleLines(51, 81, 918, 518, (Color){border_main.r + 50, border_main.g + 50, border_main.b + 50, 120});
    
    if (env->town_interface.current_tab == TOWN_TAB_CHARACTER) {
        render_character_tab(env);
    } else if (env->town_interface.current_tab == TOWN_TAB_SHOP) {
        render_shop_tab(env);
    }
    
    // Timer and status
    Color timer_color = GREEN;
    if (env->town_interface.frames_remaining < 120) timer_color = RED;
    else if (env->town_interface.frames_remaining < 200) timer_color = ORANGE;
    
    float time_remaining = (float)env->town_interface.frames_remaining / 60.0f;
    char timer_text[32];
    sprintf(timer_text, "⏰ Time: %.1fs", time_remaining);
    DrawText(timer_text, TOP_UI_X + TOP_UI_WIDTH - 180, TOP_UI_Y + 15, 18, timer_color);
    
    // Rift level display
    char rift_text[32];
    sprintf(rift_text, "Rift %d", env->current_rift_level);
    DrawText(rift_text, TOP_UI_X + 40, TOP_UI_Y + 15, 20, (Color){255, 215, 0, 255});
    
    // Remove duplicate level/gold display - already shown in bottom UI
    
    // Removed floating navigation instructions - interface should be intuitive
}

void render_character_tab(Rift* env) {
    // PROPER LAYOUT USING DEFINED CONSTANTS
    // All positioning now uses the container hierarchy constants
    
    int start_y = 120;
    float time = GetTime();
    float pulse = sinf(time * 3.0f) * 0.2f + 0.8f;
    
    if (env->town_interface.character_mode == CHARACTER_MODE_EQUIPMENT) {
        // DIABLO 3 WARRIOR LAYOUT - 3 COLUMNS WITHIN EQUIPMENT AREA
        DrawText("⚔ HERO EQUIPMENT ⚔", CONTENT_AREA_X, start_y, 24, (Color){255, 215, 0, 255});
        
        // Equipment slot size and layout constants (closer together like original)
        int slot_size = 60;
        int column_spacing = 80;  // Fixed reasonable spacing (like original was ~120, now 80)
        int row_spacing = 80;
        
        // Center equipment within allocated area using constants
        int equipment_start_x = CONTENT_AREA_X + 20;  // Small internal padding
        int left_col_x = equipment_start_x;
        int middle_col_x = left_col_x + slot_size + column_spacing;  
        int right_col_x = middle_col_x + slot_size + column_spacing;
        int base_y = start_y + 80;
        
        // Clean layout without background silhouette
        
        // LEFT COLUMN (from top to bottom)
        // SHOULDERS (0)
        int shoulders_x = left_col_x, shoulders_y = base_y;
        Color shoulders_quality_color = (env->equipment.shoulders_type != EQUIPMENT_NONE) ? GetQualityColor(env->equipment.shoulders_quality) : (Color){80, 80, 80, 255};
        if (env->town_interface.selected_item_index == 0) {
            DrawRectangle(shoulders_x - 6, shoulders_y - 6, slot_size + 12, slot_size + 12, (Color){shoulders_quality_color.r, shoulders_quality_color.g, shoulders_quality_color.b, (uint32_t)(120 * pulse)});
        }
        DrawRectangleGradientV(shoulders_x, shoulders_y, slot_size, slot_size, (Color){shoulders_quality_color.r/4, shoulders_quality_color.g/4, shoulders_quality_color.b/4, 200}, (Color){shoulders_quality_color.r/8, shoulders_quality_color.g/8, shoulders_quality_color.b/8, 200});
        DrawRectangleLines(shoulders_x, shoulders_y, slot_size, slot_size, shoulders_quality_color);
        if (env->equipment.shoulders_type != EQUIPMENT_NONE) {
            DrawEpicShoulders(shoulders_x, shoulders_y, slot_size, GetQualityColor(env->equipment.shoulders_quality));
        } else {
            DrawText("🛡", shoulders_x + 15, shoulders_y + 10, 25, (Color){80, 80, 80, 255});
        }
        DrawText("SHOULDERS", shoulders_x - 2, shoulders_y + slot_size + 5, 8, (Color){180, 180, 180, 255});
        
        // GLOVES (1)
        int gloves_x = left_col_x, gloves_y = base_y + row_spacing;
        Color gloves_quality_color = (env->equipment.gloves_type != EQUIPMENT_NONE) ? GetQualityColor(env->equipment.gloves_quality) : (Color){80, 80, 80, 255};
        if (env->town_interface.selected_item_index == 1) {
            DrawRectangle(gloves_x - 6, gloves_y - 6, slot_size + 12, slot_size + 12, (Color){gloves_quality_color.r, gloves_quality_color.g, gloves_quality_color.b, (uint32_t)(120 * pulse)});
        }
        DrawRectangleGradientV(gloves_x, gloves_y, slot_size, slot_size, (Color){gloves_quality_color.r/4, gloves_quality_color.g/4, gloves_quality_color.b/4, 200}, (Color){gloves_quality_color.r/8, gloves_quality_color.g/8, gloves_quality_color.b/8, 200});
        DrawRectangleLines(gloves_x, gloves_y, slot_size, slot_size, gloves_quality_color);
        if (env->equipment.gloves_type != EQUIPMENT_NONE) {
            DrawEpicGloves(gloves_x, gloves_y, slot_size, GetQualityColor(env->equipment.gloves_quality));
        } else {
            DrawText("🧤", gloves_x + 15, gloves_y + 10, 25, (Color){80, 80, 80, 255});
        }
        DrawText("GLOVES", gloves_x + 5, gloves_y + slot_size + 5, 8, (Color){180, 180, 180, 255});
        
        // RING LEFT (2)
        int ring_left_x = left_col_x, ring_left_y = base_y + 2 * row_spacing;
        Color ring_left_quality_color = (env->equipment.ring_left_type != EQUIPMENT_NONE) ? GetQualityColor(env->equipment.ring_left_quality) : (Color){80, 80, 80, 255};
        if (env->town_interface.selected_item_index == 2) {
            DrawRectangle(ring_left_x - 6, ring_left_y - 6, slot_size + 12, slot_size + 12, (Color){ring_left_quality_color.r, ring_left_quality_color.g, ring_left_quality_color.b, (uint32_t)(120 * pulse)});
        }
        DrawRectangleGradientV(ring_left_x, ring_left_y, slot_size, slot_size, (Color){ring_left_quality_color.r/4, ring_left_quality_color.g/4, ring_left_quality_color.b/4, 200}, (Color){ring_left_quality_color.r/8, ring_left_quality_color.g/8, ring_left_quality_color.b/8, 200});
        DrawRectangleLines(ring_left_x, ring_left_y, slot_size, slot_size, ring_left_quality_color);
        if (env->equipment.ring_left_type != EQUIPMENT_NONE) {
            DrawEpicRing(ring_left_x, ring_left_y, slot_size, GetQualityColor(env->equipment.ring_left_quality));
        } else {
            DrawText("💍", ring_left_x + 15, ring_left_y + 10, 20, (Color){80, 80, 80, 255});
        }
        DrawText("RING L", ring_left_x + 8, ring_left_y + slot_size + 5, 8, (Color){180, 180, 180, 255});
        
        // WEAPON (3)
        int weapon_x = left_col_x, weapon_y = base_y + 3 * row_spacing;
        Color weapon_quality_color = (env->equipment.weapon_type != EQUIPMENT_NONE) ? GetQualityColor(env->equipment.weapon_quality) : (Color){80, 80, 80, 255};
        if (env->town_interface.selected_item_index == 3) {
            DrawRectangle(weapon_x - 6, weapon_y - 6, slot_size + 12, slot_size + 12, (Color){weapon_quality_color.r, weapon_quality_color.g, weapon_quality_color.b, (uint32_t)(120 * pulse)});
        }
        DrawRectangleGradientV(weapon_x, weapon_y, slot_size, slot_size, (Color){weapon_quality_color.r/4, weapon_quality_color.g/4, weapon_quality_color.b/4, 200}, (Color){weapon_quality_color.r/8, weapon_quality_color.g/8, weapon_quality_color.b/8, 200});
        DrawRectangleLines(weapon_x, weapon_y, slot_size, slot_size, weapon_quality_color);
        if (env->equipment.weapon_type != EQUIPMENT_NONE) {
            DrawEpicWeapon(weapon_x, weapon_y, slot_size, GetQualityColor(env->equipment.weapon_quality));
        } else {
            DrawText("⚔", weapon_x + 15, weapon_y + 10, 25, (Color){80, 80, 80, 255});
        }
        DrawText("WEAPON", weapon_x + 5, weapon_y + slot_size + 5, 8, (Color){180, 180, 180, 255});
        
        // MIDDLE COLUMN (from top to bottom)
        // HELMET (4)
        int helmet_x = middle_col_x, helmet_y = base_y;
        Color helmet_quality_color = (env->equipment.helmet_type != EQUIPMENT_NONE) ? GetQualityColor(env->equipment.helmet_quality) : (Color){80, 80, 80, 255};
        if (env->town_interface.selected_item_index == 4) {
            DrawRectangle(helmet_x - 6, helmet_y - 6, slot_size + 12, slot_size + 12, (Color){helmet_quality_color.r, helmet_quality_color.g, helmet_quality_color.b, (uint32_t)(120 * pulse)});
        }
        DrawRectangleGradientV(helmet_x, helmet_y, slot_size, slot_size, (Color){helmet_quality_color.r/4, helmet_quality_color.g/4, helmet_quality_color.b/4, 200}, (Color){helmet_quality_color.r/8, helmet_quality_color.g/8, helmet_quality_color.b/8, 200});
        DrawRectangleLines(helmet_x, helmet_y, slot_size, slot_size, helmet_quality_color);
        if (env->equipment.helmet_type != EQUIPMENT_NONE) {
            DrawEpicHelmet(helmet_x, helmet_y, slot_size, GetQualityColor(env->equipment.helmet_quality));
        } else {
            DrawText("⛑", helmet_x + 15, helmet_y + 10, 25, (Color){80, 80, 80, 255});
        }
        DrawText("HELMET", helmet_x + 5, helmet_y + slot_size + 5, 8, (Color){180, 180, 180, 255});
        
        // ARMOR (5)
        int armor_x = middle_col_x, armor_y = base_y + row_spacing;
        Color armor_quality_color = (env->equipment.armor_type != EQUIPMENT_NONE) ? GetQualityColor(env->equipment.armor_quality) : (Color){80, 80, 80, 255};
        if (env->town_interface.selected_item_index == 5) {
            DrawRectangle(armor_x - 6, armor_y - 6, slot_size + 12, slot_size + 12, (Color){armor_quality_color.r, armor_quality_color.g, armor_quality_color.b, (uint32_t)(120 * pulse)});
        }
        DrawRectangleGradientV(armor_x, armor_y, slot_size, slot_size, (Color){armor_quality_color.r/4, armor_quality_color.g/4, armor_quality_color.b/4, 200}, (Color){armor_quality_color.r/8, armor_quality_color.g/8, armor_quality_color.b/8, 200});
        DrawRectangleLines(armor_x, armor_y, slot_size, slot_size, armor_quality_color);
        if (env->equipment.armor_type != EQUIPMENT_NONE) {
            DrawEpicArmor(armor_x, armor_y, slot_size, GetQualityColor(env->equipment.armor_quality));
        } else {
            DrawText("🛡", armor_x + 15, armor_y + 10, 25, (Color){80, 80, 80, 255});
        }
        DrawText("ARMOR", armor_x + 8, armor_y + slot_size + 5, 8, (Color){180, 180, 180, 255});
        
        // BELT (6)
        int belt_x = middle_col_x, belt_y = base_y + 2 * row_spacing;
        Color belt_quality_color = (env->equipment.belt_type != EQUIPMENT_NONE) ? GetQualityColor(env->equipment.belt_quality) : (Color){80, 80, 80, 255};
        if (env->town_interface.selected_item_index == 6) {
            DrawRectangle(belt_x - 6, belt_y - 6, slot_size + 12, slot_size + 12, (Color){belt_quality_color.r, belt_quality_color.g, belt_quality_color.b, (uint32_t)(120 * pulse)});
        }
        DrawRectangleGradientV(belt_x, belt_y, slot_size, slot_size, (Color){belt_quality_color.r/4, belt_quality_color.g/4, belt_quality_color.b/4, 200}, (Color){belt_quality_color.r/8, belt_quality_color.g/8, belt_quality_color.b/8, 200});
        DrawRectangleLines(belt_x, belt_y, slot_size, slot_size, belt_quality_color);
        if (env->equipment.belt_type != EQUIPMENT_NONE) {
            DrawEpicBelt(belt_x, belt_y, slot_size, GetQualityColor(env->equipment.belt_quality));
        } else {
            DrawText("🎗", belt_x + 15, belt_y + 10, 25, (Color){80, 80, 80, 255});
        }
        DrawText("BELT", belt_x + 18, belt_y + slot_size + 5, 8, (Color){180, 180, 180, 255});
        
        // PANTS (7)
        int pants_x = middle_col_x, pants_y = base_y + 3 * row_spacing;
        Color pants_quality_color = (env->equipment.pants_type != EQUIPMENT_NONE) ? GetQualityColor(env->equipment.pants_quality) : (Color){80, 80, 80, 255};
        if (env->town_interface.selected_item_index == 7) {
            DrawRectangle(pants_x - 6, pants_y - 6, slot_size + 12, slot_size + 12, (Color){pants_quality_color.r, pants_quality_color.g, pants_quality_color.b, (uint32_t)(120 * pulse)});
        }
        DrawRectangleGradientV(pants_x, pants_y, slot_size, slot_size, (Color){pants_quality_color.r/4, pants_quality_color.g/4, pants_quality_color.b/4, 200}, (Color){pants_quality_color.r/8, pants_quality_color.g/8, pants_quality_color.b/8, 200});
        DrawRectangleLines(pants_x, pants_y, slot_size, slot_size, pants_quality_color);
        if (env->equipment.pants_type != EQUIPMENT_NONE) {
            DrawEpicPants(pants_x, pants_y, slot_size, GetQualityColor(env->equipment.pants_quality));
        } else {
            DrawText("👖", pants_x + 15, pants_y + 10, 25, (Color){80, 80, 80, 255});
        }
        DrawText("PANTS", pants_x + 8, pants_y + slot_size + 5, 8, (Color){180, 180, 180, 255});
        
        // BOOTS (8)
        int boots_x = middle_col_x, boots_y = base_y + 4 * row_spacing;
        Color boots_quality_color = (env->equipment.boots_type != EQUIPMENT_NONE) ? GetQualityColor(env->equipment.boots_quality) : (Color){80, 80, 80, 255};
        if (env->town_interface.selected_item_index == 8) {
            DrawRectangle(boots_x - 6, boots_y - 6, slot_size + 12, slot_size + 12, (Color){boots_quality_color.r, boots_quality_color.g, boots_quality_color.b, (uint32_t)(120 * pulse)});
        }
        DrawRectangleGradientV(boots_x, boots_y, slot_size, slot_size, (Color){boots_quality_color.r/4, boots_quality_color.g/4, boots_quality_color.b/4, 200}, (Color){boots_quality_color.r/8, boots_quality_color.g/8, boots_quality_color.b/8, 200});
        DrawRectangleLines(boots_x, boots_y, slot_size, slot_size, boots_quality_color);
        if (env->equipment.boots_type != EQUIPMENT_NONE) {
            DrawEpicBoots(boots_x, boots_y, slot_size, GetQualityColor(env->equipment.boots_quality));
        } else {
            DrawText("👢", boots_x + 15, boots_y + 10, 25, (Color){80, 80, 80, 255});
        }
        DrawText("BOOTS", boots_x + 8, boots_y + slot_size + 5, 8, (Color){180, 180, 180, 255});
        
        // RIGHT COLUMN (from top to bottom)
        // AMULET (9)
        int amulet_x = right_col_x, amulet_y = base_y;
        Color amulet_quality_color = (env->equipment.amulet_type != EQUIPMENT_NONE) ? GetQualityColor(env->equipment.amulet_quality) : (Color){80, 80, 80, 255};
        if (env->town_interface.selected_item_index == 9) {
            DrawRectangle(amulet_x - 6, amulet_y - 6, slot_size + 12, slot_size + 12, (Color){amulet_quality_color.r, amulet_quality_color.g, amulet_quality_color.b, (uint32_t)(120 * pulse)});
        }
        DrawRectangleGradientV(amulet_x, amulet_y, slot_size, slot_size, (Color){amulet_quality_color.r/4, amulet_quality_color.g/4, amulet_quality_color.b/4, 200}, (Color){amulet_quality_color.r/8, amulet_quality_color.g/8, amulet_quality_color.b/8, 200});
        DrawRectangleLines(amulet_x, amulet_y, slot_size, slot_size, amulet_quality_color);
        if (env->equipment.amulet_type != EQUIPMENT_NONE) {
            DrawEpicAmulet(amulet_x, amulet_y, slot_size, GetQualityColor(env->equipment.amulet_quality));
        } else {
            DrawText("🔮", amulet_x + 15, amulet_y + 10, 20, (Color){80, 80, 80, 255});
        }
        DrawText("AMULET", amulet_x + 2, amulet_y + slot_size + 5, 8, (Color){180, 180, 180, 255});
        
        // BRACERS (10)
        int bracers_x = right_col_x, bracers_y = base_y + row_spacing;
        Color bracers_quality_color = (env->equipment.bracers_type != EQUIPMENT_NONE) ? GetQualityColor(env->equipment.bracers_quality) : (Color){80, 80, 80, 255};
        if (env->town_interface.selected_item_index == 10) {
            DrawRectangle(bracers_x - 6, bracers_y - 6, slot_size + 12, slot_size + 12, (Color){bracers_quality_color.r, bracers_quality_color.g, bracers_quality_color.b, (uint32_t)(120 * pulse)});
        }
        DrawRectangleGradientV(bracers_x, bracers_y, slot_size, slot_size, (Color){bracers_quality_color.r/4, bracers_quality_color.g/4, bracers_quality_color.b/4, 200}, (Color){bracers_quality_color.r/8, bracers_quality_color.g/8, bracers_quality_color.b/8, 200});
        DrawRectangleLines(bracers_x, bracers_y, slot_size, slot_size, bracers_quality_color);
        if (env->equipment.bracers_type != EQUIPMENT_NONE) {
            DrawEpicBracers(bracers_x, bracers_y, slot_size, GetQualityColor(env->equipment.bracers_quality));
        } else {
            DrawText("⚔", bracers_x + 15, bracers_y + 10, 25, (Color){80, 80, 80, 255});
        }
        DrawText("BRACERS", bracers_x - 2, bracers_y + slot_size + 5, 8, (Color){180, 180, 180, 255});
        
        // RING RIGHT (11)
        int ring_right_x = right_col_x, ring_right_y = base_y + 2 * row_spacing;
        Color ring_right_quality_color = (env->equipment.ring_right_type != EQUIPMENT_NONE) ? GetQualityColor(env->equipment.ring_right_quality) : (Color){80, 80, 80, 255};
        if (env->town_interface.selected_item_index == 11) {
            DrawRectangle(ring_right_x - 6, ring_right_y - 6, slot_size + 12, slot_size + 12, (Color){ring_right_quality_color.r, ring_right_quality_color.g, ring_right_quality_color.b, (uint32_t)(120 * pulse)});
        }
        DrawRectangleGradientV(ring_right_x, ring_right_y, slot_size, slot_size, (Color){ring_right_quality_color.r/4, ring_right_quality_color.g/4, ring_right_quality_color.b/4, 200}, (Color){ring_right_quality_color.r/8, ring_right_quality_color.g/8, ring_right_quality_color.b/8, 200});
        DrawRectangleLines(ring_right_x, ring_right_y, slot_size, slot_size, ring_right_quality_color);
        if (env->equipment.ring_right_type != EQUIPMENT_NONE) {
            DrawEpicRing(ring_right_x, ring_right_y, slot_size, GetQualityColor(env->equipment.ring_right_quality));
        } else {
            DrawText("💍", ring_right_x + 15, ring_right_y + 10, 20, (Color){80, 80, 80, 255});
        }
        DrawText("RING R", ring_right_x + 5, ring_right_y + slot_size + 5, 8, (Color){180, 180, 180, 255});
        
        // OFFHAND (12)
        int offhand_x = right_col_x, offhand_y = base_y + 3 * row_spacing;
        Color offhand_quality_color = (env->equipment.offhand_type != EQUIPMENT_NONE) ? GetQualityColor(env->equipment.offhand_quality) : (Color){80, 80, 80, 255};
        if (env->town_interface.selected_item_index == 12) {
            DrawRectangle(offhand_x - 6, offhand_y - 6, slot_size + 12, slot_size + 12, (Color){offhand_quality_color.r, offhand_quality_color.g, offhand_quality_color.b, (uint32_t)(120 * pulse)});
        }
        DrawRectangleGradientV(offhand_x, offhand_y, slot_size, slot_size, (Color){offhand_quality_color.r/4, offhand_quality_color.g/4, offhand_quality_color.b/4, 200}, (Color){offhand_quality_color.r/8, offhand_quality_color.g/8, offhand_quality_color.b/8, 200});
        DrawRectangleLines(offhand_x, offhand_y, slot_size, slot_size, offhand_quality_color);
        if (env->equipment.offhand_type != EQUIPMENT_NONE) {
            DrawEpicOffhand(offhand_x, offhand_y, slot_size, GetQualityColor(env->equipment.offhand_quality));
        } else {
            DrawText("🛡", offhand_x + 15, offhand_y + 10, 25, (Color){80, 80, 80, 255});
        }
        DrawText("OFFHAND", offhand_x - 2, offhand_y + slot_size + 5, 8, (Color){180, 180, 180, 255});
        
        // DIABLO-STYLE ITEM STATS CARD - Using container constants
        int stats_panel_y = base_y;
        int available_height = TOWN_CONTAINER_HEIGHT - (stats_panel_y - TOWN_CONTAINER_Y) - 20; // Leave 20px bottom margin
        int stats_panel_height = available_height;
        
        // Panel background with border using constants
        DrawRectangleGradientV(STATS_PANEL_X, stats_panel_y, STATS_PANEL_WIDTH, stats_panel_height, 
                              (Color){25, 20, 15, 240}, (Color){15, 10, 8, 240});
        DrawRectangleLines(STATS_PANEL_X, stats_panel_y, STATS_PANEL_WIDTH, stats_panel_height, 
                          (Color){150, 120, 80, 255});
        
        // Get currently selected equipment
        uint32_t selected_slot = env->town_interface.selected_item_index;
        uint32_t item_type = EQUIPMENT_NONE;
        uint32_t item_quality = QUALITY_COMMON;
        uint32_t item_level = 0;
        uint32_t stat_bonuses[4] = {0, 0, 0, 0};
        char* slot_name = "NONE";
        
        // Map selected index to equipment data
        switch (selected_slot) {
            case 0: // Shoulders
                item_type = env->equipment.shoulders_type;
                item_quality = env->equipment.shoulders_quality;
                item_level = env->equipment.shoulders_level;
                for (int i = 0; i < 4; i++) stat_bonuses[i] = env->equipment.shoulders_stat_bonuses[i];
                slot_name = "SHOULDERS";
                break;
            case 1: // Gloves
                item_type = env->equipment.gloves_type;
                item_quality = env->equipment.gloves_quality;
                item_level = env->equipment.gloves_level;
                for (int i = 0; i < 4; i++) stat_bonuses[i] = env->equipment.gloves_stat_bonuses[i];
                slot_name = "GLOVES";
                break;
            case 2: // Ring Left
                item_type = env->equipment.ring_left_type;
                item_quality = env->equipment.ring_left_quality;
                item_level = env->equipment.ring_left_level;
                for (int i = 0; i < 4; i++) stat_bonuses[i] = env->equipment.ring_left_stat_bonuses[i];
                slot_name = "RING (LEFT)";
                break;
            case 3: // Weapon
                item_type = env->equipment.weapon_type;
                item_quality = env->equipment.weapon_quality;
                item_level = env->equipment.weapon_level;
                for (int i = 0; i < 4; i++) stat_bonuses[i] = env->equipment.weapon_stat_bonuses[i];
                slot_name = "WEAPON";
                break;
            case 4: // Helmet
                item_type = env->equipment.helmet_type;
                item_quality = env->equipment.helmet_quality;
                item_level = env->equipment.helmet_level;
                for (int i = 0; i < 4; i++) stat_bonuses[i] = env->equipment.helmet_stat_bonuses[i];
                slot_name = "HELMET";
                break;
            case 5: // Armor
                item_type = env->equipment.armor_type;
                item_quality = env->equipment.armor_quality;
                item_level = env->equipment.armor_level;
                for (int i = 0; i < 4; i++) stat_bonuses[i] = env->equipment.armor_stat_bonuses[i];
                slot_name = "CHEST ARMOR";
                break;
            case 6: // Belt
                item_type = env->equipment.belt_type;
                item_quality = env->equipment.belt_quality;
                item_level = env->equipment.belt_level;
                for (int i = 0; i < 4; i++) stat_bonuses[i] = env->equipment.belt_stat_bonuses[i];
                slot_name = "BELT";
                break;
            case 7: // Pants
                item_type = env->equipment.pants_type;
                item_quality = env->equipment.pants_quality;
                item_level = env->equipment.pants_level;
                for (int i = 0; i < 4; i++) stat_bonuses[i] = env->equipment.pants_stat_bonuses[i];
                slot_name = "PANTS";
                break;
            case 8: // Boots
                item_type = env->equipment.boots_type;
                item_quality = env->equipment.boots_quality;
                item_level = env->equipment.boots_level;
                for (int i = 0; i < 4; i++) stat_bonuses[i] = env->equipment.boots_stat_bonuses[i];
                slot_name = "BOOTS";
                break;
            case 9: // Amulet
                item_type = env->equipment.amulet_type;
                item_quality = env->equipment.amulet_quality;
                item_level = env->equipment.amulet_level;
                for (int i = 0; i < 4; i++) stat_bonuses[i] = env->equipment.amulet_stat_bonuses[i];
                slot_name = "AMULET";
                break;
            case 10: // Bracers
                item_type = env->equipment.bracers_type;
                item_quality = env->equipment.bracers_quality;
                item_level = env->equipment.bracers_level;
                for (int i = 0; i < 4; i++) stat_bonuses[i] = env->equipment.bracers_stat_bonuses[i];
                slot_name = "BRACERS";
                break;
            case 11: // Ring Right
                item_type = env->equipment.ring_right_type;
                item_quality = env->equipment.ring_right_quality;
                item_level = env->equipment.ring_right_level;
                for (int i = 0; i < 4; i++) stat_bonuses[i] = env->equipment.ring_right_stat_bonuses[i];
                slot_name = "RING (RIGHT)";
                break;
            case 12: // Offhand
                item_type = env->equipment.offhand_type;
                item_quality = env->equipment.offhand_quality;
                item_level = env->equipment.offhand_level;
                for (int i = 0; i < 4; i++) stat_bonuses[i] = env->equipment.offhand_stat_bonuses[i];
                slot_name = "OFFHAND";
                break;
        }
        
        if (item_type != EQUIPMENT_NONE) {
            // Item card header
            Color quality_color = GetQualityColor(item_quality);
            DrawText(slot_name, STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT, 14, quality_color);
            
            // Item level
            char ilvl_text[32];
            sprintf(ilvl_text, "Item Level %d", item_level);
            DrawText(ilvl_text, STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT + STATS_LINE_HEIGHT, 12, (Color){200, 200, 200, 255});
            
            // Quality tier
            char* quality_names[] = {"COMMON", "RARE", "EPIC", "LEGENDARY"};
            char* quality_name = (item_quality < 4) ? quality_names[item_quality] : "UNKNOWN";
            DrawText(quality_name, STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT + 2 * STATS_LINE_HEIGHT, 12, quality_color);
            
            // Item visual in card (small version)
            int icon_x = STATS_PANEL_X + STATS_PANEL_WIDTH - 70;
            int icon_y = stats_panel_y + 15;
            int icon_size = 50;
            DrawRectangleGradientV(icon_x, icon_y, icon_size, icon_size, 
                                  (Color){40, 35, 30, 200}, (Color){20, 15, 10, 200});
            DrawRectangleLines(icon_x, icon_y, icon_size, icon_size, quality_color);
            
            // Draw mini equipment visual
            switch (selected_slot) {
                case 0: DrawEpicShoulders(icon_x, icon_y, icon_size, quality_color); break;
                case 1: DrawEpicGloves(icon_x, icon_y, icon_size, quality_color); break;
                case 2: DrawEpicRing(icon_x, icon_y, icon_size, quality_color); break;
                case 3: DrawEpicWeapon(icon_x, icon_y, icon_size, quality_color); break;
                case 4: DrawEpicHelmet(icon_x, icon_y, icon_size, quality_color); break;
                case 5: DrawEpicArmor(icon_x, icon_y, icon_size, quality_color); break;
                case 6: DrawEpicBelt(icon_x, icon_y, icon_size, quality_color); break;
                case 7: DrawEpicPants(icon_x, icon_y, icon_size, quality_color); break;
                case 8: DrawEpicBoots(icon_x, icon_y, icon_size, quality_color); break;
                case 9: DrawEpicAmulet(icon_x, icon_y, icon_size, quality_color); break;
                case 10: DrawEpicBracers(icon_x, icon_y, icon_size, quality_color); break;
                case 11: DrawEpicRing(icon_x, icon_y, icon_size, quality_color); break;
                case 12: DrawEpicOffhand(icon_x, icon_y, icon_size, quality_color); break;
            }
            
            // Stats section
            DrawText("STATS:", STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT + 2 * STATS_LINE_HEIGHT + STATS_SECTION_SPACING, 12, (Color){255, 215, 0, 255});
            int stat_y = stats_panel_y + 110;
            char* stat_names[] = {"Strength", "Dexterity", "Intelligence", "Vitality"};
            Color stat_colors[] = {
                {160, 160, 160, 255}, // STR - gray
                {160, 160, 160, 255}, // DEX - gray  
                {160, 160, 160, 255}, // INT - gray
                {160, 160, 160, 255}  // VIT - gray
            };
            
            for (int i = 0; i < 4; i++) {
                if (stat_bonuses[i] > 0) {
                    char stat_text[64];
                    sprintf(stat_text, "+%d %s", stat_bonuses[i], stat_names[i]);
                    DrawText(stat_text, STATS_PANEL_X + STATS_CONTENT_MARGIN_INDENT, stat_y, 11, stat_colors[i]);
                    stat_y += 18;
                }
            }
            
            // Separator line
            DrawRectangle(STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stat_y + 5, STATS_PANEL_WIDTH - 2 * STATS_CONTENT_MARGIN_LEFT, 1, (Color){100, 80, 50, 255});
            
        } else {
            // Empty slot message
            DrawText("EMPTY SLOT", STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT, 14, (Color){100, 100, 100, 255});
            DrawText("No equipment equipped", STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT + STATS_LINE_HEIGHT, 11, (Color){150, 150, 150, 255});
        }
        
    } else {
        // EPIC Inventory Display (unchanged)
        DrawText("🎒 HERO INVENTORY 🎒", CONTENT_AREA_X, start_y, 24, (Color){255, 215, 0, 255});
        
        // Inventory grid with stunning visual slots
        for (int i = 0; i < INVENTORY_SLOTS; i++) {
            int row = i / 4;
            int col = i % 4;
            int x = CONTENT_AREA_X + col * 120;
            int y = start_y + 60 + row * 100;
            
            Color slot_bg = {30, 35, 50, 220};
            Color slot_border = {100, 120, 150, 255};
            
            if (env->town_interface.selected_item_index == i) {
                DrawRectangle(x - 8, y - 8, 96, 96, (Color){255, 255, 0, (uint32_t)(150 * pulse)});
            }
            
            // Slot background with gradient
            DrawRectangleGradientV(x, y, 80, 80, slot_bg, (Color){slot_bg.r/2, slot_bg.g/2, slot_bg.b/2, slot_bg.a});
            
            if (env->player.inventory[i].item_type != EQUIPMENT_NONE) {
                // Item present - epic visual treatment
                Color item_quality_color = GetQualityColor(env->player.inventory[i].item_quality);
                DrawRectangle(x - 2, y - 2, 84, 84, (Color){item_quality_color.r, item_quality_color.g, item_quality_color.b, 80});
                
                // Use new visual item drawing functions
                switch (env->player.inventory[i].item_type) {
                    case EQUIPMENT_WEAPON:
                        DrawEpicWeapon(x, y, 60, item_quality_color);
                        break;
                    case EQUIPMENT_OFFHAND:
                        DrawEpicOffhand(x, y, 60, item_quality_color);
                        break;
                    case EQUIPMENT_RING:
                        DrawEpicRing(x, y, 60, item_quality_color);
                        break;
                    case EQUIPMENT_AMULET:
                        DrawEpicAmulet(x, y, 60, item_quality_color);
                        break;
                    case EQUIPMENT_BOOTS:
                        DrawEpicBoots(x, y, 60, item_quality_color);
                        break;
                    case EQUIPMENT_GLOVES:
                        DrawEpicGloves(x, y, 60, item_quality_color);
                        break;
                    case EQUIPMENT_HELMET:
                        DrawEpicHelmet(x, y, 60, item_quality_color);
                        break;
                    default:
                        DrawText("🧪", x + 20, y + 15, 40, item_quality_color);
                        break;
                }
                
                // Item level display
                char ilvl_text[8];
                sprintf(ilvl_text, "ilvl%d", env->player.inventory[i].item_level);
                DrawText(ilvl_text, x + 5, y + 65, 10, (Color){255, 255, 255, 200});
            } else {
                DrawText("✧", x + 30, y + 25, 30, (Color){80, 80, 80, 150});
            }
            
            DrawRectangleLines(x, y, 80, 80, slot_border);
            DrawRectangleLines(x - 1, y - 1, 82, 82, (Color){slot_border.r + 30, slot_border.g + 30, slot_border.b + 30, 100});
        }
    }
}

void render_shop_tab(Rift* env) {
    // PROPER LAYOUT USING SAME CONSTANTS AS CHARACTER SCREEN
    // All positioning now uses the container hierarchy constants
    
    int start_y = 120;
    float time = GetTime();
    float pulse = sinf(time * 4.0f) * 0.3f + 0.7f;
    // Removed sparkle animation
    
    // Simple shop header
    DrawText("SHOP", CONTENT_AREA_X, start_y, 24, (Color){255, 215, 0, 255});
    
    
    // Define shop container bounds - using constants
    int container_x = CONTENT_AREA_X;
    int container_y = start_y + 50; 
    int container_width = CONTENT_AREA_WIDTH - 40; // Leave some internal padding
    int container_height = 400;
    
    // Enable scissor clipping to contain shop items
    BeginScissorMode(container_x, container_y, container_width, container_height);
    
    // Shop items in elegant vertical display cases with scrolling
    for (int i = 0; i < SHOP_ITEMS_COUNT; i++) {
        int x = container_x + 20; // Positioned within container with padding
        int y = container_y + 10 + i * 90 + env->town_interface.shop_scroll_offset;
        
        Color case_bg = {20, 25, 40, 240};
        Color case_border = {100, 120, 160, 255};
        Color item_glow;
        
        if (env->shop_items[i].available) {
            Color item_quality_color = GetQualityColor(env->shop_items[i].item_quality);
            item_glow = (Color){item_quality_color.r, item_quality_color.g, item_quality_color.b, (uint32_t)(100 * pulse)};
            
            // Epic selection highlight
            if (env->town_interface.selected_item_index == i) {
                DrawRectangle(x - 12, y - 12, 300, 104, (Color){255, 255, 0, (uint32_t)(120 * pulse)}); // Reduced width
                DrawRectangle(x - 8, y - 8, 316, 96, item_glow);
            }
            
            // Display case with gradient
            DrawRectangleGradientV(x, y, 300, 80, case_bg, (Color){case_bg.r/2, case_bg.g/2, case_bg.b/2, case_bg.a});
            DrawRectangleLines(x, y, 300, 80, case_border);
            DrawRectangleLines(x - 1, y - 1, 302, 82, (Color){case_border.r + 30, case_border.g + 30, case_border.b + 30, 150});
            
            // Item icon slot
            int icon_x = x + 10, icon_y = y + 10;
            DrawRectangleGradientV(icon_x, icon_y, 60, 60, (Color){40, 45, 60, 200}, (Color){20, 25, 30, 200});
            DrawRectangleLines(icon_x, icon_y, 60, 60, item_quality_color);
            DrawRectangleLines(icon_x - 1, icon_y - 1, 62, 62, item_quality_color);
            
            // Epic item visuals using shared drawing functions
            switch (env->shop_items[i].item_type) {
                case EQUIPMENT_WEAPON:
                    DrawEpicWeapon(icon_x, icon_y, 60, item_quality_color);
                    break;
                case EQUIPMENT_OFFHAND:
                    DrawEpicOffhand(icon_x, icon_y, 60, item_quality_color);
                    break;
                case EQUIPMENT_RING_LEFT:
                case EQUIPMENT_RING_RIGHT:
                    DrawEpicRing(icon_x, icon_y, 60, item_quality_color);
                    break;
                case EQUIPMENT_AMULET:
                    DrawEpicAmulet(icon_x, icon_y, 60, item_quality_color);
                    break;
                case EQUIPMENT_BOOTS:
                    DrawEpicBoots(icon_x, icon_y, 60, item_quality_color);
                    break;
                case EQUIPMENT_GLOVES:
                    DrawEpicGloves(icon_x, icon_y, 60, item_quality_color);
                    break;
                case EQUIPMENT_HELMET:
                    DrawEpicHelmet(icon_x, icon_y, 60, item_quality_color);
                    break;
                case EQUIPMENT_SHOULDERS:
                    DrawEpicShoulders(icon_x, icon_y, 60, item_quality_color);
                    break;
                case EQUIPMENT_ARMOR:
                    DrawEpicArmor(icon_x, icon_y, 60, item_quality_color);
                    break;
                case EQUIPMENT_BELT:
                    DrawEpicBelt(icon_x, icon_y, 60, item_quality_color);
                    break;
                case EQUIPMENT_PANTS:
                    DrawEpicPants(icon_x, icon_y, 60, item_quality_color);
                    break;
                case EQUIPMENT_BRACERS:
                    DrawEpicBracers(icon_x, icon_y, 60, item_quality_color);
                    break;
                default:
                    // Fallback for unknown item types
                    DrawText("?", icon_x + 20, icon_y + 15, 30, item_quality_color);
                    break;
            }
            
            // Item details with updated type names for all equipment
            char* type_names[] = {"None", "Shoulders", "Gloves", "Ring (L)", "Weapon", "Helmet", "Armor", "Belt", "Pants", "Boots", "Amulet", "Bracers", "Ring (R)", "Offhand", "Consumable"};
            char* quality_names[] = {"Common", "Rare", "Epic", "Legendary"};
            
            DrawText(type_names[env->shop_items[i].item_type], x + 80, y + 10, 16, item_quality_color);
            
            char quality_text[32];
            sprintf(quality_text, "%s Quality", quality_names[env->shop_items[i].item_quality]);
            DrawText(quality_text, x + 80, y + 30, 12, item_quality_color);
            
            char ilvl_text[16];
            sprintf(ilvl_text, "Item Level %d", env->shop_items[i].item_level);
            DrawText(ilvl_text, x + 80, y + 45, 12, (Color){180, 180, 180, 255});
            
            // Price with coin effect
            char price_text[16];
            sprintf(price_text, "%d Gold", env->shop_items[i].price);
            Color price_color = (env->player.gold >= env->shop_items[i].price) ? 
                (Color){0, 255, 100, 255} : (Color){255, 100, 100, 255};
            DrawText(price_text, x + 190, y + 25, 16, price_color);
            
            // Affordability indicator
            if (env->player.gold >= env->shop_items[i].price) {
                DrawText("✓ Can Buy", x + 190, y + 50, 12, (Color){0, 255, 100, 200});
            } else {
                DrawText("✗ Too Expensive", x + 190, y + 50, 12, (Color){255, 100, 100, 200});
            }
            
        } else {
            // SOLD item - grayed out
            DrawRectangleGradientV(x, y, 300, 80, (Color){30, 30, 30, 180}, (Color){15, 15, 15, 180});
            DrawRectangleLines(x, y, 300, 80, (Color){60, 60, 60, 255});
            DrawText("❌ SOLD OUT", x + 100, y + 30, 18, (Color){120, 120, 120, 255});
        }
    }
    
    // End scissor clipping
    EndScissorMode();
    
    // DIABLO-STYLE SHOP STATS COMPARISON - Using constants
    int stats_panel_y = container_y;
    int available_height = TOWN_CONTAINER_HEIGHT - (stats_panel_y - TOWN_CONTAINER_Y) - 20; // Leave 20px bottom margin
    int stats_panel_height = available_height;
    
    // Panel background with border using constants
    DrawRectangleGradientV(STATS_PANEL_X, stats_panel_y, STATS_PANEL_WIDTH, stats_panel_height, 
                          (Color){25, 20, 15, 240}, (Color){15, 10, 8, 240});
    DrawRectangleLines(STATS_PANEL_X, stats_panel_y, STATS_PANEL_WIDTH, stats_panel_height, 
                      (Color){150, 120, 80, 255});
    
    // Get currently selected shop item
    int selected_idx = env->town_interface.selected_item_index;
    if (selected_idx >= 0 && selected_idx < SHOP_ITEMS_COUNT && env->shop_items[selected_idx].available) {
        ShopItem* shop_item = &env->shop_items[selected_idx];
        
        // Shop item header
        Color quality_color = GetQualityColor(shop_item->item_quality);
        char* type_names[] = {"None", "Shoulders", "Gloves", "Ring (L)", "Weapon", "Helmet", "Armor", "Belt", "Pants", "Boots", "Amulet", "Bracers", "Ring (R)", "Offhand", "Consumable"};
        DrawText(type_names[shop_item->item_type], STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT, 14, quality_color);
        
        // Item level and quality
        char ilvl_text[32];
        sprintf(ilvl_text, "Item Level %d", shop_item->item_level);
        DrawText(ilvl_text, STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT + STATS_LINE_HEIGHT, 12, (Color){200, 200, 200, 255});
        
        char* quality_names[] = {"COMMON", "RARE", "EPIC", "LEGENDARY"};
        char* quality_name = (shop_item->item_quality < 4) ? quality_names[shop_item->item_quality] : "UNKNOWN";
        DrawText(quality_name, STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT + 2 * STATS_LINE_HEIGHT, 12, quality_color);
        
        // Price display
        char price_text[32];
        sprintf(price_text, "💰 %d Gold", shop_item->price);
        Color price_color = (env->player.gold >= shop_item->price) ? (Color){0, 255, 100, 255} : (Color){255, 100, 100, 255};
        DrawText(price_text, STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT + 3 * STATS_LINE_HEIGHT, 12, price_color);
        
        // Item visual in card
        int icon_x = STATS_PANEL_X + STATS_PANEL_WIDTH - 70;
        int icon_y = stats_panel_y + 15;
        int icon_size = 50;
        DrawRectangleGradientV(icon_x, icon_y, icon_size, icon_size, 
                              (Color){40, 35, 30, 200}, (Color){20, 15, 10, 200});
        DrawRectangleLines(icon_x, icon_y, icon_size, icon_size, quality_color);
        
        // Draw shop item visual using shared functions
        switch (shop_item->item_type) {
            case EQUIPMENT_WEAPON: DrawEpicWeapon(icon_x, icon_y, icon_size, quality_color); break;
            case EQUIPMENT_OFFHAND: DrawEpicOffhand(icon_x, icon_y, icon_size, quality_color); break;
            case EQUIPMENT_RING_LEFT:
            case EQUIPMENT_RING_RIGHT: DrawEpicRing(icon_x, icon_y, icon_size, quality_color); break;
            case EQUIPMENT_AMULET: DrawEpicAmulet(icon_x, icon_y, icon_size, quality_color); break;
            case EQUIPMENT_BOOTS: DrawEpicBoots(icon_x, icon_y, icon_size, quality_color); break;
            case EQUIPMENT_GLOVES: DrawEpicGloves(icon_x, icon_y, icon_size, quality_color); break;
            case EQUIPMENT_HELMET: DrawEpicHelmet(icon_x, icon_y, icon_size, quality_color); break;
            case EQUIPMENT_SHOULDERS: DrawEpicShoulders(icon_x, icon_y, icon_size, quality_color); break;
            case EQUIPMENT_ARMOR: DrawEpicArmor(icon_x, icon_y, icon_size, quality_color); break;
            case EQUIPMENT_BELT: DrawEpicBelt(icon_x, icon_y, icon_size, quality_color); break;
            case EQUIPMENT_PANTS: DrawEpicPants(icon_x, icon_y, icon_size, quality_color); break;
            case EQUIPMENT_BRACERS: DrawEpicBracers(icon_x, icon_y, icon_size, quality_color); break;
        }
        
        // Stats comparison section
        DrawText("STATS:", STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT + 3 * STATS_LINE_HEIGHT + STATS_SECTION_SPACING, 12, (Color){255, 215, 0, 255});
        int stat_y = stats_panel_y + 130;
        char* stat_names[] = {"Strength", "Dexterity", "Intelligence", "Vitality"};
        Color stat_colors[] = {
            {160, 160, 160, 255}, // STR - gray
            {160, 160, 160, 255}, // DEX - gray  
            {160, 160, 160, 255}, // INT - gray
            {160, 160, 160, 255}  // VIT - gray
        };
        
        // ITEM BONUSES - Show what the item itself provides
        DrawText("ITEM BONUSES:", STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stat_y - 5, 12, (Color){255, 215, 0, 255});
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
        
        // Get currently equipped item stats for comparison
        uint32_t equipped_stat_bonuses[4] = {0, 0, 0, 0};
        bool has_equipped = false;
        
        // Map shop item type to equipped slot
        switch (shop_item->item_type) {
            case EQUIPMENT_WEAPON:
                if (env->equipment.weapon_type != EQUIPMENT_NONE) {
                    has_equipped = true;
                    for (int i = 0; i < 4; i++) equipped_stat_bonuses[i] = env->equipment.weapon_stat_bonuses[i];
                }
                break;
            case EQUIPMENT_OFFHAND:
                if (env->equipment.offhand_type != EQUIPMENT_NONE) {
                    has_equipped = true;
                    for (int i = 0; i < 4; i++) equipped_stat_bonuses[i] = env->equipment.offhand_stat_bonuses[i];
                }
                break;
            case EQUIPMENT_HELMET:
                if (env->equipment.helmet_type != EQUIPMENT_NONE) {
                    has_equipped = true;
                    for (int i = 0; i < 4; i++) equipped_stat_bonuses[i] = env->equipment.helmet_stat_bonuses[i];
                }
                break;
            // Add more equipment types as needed...
        }
        
        // NET EFFECT ON HERO STATS - Show total change to hero stats
        DrawText("NET EFFECT ON HERO:", STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stat_y, 12, (Color){100, 255, 255, 255});
        stat_y += 20;
        
        bool has_net_changes = false;
        for (int i = 0; i < 4; i++) {
            int stat_diff = shop_item->stat_bonuses[i] - equipped_stat_bonuses[i];
            if (stat_diff != 0) {
                char net_effect_text[64];
                Color effect_color;
                if (stat_diff > 0) {
                    sprintf(net_effect_text, "%s: +%d (↑ GAIN)", stat_names[i], stat_diff);
                    effect_color = (Color){100, 200, 100, 255}; // Green for stat gain
                } else {
                    sprintf(net_effect_text, "%s: %d (↓ LOSS)", stat_names[i], stat_diff);
                    effect_color = (Color){200, 100, 100, 255}; // Red for stat loss
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
        
        // Show currently equipped item for reference
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
        // No valid selection
        DrawText("SELECT ITEM", STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT, 14, (Color){100, 100, 100, 255});
        DrawText("Choose an item to view stats", STATS_PANEL_X + STATS_CONTENT_MARGIN_LEFT, stats_panel_y + STATS_CONTENT_MARGIN_LEFT + STATS_LINE_HEIGHT, 11, (Color){150, 150, 150, 255});
    }
    
    // Removed navigation instructions - interface should be intuitive
}


void c_render(Rift* env) {
    if (!env->client) {
        env->client = make_client(env);
    }
    
    if (IsKeyDown(KEY_ESCAPE)) exit(0);
    
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
