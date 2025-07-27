#!/usr/bin/env python3
"""
PufferLib Self-Play Generation Manager

This utility helps manage and analyze self-play training generations.
"""

import os
import json
import argparse
from pathlib import Path
from datetime import datetime
import pandas as pd


def find_generations(env_name):
    """Find all self-play generations for an environment."""
    base_dir = Path(f"experiments/self_play_{env_name}")
    if not base_dir.exists():
        return []
    
    generations = []
    for gen_dir in base_dir.glob("gen_*"):
        if gen_dir.is_dir():
            info_file = gen_dir / "training_info.json"
            if info_file.exists():
                try:
                    with open(info_file) as f:
                        info = json.load(f)
                    info['path'] = str(gen_dir)
                    generations.append(info)
                except Exception as e:
                    print(f"Warning: Could not read {info_file}: {e}")
    
    return sorted(generations, key=lambda x: x.get('start_time', ''))


def list_generations(env_name):
    """List all generations with their stats."""
    generations = find_generations(env_name)
    
    if not generations:
        print(f"No self-play generations found for {env_name}")
        return
    
    print(f"🐡 Self-Play Generations for {env_name}")
    print("=" * 60)
    print()
    
    for i, gen in enumerate(generations):
        status_emoji = "✅" if gen.get('status') == 'completed' else "❌" if gen.get('status') == 'failed' else "🔄"
        opponent = gen.get('opponent_mode', 'unknown')
        
        print(f"{status_emoji} Generation {i+1}: {gen['generation']}")
        print(f"   📅 Started: {gen.get('start_time', 'unknown')}")
        print(f"   🤖 Opponent: {opponent}")
        if gen.get('end_time'):
            print(f"   🏁 Ended: {gen['end_time']}")
        print(f"   📁 Path: {gen['path']}")
        
        # Count models
        gen_path = Path(gen['path'])
        models = list(gen_path.glob("model_*.pt"))
        print(f"   🎯 Models: {len(models)}")
        print()


def get_latest_generation(env_name):
    """Get the latest completed generation."""
    generations = find_generations(env_name)
    completed = [g for g in generations if g.get('status') == 'completed']
    return completed[-1] if completed else None


def analyze_generation(gen_path):
    """Analyze a specific generation's performance."""
    gen_path = Path(gen_path)
    
    if not gen_path.exists():
        print(f"❌ Generation directory not found: {gen_path}")
        return
    
    info_file = gen_path / "training_info.json"
    if not info_file.exists():
        print(f"❌ Training info not found: {info_file}")
        return
    
    with open(info_file) as f:
        info = json.load(f)
    
    print(f"🔍 Analyzing Generation: {info['generation']}")
    print("=" * 50)
    print(f"Environment: {info['environment']}")
    print(f"Status: {info.get('status', 'unknown')}")
    print(f"Started: {info.get('start_time', 'unknown')}")
    print(f"Opponent: {info.get('opponent_mode', 'unknown')}")
    print()
    
    # Count models
    models = sorted(gen_path.glob("model_*.pt"))
    print(f"📚 Models Generated: {len(models)}")
    if models:
        print(f"   First: {models[0].name}")
        print(f"   Latest: {models[-1].name}")
    print()
    
    # Check for log file and extract final stats
    log_file = gen_path / "training.log"
    if log_file.exists():
        print("📊 Final Training Stats:")
        try:
            with open(log_file) as f:
                lines = f.readlines()
            
            # Look for the last stats block
            for line in reversed(lines[-100:]):  # Check last 100 lines
                if "bb_per_100" in line:
                    # Parse key stats from the line
                    if "episode_return" in line:
                        try:
                            # This is a rough parser - could be improved
                            parts = line.strip().split()
                            for i, part in enumerate(parts):
                                if part == "bb_per_100" and i+1 < len(parts):
                                    print(f"   💰 BB/100: {parts[i+1]}")
                                elif part == "hands_played" and i+1 < len(parts):
                                    print(f"   🃏 Hands: {parts[i+1]}")
                                elif part == "vpip_hands" and i+1 < len(parts):
                                    print(f"   📈 VPIP: {parts[i+1]}")
                        except:
                            pass
                    break
        except Exception as e:
            print(f"   Could not parse log file: {e}")
    print()


def compare_generations(env_name):
    """Compare performance across generations."""
    generations = find_generations(env_name)
    completed = [g for g in generations if g.get('status') == 'completed']
    
    if len(completed) < 2:
        print(f"Need at least 2 completed generations to compare (found {len(completed)})")
        return
    
    print(f"📊 Generation Comparison for {env_name}")
    print("=" * 60)
    print()
    
    for i, gen in enumerate(completed):
        print(f"Gen {i+1}: {gen['generation']}")
        print(f"  Opponent: {gen.get('opponent_mode', 'unknown')}")
        analyze_generation(gen['path'])
        print("-" * 40)


def create_tournament_config(env_name, num_generations=None):
    """Create a tournament configuration to test generations against each other."""
    generations = find_generations(env_name)
    completed = [g for g in generations if g.get('status') == 'completed']
    
    if len(completed) < 2:
        print(f"Need at least 2 completed generations for tournament (found {len(completed)})")
        return
    
    if num_generations:
        completed = completed[-num_generations:]
    
    tournament_dir = Path(f"experiments/tournament_{env_name}_{datetime.now().strftime('%Y%m%d_%H%M%S')}")
    tournament_dir.mkdir(parents=True, exist_ok=True)
    
    tournament_config = {
        "environment": env_name,
        "participants": [],
        "created": datetime.now().isoformat()
    }
    
    for i, gen in enumerate(completed):
        gen_path = Path(gen['path'])
        models = list(gen_path.glob("model_*.pt"))
        if models:
            latest_model = sorted(models)[-1]
            participant = {
                "name": f"gen_{i+1}_{gen['generation']}",
                "generation": gen['generation'],
                "model_path": str(latest_model),
                "training_info": gen
            }
            tournament_config["participants"].append(participant)
    
    config_file = tournament_dir / "tournament_config.json"
    with open(config_file, 'w') as f:
        json.dump(tournament_config, f, indent=2)
    
    print(f"🏆 Tournament configuration created: {config_file}")
    print(f"Participants: {len(tournament_config['participants'])}")
    for p in tournament_config['participants']:
        print(f"  - {p['name']}")


def main():
    parser = argparse.ArgumentParser(description="Manage PufferLib self-play generations")
    parser.add_argument("env_name", help="Environment name (e.g., poker)")
    parser.add_argument("--list", "-l", action="store_true", help="List all generations")
    parser.add_argument("--analyze", "-a", help="Analyze specific generation path")
    parser.add_argument("--compare", "-c", action="store_true", help="Compare all generations")
    parser.add_argument("--latest", action="store_true", help="Show latest generation info")
    parser.add_argument("--tournament", "-t", type=int, metavar="N", help="Create tournament with last N generations")
    
    args = parser.parse_args()
    
    if args.list:
        list_generations(args.env_name)
    elif args.analyze:
        analyze_generation(args.analyze)
    elif args.compare:
        compare_generations(args.env_name)
    elif args.latest:
        latest = get_latest_generation(args.env_name)
        if latest:
            print(f"Latest generation: {latest['path']}")
            analyze_generation(latest['path'])
        else:
            print(f"No completed generations found for {args.env_name}")
    elif args.tournament:
        create_tournament_config(args.env_name, args.tournament)
    else:
        print("Use --help for available commands")
        list_generations(args.env_name)


if __name__ == "__main__":
    main()