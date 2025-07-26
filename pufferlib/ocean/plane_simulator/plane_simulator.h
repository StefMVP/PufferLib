#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include "raylib.h"

// ============================================================================
// CONSTANTS
// ============================================================================

#define RUNWAY_LENGTH 3000.0f
#define RUNWAY_WIDTH 150.0f
#define MAX_EPISODE_LENGTH 30
#define PHYSICS_DT 0.016667f

#define OBS_SIZE 22
#define ACTION_SIZE 4

#define TAKEOFF_ALTITUDE 200.0f
#define TAKEOFF_SPEED 65.0f
#define STALL_AOA 15.0f

#define AIR_DENSITY 1.225f
#define GRAVITY 9.81f

// ============================================================================
// CONFIGURATION
// ============================================================================

typedef enum {
    MODE_TAKEOFF = 0,
    MODE_CLIMB = 1,
    MODE_CRUISE = 2,
    MODE_APPROACH = 3,
    MODE_LANDING = 4
} FlightMode;

typedef struct {
    float mass;
    float wing_area;
    float max_thrust;
    float drag_coeff;
} AircraftConfig;

static const AircraftConfig DEFAULT_AIRCRAFT = {
    .mass = 1100.0f,
    .wing_area = 16.0f,
    .max_thrust = 10000.0f,
    .drag_coeff = 0.025f
};

// ============================================================================
// VECTOR_MATH
// ============================================================================

typedef struct {
    float x, y, z;
} Vec3;

static inline Vec3 vec3_add(Vec3 a, Vec3 b) {
    return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

static inline Vec3 vec3_scale(Vec3 v, float s) {
    return (Vec3){v.x * s, v.y * s, v.z * s};
}

static inline float vec3_magnitude(Vec3 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

static inline Vec3 vec3_normalize(Vec3 v) {
    float mag = vec3_magnitude(v);
    return mag > 0.0f ? vec3_scale(v, 1.0f / mag) : (Vec3){0, 0, 0};
}

// ============================================================================
// AIRCRAFT_PHYSICS
// ============================================================================

typedef struct {
    Vec3 position;
    Vec3 velocity;
    Vec3 acceleration;
    
    float pitch, roll, yaw;
    Vec3 angular_vel;
    
    float throttle;
    float elevator;
    float rudder;
    float aileron;
    
    AircraftConfig config;
} Aircraft;

// ============================================================================
// LOGGING_SYSTEM
// ============================================================================

typedef struct {
    float episode_return;
    float episode_length;
    float successful_takeoffs;
    float crashes;
    float runway_excursions;
    float max_altitude;
    float n;
} Log;

// ============================================================================
// MAIN_ENVIRONMENT
// ============================================================================

typedef struct {
    Log log;
    float* observations;
    float* actions;
    float* rewards;
    unsigned char* terminals;
    
    Aircraft aircraft;
    FlightMode mode;
    
    float episode_time;
    float episode_return;
    int tick;
    
    float wind_speed;
    float wind_direction;
    
    int human_control_mode;
    
} PlaneSimulator;

// ============================================================================
// PHYSICS_SIMULATION
// ============================================================================

static inline float compute_lift_coefficient(float angle_of_attack) {
    if (angle_of_attack > STALL_AOA) return 0.8f;
    if (angle_of_attack < -STALL_AOA) return -0.8f;
    return 0.1f * angle_of_attack;
}

static inline float get_angle_of_attack(Aircraft* plane) {
    float airspeed = vec3_magnitude(plane->velocity);
    if (airspeed < 1.0f) return 0.0f;
    
    Vec3 vel_norm = vec3_normalize(plane->velocity);
    float pitch_rad = plane->pitch * PI / 180.0f;
    float velocity_angle = atan2f(vel_norm.z, vel_norm.x);
    
    return (pitch_rad - velocity_angle) * 180.0f / PI;
}

static Vec3 compute_thrust(Aircraft* plane) {
    float thrust_magnitude = plane->throttle * plane->config.max_thrust;
    float pitch_rad = plane->pitch * PI / 180.0f;
    float yaw_rad = plane->yaw * PI / 180.0f;
    
    return (Vec3){
        thrust_magnitude * cosf(pitch_rad) * cosf(yaw_rad),
        thrust_magnitude * cosf(pitch_rad) * sinf(yaw_rad),
        thrust_magnitude * sinf(pitch_rad)
    };
}

static Vec3 compute_drag(Aircraft* plane) {
    float speed = vec3_magnitude(plane->velocity);
    if (speed < 0.1f) return (Vec3){0, 0, 0};
    
    float drag_force = 0.5f * AIR_DENSITY * plane->config.drag_coeff * 
                      plane->config.wing_area * speed * speed;
    
    Vec3 vel_norm = vec3_normalize(plane->velocity);
    return vec3_scale(vel_norm, -drag_force);
}

static Vec3 compute_lift(Aircraft* plane) {
    float airspeed = vec3_magnitude(plane->velocity);
    if (airspeed < 1.0f) return (Vec3){0, 0, 0};
    
    float angle_of_attack = get_angle_of_attack(plane);
    float lift_coeff = compute_lift_coefficient(angle_of_attack);
    float lift_force = 0.5f * AIR_DENSITY * airspeed * airspeed * 
                      plane->config.wing_area * lift_coeff;
    
    float roll_rad = plane->roll * PI / 180.0f;
    return (Vec3){
        0,
        -lift_force * sinf(roll_rad),
        lift_force * cosf(roll_rad)
    };
}

static Vec3 compute_weight(Aircraft* plane) {
    return (Vec3){0, 0, -plane->config.mass * GRAVITY};
}

void update_physics(Aircraft* plane, float dt) {
    Vec3 thrust = compute_thrust(plane);
    Vec3 drag = compute_drag(plane);
    Vec3 lift = compute_lift(plane);
    Vec3 weight = compute_weight(plane);
    
    Vec3 total_force = vec3_add(vec3_add(thrust, drag), vec3_add(lift, weight));
    
    plane->acceleration = vec3_scale(total_force, 1.0f / plane->config.mass);
    
    plane->velocity = vec3_add(plane->velocity, vec3_scale(plane->acceleration, dt));
    plane->position = vec3_add(plane->position, vec3_scale(plane->velocity, dt));
    
    plane->pitch += plane->elevator * 45.0f * dt;
    plane->roll += plane->aileron * 60.0f * dt;
    plane->yaw += plane->rudder * 35.0f * dt;
    
    plane->pitch = fmaxf(-30.0f, fminf(30.0f, plane->pitch));
    plane->roll = fmaxf(-60.0f, fminf(60.0f, plane->roll));
    
    if (plane->position.z <= 0.0f) {
        plane->position.z = 0.0f;
        if (plane->velocity.z < 0.0f) {
            plane->velocity.z = 0.0f;
        }
        
        float friction = 0.005f;
        plane->velocity.x *= (1.0f - friction);
        plane->velocity.y *= (1.0f - friction);
    }
}

// ============================================================================
// OBSERVATION_SYSTEM
// ============================================================================

void compute_observations(PlaneSimulator* env) {
    Aircraft* plane = &env->aircraft;
    int obs_idx = 0;
    
    env->observations[obs_idx++] = plane->position.x / RUNWAY_LENGTH;
    env->observations[obs_idx++] = plane->position.y / RUNWAY_WIDTH;
    env->observations[obs_idx++] = plane->position.z / TAKEOFF_ALTITUDE;
    
    env->observations[obs_idx++] = plane->velocity.x / 100.0f;
    env->observations[obs_idx++] = plane->velocity.y / 100.0f;
    env->observations[obs_idx++] = plane->velocity.z / 50.0f;
    
    env->observations[obs_idx++] = plane->pitch / 30.0f;
    env->observations[obs_idx++] = plane->roll / 60.0f;
    env->observations[obs_idx++] = plane->yaw / 180.0f;
    
    float airspeed = vec3_magnitude(plane->velocity);
    env->observations[obs_idx++] = airspeed / 100.0f;
    env->observations[obs_idx++] = plane->velocity.z / 20.0f;
    
    float ground_speed = sqrtf(plane->velocity.x * plane->velocity.x + 
                              plane->velocity.y * plane->velocity.y);
    env->observations[obs_idx++] = ground_speed / 100.0f;
    
    float angle_of_attack = get_angle_of_attack(plane);
    env->observations[obs_idx++] = angle_of_attack / 20.0f;
    env->observations[obs_idx++] = plane->throttle;
    
    float distance_from_centerline = fabsf(plane->position.y);
    env->observations[obs_idx++] = distance_from_centerline / RUNWAY_WIDTH;
    
    float runway_remaining = RUNWAY_LENGTH - plane->position.x;
    env->observations[obs_idx++] = runway_remaining / RUNWAY_LENGTH;
    
    float runway_heading = atan2f(plane->velocity.y, plane->velocity.x) * 180.0f / PI;
    env->observations[obs_idx++] = runway_heading / 180.0f;
    
    float ground_contact = (plane->position.z <= 0.1f) ? 1.0f : 0.0f;
    env->observations[obs_idx++] = ground_contact;
    
    env->observations[obs_idx++] = env->wind_speed / 20.0f;
    env->observations[obs_idx++] = env->wind_direction / 180.0f;
    
    env->observations[obs_idx++] = env->episode_time / MAX_EPISODE_LENGTH;
    
    // TODO: Add AoA and sideslip to observations
    // TODO: Add control surface feedback to observations
}

// ============================================================================
// LOGGING_SYSTEM
// ============================================================================

static void add_log(PlaneSimulator* env) {
    env->log.episode_return += env->episode_return;
    env->log.episode_length += env->episode_time;
    env->log.max_altitude += env->aircraft.position.z;
    env->log.n += 1;
}

// ============================================================================
// RESET_SYSTEM
// ============================================================================

void c_reset(PlaneSimulator* env) {
    Aircraft* plane = &env->aircraft;
    
    plane->position = (Vec3){0, 0, 0};
    plane->velocity = (Vec3){0, 0, 0};
    plane->acceleration = (Vec3){0, 0, 0};
    
    plane->pitch = 0.0f;
    plane->roll = 0.0f;
    plane->yaw = 0.0f;
    plane->angular_vel = (Vec3){0, 0, 0};
    
    plane->throttle = 0.0f;
    plane->elevator = 0.0f;
    plane->rudder = 0.0f;
    plane->aileron = 0.0f;
    
    plane->config = DEFAULT_AIRCRAFT;
    
    env->episode_time = 0.0f;
    env->episode_return = 0.0f;
    env->tick = 0;
    
    env->wind_speed = 0.0f;
    env->wind_direction = 0.0f;
    env->human_control_mode = 0;
    
    compute_observations(env);
}

// ============================================================================
// REWARD_SYSTEM
// ============================================================================

static float compute_step_reward(PlaneSimulator* env) {
    Aircraft* plane = &env->aircraft;
    float reward = 0.0f;
    
    float airspeed = vec3_magnitude(plane->velocity);
    if (airspeed >= TAKEOFF_SPEED * 0.8f) {
        reward += 5.0f;
    }
    
    if (plane->position.z > 0.0f) {
        reward += plane->position.z * 0.5f;
    }
    
    float centerline_error = fabsf(plane->position.y);
    if (centerline_error < 10.0f) {
        reward += 2.0f;
    }
    
    return reward;
}

// ============================================================================
// GAME_STEP
// ============================================================================

void c_step(PlaneSimulator* env) {
    env->terminals[0] = 0;
    env->rewards[0] = 0.0f;
    
    Aircraft* plane = &env->aircraft;
    
    if (!env->human_control_mode) {
        plane->throttle = fmaxf(0.0f, fminf(1.0f, env->actions[0]));
        plane->elevator = fmaxf(-1.0f, fminf(1.0f, env->actions[1]));
        plane->rudder = fmaxf(-1.0f, fminf(1.0f, env->actions[2]));
        plane->aileron = fmaxf(-1.0f, fminf(1.0f, env->actions[3]));
    }
    
    update_physics(plane, PHYSICS_DT);
    
    float step_reward = compute_step_reward(env);
    env->rewards[0] += step_reward;
    env->episode_return += step_reward;
    
    env->episode_time += PHYSICS_DT;
    env->tick++;
    
    float airspeed = vec3_magnitude(plane->velocity);
    float angle_of_attack = get_angle_of_attack(plane);
    
    if (plane->position.z >= TAKEOFF_ALTITUDE && plane->velocity.z > 0.0f) {
        env->rewards[0] += 100.0f;
        env->episode_return += 100.0f;
        env->log.successful_takeoffs += 1;
        env->terminals[0] = 1;
        add_log(env);
        c_reset(env);
        return;
    }
    
    if (fabsf(plane->position.y) > RUNWAY_WIDTH * 0.5f) {
        env->rewards[0] -= 100.0f;
        env->episode_return -= 100.0f;
        env->log.runway_excursions += 1;
        env->terminals[0] = 1;
        add_log(env);
        c_reset(env);
        return;
    }
    
    if (plane->position.x > RUNWAY_LENGTH) {
        env->rewards[0] -= 300.0f;
        env->episode_return -= 300.0f;
        env->terminals[0] = 1;
        add_log(env);
        c_reset(env);
        return;
    }
    
    if (fabsf(angle_of_attack) > STALL_AOA && airspeed < 30.0f) {
        env->rewards[0] -= 200.0f;
        env->episode_return -= 200.0f;
        env->log.crashes += 1;
        env->terminals[0] = 1;
        add_log(env);
        c_reset(env);
        return;
    }
    
    if (env->episode_time >= MAX_EPISODE_LENGTH) {
        env->terminals[0] = 1;
        add_log(env);
        c_reset(env);
        return;
    }
    
    compute_observations(env);
}

// ============================================================================
// RENDERING_SYSTEM
// ============================================================================

void draw_circular_gauge(float center_x, float center_y, float radius, float min_val, float max_val, 
                        float current_val, const char* label, Color gauge_color) {
    DrawCircleLines(center_x, center_y, radius, WHITE);
    DrawCircleLines(center_x, center_y, radius - 2, GRAY);
    
    DrawText(label, center_x - MeasureText(label, 12) / 2, center_y + radius - 25, 12, WHITE);
    
    float angle_range = 240.0f;
    float start_angle = 150.0f;
    float value_ratio = (current_val - min_val) / (max_val - min_val);
    value_ratio = fmaxf(0.0f, fminf(1.0f, value_ratio));
    float needle_angle = start_angle + value_ratio * angle_range;
    
    float needle_length = radius * 0.8f;
    float needle_x = center_x + cosf(needle_angle * PI / 180.0f) * needle_length;
    float needle_y = center_y + sinf(needle_angle * PI / 180.0f) * needle_length;
    
    DrawLineEx((Vector2){center_x, center_y}, (Vector2){needle_x, needle_y}, 3, gauge_color);
    DrawCircle(center_x, center_y, 5, gauge_color);
    
    DrawText(TextFormat("%.0f", current_val), center_x - 15, center_y + 10, 14, WHITE);
}

void draw_attitude_indicator(float center_x, float center_y, float radius, float pitch, float roll) {
    DrawCircleLines(center_x, center_y, radius, WHITE);
    
    float horizon_y = center_y - pitch * 2.0f;
    
    Color sky_color = (Color){135, 206, 235, 255};
    Color ground_color = (Color){139, 69, 19, 255};
    
    Rectangle sky_rect = {center_x - radius, center_y - radius, radius * 2, horizon_y - (center_y - radius)};
    Rectangle ground_rect = {center_x - radius, horizon_y, radius * 2, (center_y + radius) - horizon_y};
    
    DrawRectangleRec(sky_rect, sky_color);
    DrawRectangleRec(ground_rect, ground_color);
    
    DrawLineEx((Vector2){center_x - radius * 0.8f, horizon_y}, 
               (Vector2){center_x + radius * 0.8f, horizon_y}, 3, WHITE);
    
    float wing_width = radius * 0.6f;
    DrawLineEx((Vector2){center_x - wing_width, center_y}, 
               (Vector2){center_x + wing_width, center_y}, 4, YELLOW);
    DrawLineEx((Vector2){center_x, center_y - 10}, (Vector2){center_x, center_y + 10}, 4, YELLOW);
    
    DrawText("ATT", center_x - 15, center_y + radius - 25, 12, WHITE);
}

void draw_vertical_tape(float x, float y, float width, float height, float min_val, float max_val, 
                       float current_val, const char* label, Color tape_color) {
    DrawRectangle(x, y, width, height, (Color){30, 30, 30, 200});
    DrawRectangleLines(x, y, width, height, WHITE);
    
    float value_ratio = (current_val - min_val) / (max_val - min_val);
    value_ratio = fmaxf(0.0f, fminf(1.0f, value_ratio));
    float indicator_y = y + height - (value_ratio * height);
    
    DrawRectangle(x, indicator_y - 3, width, 6, tape_color);
    DrawText(TextFormat("%.0f", current_val), x + width + 5, indicator_y - 8, 14, WHITE);
    
    DrawText(label, x, y - 20, 12, WHITE);
}

void draw_cockpit_view(PlaneSimulator* env) {
    Aircraft* plane = &env->aircraft;
    
    int view_width = 700;
    int view_height = 350;
    int view_x = 250;
    int view_y = 30;
    
    Color sky_color = (Color){135, 206, 250, 255};
    Color ground_color = (Color){34, 139, 34, 255};
    
    float horizon_offset = plane->pitch * 3.0f;
    float horizon_y = view_y + view_height / 2 + horizon_offset - plane->position.z * 0.5f;
    
    DrawRectangle(view_x, view_y, view_width, horizon_y - view_y, sky_color);
    DrawRectangle(view_x, horizon_y, view_width, view_y + view_height - horizon_y, ground_color);
    
    DrawRectangleLines(view_x, view_y, view_width, view_height, WHITE);
    
    if (plane->position.z <= 5.0f) {
        float runway_width_pixels = 300.0f;
        float runway_distance_factor = 1.0f - (plane->position.x / RUNWAY_LENGTH);
        runway_distance_factor = fmaxf(0.1f, runway_distance_factor);
        
        float perspective_width = runway_width_pixels * runway_distance_factor;
        float runway_center_x = view_x + view_width / 2 - plane->position.y * 2.0f;
        
        float runway_bottom = view_y + view_height - 20;
        float runway_top = view_y + view_height / 2 + 20;
        
        Vector2 runway_corners[4] = {
            {runway_center_x - perspective_width / 2, runway_bottom},
            {runway_center_x + perspective_width / 2, runway_bottom},
            {runway_center_x + 80, runway_top},
            {runway_center_x - 80, runway_top}
        };
        
        for (int i = 0; i < 4; i++) {
            DrawLineEx(runway_corners[i], runway_corners[(i + 1) % 4], 3, (Color){50, 50, 50, 255});
        }
        
        for (int i = 0; i < 8; i++) {
            float t = i / 7.0f;
            float line_y = runway_bottom + t * (runway_top - runway_bottom);
            float line_width = perspective_width * (1.0f - t * 0.8f);
            
            if (i % 2 == 0) {
                DrawLineEx((Vector2){runway_center_x - line_width / 8, line_y}, 
                          (Vector2){runway_center_x + line_width / 8, line_y}, 2, WHITE);
            }
        }
        
        DrawLineEx((Vector2){runway_center_x, runway_bottom}, 
                  (Vector2){runway_center_x, runway_top}, 2, YELLOW);
    }
    
    DrawLineEx((Vector2){view_x + view_width / 2 - 50, horizon_y}, 
               (Vector2){view_x + view_width / 2 + 50, horizon_y}, 2, WHITE);
    
    float crosshair_x = view_x + view_width / 2;
    float crosshair_y = view_y + view_height / 2;
    DrawLineEx((Vector2){crosshair_x - 20, crosshair_y}, (Vector2){crosshair_x + 20, crosshair_y}, 2, RED);
    DrawLineEx((Vector2){crosshair_x, crosshair_y - 20}, (Vector2){crosshair_x, crosshair_y + 20}, 2, RED);
    
    const char* flight_status = plane->position.z > 10.0f ? "AIRBORNE" : "GROUND ROLL";
    Color status_color = plane->position.z > 10.0f ? GREEN : ORANGE;
    DrawText(flight_status, view_x + 10, view_y + 10, 18, status_color);
}

void draw_mode_indicator(PlaneSimulator* env) {
    const char* mode_text = env->human_control_mode ? "HUMAN" : "AI";
    Color mode_color = env->human_control_mode ? GREEN : SKYBLUE;
    int text_width = MeasureText(mode_text, 24);
    
    DrawRectangle(1200 - text_width - 20, 10, text_width + 15, 35, (Color){0, 0, 0, 180});
    DrawRectangleLines(1200 - text_width - 20, 10, text_width + 15, 35, mode_color);
    DrawText(mode_text, 1200 - text_width - 12, 20, 24, mode_color);
}

void draw_organized_hud(PlaneSimulator* env) {
    Aircraft* plane = &env->aircraft;
    int hud_y = 680;
    int hud_height = 120;
    
    DrawRectangle(0, hud_y, 1200, hud_height, (Color){25, 25, 35, 240});
    DrawLine(0, hud_y, 1200, hud_y, (Color){100, 100, 120, 255});
    
    DrawText("FLIGHT SIMULATOR CONTROLS", 20, hud_y + 8, 18, YELLOW);
    
    if (env->human_control_mode) {
        DrawText("THROTTLE: W/S", 20, hud_y + 35, 14, WHITE);
        DrawText("ELEVATOR: UP/DOWN", 20, hud_y + 52, 14, WHITE);
        DrawText("RUDDER: LEFT/RIGHT", 20, hud_y + 69, 14, WHITE);
        DrawText("AILERON: A/D", 20, hud_y + 86, 14, WHITE);
        
        DrawText("SHIFT: Toggle AI/Human", 250, hud_y + 35, 14, GREEN);
        DrawText("SPACE: Reset Episode", 250, hud_y + 52, 14, ORANGE);
        DrawText("ESC: Exit", 250, hud_y + 69, 14, RED);
    } else {
        DrawText("AI CONTROL ACTIVE", 20, hud_y + 45, 16, SKYBLUE);
        DrawText("SHIFT: Enable Human Control", 20, hud_y + 65, 14, GREEN);
        DrawText("SPACE: Reset  |  ESC: Exit", 20, hud_y + 85, 14, WHITE);
    }
    
    int controls_x = 450;
    DrawText("CONTROL POSITIONS", controls_x, hud_y + 8, 16, YELLOW);
    DrawText(TextFormat("PWR: %3.0f%%", plane->throttle * 100), controls_x, hud_y + 32, 14, 
             plane->throttle > 0.8f ? GREEN : (plane->throttle > 0.5f ? YELLOW : WHITE));
    DrawText(TextFormat("ELEV: %+.2f", plane->elevator), controls_x, hud_y + 49, 14,
             plane->elevator > 0.1f ? GREEN : (plane->elevator < -0.1f ? RED : WHITE));
    DrawText(TextFormat("RUD:  %+.2f", plane->rudder), controls_x, hud_y + 66, 14,
             fabsf(plane->rudder) > 0.1f ? ORANGE : WHITE);
    DrawText(TextFormat("AIL:  %+.2f", plane->aileron), controls_x, hud_y + 83, 14,
             fabsf(plane->aileron) > 0.1f ? PURPLE : WHITE);
    
    int status_x = 700;
    float airspeed = vec3_magnitude(plane->velocity);
    DrawText("FLIGHT STATUS", status_x, hud_y + 8, 16, YELLOW);
    
    Color speed_color = airspeed >= TAKEOFF_SPEED ? GREEN : (airspeed >= TAKEOFF_SPEED * 0.8f ? YELLOW : WHITE);
    DrawText(TextFormat("SPEED: %3.0f kts", airspeed), status_x, hud_y + 32, 14, speed_color);
    
    Color alt_color = plane->position.z >= TAKEOFF_ALTITUDE ? GREEN : (plane->position.z > 50.0f ? YELLOW : WHITE);
    DrawText(TextFormat("ALT:   %3.0f ft", plane->position.z), status_x, hud_y + 49, 14, alt_color);
    
    DrawText(TextFormat("DIST:  %4.0f ft", plane->position.x), status_x, hud_y + 66, 14, WHITE);
    DrawText(TextFormat("TIME:  %4.1f s", env->episode_time), status_x, hud_y + 83, 14, WHITE);
    
    int perf_x = 950;
    DrawText("PERFORMANCE", perf_x, hud_y + 8, 16, YELLOW);
    float angle_of_attack = get_angle_of_attack(plane);
    Color aoa_color = fabsf(angle_of_attack) > 12.0f ? RED : (fabsf(angle_of_attack) > 8.0f ? YELLOW : WHITE);
    DrawText(TextFormat("AOA: %+4.1f°", angle_of_attack), perf_x, hud_y + 32, 14, aoa_color);
    DrawText(TextFormat("PITCH: %+3.0f°", plane->pitch), perf_x, hud_y + 49, 14, WHITE);
    DrawText(TextFormat("ROLL:  %+3.0f°", plane->roll), perf_x, hud_y + 66, 14, WHITE);
    
    float runway_remaining = (RUNWAY_LENGTH - plane->position.x) / RUNWAY_LENGTH * 100.0f;
    runway_remaining = fmaxf(0.0f, runway_remaining);
    Color runway_color = runway_remaining > 50.0f ? GREEN : (runway_remaining > 25.0f ? YELLOW : RED);
    DrawText(TextFormat("RWY: %3.0f%%", runway_remaining), perf_x, hud_y + 83, 14, runway_color);
}

void handle_input_and_control(PlaneSimulator* env) {
    Aircraft* plane = &env->aircraft;
    
    if (IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT)) {
        env->human_control_mode = !env->human_control_mode;
        
        if (env->human_control_mode) {
            env->actions[0] = plane->throttle;
            env->actions[1] = plane->elevator;
            env->actions[2] = plane->rudder;
            env->actions[3] = plane->aileron;
        } else {
            plane->throttle = fmaxf(0.0f, fminf(1.0f, env->actions[0]));
            plane->elevator = fmaxf(-1.0f, fminf(1.0f, env->actions[1]));
            plane->rudder = fmaxf(-1.0f, fminf(1.0f, env->actions[2]));
            plane->aileron = fmaxf(-1.0f, fminf(1.0f, env->actions[3]));
        }
    }
    
    if (IsKeyPressed(KEY_SPACE)) {
        c_reset(env);
    }
    
    if (env->human_control_mode) {
        if (IsKeyDown(KEY_W)) plane->throttle = fminf(1.0f, plane->throttle + 0.015f);
        if (IsKeyDown(KEY_S)) plane->throttle = fmaxf(0.0f, plane->throttle - 0.015f);
        
        if (IsKeyDown(KEY_UP)) plane->elevator = fminf(1.0f, plane->elevator + 0.02f);
        if (IsKeyDown(KEY_DOWN)) plane->elevator = fmaxf(-1.0f, plane->elevator - 0.02f);
        
        if (IsKeyDown(KEY_LEFT)) plane->rudder = fmaxf(-1.0f, plane->rudder - 0.02f);
        if (IsKeyDown(KEY_RIGHT)) plane->rudder = fminf(1.0f, plane->rudder + 0.02f);
        
        if (IsKeyDown(KEY_A)) plane->aileron = fmaxf(-1.0f, plane->aileron - 0.02f);
        if (IsKeyDown(KEY_D)) plane->aileron = fminf(1.0f, plane->aileron + 0.02f);
        
        if (!IsKeyDown(KEY_UP) && !IsKeyDown(KEY_DOWN)) {
            plane->elevator *= 0.92f;
        }
        if (!IsKeyDown(KEY_LEFT) && !IsKeyDown(KEY_RIGHT)) {
            plane->rudder *= 0.92f;
        }
        if (!IsKeyDown(KEY_A) && !IsKeyDown(KEY_D)) {
            plane->aileron *= 0.92f;
        }
        
        env->actions[0] = plane->throttle;
        env->actions[1] = plane->elevator;
        env->actions[2] = plane->rudder;
        env->actions[3] = plane->aileron;
    }
}

void c_render(PlaneSimulator* env) {
    if (!IsWindowReady()) {
        InitWindow(1200, 800, "PufferLib Flight Simulator - Takeoff Training");
        SetTargetFPS(60);
    }
    
    if (IsKeyDown(KEY_ESCAPE)) {
        exit(0);
    }
    
    BeginDrawing();
    ClearBackground((Color){15, 25, 35, 255});
    
    Aircraft* plane = &env->aircraft;
    float airspeed = vec3_magnitude(plane->velocity);
    float angle_of_attack = get_angle_of_attack(plane);
    
    draw_cockpit_view(env);
    draw_mode_indicator(env);
    
    draw_circular_gauge(80, 450, 55, 0, 150, airspeed, "AIRSPEED", GREEN);
    draw_circular_gauge(180, 450, 55, 0, 500, plane->position.z, "ALTITUDE", BLUE);
    draw_attitude_indicator(80, 580, 50, plane->pitch, plane->roll);
    draw_circular_gauge(180, 580, 50, -20, 20, angle_of_attack, "AOA", RED);
    
    draw_vertical_tape(1000, 120, 35, 180, 0, 100, plane->throttle * 100, "PWR", GREEN);
    draw_vertical_tape(1045, 120, 35, 180, -30, 30, plane->elevator * 30, "ELEV", BLUE);
    draw_vertical_tape(1090, 120, 35, 180, -30, 30, plane->rudder * 30, "RUD", YELLOW);
    draw_vertical_tape(1135, 120, 35, 180, -30, 30, plane->aileron * 30, "AIL", PURPLE);
    
    handle_input_and_control(env);
    draw_organized_hud(env);
    
    if (env->terminals[0]) {
        const char* msg = "";
        Color msg_color = RED;
        if (plane->position.z >= TAKEOFF_ALTITUDE) {
            msg = "✈ TAKEOFF SUCCESSFUL!";
            msg_color = GREEN;
        } else if (fabsf(plane->position.y) > RUNWAY_WIDTH * 0.5f) {
            msg = "⚠ RUNWAY EXCURSION!";
        } else if (plane->position.x > RUNWAY_LENGTH) {
            msg = "⚠ RUNWAY OVERRUN!";
        } else if (fabsf(angle_of_attack) > STALL_AOA) {
            msg = "⚠ AIRCRAFT STALL!";
        } else {
            msg = "EPISODE COMPLETE";
        }
        
        int msg_width = MeasureText(msg, 28);
        DrawRectangle(600 - msg_width/2 - 25, 400, msg_width + 50, 60, (Color){0, 0, 0, 220});
        DrawRectangleLines(600 - msg_width/2 - 25, 400, msg_width + 50, 60, msg_color);
        DrawText(msg, 600 - msg_width/2, 420, 28, msg_color);
        
        const char* continue_msg = "Press SPACE to reset";
        int continue_width = MeasureText(continue_msg, 16);
        DrawText(continue_msg, 600 - continue_width/2, 440, 16, WHITE);
    }
    
    EndDrawing();
}

void c_close(PlaneSimulator* env) {
    if (IsWindowReady()) {
        CloseWindow();
    }
}