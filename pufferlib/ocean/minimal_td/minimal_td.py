'''Minimal Tower Defense Environment

A simple grid-based tower defense environment where agents place towers
to stop enemies traveling along a fixed path. Features discrete action
space for tower placement and multi-channel observations.
'''

import numpy as np
import gymnasium

import pufferlib
from pufferlib.ocean.minimal_td import binding

class MinimalTD(pufferlib.PufferEnv):
    def __init__(
        self,
        num_envs=1,
        render_mode=None,
        width=800,
        height=600,
        cell_size=64,
        report_interval=128,
        buf=None,
        seed=0,
    ):
        
        # 16x12 grid with single multi-value channel + 3 scalars (195 total)
        self.single_observation_space = gymnasium.spaces.Box(
            low=0, high=1, shape=(16 * 12 + 3,), dtype=np.float32
        )
        
        # Discrete: 0=noop, 1-192=rapid tower at position, 193-384=splash tower at position, 385-576=sniper tower at position
        # 1 + 3*(16*12) = 1 + 3*192 = 577 total actions
        self.single_action_space = gymnasium.spaces.Discrete(577)
        
        self.num_agents = num_envs
        self.render_mode = render_mode
        self.report_interval = report_interval
        self.tick = 0

        super().__init__(buf)
        self.actions = self.actions.astype(np.float32)

        kwargs = {
            'width': width,
            'height': height,
            'cell_size': cell_size,
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

minimal_td = MinimalTD

def test_performance(timeout=10, atn_cache=1024):
    env = MinimalTD(num_envs=1000)
    env.reset()
    tick = 0

    actions = [env.action_space.sample() for _ in range(atn_cache)]

    import time
    start = time.time()
    while time.time() - start < timeout:
        atn = actions[tick % atn_cache]
        env.step(atn)
        tick += 1

    print(f"SPS: {env.num_agents * tick / (time.time() - start)}")

if __name__ == "__main__":
    test_performance()