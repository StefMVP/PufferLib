'''Rift Environment

A Diablo-style action RPG environment where agents play as a sorceress
navigating between rift dungeons (fighting monsters and bosses) and 
town (managing inventory, equipment, and vendors).

Features two distinct phases:
- RIFT PHASE: Combat, exploration, monster/boss fights
- TOWN PHASE: Equipment management, shop purchases, stat allocation

TOWN TESTING MODE: When TOWN_TESTING_MODE=1 in constants.h, rifts are
auto-completed instantly with rewards, allowing focused training on town
mechanics (equipment upgrades, purchasing decisions, stat management).
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
        completion_reward=10.0,
        monster_kill_reward=10.0,
        death_penalty=0.0,
        episode_length_limit=12000,
        town_stat_increase_reward=1.0,
        town_ilvl_increase_reward=2.0,
        town_stat_decrease_penalty=-1.0,
        town_ilvl_decrease_penalty=-2.0,
        town_no_purchases_penalty=-5.0,
        low_health_penalty=-2.0,
    ):
        
        # Full observation space: 22 player stats + 100 grid cells + 101 town interface stats  
        # Player obs: 17 base + 5 extra (duplicate positions + phase) = 22
        # Town obs: 40 shop slots + 52 equipment slots + 9 interface = 101
        max_obs_size = 223  # Actual observations (22 + 100 + 101)
        self.single_observation_space = gymnasium.spaces.Box(
            low=0, high=1, shape=(max_obs_size,), dtype=np.float32
        )
        
        # Action space: 17 discrete actions (works for both rift and town phases)
        # RIFT PHASE:
        #   0-3: Cardinal movement (up, down, left, right)
        #   4-7: Diagonal movement (up-left, up-right, down-left, down-right)
        #   8: Blizzard, 9: Health potion, 10: Mana potion, 11: Interact, 12: Noop
        #   13: Exit town and go to next rift (skip remaining town time)
        # TOWN PHASE:
        #   0-3: Navigate equipment/shop (up, down, left, right for WASD navigation)
        #   11: Interact (buy items, equip items, enter rift portal)
        #   12: Noop
        #   13: Exit town and go to next rift (skip remaining town time)
        #   14: Switch to shop tab (1 key)
        #   15: Switch to character tab (2 key)
        #   16: Reroll shop items (R key, costs gold)
        #   Other actions ignored in town
        self.single_action_space = gymnasium.spaces.Discrete(17)
        
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
            'completion_reward': completion_reward,
            'monster_kill_reward': monster_kill_reward,
            'death_penalty': death_penalty,
            'episode_length_limit': episode_length_limit,
            'town_stat_increase_reward': town_stat_increase_reward,
            'town_ilvl_increase_reward': town_ilvl_increase_reward,
            'town_stat_decrease_penalty': town_stat_decrease_penalty,
            'town_ilvl_decrease_penalty': town_ilvl_decrease_penalty,
            'town_no_purchases_penalty': town_no_purchases_penalty,
            'low_health_penalty': low_health_penalty,
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