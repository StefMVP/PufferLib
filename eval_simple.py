#!/usr/bin/env python3
import sys
import argparse
import numpy as np
import torch
from pufferlib.ocean.poker import poker
from pufferlib.models import Default, LSTMWrapper

def load_model(model_path, env):
    """Load a trained model from the given path"""
    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    
    if model_path:
        try:
            # Recreate the exact architecture used during training
            policy = Default(env, hidden_size=32)
            policy = LSTMWrapper(env, policy, input_size=32, hidden_size=32)
            
            # Load the trained weights
            state_dict = torch.load(model_path, map_location=device)
            state_dict = {k.replace('module.', ''): v for k, v in state_dict.items()}
            policy.load_state_dict(state_dict)
            policy.to(device)
            policy.eval()
            
            # Initialize LSTM state
            lstm_state = {
                'lstm_h': torch.zeros(1, 32, device=device),
                'lstm_c': torch.zeros(1, 32, device=device)
            }
            
            return policy, lstm_state, device
        except Exception as e:
            print(f"Warning: Failed to load model from {model_path}: {e}")
            return None, None, device
    return None, None, device

def get_action_from_model(model, lstm_state, obs, device):
    """Get action from trained model"""
    if model is None:
        return np.random.randint(0, 7)
    
    try:
        with torch.no_grad():
            obs_tensor = torch.FloatTensor(obs).unsqueeze(0).to(device)
            logits, value = model.forward_eval(obs_tensor, lstm_state)
            probs = torch.softmax(logits, dim=-1)
            action = torch.multinomial(probs, 1).item()
            return action
    except Exception as e:
        print(f"Warning: Model inference failed: {e}")
        return np.random.randint(0, 7)

def evaluate_matchup(player_model_path, self_play_mode, opponent_generation, hands, seed=42):
    """Run evaluation and return BB/100 using trained models"""
    
    # Create environment with correct generation setup
    # When self_play_mode=True, environment loads generation_number-1 as opponent
    # So to get opponent_generation as opponent, we need generation_number = opponent_generation + 1
    if self_play_mode:
        env_generation = opponent_generation + 1
        print(f"Setting up evaluation: environment gen {env_generation} to load opponent gen {opponent_generation}")
    else:
        env_generation = 1  # For random opponents
    
    env = poker.Poker(
        num_envs=1,
        render_mode='None',
        self_play_mode=self_play_mode,
        generation_number=env_generation,
        max_episode_length=hands * 100,  # Allow plenty of steps per hand
        report_interval=1,
        seed=seed
    )
    
    # Load player model
    player_model, lstm_state, device = load_model(player_model_path, env)
    
    # Reset environment
    obs, _ = env.reset()
    
    total_return = 0.0
    total_hands = 0
    
    step_count = 0
    hands_played = 0
    
    while hands_played < hands:
        # Get action from trained model or random
        action = get_action_from_model(player_model, lstm_state, obs[0], device)
        actions = [action]
        obs, rewards, terms, truncs, info = env.step(actions)
        
        total_return += rewards[0]
        step_count += 1
        
        # Check if this was the end of a hand (reward was given)
        if rewards[0] != 0.0:
            hands_played += 1
        
        if terms[0] or truncs[0] or step_count > hands * 50:
            break
    
    # Clean up environment and models before calculating results
    try:
        env.close()
    except:
        pass
    
    # Clear model references to prevent cleanup issues
    if player_model is not None:
        try:
            del player_model
        except:
            pass
    
    # Calculate BB/100 
    if hands_played > 0:
        bb_per_100 = (total_return / 2.0) * 100.0 / hands_played  # big_blind = 2
        return bb_per_100, hands_played, total_return
    else:
        return 0.0, 0, 0.0

def main():
    parser = argparse.ArgumentParser(description='Simple poker evaluation')
    parser.add_argument('--player-model', type=str, help='Path to player model')
    parser.add_argument('--vs-generation', type=int, help='Opponent generation number (default: player generation - 1)')
    parser.add_argument('--vs-random', action='store_true', help='Play against random opponent')
    parser.add_argument('--hands', type=int, default=200, help='Number of hands to play')
    parser.add_argument('--seed', type=int, default=42, help='Random seed')
    
    args = parser.parse_args()
    
    # Determine player generation from model path
    player_generation = None
    if args.player_model:
        import re
        match = re.search(r'gen(\d+)', args.player_model)
        if match:
            player_generation = int(match.group(1))
    
    # Determine opponent
    if args.vs_random:
        self_play_mode = False
        opponent_generation = 0
        opponent_name = "Random"
    else:
        self_play_mode = True
        if args.vs_generation is not None:
            opponent_generation = args.vs_generation
        elif player_generation is not None:
            opponent_generation = player_generation - 1
        else:
            opponent_generation = 1
        opponent_name = f"Generation {opponent_generation}"
    
    player_name = "Random" if not args.player_model else f"Generation {player_generation}" if player_generation else f"Model ({args.player_model.split('/')[-1]})"
    
    print(f"🥊 {player_name} vs {opponent_name}")
    print(f"🃏 Playing {args.hands} hands...")
    
    try:
        bb_100, hands_played, total_return = evaluate_matchup(
            args.player_model,
            self_play_mode,
            opponent_generation,
            args.hands,
            args.seed
        )
        
        print(f"📊 Results:")
        print(f"   BB/100: {bb_100:.1f}")
        print(f"   Hands: {hands_played}")
        print(f"   Total Return: {total_return:.1f}")
        
        # Output in format that eval script can parse
        print(f"RESULT: {bb_100:.1f},{hands_played},{total_return:.1f}")
        
    except Exception as e:
        print(f"❌ Error: {e}")
        sys.exit(1)
    finally:
        # Force cleanup to prevent segfaults
        import gc
        gc.collect()
        
        # Clear any remaining CUDA cache
        try:
            import torch
            if torch.cuda.is_available():
                torch.cuda.empty_cache()
        except:
            pass

if __name__ == "__main__":
    main()