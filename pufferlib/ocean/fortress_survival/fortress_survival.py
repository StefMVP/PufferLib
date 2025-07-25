'''Fortress Survival Environment

A tower defense game inspired by the Warcraft 3 custom map. Players must build walls, towers, 
and resource buildings to defend against waves of increasingly difficult enemies. The goal is 
to survive as many waves as possible while managing lumber and gold resources effectively.

Game mechanics include:
- Wave-based enemy spawning with different enemy types (normal, air, invisible, boss)
- Building system with walls, towers, and lumber mills
- Resource management (lumber production, building costs)
- Progressive difficulty scaling with each wave
- Victory conditions based on wave completion and survival time
'''

import numpy as np
import gymnasium

import pufferlib
from pufferlib.ocean.fortress_survival import binding

class FortressSurvival(pufferlib.PufferEnv):
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
        
        self.single_observation_space = gymnasium.spaces.Box(
            low=0, high=1, shape=(32 * 24 + 25,), dtype=np.float32
        )
        
        self.single_action_space = gymnasium.spaces.Discrete(32 * 24 + 1)  # Full grid + no-op
        
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

fortress_survival = FortressSurvival

def test_performance(timeout=10, atn_cache=1024):
    env = FortressSurvival(num_envs=1000)
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