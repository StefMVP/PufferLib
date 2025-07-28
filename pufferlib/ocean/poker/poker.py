'''Poker Environment

Heads-Up No-Limit Texas Hold'em environment for reinforcement learning.
Supports self-play training with configurable stack sizes and blind structure.
'''

import numpy as np
import gymnasium
import torch
import os
import threading
import multiprocessing
import random
import glob

import pufferlib
from pufferlib.ocean.poker import binding

# Shared opponent model for ALL environments to prevent memory issues
_shared_opponent_model = None
_shared_model_generation = None
_shared_model_lock = threading.Lock()

def preload_opponent_model(generation_number):
    """Pre-load opponent model BEFORE creating any environments"""
    global _shared_opponent_model, _shared_model_generation, _shared_model_lock
    
    if generation_number <= 1:
        print("ℹ️  Generation 1: No opponent model to pre-load")
        return
    
    opponent_generation = generation_number - 1
    
    with _shared_model_lock:
        if (_shared_opponent_model is not None and 
            _shared_model_generation == opponent_generation):
            print(f"✅ Opponent model for generation {opponent_generation} already loaded")
            return
        
        print(f"🔄 PRE-LOADING opponent model for generation {opponent_generation}...")
        
        # Create a minimal temporary environment just for loading
        temp_env = Poker(num_envs=1, self_play_mode=False, generation_number=1)
        temp_env._load_opponent_model(opponent_generation)
        
        if temp_env.opponent_model is not None:
            _shared_opponent_model = temp_env.opponent_model
            _shared_model_generation = opponent_generation
            print(f"✅ PRE-LOADED opponent model for generation {opponent_generation}")
        else:
            raise RuntimeError(f"Failed to pre-load opponent model for generation {opponent_generation}")
        
        temp_env.close()

class Poker(pufferlib.PufferEnv):
    def __init__(
        self,
        num_envs=1,
        render_mode="human",
        width=1000,
        height=750,
        starting_stack=200,
        small_blind=1,
        big_blind=2,
        self_play_mode=False,
        generation_number=1,
        opponent_generation=None,
        max_episode_length=1000,
        max_hands_per_episode=200,
        report_interval=128,
        buf=None,
        seed=0,
    ):
        
        self.single_observation_space = gymnasium.spaces.Box(
            low=0, high=2, shape=(31,), dtype=np.float32  # Strategic features for full poker game
        )
        
        self.single_action_space = gymnasium.spaces.Discrete(5)  # fold, call, check, bet_pot, all_in
        
        # Number of agents (always equal to num_envs for poker)
        self.num_agents = num_envs
        self.render_mode = render_mode
        self.report_interval = report_interval
        self.tick = 0
        self.self_play_mode = self_play_mode
        self.generation_number = generation_number
        self.effective_self_play_mode = False  # Will be set later

        super().__init__(buf)
        self.actions = self.actions.astype(np.float32)
        
        # Initialize opponent model handling
        self.opponent_model = None
        self.opponent_lstm_state = None
        self.device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
        
        # Convert boolean strings to proper types
        if isinstance(self_play_mode, str):
            self_play_mode = self_play_mode.lower() in ('true', '1', 'yes', 'on')
        
        # Self-play ALWAYS loads Python opponent models - NO exceptions
        if True:
            # Original Python model loading logic
            import multiprocessing
            is_worker_process = multiprocessing.current_process().name != 'MainProcess'
            
            # Initialize effective_self_play_mode
            effective_self_play_mode = False
            
            if self_play_mode and generation_number > 1:
                effective_self_play_mode = True
                self.effective_self_play_mode = True
                
                # League training: select opponent from available pool
                available_generations = self._get_available_generations(generation_number)
                if available_generations:
                    self.opponent_generation = random.choice(available_generations)
                else:
                    # Fallback to heuristic if no models available
                    self.opponent_generation = 0
                
                # Handle league training opponent loading
                if self.opponent_generation == 0:
                    # Use C heuristic archetypes (no Python model needed)
                    print(f"🎯 League training: Using heuristic archetypes (generation 0)")
                    self.opponent_model = None
                    self.opponent_lstm_state = None
                else:
                    # AUTO-PRELOAD neural network opponent
                    global _shared_opponent_model, _shared_model_generation, _shared_model_lock
                    with _shared_model_lock:
                        if (_shared_opponent_model is None or 
                            _shared_model_generation != self.opponent_generation):
                            print(f"🔄 AUTO-PRELOADING opponent model for generation {self.opponent_generation} (first env)")
                            self._load_opponent_model(self.opponent_generation)
                            if self.opponent_model is not None:
                                _shared_opponent_model = self.opponent_model
                                _shared_model_generation = self.opponent_generation
                                print(f"✅ AUTO-PRELOADED opponent model for generation {self.opponent_generation}")
                            else:
                                raise RuntimeError(f"Failed to auto-preload opponent model for generation {self.opponent_generation}")
                        else:
                            # Use existing shared model
                            self.opponent_model = _shared_opponent_model
                            device = 'cpu'
                            # Detect hidden size from shared model
                            shared_hidden_size = 32  # Default fallback
                            if hasattr(_shared_opponent_model, 'policy') and hasattr(_shared_opponent_model.policy, 'encoder'):
                                if len(_shared_opponent_model.policy.encoder) > 0:
                                    shared_hidden_size = _shared_opponent_model.policy.encoder[0].out_features
                            self.opponent_lstm_state = [{
                                'lstm_h': torch.zeros(1, shared_hidden_size, device=device),
                                'lstm_c': torch.zeros(1, shared_hidden_size, device=device)
                            } for _ in range(self.num_agents)]
                            print(f"✅ Using existing shared opponent model for generation {self.opponent_generation}")
            elif self_play_mode and generation_number == 1:
                print("ℹ️  Generation 1: Self-play mode disabled (no previous generation)")
                effective_self_play_mode = False
                self.effective_self_play_mode = False
                self.opponent_generation = None
            else:
                self.opponent_generation = None
        
        # Calculate intended opponent generation using the actual selected opponent
        if opponent_generation is not None:
            intended_opponent_generation = opponent_generation
        elif hasattr(self, 'opponent_generation') and self.opponent_generation is not None:
            intended_opponent_generation = self.opponent_generation
        else:
            intended_opponent_generation = 0
        
        kwargs = {
            'width': width,
            'height': height,
            'starting_stack': starting_stack,
            'small_blind': small_blind,
            'big_blind': big_blind,
            'self_play_mode': int(effective_self_play_mode),
            'generation_number': int(generation_number),
            'intended_opponent_generation': int(intended_opponent_generation),
            'max_episode_length': int(max_episode_length),
            'max_hands_per_episode': int(max_hands_per_episode),
            'render_mode': render_mode or 'None',
        }
        
        self.c_envs = binding.vec_init(
            self.observations, self.actions, self.rewards,
            self.terminals, self.truncations, num_envs, seed,
            **kwargs
        )
        
        # Log final self-play configuration for debugging
        import multiprocessing
        process_name = multiprocessing.current_process().name
        print(f"🎯 POKER SELF-PLAY STATUS [{process_name}]:")
        print(f"   • self_play_mode: {self_play_mode}")
        print(f"   • effective_self_play_mode: {effective_self_play_mode}")
        print(f"   • opponent_model loaded: {self.opponent_model is not None}")
        print(f"   • generation: {generation_number}")
        print(f"   • intended_opponent_generation: {intended_opponent_generation}")
        
        # STRICT self-play validation - CRASH if anything is wrong
        if effective_self_play_mode:
            if self.opponent_generation == 0:
                print(f"   🎯 LEAGUE TRAINING: Using heuristic archetypes (Gen 0)")
            elif self.opponent_model is not None:
                print(f"   ✅ LEAGUE TRAINING: Neural opponent (Gen {intended_opponent_generation})")
            else:
                print(f"   💥 FATAL ERROR: Self-play enabled but NO opponent model!")
                print(f"   • Expected generation {intended_opponent_generation} model")
                print(f"   • Training would be worthless without opponent")
                raise RuntimeError(f"SELF-PLAY FAILED: Generation {generation_number} missing opponent model {intended_opponent_generation}!")
        else:
            print(f"   ℹ️  Generation 1 (no opponent needed)")

    def _get_available_generations(self, current_generation):
        """Get list of available opponent generations for league training"""
        available = []
        
        # Add all previous generations that have model files
        for gen in range(1, current_generation):
            if self._model_exists(gen):
                available.append(gen)
        
        # Add random archetype opponents (represented as generation 0)
        # Include multiple copies to increase probability of selection
        available.extend([0, 0, 0])  # 3x weight for heuristic archetypes
        
        print(f"🎯 League training pool: {available}")
        return available
    
    def _model_exists(self, generation):
        """Check if a model file exists for the given generation in the current batch"""
        # Try to detect current batch by finding the most recent generation directory
        current_batch_date = self._get_current_batch_date()
        
        if current_batch_date:
            # Look for models only within the current batch (same date prefix)
            patterns = [
                f"experiments/self_play_poker/gen{generation}_{current_batch_date}*/model_gen{generation}.pt",
                f"experiments/self_play_poker/gen{generation}_{current_batch_date}*/opponent_model.pt",
            ]
        else:
            # Fallback to old behavior if batch detection fails
            patterns = [
                f"experiments/self_play_poker/gen{generation}_*/model_gen{generation}.pt",
                f"experiments/self_play_poker/gen{generation}_*/opponent_model.pt",
            ]
        
        for pattern in patterns:
            model_files = glob.glob(pattern)
            if model_files:
                return True
        return False

    def _get_current_batch_date(self):
        """Get the date prefix of the current evolution batch"""
        try:
            # Look for the most recent generation directory to infer current batch
            all_dirs = glob.glob("experiments/self_play_poker/gen*_*/")
            if not all_dirs:
                return None
            
            # Sort by generation number and timestamp to get the most recent
            all_dirs.sort(key=lambda x: (
                int(x.split('gen')[1].split('_')[0]),  # generation number
                x.split('_', 1)[1].rstrip('/')  # timestamp
            ))
            
            latest_dir = all_dirs[-1]
            
            # Extract date prefix (e.g., "20250728" from "gen7_20250728_172643/")
            # Format: experiments/self_play_poker/genN_YYYYMMDD_HHMMSS/
            dir_name = latest_dir.split('/')[-2]  # Get just the directory name
            parts = dir_name.split('_')
            
            if len(parts) >= 2:
                date_prefix = parts[1]  # Should be YYYYMMDD
                print(f"🎯 Detected current batch date: {date_prefix}")
                return date_prefix
            else:
                print(f"⚠️  Unexpected directory format: {dir_name}")
                return None
            
        except Exception as e:
            print(f"⚠️  Could not detect current batch date: {e}")
            return None

    def reset(self, seed=None):
        self.tick = 0
        binding.vec_reset(self.c_envs, seed or 0)
        return self.observations, []


    def render(self):
        if self.render_mode == 'None':
            return None
        binding.vec_render(self.c_envs, 0)

    def close(self):
        try:
            binding.vec_close(self.c_envs)
        except Exception as e:
            print(f"Warning: Error during environment close: {e}")
        
        # Clear opponent model references to prevent memory issues
        self.opponent_model = None
        self.opponent_lstm_state = None
    
    def _load_shared_opponent_model(self, opponent_generation):
        """Load opponent model ONCE and share across all environments"""
        global _shared_opponent_model, _shared_model_generation, _shared_model_lock
        
        print(f"🔄 Entering shared model loading for generation {opponent_generation}")
        
        # Add timeout to prevent infinite hangs
        import signal
        def timeout_handler(signum, frame):
            raise TimeoutError(f"Model loading timed out after 60 seconds for generation {opponent_generation}")
        
        signal.signal(signal.SIGALRM, timeout_handler)
        signal.alarm(60)  # 60 second timeout
        
        try:
            with _shared_model_lock:
                print(f"🔄 Acquired lock for generation {opponent_generation}")
                
                # Check if we already have the right model loaded
                if (_shared_opponent_model is not None and 
                    _shared_model_generation == opponent_generation):
                    print(f"🔄 Using existing shared opponent model for generation {opponent_generation}")
                    self.opponent_model = _shared_opponent_model
                    # Each env gets its own LSTM state
                    device = 'cpu'
                    self.opponent_lstm_state = [{
                        'lstm_h': torch.zeros(1, 32, device=device),
                        'lstm_c': torch.zeros(1, 32, device=device)
                    } for _ in range(self.num_agents)]
                    return
                
                # Load model for the first time or different generation
                print(f"🔄 Loading SHARED opponent model for generation {opponent_generation}...")
                self._load_opponent_model(opponent_generation)
                
                if self.opponent_model is not None:
                    # Store as shared model
                    _shared_opponent_model = self.opponent_model
                    _shared_model_generation = opponent_generation
                    print(f"✅ Shared opponent model loaded and cached for generation {opponent_generation}")
                else:
                    raise RuntimeError(f"Failed to load opponent model for generation {opponent_generation}")
        finally:
            signal.alarm(0)  # Cancel timeout

    def _load_opponent_model(self, opponent_generation):
        """Load opponent model for self-play"""
        try:
            # Find opponent model file - try multiple naming patterns within current batch
            import glob
            
            # Get current batch date to avoid loading incompatible models
            current_batch_date = self._get_current_batch_date()
            
            if current_batch_date:
                # Look for models only within the current batch (same date prefix)
                pattern1 = f"experiments/self_play_poker/gen{opponent_generation}_{current_batch_date}*/model_gen{opponent_generation}.pt"
                pattern2 = f"experiments/self_play_poker/gen{opponent_generation}_{current_batch_date}*/opponent_model.pt"
            else:
                # Fallback to old behavior if batch detection fails
                pattern1 = f"experiments/self_play_poker/gen{opponent_generation}_*/model_gen{opponent_generation}.pt"
                pattern2 = f"experiments/self_play_poker/gen{opponent_generation}_*/opponent_model.pt"
            
            # Primary pattern: model_gen{N}.pt (preferred format)
            model_files = glob.glob(pattern1)
            
            # Fallback pattern: opponent_model.pt (legacy format)
            if not model_files:
                model_files = glob.glob(pattern2)
            
            if not model_files:
                print(f"💥 FATAL: No opponent model found for generation {opponent_generation}")
                print(f"Tried patterns: {pattern1}, {pattern2}")
                print(f"Current batch date: {current_batch_date}")
                print(f"Available directories: {glob.glob('experiments/self_play_poker/gen*/')}")
                raise FileNotFoundError(f"REQUIRED opponent model for generation {opponent_generation} not found!")
                
            model_path = model_files[0]  # Use the first match
            print(f"Loading opponent model: {model_path}")
            
            # Import models here to avoid circular imports
            from pufferlib.models import Default, LSTMWrapper
            
            # Detect architecture from saved model to handle different hidden_sizes
            print("Detecting model architecture...")
            state_dict = torch.load(model_path, map_location='cpu')
            state_dict = {k.replace('module.', ''): v for k, v in state_dict.items()}
            
            # Detect hidden_size from encoder weight shape
            if 'policy.encoder.0.weight' in state_dict:
                detected_hidden_size = state_dict['policy.encoder.0.weight'].shape[0]
                print(f"Detected hidden_size: {detected_hidden_size}")
            else:
                detected_hidden_size = 32  # Fallback
                print(f"Could not detect hidden_size, using fallback: {detected_hidden_size}")
            
            # Recreate the exact architecture used during training
            print("Creating model architecture...")
            policy = Default(self, hidden_size=detected_hidden_size)
            policy = LSTMWrapper(self, policy, input_size=detected_hidden_size, hidden_size=detected_hidden_size)
            
            # Load the trained weights  
            # FORCE CPU FOR ALL OPPONENT MODELS - NO CUDA EVER
            device = 'cpu'  # Always CPU for opponent models
            policy.load_state_dict(state_dict)
            policy.to(device)
            policy.eval()
            
            # Initialize LSTM state for each environment (use same device as model)
            self.opponent_model = policy
            self.opponent_lstm_state = [{
                'lstm_h': torch.zeros(1, detected_hidden_size, device=device),
                'lstm_c': torch.zeros(1, detected_hidden_size, device=device)
            } for _ in range(self.num_agents)]
            
            print(f"✅ Opponent model loaded successfully on {device}")
            
        except Exception as e:
            print(f"💥 FATAL: Failed to load opponent model: {e}")
            print(f"   • Generation: {opponent_generation}")
            print(f"   • Error type: {type(e).__name__}")
            print(f"   • Model path attempted: {model_path if 'model_path' in locals() else 'Unknown'}")
            raise RuntimeError(f"REQUIRED opponent model loading failed for generation {opponent_generation}: {e}") from e
    
    def _get_opponent_action(self, env_idx, obs):
        """DEPRECATED: Use _set_batched_opponent_actions for performance"""
        # This method is no longer used but kept for compatibility
        if self.opponent_model is None:
            raise RuntimeError("DEPRECATED: opponent_model is None - this should not happen!")
        
        with torch.no_grad():
            obs_tensor = torch.FloatTensor(obs).unsqueeze(0).to('cpu')
            
            if env_idx >= len(self.opponent_lstm_state):
                raise IndexError(f"env_idx {env_idx} >= {len(self.opponent_lstm_state)}")
            
            lstm_state = self.opponent_lstm_state[env_idx]
            if lstm_state['lstm_h'].device != torch.device('cpu'):
                lstm_state['lstm_h'] = lstm_state['lstm_h'].to('cpu')
                lstm_state['lstm_c'] = lstm_state['lstm_c'].to('cpu')
            
            if next(self.opponent_model.parameters()).device != torch.device('cpu'):
                self.opponent_model = self.opponent_model.to('cpu')
            
            logits, value = self.opponent_model.forward_eval(obs_tensor, lstm_state)
            probs = torch.softmax(logits, dim=-1)
            action = torch.multinomial(probs, 1).item()
            
            return action
    
    def _set_batched_opponent_actions(self):
        """Set opponent actions for all environments in a single batched inference"""
        if self.opponent_model is None:
            if self.effective_self_play_mode and self.opponent_generation != 0:
                # CRITICAL: Neural opponent expected but model is None - CRASH!
                print(f"💥 FATAL: Neural opponent expected but model is None!")
                print(f"   • Generation: {self.generation_number}")
                print(f"   • Expected opponent generation: {self.opponent_generation}")
                print(f"   • This should NEVER happen - model loading failed!")
                raise RuntimeError(f"Neural opponent required but got None for generation {self.generation_number}")
            else:
                # Generation 1, heuristic archetypes (gen 0), or --new mode: Use C opponent
                return
        
        # PERFORMANCE CRITICAL: Batch all observations for single inference
        with torch.no_grad():
            # Prepare batch of observations
            batch_obs = torch.FloatTensor(self.observations).to('cpu')  # Shape: [num_agents, obs_size]
            
            # Prepare batch of LSTM states  
            batch_lstm_h = torch.stack([state['lstm_h'] for state in self.opponent_lstm_state]).squeeze(1)  # [num_agents, 32]
            batch_lstm_c = torch.stack([state['lstm_c'] for state in self.opponent_lstm_state]).squeeze(1)  # [num_agents, 32]
            batch_lstm_state = {'lstm_h': batch_lstm_h, 'lstm_c': batch_lstm_c}
            
            # Force model to CPU if not already
            if next(self.opponent_model.parameters()).device != torch.device('cpu'):
                self.opponent_model = self.opponent_model.to('cpu')
            
            # Single batched forward pass for all environments
            logits, values = self.opponent_model.forward_eval(batch_obs, batch_lstm_state)
            probs = torch.softmax(logits, dim=-1)
            actions = torch.multinomial(probs, 1).squeeze(-1)  # [num_agents]
            
            # Update LSTM states
            for env_idx in range(self.num_agents):
                self.opponent_lstm_state[env_idx]['lstm_h'] = batch_lstm_state['lstm_h'][env_idx:env_idx+1]
                self.opponent_lstm_state[env_idx]['lstm_c'] = batch_lstm_state['lstm_c'][env_idx:env_idx+1]
            
            # Set actions for all environments
            for env_idx in range(self.num_agents):
                action = actions[env_idx].item()
                binding.vec_set_opponent_action(self.c_envs, env_idx, action)
    
    def step(self, actions):
        """Override step to handle opponent actions in self-play"""
        self.actions[:] = actions
        
        # PERFORMANCE FIX: Batch opponent actions for massive speedup
        if self.opponent_model is not None:
            self._set_batched_opponent_actions()
        
        self.tick += 1
        binding.vec_step(self.c_envs)

        info = []
        if self.tick % self.report_interval == 0:
            log_data = binding.vec_log(self.c_envs)
            if log_data:
                info.append(log_data)

        return (self.observations, self.rewards, self.terminals, self.truncations, info)

poker = Poker

def test_performance(timeout=10, atn_cache=1024):
    env = Poker(num_envs=1000)
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