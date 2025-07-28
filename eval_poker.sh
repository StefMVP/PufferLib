#!/bin/bash

# PufferLib Poker Evaluation Script
# Usage: ./eval_poker.sh [options]
# Examples:
#   ./eval_poker.sh --generation 10                    # Eval gen 10 vs gen 9
#   ./eval_poker.sh --generation 10 --vs-random        # Eval gen 10 vs random
#   ./eval_poker.sh --generation 10 --vs-generation 5  # Eval gen 10 vs gen 5
#   ./eval_poker.sh --random --vs-generation 10        # Eval random vs gen 10

echo "🎯 PufferLib Poker Evaluation"
echo "============================="

# Default parameters
PLAYER_GENERATION=""
OPPONENT_GENERATION=""
VS_RANDOM=false
PLAYER_RANDOM=false
VS_ALL=false
HANDS_TO_PLAY=200
RENDER_MODE="None"
SEED=42
CONTINUOUS=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --generation)
            PLAYER_GENERATION=$2
            shift 2
            ;;
        --vs-generation)
            OPPONENT_GENERATION=$2
            shift 2
            ;;
        --vs-random)
            VS_RANDOM=true
            shift
            ;;
        --vs-all)
            VS_ALL=true
            shift
            ;;
        --random)
            PLAYER_RANDOM=true
            shift
            ;;
        --hands)
            HANDS_TO_PLAY=$2
            shift 2
            ;;
        --headless)
            RENDER_MODE="None"
            shift
            ;;
        --human)
            RENDER_MODE="human"
            shift
            ;;
        --seed)
            SEED=$2
            shift 2
            ;;
        --continuous)
            CONTINUOUS=true
            HANDS_TO_PLAY=999999999
            shift
            ;;
        --help)
            echo ""
            echo "Usage: $0 [options]"
            echo ""
            echo "Player Options:"
            echo "  --generation N       Use generation N as player"
            echo "  --random            Use random agent as player"
            echo ""
            echo "Opponent Options:"
            echo "  --vs-generation N    Play against generation N"
            echo "  --vs-random         Play against random agent (default if no opponent specified)"
            echo "  --vs-all            Play against all previous generations + random (comprehensive evaluation)"
            echo ""
            echo "Game Options:"
            echo "  --hands N           Number of hands to play (default: 200)"
            echo "  --continuous        Run continuously (basically never stop)"
            echo "  --headless          Run without GUI (default)"
            echo "  --human             Run with GUI"
            echo "  --seed N            Random seed (default: 42)"
            echo ""
            echo "Examples:"
            echo "  $0 --generation 10                    # Gen 10 vs random"
            echo "  $0 --generation 10 --vs-generation 5  # Gen 10 vs Gen 5"
            echo "  $0 --generation 5 --vs-all            # Gen 5 vs all previous + random"
            echo "  $0 --random --vs-generation 10        # Random vs Gen 10"
            echo "  $0 --generation 8 --hands 2000        # Gen 8 vs random, 2000 hands"
            echo ""
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Validate arguments
if [ -z "$PLAYER_GENERATION" ] && [ "$PLAYER_RANDOM" = false ]; then
    echo "❌ ERROR: Must specify either --generation N or --random for player"
    echo "Use --help for usage information"
    exit 1
fi

if [ -n "$PLAYER_GENERATION" ] && [ "$PLAYER_RANDOM" = true ]; then
    echo "❌ ERROR: Cannot specify both --generation and --random for player"
    exit 1
fi

# Set default opponent to random if not specified
if [ -z "$OPPONENT_GENERATION" ] && [ "$VS_RANDOM" = false ] && [ "$VS_ALL" = false ]; then
    VS_RANDOM=true
fi

# Validate --vs-all flag
if [ "$VS_ALL" = true ]; then
    if [ "$VS_RANDOM" = true ] || [ -n "$OPPONENT_GENERATION" ]; then
        echo "❌ ERROR: --vs-all cannot be used with --vs-random or --vs-generation"
        exit 1
    fi
fi

# Activate virtual environment
source .venv/bin/activate

if [[ "$VIRTUAL_ENV" == "" ]]; then
    echo "❌ ERROR: Failed to activate virtual environment"
    exit 1
fi

echo "✅ Virtual environment activated: $VIRTUAL_ENV"

# Build environment
echo "🔨 Building poker environment..."
ENV=poker python setup.py build_ext --inplace --force > /dev/null 2>&1

if [ $? -ne 0 ]; then
    echo "❌ ERROR: Build failed"
    exit 1
fi

echo "✅ Build successful!"

# Function to run a single evaluation
run_single_evaluation() {
    local player_model="$1"
    local player_desc="$2"
    local opponent_desc="$3"
    local self_play_mode="$4"
    local opponent_gen_num="$5"
    local output_suffix="$6"
    
    echo ""
    echo "🥊 Matchup: $player_desc vs $opponent_desc"
    
    # Create evaluation directory
    EVAL_DIR="experiments/evaluations"
    mkdir -p "$EVAL_DIR"
    
    TIMESTAMP=$(date +%Y%m%d_%H%M%S)
    EVAL_LOG="$EVAL_DIR/eval_${output_suffix}_${TIMESTAMP}.log"
    
    # Build evaluation command using simple Python script
    EVAL_CMD="python eval_simple.py"
    
    # Add player model if not random
    if [ -n "$player_model" ]; then
        EVAL_CMD="$EVAL_CMD --player-model $player_model"
    fi
    
    # Add opponent parameters
    if [ "$self_play_mode" = "true" ]; then
        EVAL_CMD="$EVAL_CMD --vs-generation $opponent_gen_num"
    else
        EVAL_CMD="$EVAL_CMD --vs-random"
    fi
    
    EVAL_CMD="$EVAL_CMD --hands $HANDS_TO_PLAY"
    
    # Create evaluation metadata
    cat > "$EVAL_DIR/eval_${output_suffix}_${TIMESTAMP}_info.json" << EOF
{
    "timestamp": "$TIMESTAMP",
    "player": "$player_desc",
    "player_model": "${player_model:-'random'}",
    "opponent": "$opponent_desc",
    "self_play_mode": $self_play_mode,
    "opponent_generation": $opponent_gen_num,
    "hands_to_play": $HANDS_TO_PLAY,
    "render_mode": "None",
    "seed": $SEED,
    "command": "$EVAL_CMD"
}
EOF
    
    echo "📋 Running: $EVAL_CMD"
    echo "📝 Log: $EVAL_LOG"
    
    # Run evaluation
    $EVAL_CMD > "$EVAL_LOG" 2>&1
    
    local exit_code=$?
    
    # Extract final stats from the log
    if [ $exit_code -eq 0 ] && [ -f "$EVAL_LOG" ]; then
        # Extract stats from simple evaluation output
        local result_line=$(grep "RESULT:" "$EVAL_LOG" | tail -1)
        if [ -n "$result_line" ]; then
            # Parse: RESULT: bb100,hands,return
            local stats=${result_line#*: }
            local final_bb100=$(echo $stats | cut -d',' -f1)
            local final_hands=$(echo $stats | cut -d',' -f2)
            local final_return=$(echo $stats | cut -d',' -f3)
        else
            local final_bb100=""
            local final_hands=""
            local final_return=""
        fi
        
        # If no stats found, check if evaluation actually ran
        if [ -z "$final_bb100" ] && [ -z "$final_hands" ]; then
            echo "⚠️  No stats found in log - evaluation may not have completed properly"
            echo "$opponent_desc,NO_STATS,NO_STATS,NO_STATS,$exit_code" >> "/tmp/eval_results_$$"
        else
            echo "📊 Results: BB/100=${final_bb100:-'N/A'}, Hands=${final_hands:-'N/A'}, Return=${final_return:-'N/A'}"
            echo "$opponent_desc,$final_bb100,$final_hands,$final_return,$exit_code" >> "/tmp/eval_results_$$"
        fi
    else
        echo "❌ Evaluation failed with exit code: $exit_code"
        echo "$opponent_desc,FAILED,FAILED,FAILED,$exit_code" >> "/tmp/eval_results_$$"
    fi
}

# Determine player model
if [ "$PLAYER_RANDOM" = true ]; then
    PLAYER_MODEL=""
    PLAYER_DESC="Random Agent"
else
    # Find all generations that have models
    AVAILABLE_GENS=($(find experiments/self_play_poker -name "model_gen*.pt" -exec dirname {} \; | sort -V))
    
    if [ ${#AVAILABLE_GENS[@]} -eq 0 ]; then
        echo "❌ ERROR: No generations with models found"
        exit 1
    fi
    
    if [ $PLAYER_GENERATION -gt ${#AVAILABLE_GENS[@]} ]; then
        echo "❌ ERROR: Generation $PLAYER_GENERATION not found. Available generations: 1-${#AVAILABLE_GENS[@]}"
        echo "Available generations with models:"
        for i in "${!AVAILABLE_GENS[@]}"; do
            echo "  Generation $((i+1)): $(basename ${AVAILABLE_GENS[$i]})"
        done
        exit 1
    fi
    
    PLAYER_DIR="${AVAILABLE_GENS[$((PLAYER_GENERATION-1))]}"
    PLAYER_MODEL=$(find "$PLAYER_DIR" -name "model_gen*.pt" | sort -V | tail -1)
    
    if [ -z "$PLAYER_MODEL" ]; then
        echo "❌ ERROR: No model found in $PLAYER_DIR"
        exit 1
    fi
    
    PLAYER_DESC="Generation $PLAYER_GENERATION ($(basename $PLAYER_DIR))"
fi

# Determine opponent
if [ "$VS_RANDOM" = true ]; then
    OPPONENT_DESC="Random Agent"
    SELF_PLAY_MODE="false"
    OPPONENT_GENERATION_NUM=0
else
    # Use the same available generations array for opponent
    if [ -z "${AVAILABLE_GENS+x}" ]; then
        # If not already set, find available generations
        AVAILABLE_GENS=($(find experiments/self_play_poker -name "model_gen*.pt" -exec dirname {} \; | sort -V))
    fi
    
    if [ -n "$OPPONENT_GENERATION" ] && [ $OPPONENT_GENERATION -gt ${#AVAILABLE_GENS[@]} ]; then
        echo "❌ ERROR: Opponent generation $OPPONENT_GENERATION not found. Available generations: 1-${#AVAILABLE_GENS[@]}"
        exit 1
    fi
    
    OPPONENT_DIR="${AVAILABLE_GENS[$((OPPONENT_GENERATION-1))]}"
    OPPONENT_DESC="Generation $OPPONENT_GENERATION ($(basename $OPPONENT_DIR))"
    SELF_PLAY_MODE="true"
    OPPONENT_GENERATION_NUM=$OPPONENT_GENERATION
fi

# Handle --vs-all mode
if [ "$VS_ALL" = true ]; then
    echo ""
    echo "🏟️  Comprehensive Evaluation: $PLAYER_DESC vs All Opponents"
    echo "========================================="
    echo "Hands per opponent: $HANDS_TO_PLAY"
    echo ""
    
    # Clear results file
    rm -f "/tmp/eval_results_$$"
    echo "Opponent,BB/100,Hands,Episode_Return,Exit_Code" >> "/tmp/eval_results_$$"
    
    # Fight against random first
    echo "🎲 Starting evaluation vs Random Agent..."
    run_single_evaluation "$PLAYER_MODEL" "$PLAYER_DESC" "Random Agent" "false" "0" "vs_random"
    
    # Fight against all previous generations (from generation-1 down to 1)
    if [ -n "${AVAILABLE_GENS+x}" ] && [ ${#AVAILABLE_GENS[@]} -gt 0 ]; then
        # Start from player generation - 1 and work backwards
        for ((i=$((PLAYER_GENERATION-1)); i>=1; i--)); do
            if [ $i -le ${#AVAILABLE_GENS[@]} ]; then
                opponent_dir="${AVAILABLE_GENS[$((i-1))]}"
                opponent_desc="Generation $i ($(basename $opponent_dir))"
                
                echo "🤖 Starting evaluation vs $opponent_desc..."
                run_single_evaluation "$PLAYER_MODEL" "$PLAYER_DESC" "$opponent_desc" "true" "$i" "vs_gen$i"
            fi
        done
    fi
    
    # Display comprehensive results
    echo ""
    echo "📊 COMPREHENSIVE EVALUATION RESULTS"
    echo "====================================="
    echo ""
    printf "%-20s %-10s %-10s %-15s %-6s\n" "Opponent" "BB/100" "Hands" "Episode_Return" "Status"
    echo "--------------------------------------------------------------------"
    
    while IFS=',' read -r opponent bb100 hands return_val exit_code; do
        if [ "$opponent" != "Opponent" ]; then  # Skip header
            if [ "$exit_code" = "0" ]; then
                status="✅ OK"
            else
                status="❌ FAIL"
            fi
            printf "%-20s %-10s %-10s %-15s %-6s\n" "$opponent" "$bb100" "$hands" "$return_val" "$status"
        fi
    done < "/tmp/eval_results_$$"
    
    echo ""
    echo "📁 Detailed logs saved in: experiments/evaluations/"
    echo "📋 Summary results: /tmp/eval_results_$$"
    
    # Cleanup
    rm -f "/tmp/eval_results_$$"
    
    exit 0
fi

# Display matchup for single evaluation
echo ""
echo "🥊 Matchup:"
echo "   Player: $PLAYER_DESC"
echo "   Opponent: $OPPONENT_DESC"
echo "   Hands: $HANDS_TO_PLAY"
echo "   Render: $RENDER_MODE"
echo ""

# Create evaluation directory
EVAL_DIR="experiments/evaluations"
mkdir -p "$EVAL_DIR"

TIMESTAMP=$(date +%Y%m%d_%H%M%S)
EVAL_LOG="$EVAL_DIR/eval_${TIMESTAMP}.log"

# Build evaluation command using simple Python script
EVAL_CMD="python eval_simple.py"

# Add player model if not random
if [ -n "$PLAYER_MODEL" ]; then
    EVAL_CMD="$EVAL_CMD --player-model $PLAYER_MODEL"
fi

# Add opponent parameters
if [ "$SELF_PLAY_MODE" = "true" ]; then
    EVAL_CMD="$EVAL_CMD --vs-generation $OPPONENT_GENERATION_NUM"
else
    EVAL_CMD="$EVAL_CMD --vs-random"
fi

EVAL_CMD="$EVAL_CMD --hands $HANDS_TO_PLAY"

# For continuous mode, set very high values
if [ "$CONTINUOUS" = true ]; then
    echo "🔄 Continuous mode enabled - setting high limits"
    HANDS_TO_PLAY=999999999
    FRAMES_NEEDED=$((HANDS_TO_PLAY * 20))
    # Rebuild the command with new values
    EVAL_CMD="puffer eval puffer_poker"
    if [ -n "$PLAYER_MODEL" ]; then
        EVAL_CMD="$EVAL_CMD --load-model-path $PLAYER_MODEL"
    fi
    EVAL_CMD="$EVAL_CMD --env.self-play-mode $SELF_PLAY_MODE"
    EVAL_CMD="$EVAL_CMD --env.generation-number $OPPONENT_GENERATION_NUM"
    EVAL_CMD="$EVAL_CMD --env.max-episode-length $HANDS_TO_PLAY"
    EVAL_CMD="$EVAL_CMD --save-frames $FRAMES_NEEDED"
    EVAL_CMD="$EVAL_CMD --render-mode $RENDER_MODE"
fi

echo "🚀 Starting evaluation..."
echo "📋 Command: $EVAL_CMD"
echo "📝 Log file: $EVAL_LOG"
echo ""

# Create evaluation metadata
cat > "$EVAL_DIR/eval_${TIMESTAMP}_info.json" << EOF
{
    "timestamp": "$TIMESTAMP",
    "player": "$PLAYER_DESC",
    "player_model": "${PLAYER_MODEL:-'random'}",
    "opponent": "$OPPONENT_DESC",
    "self_play_mode": $SELF_PLAY_MODE,
    "opponent_generation": $OPPONENT_GENERATION_NUM,
    "hands_to_play": $HANDS_TO_PLAY,
    "render_mode": "$RENDER_MODE",
    "seed": $SEED,
    "command": "$EVAL_CMD"
}
EOF

# Run evaluation
$EVAL_CMD 2>&1 | tee "$EVAL_LOG"

EVAL_EXIT_CODE=$?

if [ $EVAL_EXIT_CODE -eq 0 ]; then
    echo ""
    echo "🎉 Evaluation completed successfully!"
    echo "📁 Results saved to: $EVAL_LOG"
    
    # Extract final stats if available
    if [ -f "$EVAL_LOG" ]; then
        echo ""
        echo "📊 Final Statistics:"
        echo "==================="
        
        # Extract stats from simple evaluation output
        result_line=$(grep "RESULT:" "$EVAL_LOG" | tail -1)
        if [ -n "$result_line" ]; then
            # Parse: RESULT: bb100,hands,return
            stats=${result_line#*: }
            FINAL_BB100=$(echo $stats | cut -d',' -f1)
            FINAL_HANDS=$(echo $stats | cut -d',' -f2)
            FINAL_RETURN=$(echo $stats | cut -d',' -f3)
        fi
        
        if [ -n "$FINAL_BB100" ]; then
            echo "💰 Final BB/100: $FINAL_BB100"
        fi
        if [ -n "$FINAL_HANDS" ]; then
            echo "🃏 Hands Played: $FINAL_HANDS"
        fi
        if [ -n "$FINAL_RETURN" ]; then
            echo "📈 Episode Return: $FINAL_RETURN"
        fi
    fi
else
    echo ""
    echo "❌ Evaluation failed with exit code: $EVAL_EXIT_CODE"
    echo "📋 Check logs: $EVAL_LOG"
fi

echo ""
echo "🔗 Next steps:"
echo "  📊 Review detailed logs: $EVAL_LOG"
echo "  🔄 Run another evaluation: $0 --help"

exit $EVAL_EXIT_CODE