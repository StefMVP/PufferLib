'''
Mazing Contest - A simplified single-player maze building game
The player builds walls and thunderclap towers to create a maze that maximizes
the time it takes for a runner to navigate from entrance to exit.
'''

import numpy as np
import gymnasium

import pufferlib
from pufferlib.ocean.mazing_contest import binding

class MazingContest(pufferlib.PufferEnv):
    def __init__(self, num_envs=1, render_mode=None,
            build_time_limit=None, max_moves=None, max_rounds=None,
            min_gold=None, max_gold=None, min_lumber=None, max_lumber=None,
            log_interval=128, buf=None, seed=0):
        
        # Set up observation space - 108 dimensional  
        # Grid (10x10=100) + resources (2) + phase info (3) + goal info (3) = 108
        self.single_observation_space = gymnasium.spaces.Box(
            low=0, high=1, shape=(108,), dtype=np.float32
        )
        
        # Action space - 200 actions total (walls + thunderclaps)
        # 0-99: Build wall at position (x,y) where pos = y*10 + x
        # 100-199: Build thunderclap at position (x,y) where pos = y*10 + x  
        self.single_action_space = gymnasium.spaces.Discrete(200)
        
        # Single agent per environment
        self.num_agents = num_envs
        
        # Store render mode
        self.render_mode = render_mode
        
        # Initialize parent
        super().__init__(buf=buf)
        
        self.actions = self.actions.astype(np.int32)
        
        # Build kwargs for optional config
        kwargs = {}
        if build_time_limit is not None:
            kwargs['build_time_limit'] = build_time_limit
        if max_moves is not None:
            kwargs['max_moves'] = max_moves
        if max_rounds is not None:
            kwargs['max_rounds'] = max_rounds
        if min_gold is not None:
            kwargs['min_gold'] = min_gold
        if max_gold is not None:
            kwargs['max_gold'] = max_gold
        if min_lumber is not None:
            kwargs['min_lumber'] = min_lumber
        if max_lumber is not None:
            kwargs['max_lumber'] = max_lumber
        
        # Initialize C environments
        self.c_envs = binding.vec_init(
            self.observations, self.actions, self.rewards,
            self.terminals, self.truncations, num_envs, seed,
            **kwargs
        )
        
        # Setup logging
        self.log_interval = log_interval
        self.log_idx = 0
        
        # Environment metadata
        self.metadata = {
            'render_modes': ['human', 'rgb_array'],
            'render_fps': 60,
            'name': 'mazing_contest'
        }
        
        print(f"Mazing Contest initialized with {num_envs} environments")
        print(f"Observation space: {self.single_observation_space}")
        print(f"Action space: {self.single_action_space}")

    def reset(self, seed=0):
        binding.vec_reset(self.c_envs, seed)
        self.log_idx = 0
        return self.observations, []

    def step(self, actions):
        self.actions[:] = actions
        binding.vec_step(self.c_envs)
        
        # Logging - match minimal_td format
        self.log_idx += 1
        info = []
        if self.log_idx % self.log_interval == 0:
            log_data = binding.vec_log(self.c_envs)
            if log_data:
                info.append(log_data)
        
        # Handle truncation/termination
        truncated = np.zeros_like(self.terminals)
        terminated = self.terminals.astype(bool)
        
        return self.observations, self.rewards, terminated, truncated, info

    def decode_log(self, log_data):
        """Decode log data from C environment"""
        if log_data is None or len(log_data) == 0:
            return {}
        
        # Extract meaningful statistics - FOCUSED ON PATH LENGTH
        return {
            'episode_return': np.mean(log_data.get('episode_return', [0])),
            'episode_length': np.mean(log_data.get('episode_length', [0])),
            'final_path_length': np.mean(log_data.get('final_path_length', [0])),  # Main metric
            'path_length_rewards': np.mean(log_data.get('path_length_rewards', [0])),  # Reward tracking
            'best_time': np.mean(log_data.get('best_time', [0])),  # Best runner time achieved
            'rounds_completed': np.mean(log_data.get('rounds_completed', [0])),
            'average_path_length': np.mean(log_data.get('average_path_length', [0])),  # Average path length
            'towers_built': np.mean(log_data.get('towers_built', [0])),
            'total_gold_spent': np.mean(log_data.get('total_gold_spent', [0])),
        }

    def render(self):
        binding.vec_render(self.c_envs, 0)
        
        if self.render_mode == 'ansi':
            # Return ASCII representation
            return f"Mazing Contest - Round {1}, Phase: BUILD"
        return None

    def close(self):
        binding.vec_close(self.c_envs)

def test_performance(timeout=10, atn_cache=1024):
    """Test environment performance"""
    env = mazing_contest(num_envs=1000)
    env.reset()
    tick = 0

    actions = np.random.randint(0, 200, (atn_cache, env.num_agents))

    import time
    start = time.time()
    while time.time() - start < timeout:
        env.step(actions[tick % atn_cache])
        tick += 1

    fps = tick / (time.time() - start)
    env.close()
    return fps

if __name__ == '__main__':
    env = mazing_contest(num_envs=1, render_mode='human')
    obs, info = env.reset()
    
    print(f"Observation shape: {obs.shape}")
    print(f"Action space: {env.single_action_space}")
    print(f"Number of agents: {env.num_agents}")
    
    # Run a few random steps
    for i in range(1000):
        # Random action
        action = np.random.randint(0, 200, (env.num_envs, env.num_agents))
        obs, reward, terminated, truncated, info = env.step(action)
        
        if i % 100 == 0:
            print(f"Step {i}: reward={reward[0]:.3f}, terminated={terminated[0]}")
        
        if terminated[0]:
            print(f"Episode ended at step {i}")
            obs, info = env.reset()
    
    env.close()