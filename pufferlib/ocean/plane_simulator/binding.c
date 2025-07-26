#include "plane_simulator.h"

#define Env PlaneSimulator
#include "../env_binding.h"

static int my_init(Env* env, PyObject* args, PyObject* kwargs) {
    int flight_mode = unpack(kwargs, "flight_mode");
    env->mode = (FlightMode)flight_mode;
    env->wind_speed = unpack(kwargs, "wind_speed");
    env->wind_direction = unpack(kwargs, "wind_direction");
    memset(&env->log, 0, sizeof(Log));
    c_reset(env);
    return 0;
}

static int my_log(PyObject* dict, Log* log) {
    assign_to_dict(dict, "episode_return", log->episode_return);
    assign_to_dict(dict, "episode_length", log->episode_length);
    assign_to_dict(dict, "successful_takeoffs", log->successful_takeoffs);
    assign_to_dict(dict, "crashes", log->crashes);
    assign_to_dict(dict, "runway_excursions", log->runway_excursions);
    assign_to_dict(dict, "max_altitude", log->max_altitude);
    return 0;
}