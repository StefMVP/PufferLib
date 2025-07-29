#!/bin/bash

# PufferLib Self-Play Training System with Auto-Evolution
# Usage: ./train_self_play.sh <env_name> [options]
# Examples:
#   ./train_self_play.sh poker                    # Single generation
#   ./train_self_play.sh poker --evolve           # Continuous evolution
#   ./train_self_play.sh poker --evolve 5         # Evolve 5 generations
#   ./train_self_play.sh poker --wandb --evolve   # With WandB logging

if [ $# -eq 0 ]; then
    echo "Usage: $0 <env_name> [options]"
    echo ""
    echo "Options:"
    echo "  --evolve [N]     Continuously evolve for N generations (unlimited if N not specified)"
    echo "  --wandb          Enable Weights & Biases logging"
    echo "  --new            Start fresh from generation 1 (ignore existing generations)"
    echo "  opponent_dir     Use specific opponent directory"
    echo ""
    echo "Examples:"
    echo "  $0 poker                           # Train one generation"
    echo "  $0 poker --evolve                  # Continuous evolution"
    echo "  $0 poker --evolve 10 --wandb       # Evolve 10 gens with WandB"
    echo "  $0 poker --new --evolve            # Start fresh evolution"
    echo "  $0 poker experiments/gen_xyz       # Train vs specific opponent"
    exit 1
fi

ENV_NAME=$1
shift  # Remove env_name from arguments

# Parse arguments
EVOLVE_MODE=false
MAX_GENERATIONS=999999
WANDB_FLAG=""
OPPONENT_DIR=""
START_NEW=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --evolve)
            EVOLVE_MODE=true
            # Check if next argument is a number
            if [[ $2 =~ ^[0-9]+$ ]]; then
                MAX_GENERATIONS=$2
                shift 2
            else
                shift
            fi
            ;;
        --wandb)
            WANDB_FLAG="--wandb"
            shift
            ;;
        --new)
            START_NEW=true
            shift
            ;;
        experiments/*)
            OPPONENT_DIR=$1
            shift
            ;;
        *)
            # Assume it's an opponent directory if it exists
            if [ -d "$1" ]; then
                OPPONENT_DIR=$1
            fi
            shift
            ;;
    esac
done

if [ "$EVOLVE_MODE" = true ]; then
    echo "🧬 PufferLib Auto-Evolution Self-Play"
    echo "====================================="
    echo "Environment: $ENV_NAME"
    echo "Max Generations: $MAX_GENERATIONS"
    echo "WandB: ${WANDB_FLAG:-'disabled'}"
    echo ""
else
    echo "🐡 PufferLib Self-Play Training"
    echo "================================"
    echo "Environment: $ENV_NAME"
    echo "Mode: Single Generation"
    echo "WandB: ${WANDB_FLAG:-'disabled'}"
    echo ""
fi

# Activate virtual environment
source .venv/bin/activate

if [[ "$VIRTUAL_ENV" == "" ]]; then
    echo "❌ ERROR: Failed to activate virtual environment"
    exit 1
fi

echo "✅ Virtual environment activated: $VIRTUAL_ENV"

# Build the environment once at startup
echo "🔨 Building $ENV_NAME environment..."
ENV=$ENV_NAME python setup.py build_ext --inplace --force

if [ $? -ne 0 ]; then
    echo "❌ ERROR: Build failed for environment: $ENV_NAME"
    exit 1
fi

echo "✅ Build successful!"

# Function to run a single generation
run_generation() {
    local gen_count=$1
    
    echo ""
    if [ "$EVOLVE_MODE" = true ]; then
        echo "🚀 Starting Generation $gen_count of $MAX_GENERATIONS"
        echo "======================================================="
    fi
    
    # Create self-play experiments directory
    SELF_PLAY_DIR="experiments/self_play_$ENV_NAME"
    mkdir -p "$SELF_PLAY_DIR"

    # Find opponent directory if not specified
    if [ -z "$OPPONENT_DIR" ]; then
        echo "🔍 Auto-detecting opponent directory for generation $gen_count..."
        
        if [ $gen_count -eq 1 ]; then
            echo "🎯 Generation 1: Pure self-play mode (same model for both players)."
            OPPONENT_MODE="self_play"
            OPPONENT_GENERATION="1"
        else
            # Walk backwards from current-1 down to 1 to find the most recent valid generation
            OPPONENT_DIR=""
            OPPONENT_MODE=""
            OPPONENT_GENERATION=""
            
            echo "🔍 Searching backwards from generation $((gen_count - 1)) down to 1..."
            for target_gen in $(seq $((gen_count - 1)) -1 1); do
                echo "🎯 Looking for generation $target_gen as opponent..."
                
                candidate_dir=$(find experiments -path "*/self_play_${ENV_NAME}/gen${target_gen}_*" -type d | sort -V | tail -1)
                if [ -n "$candidate_dir" ]; then
                    # Check if this generation has a model file
                    if [ -f "$candidate_dir/model_gen${target_gen}.pt" ]; then
                        OPPONENT_DIR="$candidate_dir"
                        OPPONENT_GENERATION="$target_gen"
                        echo "📁 Found valid opponent generation $target_gen: $OPPONENT_DIR"
                        OPPONENT_MODE="self_play"
                        break
                    else
                        echo "⚠️  Generation $target_gen directory exists but no model file found"
                    fi
                else
                    echo "❌ Generation $target_gen not found"
                fi
            done
            
            if [ -z "$OPPONENT_DIR" ]; then
                echo "💥 FATAL: No valid opponent generations found with model files!"
                echo "Available directories:"
                find experiments -path "*/self_play_${ENV_NAME}/gen*_*" -type d | sort -V
                echo "Use --new flag for first generation with random opponent"
                exit 1
            fi
        fi
    else
        if [ ! -d "$OPPONENT_DIR" ]; then
            echo "❌ ERROR: Opponent directory not found: $OPPONENT_DIR"
            exit 1
        fi
        OPPONENT_MODE="self_play"
    fi

    # Create generation metadata
    GENERATION=$(date +%Y%m%d_%H%M%S)
    GEN_DIR="$SELF_PLAY_DIR/gen${gen_count}_$GENERATION"
    mkdir -p "$GEN_DIR"

    echo "🎯 Training Generation: $GENERATION"
    echo "📂 Output Directory: $GEN_DIR"

    # Prepare self-play mode flag
    if [ "$OPPONENT_MODE" = "self_play" ]; then
        SELF_PLAY_FLAG="--env.self-play-mode true"
        echo "🤖 Enabled self-play mode"
    else
        SELF_PLAY_FLAG="--env.self-play-mode false"
        echo "🎲 Using random opponent (no previous models found)"
    fi

    # Create training metadata
    cat > "$GEN_DIR/training_info.json" << EOF
{
    "generation": "$GENERATION",
    "generation_number": $gen_count,
    "environment": "$ENV_NAME",
    "opponent_dir": "${OPPONENT_DIR:-'random'}",
    "opponent_mode": "$OPPONENT_MODE",
    "evolve_mode": $EVOLVE_MODE,
    "max_generations": $MAX_GENERATIONS,
    "start_time": "$(date -Iseconds)",
    "self_play_flag": "$SELF_PLAY_FLAG",
    "command": "$0 $ENV_NAME ${WANDB_FLAG} $([ "$EVOLVE_MODE" = true ] && echo "--evolve $MAX_GENERATIONS")"
}
EOF

    # Store opponent info if using self-play
    if [ "$OPPONENT_MODE" = "self_play" ] && [ -n "$OPPONENT_DIR" ]; then
        echo "📋 Recording opponent information..."
        
        # Find the latest model in opponent directory
        LATEST_OPPONENT=$(find "$OPPONENT_DIR" -name "model_*.pt" | sort -V | tail -1)
        if [ -n "$LATEST_OPPONENT" ]; then
            cp "$LATEST_OPPONENT" "$GEN_DIR/opponent_model.pt"
            echo "📁 Copied opponent model: $(basename $LATEST_OPPONENT)"
        fi
        
        # Copy opponent stats if available
        if [ -f "$OPPONENT_DIR/training_info.json" ]; then
            cp "$OPPONENT_DIR/training_info.json" "$GEN_DIR/opponent_info.json"
        fi
    fi

    # Run training with self-play configuration
    echo ""
    echo "🚀 Starting self-play training..."
    if [ "$EVOLVE_MODE" = true ]; then
        echo "🧬 Evolution Mode: Generation $gen_count"
    fi
    echo "⏱️  Training will save checkpoints every 200 epochs"
    echo "📊 Monitor progress with: tail -f $GEN_DIR/training.log"
    echo ""

    # Add generation info to training command for logs
    TRAIN_TAG="--tag gen_${gen_count}_${GENERATION}"
    GEN_NUMBER_FLAG="--env.generation-number $gen_count"
    
    # Opponent generation is auto-detected by the environment
    OPPONENT_GEN_FLAG=""
    if [ -n "$OPPONENT_GENERATION" ]; then
        echo "🎯 Using opponent generation: $OPPONENT_GENERATION (auto-detected)"
    fi
    
    if [ "$WANDB_FLAG" = "--wandb" ]; then
        echo "📈 WandB logging enabled"
        puffer train puffer_$ENV_NAME $SELF_PLAY_FLAG $GEN_NUMBER_FLAG $OPPONENT_GEN_FLAG $TRAIN_TAG $WANDB_FLAG 2>&1 | tee "$GEN_DIR/training.log"
    else
        puffer train puffer_$ENV_NAME $SELF_PLAY_FLAG $GEN_NUMBER_FLAG $OPPONENT_GEN_FLAG $TRAIN_TAG 2>&1 | tee "$GEN_DIR/training.log"
    fi

    TRAINING_EXIT_CODE=$?

    # Update training metadata with completion info
    python3 -c "
import json
from datetime import datetime

with open('$GEN_DIR/training_info.json', 'r') as f:
    info = json.load(f)

info['end_time'] = datetime.now().isoformat()
info['exit_code'] = $TRAINING_EXIT_CODE
info['status'] = 'completed' if $TRAINING_EXIT_CODE == 0 else 'failed'

with open('$GEN_DIR/training_info.json', 'w') as f:
    json.dump(info, f, indent=2)
"

    if [ $TRAINING_EXIT_CODE -eq 0 ]; then
        echo ""
        echo "🎉 Generation $gen_count completed successfully!"
        echo "📁 Results saved to: $GEN_DIR"
        
        # Extract final stats
        if [ -f "$GEN_DIR/training.log" ]; then
            FINAL_BB100=$(tail -100 "$GEN_DIR/training.log" | grep -o 'bb_per_100[[:space:]]*[0-9.-]*' | tail -1 | awk '{print $2}')
            FINAL_HANDS=$(tail -100 "$GEN_DIR/training.log" | grep -o 'hands_played[[:space:]]*[0-9.-]*' | tail -1 | awk '{print $2}')
            
            if [ -n "$FINAL_BB100" ]; then
                echo "💰 Final BB/100: $FINAL_BB100"
            fi
            if [ -n "$FINAL_HANDS" ]; then
                echo "🃏 Hands Played: $FINAL_HANDS"
            fi
        fi
        
        # Copy the final trained model to generation directory
        echo "📚 Copying trained model to generation directory..."
        LATEST_MODEL_DIR=$(find experiments -name "puffer_${ENV_NAME}_*" -type d | sort -t_ -k3 -n | tail -1)
        
        if [ -n "$LATEST_MODEL_DIR" ] && [ -d "$LATEST_MODEL_DIR" ]; then
            LATEST_MODEL=$(find "$LATEST_MODEL_DIR" -name "model_puffer_${ENV_NAME}_*.pt" | sort -V | tail -1)
            if [ -n "$LATEST_MODEL" ]; then
                cp "$LATEST_MODEL" "$GEN_DIR/model_gen${gen_count}.pt"
                echo "🤖 Saved model: model_gen${gen_count}.pt"
                echo "📁 Source: $LATEST_MODEL"
            else
                echo "⚠️  Warning: No trained model found in $LATEST_MODEL_DIR"
            fi
        else
            echo "⚠️  Warning: No training directory found for this generation"
        fi
        
        # List available models
        echo "📚 Generation models:"
        find "$GEN_DIR" -name "model_*.pt" | sort -V | while read model; do
            echo "  🤖 $(basename $model)"
        done
        
        # Reset OPPONENT_DIR for next generation (auto-detect)
        OPPONENT_DIR=""
        
        return 0
    else
        echo ""
        echo "❌ Generation $gen_count failed with exit code: $TRAINING_EXIT_CODE"
        echo "📋 Check logs: $GEN_DIR/training.log"
        return $TRAINING_EXIT_CODE
    fi
}

# Main execution
if [ "$EVOLVE_MODE" = true ]; then
    # Evolution mode - multiple generations
    CONSECUTIVE_FAILURES=0
    MAX_CONSECUTIVE_FAILURES=3
    
    # Detect last completed generation to continue from (unless --new flag is used)
    SELF_PLAY_DIR="experiments/self_play_$ENV_NAME"
    if [ "$START_NEW" = true ]; then
        GENERATION_COUNT=0
        echo "🆕 Starting fresh evolution (--new flag specified)"
    elif [ -d "$SELF_PLAY_DIR" ]; then
        echo "🔍 Checking for previous generations..."
        LAST_COMPLETED_GEN=$(find "$SELF_PLAY_DIR" -name "training_info.json" -exec grep -l '"status": "completed"' {} \; | sed 's|.*/gen_[0-9_]*||' | wc -l)
        if [ $LAST_COMPLETED_GEN -gt 0 ]; then
            GENERATION_COUNT=$LAST_COMPLETED_GEN
            echo "📊 Found $LAST_COMPLETED_GEN completed generations. Continuing from generation $((GENERATION_COUNT + 1))"
        else
            GENERATION_COUNT=0
            echo "📊 No completed generations found. Starting fresh."
        fi
    else
        GENERATION_COUNT=0
    fi
    
    # Create evolution log
    EVOLUTION_DIR="experiments/evolution_$ENV_NAME"
    mkdir -p "$EVOLUTION_DIR"
    EVOLUTION_LOG="$EVOLUTION_DIR/evolution_$(date +%Y%m%d_%H%M%S).log"
    
    echo "📜 Evolution log: $EVOLUTION_LOG"
    
    # Set up signal handler for graceful shutdown
    trap 'echo ""; echo "🛑 Evolution interrupted by user. Last completed generation preserved."; exit 130' INT TERM
    
    while [ $GENERATION_COUNT -lt $MAX_GENERATIONS ]; do
        GENERATION_COUNT=$((GENERATION_COUNT + 1))
        
        echo "Generation $GENERATION_COUNT started at $(date)" | tee -a "$EVOLUTION_LOG"
        
        run_generation $GENERATION_COUNT
        EXIT_CODE=$?
        
        if [ $EXIT_CODE -eq 0 ]; then
            echo "✅ Generation $GENERATION_COUNT completed successfully!" | tee -a "$EVOLUTION_LOG"
            CONSECUTIVE_FAILURES=0
            
            echo "🔄 Preparing for next generation (gen $((GENERATION_COUNT + 1)) vs gen $GENERATION_COUNT)..."
            echo "⏳ Waiting 3 seconds before continuing evolution..."
            sleep 3
        else
            echo "❌ Generation $GENERATION_COUNT failed with exit code $EXIT_CODE!" | tee -a "$EVOLUTION_LOG"
            echo "📋 Last 20 lines of training log:" | tee -a "$EVOLUTION_LOG"
            tail -20 "$GEN_DIR/training.log" | tee -a "$EVOLUTION_LOG"
            
            echo "🛑 Evolution stopped due to training failure. Fix the issue before restarting." | tee -a "$EVOLUTION_LOG"
            echo "🔍 Check the training logs for error details." | tee -a "$EVOLUTION_LOG"
            exit $EXIT_CODE
        fi
    done
    
    echo ""
    echo "🏁 Evolution completed! Generated $GENERATION_COUNT generations."
    echo "📜 Full log: $EVOLUTION_LOG"
    
else
    # Single generation mode
    run_generation 1
    EXIT_CODE=$?
    
    echo ""
    echo "🔗 Next steps:"
    echo "  🔍 Review training: $GEN_DIR/training.log"
    echo "  🎯 Continue evolution: ./train_self_play.sh $ENV_NAME --evolve"
    echo "  🏟️  Evaluate models: puffer eval puffer_$ENV_NAME --load_model_path $GEN_DIR/model_*_final.pt"
    
    exit $EXIT_CODE
fi