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

# GENERATION 1 SELF-PLAY: Global current training model
_current_training_model = None
_current_training_model_lock = threading.Lock()

def set_global_training_model(model):
    """Set the current training model for Generation 1 self-play"""
    global _current_training_model, _current_training_model_lock
    with _current_training_model_lock:
        _current_training_model = model

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
        self.generation_1_self_play = False  # Special flag for gen 1 self-play

        super().__init__(buf)
        self.actions = self.actions.astype(np.float32)
        
        # Initialize opponent model handling
        self.opponent_model = None
        self.opponent_lstm_state = None
        self.device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
        
        # Pre-allocated GPU tensors for zero-allocation inference
        self._gpu_obs_buffer = None
        self._gpu_lstm_h_buffer = None  
        self._gpu_lstm_c_buffer = None
        self._gpu_actions_buffer = None
        
        # NO FALLBACKS - GPU optimization must work or training stops
        # Check if we're in a multiprocessing worker process
        current_process = multiprocessing.current_process()
        is_worker_process = current_process.name != 'MainProcess'
        
        # Allow environment variable override for testing
        force_cpu_inference = os.environ.get('PUFFER_FORCE_CPU_INFERENCE', '0').lower() in ('1', 'true', 'yes')
        
        if is_worker_process and torch.cuda.is_available():
            # CUDA multiprocessing issue - STOP TRAINING
            raise RuntimeError(
                f"🚫 CUDA GPU optimization failed in multiprocessing worker process '{current_process.name}'!\n"
                f"   Fix multiprocessing configuration - NO FALLBACKS!\n"
                f"   Use Serial backend: backend = Serial"
            )
        
        # GPU ONLY - NO FALLBACKS
        if not torch.cuda.is_available():
            raise RuntimeError("🚫 GPU REQUIRED - NO CUDA AVAILABLE!")
        
        if force_cpu_inference:
            raise RuntimeError("🚫 CPU TESTING DISABLED - GPU ONLY MODE!")
        
        self.use_gpu_inference = True
        self.device = torch.device('cuda')
        
        # Convert boolean strings to proper types
        if isinstance(self_play_mode, str):
            self_play_mode = self_play_mode.lower() in ('true', '1', 'yes', 'on')
        
        # Self-play ALWAYS loads Python opponent models - NO exceptions
        if True:
            # Original Python model loading logic
            current_process = multiprocessing.current_process()
            is_worker_process = current_process.name != 'MainProcess'
            
            # Initialize effective_self_play_mode
            effective_self_play_mode = False
            
            if self_play_mode and generation_number >= 1:
                effective_self_play_mode = True
                self.effective_self_play_mode = True
                
                if generation_number == 1:
                    # Generation 1: Use special self-play mode
                    # We'll handle this by using the current model for opponent inference
                    self.opponent_generation = 1
                    self.generation_1_self_play = True
                    print(f"🎯 Generation 1: Pure self-play mode (same model both sides)")
                else:
                    # League training: select opponent from available pool
                    available_generations = self._get_available_generations(generation_number)
                    if available_generations:
                        self.opponent_generation = random.choice(available_generations)
                    else:
                        # Fallback to playing against generation 1
                        self.opponent_generation = 1
                    self.generation_1_self_play = False
                
                # AUTO-PRELOAD neural network opponent (all generations use models now)
                global _shared_opponent_model, _shared_model_generation, _shared_model_lock
                with _shared_model_lock:
                    if generation_number == 1:
                        # Generation 1: Self-play without pre-existing model
                        # The current training model will be used for both players during inference
                        print(f"🎯 Generation 1: Self-play mode (same model for both players)")
                        self.opponent_model = None  # Will be set during step() to current model
                        self.opponent_lstm_state = None
                    else:
                        # Generation 2+: Load previous generation model
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
                            # Use existing shared model on GPU
                            self.opponent_model = _shared_opponent_model.to(self.device)
                            # Detect hidden size from shared model
                            shared_hidden_size = 32  # Default fallback
                            if hasattr(_shared_opponent_model, 'policy') and hasattr(_shared_opponent_model.policy, 'encoder'):
                                if len(_shared_opponent_model.policy.encoder) > 0:
                                    shared_hidden_size = _shared_opponent_model.policy.encoder[0].out_features
                            self.opponent_lstm_state = [{
                                'lstm_h': torch.zeros(1, shared_hidden_size, device=self.device),
                                'lstm_c': torch.zeros(1, shared_hidden_size, device=self.device)
                            } for _ in range(self.num_agents)]
                            print(f"✅ Using existing shared opponent model for generation {self.opponent_generation} on GPU")
            else:
                self.opponent_generation = None
        
        # Calculate intended opponent generation using the actual selected opponent
        if opponent_generation is not None:
            intended_opponent_generation = opponent_generation
        elif hasattr(self, 'opponent_generation') and self.opponent_generation is not None:
            intended_opponent_generation = self.opponent_generation
        elif generation_number == 1 and effective_self_play_mode:
            # Generation 1 self-play: opponent generation should be 1 (same as hero)
            intended_opponent_generation = 1
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
        process_name = multiprocessing.current_process().name
        print(f"🎯 POKER SELF-PLAY STATUS [{process_name}]:")
        print(f"   • self_play_mode: {self_play_mode}")
        print(f"   • effective_self_play_mode: {effective_self_play_mode}")
        print(f"   • opponent_model loaded: {self.opponent_model is not None}")
        print(f"   • gpu_inference: ALWAYS ENABLED")
        print(f"   • device: {self.device} (GPU ONLY)")
        print(f"   • generation: {generation_number}")
        print(f"   • intended_opponent_generation: {intended_opponent_generation}")
        
        # GPU buffer allocation and warmup for opponent model inference
        if self.opponent_model is not None:
            self._allocate_gpu_buffers()
            self._warmup_gpu_inference()
        
        # STRICT self-play validation - CRASH if anything is wrong
        if effective_self_play_mode:
            if generation_number == 1:
                print(f"   🎯 GENERATION 1: Pure self-play (same model for both players)")
            elif self.opponent_model is not None:
                print(f"   ✅ LEAGUE TRAINING: Neural opponent (Gen {intended_opponent_generation})")
            else:
                print(f"   💥 FATAL ERROR: Self-play enabled but NO opponent model!")
                print(f"   • Expected generation {intended_opponent_generation} model")
                print(f"   • Training would be worthless without opponent")
                raise RuntimeError(f"SELF-PLAY FAILED: Generation {generation_number} missing opponent model {intended_opponent_generation}!")
        else:
            print(f"   ℹ️  Non-self-play mode (should not happen with new logic)")

    def _get_available_generations(self, current_generation):
        """Get list of available opponent generations for league training"""
        available = []
        
        # Add all previous generations that have model files
        for gen in range(1, current_generation):
            if self._model_exists(gen):
                available.append(gen)
        
        # If no neural opponents available, fallback to generation 1
        if not available:
            available.extend([1])  # Fallback to generation 1 self-play
        
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
        
        # Clear GPU memory
        torch.cuda.empty_cache()
    
    def set_current_model_as_opponent(self, model):
        """Set the current training model as opponent for generation 1 self-play"""
        if self.generation_number == 1 and self.effective_self_play_mode:
            print(f"🎯 Setting current training model as opponent for generation 1 self-play")
            self.opponent_model = model.to(self.device)
            self.opponent_model.eval()
            
            # Initialize opponent LSTM states
            hidden_size = self._get_model_hidden_size()
            self.opponent_lstm_state = [{
                'lstm_h': torch.zeros(1, hidden_size, device=self.device),
                'lstm_c': torch.zeros(1, hidden_size, device=self.device)
            } for _ in range(self.num_agents)]
            
            # Allocate GPU buffers for opponent inference
            self._allocate_gpu_buffers(hidden_size)
            print(f"✅ Generation 1 self-play: Same model now used for both players")
    
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
                    self.opponent_model = _shared_opponent_model.to(self.device)
                    # Each env gets its own LSTM state on GPU
                    self.opponent_lstm_state = [{
                        'lstm_h': torch.zeros(1, 32, device=self.device),
                        'lstm_c': torch.zeros(1, 32, device=self.device)
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
            
            # Load on GPU - NO FALLBACKS
            policy.load_state_dict(state_dict)
            policy.to(self.device)
            policy.eval()
            
            # Initialize LSTM state on GPU
            self.opponent_model = policy
            self.opponent_lstm_state = [{
                'lstm_h': torch.zeros(1, detected_hidden_size, device=self.device),
                'lstm_c': torch.zeros(1, detected_hidden_size, device=self.device)
            } for _ in range(self.num_agents)]
            
            print(f"✅ Opponent model loaded on GPU: {self.device}")
            
            # Pre-allocate GPU tensors for zero-allocation inference
            self._allocate_gpu_buffers(detected_hidden_size)
            
        except Exception as e:
            print(f"💥 FATAL: Failed to load opponent model: {e}")
            print(f"   • Generation: {opponent_generation}")
            print(f"   • Error type: {type(e).__name__}")
            print(f"   • Model path attempted: {model_path if 'model_path' in locals() else 'Unknown'}")
            raise RuntimeError(f"REQUIRED opponent model loading failed for generation {opponent_generation}: {e}") from e
    
    def _allocate_gpu_buffers(self, hidden_size):
        """Pre-allocate GPU tensors for zero-allocation inference"""
        print("🚀 Pre-allocating GPU tensors for zero-allocation inference...")
        
        obs_size = self.single_observation_space.shape[0]
        
        # Pre-allocate observation buffer (reused each step)
        self._gpu_obs_buffer = torch.zeros(
            self.num_agents, obs_size, 
            device=self.device, dtype=torch.float32
        )
        
        # Pre-allocate LSTM state buffers (reused and updated each step)
        self._gpu_lstm_h_buffer = torch.zeros(
            self.num_agents, hidden_size,
            device=self.device, dtype=torch.float32
        )
        self._gpu_lstm_c_buffer = torch.zeros(
            self.num_agents, hidden_size,
            device=self.device, dtype=torch.float32
        )
        
        # Pre-allocate action buffer (reused each step)
        self._gpu_actions_buffer = torch.zeros(
            self.num_agents,
            device=self.device, dtype=torch.long
        )
        
        # Initialize LSTM buffers with current states
        for i in range(self.num_agents):
            self._gpu_lstm_h_buffer[i] = self.opponent_lstm_state[i]['lstm_h'].squeeze(0)
            self._gpu_lstm_c_buffer[i] = self.opponent_lstm_state[i]['lstm_c'].squeeze(0)
        
        print(f"✅ Pre-allocated GPU buffers: obs({self._gpu_obs_buffer.shape}), "
              f"lstm_h({self._gpu_lstm_h_buffer.shape}), "
              f"lstm_c({self._gpu_lstm_c_buffer.shape}), "
              f"actions({self._gpu_actions_buffer.shape})")
    
    def _warmup_gpu_inference(self):
        """Warmup GPU inference pipeline"""
        if self.opponent_model is None:
            return
            
        print("🔥 Warming up GPU inference pipeline...")
        with torch.no_grad():
            # Warmup using pre-allocated buffers
            logits, _ = self.opponent_model.forward_eval(self._gpu_obs_buffer, {
                'lstm_h': self._gpu_lstm_h_buffer, 
                'lstm_c': self._gpu_lstm_c_buffer
            })
            probs = torch.softmax(logits, dim=-1)
            torch.multinomial(probs, 1, out=self._gpu_actions_buffer.view(-1, 1))
            
            # Force GPU synchronization
            torch.cuda.synchronize()
            
            print(f"✅ GPU inference pipeline warmed up (batch_size={self.num_agents})")
    
    def _allocate_gpu_buffers(self, hidden_size=None):
        """Allocate GPU tensors for zero-allocation inference"""
        if self.opponent_model is None:
            return
            
        print("🚀 Pre-allocating GPU tensors for zero-allocation inference...")
        
        # Get hidden size from model if not provided
        if hidden_size is None:
            hidden_size = self._get_model_hidden_size()
        
        # Pre-allocate GPU buffers
        self._gpu_obs_buffer = torch.zeros(
            self.num_agents, 31, device=self.device, dtype=torch.float32
        )
        self._gpu_lstm_h_buffer = torch.zeros(
            self.num_agents, hidden_size, device=self.device, dtype=torch.float32
        )
        self._gpu_lstm_c_buffer = torch.zeros(
            self.num_agents, hidden_size, device=self.device, dtype=torch.float32
        )
        self._gpu_actions_buffer = torch.zeros(
            self.num_agents, device=self.device, dtype=torch.long
        )
        
        print(f"✅ Pre-allocated GPU buffers: obs({self._gpu_obs_buffer.shape}), "
              f"lstm_h({self._gpu_lstm_h_buffer.shape}), "
              f"lstm_c({self._gpu_lstm_c_buffer.shape}), "
              f"actions({self._gpu_actions_buffer.shape})")
    
    def _get_model_hidden_size(self):
        """Extract hidden size from the opponent model"""
        if self.opponent_model is None:
            return 64  # Default fallback
            
        # Try to detect hidden size from model architecture
        if hasattr(self.opponent_model, 'policy'):
            if hasattr(self.opponent_model.policy, 'encoder'):
                if hasattr(self.opponent_model.policy.encoder, '__len__') and len(self.opponent_model.policy.encoder) > 0:
                    first_layer = self.opponent_model.policy.encoder[0]
                    if hasattr(first_layer, 'out_features'):
                        return first_layer.out_features
            if hasattr(self.opponent_model.policy, 'lstm'):
                if hasattr(self.opponent_model.policy.lstm, 'hidden_size'):
                    return self.opponent_model.policy.lstm.hidden_size
        
        return 64  # Default fallback
    
    def _get_model_hidden_size_from_model(self, model):
        """Extract hidden size from any model"""
        # Try to detect hidden size from model architecture
        if hasattr(model, 'policy'):
            if hasattr(model.policy, 'encoder'):
                if hasattr(model.policy.encoder, '__len__') and len(model.policy.encoder) > 0:
                    first_layer = model.policy.encoder[0]
                    if hasattr(first_layer, 'out_features'):
                        return first_layer.out_features
            if hasattr(model.policy, 'lstm'):
                if hasattr(model.policy.lstm, 'hidden_size'):
                    return model.policy.lstm.hidden_size
        
        # Direct LSTM access
        if hasattr(model, 'hidden_size'):
            return model.hidden_size
            
        return 64  # Default fallback
    
    def _get_opponent_action(self, env_idx, obs):
        """DEPRECATED: Use _set_batched_opponent_actions for performance"""
        # This method is no longer used but kept for compatibility
        if self.opponent_model is None:
            raise RuntimeError("DEPRECATED: opponent_model is None - this should not happen!")
        
        with torch.no_grad():
            device = self.device if self.use_gpu_inference else 'cpu'
            obs_tensor = torch.FloatTensor(obs).unsqueeze(0).to(device)
            
            if env_idx >= len(self.opponent_lstm_state):
                raise IndexError(f"env_idx {env_idx} >= {len(self.opponent_lstm_state)}")
            
            lstm_state = self.opponent_lstm_state[env_idx]
            # LSTM states should already be on correct device from initialization
            
            logits, value = self.opponent_model.forward_eval(obs_tensor, lstm_state)
            probs = torch.softmax(logits, dim=-1)
            action = torch.multinomial(probs, 1).item()
            
            return action
    
    def _set_batched_opponent_actions(self):
        """Set opponent actions for all environments in a single batched inference"""
        # Generation 1 self-play: Use global training model
        if self.generation_number == 1 and self.effective_self_play_mode:
            global _current_training_model, _current_training_model_lock
            with _current_training_model_lock:
                if _current_training_model is None:
                    print(f"💥 FATAL: Generation 1 self-play but global training model is None!")
                    print(f"   • Call set_global_training_model() before training")
                    raise RuntimeError("Generation 1 self-play requires global training model!")
                # Use global training model for opponent inference (keep in training mode to avoid CUDA issues)
                current_model = _current_training_model.to(self.device)
                
                with torch.no_grad():
                    # Simple batched inference using current training model
                    obs_tensor = torch.from_numpy(self.observations).float().to(self.device)
                    
                    # Create temporary LSTM state for inference
                    hidden_size = self._get_model_hidden_size_from_model(current_model)
                    temp_lstm_h = torch.zeros(self.num_agents, hidden_size, device=self.device)
                    temp_lstm_c = torch.zeros(self.num_agents, hidden_size, device=self.device)
                    
                    # Forward pass with current training model
                    logits, _ = current_model.forward_eval(obs_tensor, {
                        'lstm_h': temp_lstm_h,
                        'lstm_c': temp_lstm_c
                    })
                    probs = torch.softmax(logits, dim=-1)
                    actions = torch.multinomial(probs, 1).squeeze(-1)
                    
                    # Set opponent actions
                    actions_cpu = actions.cpu().numpy().astype(np.int32)
                    for env_idx in range(self.num_agents):
                        binding.vec_set_opponent_action(self.c_envs, env_idx, int(actions_cpu[env_idx]))
                return
        
        # Generation 2+ or non-self-play: Use loaded opponent model
        if self.opponent_model is None:
            if self.effective_self_play_mode:
                # Generation 2+: Neural opponent expected but model is None - CRASH!
                print(f"💥 FATAL: Neural opponent expected but model is None!")
                print(f"   • Generation: {self.generation_number}")
                print(f"   • Expected opponent generation: {self.opponent_generation}")
                print(f"   • This should NEVER happen - model loading failed!")
                raise RuntimeError(f"Neural opponent required but got None for generation {self.generation_number}")
            else:
                # FALLBACK: 100% fold to prevent any random heuristic agents
                print(f"🚫 FALLBACK: Using 100% fold agent (no random heuristics allowed)")
                for env_idx in range(self.num_agents):
                    binding.vec_set_opponent_action(self.c_envs, env_idx, 0)  # 0 = fold
                return
        
        # ZERO-ALLOCATION GPU INFERENCE - FAST AS FUCK
        with torch.no_grad():
            # Copy observations directly into pre-allocated GPU buffer
            self._gpu_obs_buffer.copy_(
                torch.from_numpy(self.observations).float(), non_blocking=True
            )
            
            # Single batched forward pass (all on GPU, zero allocation)
            logits, _ = self.opponent_model.forward_eval(self._gpu_obs_buffer, {
                'lstm_h': self._gpu_lstm_h_buffer, 
                'lstm_c': self._gpu_lstm_c_buffer
            })
            probs = torch.softmax(logits, dim=-1)
            
            # Sample actions directly into pre-allocated buffer  
            torch.multinomial(probs, 1, out=self._gpu_actions_buffer.view(-1, 1))
            # Actions buffer is already 1D, no squeeze needed
            
            # SKIP LSTM state updates - GPU buffers are authoritative
            # Individual LSTM states are only needed for compatibility, not performance
            
            # Minimal CPU transfer: only actions (async)
            actions_cpu = self._gpu_actions_buffer.cpu()
            
            # TEMPORARY: Use old action loop until C extension is rebuilt
            actions_numpy = actions_cpu.numpy().astype(np.int32)
            for env_idx in range(self.num_agents):
                binding.vec_set_opponent_action(self.c_envs, env_idx, int(actions_numpy[env_idx]))
    
    def step(self, actions):
        """Override step to handle opponent actions in self-play"""
        self.actions[:] = actions
        
        # PERFORMANCE FIX: Batch opponent actions for massive speedup
        if self.opponent_model is not None or (self.generation_number == 1 and self.effective_self_play_mode):
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