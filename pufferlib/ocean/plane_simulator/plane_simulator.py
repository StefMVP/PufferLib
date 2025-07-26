'''Plane Simulator Environment

A realistic flight simulator environment focused on takeoff training.
Features realistic physics, multiple flight modes, and high-performance vectorization.
'''

import numpy as np
import gymnasium

import pufferlib
from pufferlib.ocean.plane_simulator import binding

class PlaneSimulator(pufferlib.PufferEnv):
    def __init__(
        self,
        num_envs=1,
        render_mode=None,
        mode="takeoff",
        wind_speed=0.0,
        wind_direction=0.0,
        report_interval=128,
        buf=None,
        seed=0,
    ):
        
        self.single_observation_space = gymnasium.spaces.Box(
            low=-1, high=1, shape=(22,), dtype=np.float32
        )
        
        self.single_action_space = gymnasium.spaces.Box(
            low=-1, high=1, shape=(4,), dtype=np.float32
        )
        
        self.num_agents = num_envs
        self.render_mode = render_mode
        self.report_interval = report_interval
        self.tick = 0

        super().__init__(buf)
        self.actions = self.actions.astype(np.float32)

        flight_mode_map = {
            'takeoff': 0,
            'climb': 1,
            'cruise': 2,
            'approach': 3,
            'landing': 4
        }

        kwargs = {
            'flight_mode': flight_mode_map.get(mode, 0),
            'wind_speed': wind_speed,
            'wind_direction': wind_direction,
        }
        
        self.c_envs = binding.vec_init(
            self.observations, self.actions, self.rewards,
            self.terminals, self.truncations, num_envs, seed,
            **kwargs
        )

    def reset(self, seed=None):
        self.tick = 0
        binding.vec_reset(self.c_envs, seed or 0)
        return self.observations, []

    def step(self, actions):
        self.actions[:] = actions
        self.tick += 1
        binding.vec_step(self.c_envs)

        info = []
        if self.tick % self.report_interval == 0:
            log_data = binding.vec_log(self.c_envs)
            if log_data:
                info.append(log_data)

        return (self.observations, self.rewards, self.terminals, self.truncations, info)

    def render(self):
        binding.vec_render(self.c_envs, 0)

    def close(self):
        binding.vec_close(self.c_envs)

plane_simulator = PlaneSimulator

def test_performance(timeout=10, atn_cache=1024):
    env = PlaneSimulator(num_envs=1000)
    env.reset()
    tick = 0

    actions = np.random.uniform(-1, 1, (atn_cache, 1000, 4)).astype(np.float32)

    import time
    start = time.time()
    while time.time() - start < timeout:
        atn = actions[tick % atn_cache]
        env.step(atn)
        tick += 1

    print(f"PlaneSimulator SPS: {env.num_agents * tick / (time.time() - start)}")

if __name__ == "__main__":
    test_performance()