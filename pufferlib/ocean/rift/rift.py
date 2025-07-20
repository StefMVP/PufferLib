'''Rift Environment

A Diablo-style action RPG environment where agents play as a sorceress
navigating between rift dungeons (fighting monsters and bosses) and 
town (managing inventory and vendors). Features dynamic action/observation
spaces based on the current phase.
'''

import numpy as np
import gymnasium

import pufferlib
from pufferlib.ocean.rift import binding

class Rift(pufferlib.PufferEnv):
    def __init__(
        self,
        num_envs=1,
        render_mode=None,
        width=1024,
        height=768,
        cell_size=32,
        report_interval=128,
        buf=None,
        seed=0,
    ):
        
        # Grid-based observation space: 17 player stats + 100 grid cells
        max_obs_size = 117  # OBS_SIZE from rift.h (17 + 100)
        self.single_observation_space = gymnasium.spaces.Box(
            low=0, high=1, shape=(max_obs_size,), dtype=np.float32
        )
        
        # Action space: 21 discrete actions
        # 0-3: Cardinal movement (up, down, left, right)
        # 4-7: Diagonal movement (up-left, up-right, down-left, down-right)
        # 8: Blizzard, 9: Health potion, 10: Mana potion, 11: Interact, 12: Noop
        # 13-20: Blizzard while moving (up, down, left, right, up-left, up-right, down-left, down-right)
        self.single_action_space = gymnasium.spaces.Discrete(21)
        
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

rift = Rift

def test_performance(timeout=10, atn_cache=1024):
    env = Rift(num_envs=1000)
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