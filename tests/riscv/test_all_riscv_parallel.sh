#!/bin/bash

# Comprehensive test script for all RISC-V extensions with parallelization
# Tests base spec (rv32i) and all extensions

cd "$(dirname "$0")/../.."
GOODASM="./build/goodasm"
ERRORS=()
FAILED_TESTS=()
PASSED_TESTS=()
TOTAL_TESTS=0
TOTAL_PASSED=0
TOTAL_FAILED=0

# Determine number of parallel jobs (use number of CPU cores, but cap at 8)
MAX_JOBS=4
if command -v sysctl &> /dev/null 2>&1; then
    # macOS - try to get CPU count, but don't fail if it doesn't work
    CPU_COUNT=$(sysctl -n hw.ncpu 2>/dev/null || echo "4")
    MAX_JOBS=$CPU_COUNT
elif command -v nproc &> /dev/null 2>&1; then
    # Linux
    MAX_JOBS=$(nproc 2>/dev/null || echo "4")
fi
# Cap at 8 to avoid overwhelming the system, minimum of 2
if [ "$MAX_JOBS" -lt 2 ]; then
    MAX_JOBS=2
elif [ "$MAX_JOBS" -gt 32 ]; then
    MAX_JOBS=32
fi
echo "Using $MAX_JOBS parallel jobs"

# Check if goodasm exists
if [ ! -f "$GOODASM" ]; then
    echo "Error: goodasm not found at $GOODASM"
    exit 1
fi

# Function to test a single .asm file
test_file() {
    local asm_file="$1"
    local extension="$2"
    
    if [ ! -f "$asm_file" ]; then
        echo "FAIL:$extension:$(basename "$asm_file" .asm):File not found"
        return 1
    fi
    
    local base_name=$(basename "$asm_file" .asm)
    local dir=$(dirname "$asm_file")
    local bin_file="$dir/${base_name}.bin"
    local disasm_file="$dir/${base_name}.disasm"
    
    # Assemble the file
    if ! "$GOODASM" --riscv32 "$asm_file" -o "$bin_file" > /dev/null 2>&1; then
        echo "FAIL:$extension:$base_name:Assembly failed"
        return 1
    fi
    
    # Check if binary was created and is not empty
    if [ ! -s "$bin_file" ]; then
        echo "FAIL:$extension:$base_name:Empty binary file"
        return 1
    fi
    
    # Disassemble the binary
    if ! "$GOODASM" --riscv32 -d "$bin_file" > "$disasm_file" 2>&1; then
        echo "FAIL:$extension:$base_name:Disassembly failed"
        return 1
    fi
    
    # Extract all instructions from the original .asm file
    local original_instrs=()
    while IFS= read -r line; do
        if [[ "$line" =~ ^[[:space:]]*$ ]] || \
           [[ "$line" =~ ^[[:space:]]*\.lang ]] || \
           [[ "$line" =~ ^[[:space:]]*\.org ]] || \
           [[ "$line" =~ ^[[:space:]]*\; ]] || \
           [[ "$line" =~ ^[[:space:]]*[a-zA-Z_][a-zA-Z0-9_]*:[[:space:]]*$ ]]; then
            continue
        fi
        local trimmed=$(echo "$line" | sed 's/^[[:space:]]*//' | sed 's/[[:space:]]*$//')
        if [ -n "$trimmed" ]; then
            original_instrs+=("$trimmed")
        fi
    done < "$asm_file"
    
    # Get all disassembled instructions (non-empty lines, excluding collision debug messages)
    local disasm_instrs=()
    while IFS= read -r line; do
        local trimmed=$(echo "$line" | sed 's/^[[:space:]]*//' | sed 's/[[:space:]]*$//')
        if [ -z "$trimmed" ]; then
            continue
        fi
        # Skip collision debug messages:
        # - Lines starting with "Colliding matches:"
        # - Lines matching pattern "^[0-9]+:  \"" (numbered collision examples like "1:  "c.andi a0, #0xFF"")
        if [[ "$trimmed" =~ ^Colliding\ matches: ]] || [[ "$trimmed" =~ ^[0-9]+:\ \ \" ]]; then
            continue
        fi
        # Skip hex byte lines (lines with only whitespace and hex digits, like "        06 ")
        # These appear in disassembly output but aren't actual instructions
        if [[ "$trimmed" =~ ^[[:space:]]*[0-9a-fA-F]{2}[[:space:]]*$ ]]; then
            continue
        fi
        disasm_instrs+=("$trimmed")
    done < "$disasm_file"
    
    # Check if counts match
    if [ ${#original_instrs[@]} -ne ${#disasm_instrs[@]} ]; then
        echo "FAIL:$extension:$base_name:Instruction count mismatch (${#original_instrs[@]} vs ${#disasm_instrs[@]})"
        return 1
    fi
    
    # Compare each instruction pair
    for i in "${!original_instrs[@]}"; do
        local original_instr="${original_instrs[$i]}"
        local disasm_instr="${disasm_instrs[$i]}"
        
        # Normalize by removing extra whitespace, trailing spaces, and comments
        local original_normalized=$(echo "$original_instr" | sed 's/;.*$//' | sed 's/[[:space:]]\+/ /g' | sed 's/,[[:space:]]\+/, /g' | sed 's/[[:space:]]*$//')
        local disasm_normalized=$(echo "$disasm_instr" | sed 's/;.*$//' | sed 's/[[:space:]]\+/ /g' | sed 's/,[[:space:]]\+/, /g' | sed 's/[[:space:]]*$//')
        
        # Normalize immediate values: convert hex to decimal for comparison
        # The disassembler outputs hex (e.g., #0xfffff801) but tests use decimal (e.g., #-2047)
        # Convert hex immediates to signed decimal for comparison
        original_normalized=$(echo "$original_normalized" | python3 -c "
import sys
import re
for line in sys.stdin:
    # Convert #0x... hex immediates to decimal
    def hex_to_dec(match):
        val = int(match.group(1), 16)
        # Handle 32-bit sign extension
        if val >= 0x80000000:
            val = val - 0x100000000
        return '#' + str(val)
    line = re.sub(r'#0x([0-9a-fA-F]+)', hex_to_dec, line, flags=re.IGNORECASE)
    print(line, end='')
" 2>/dev/null || echo "$original_normalized")
        
        disasm_normalized=$(echo "$disasm_normalized" | python3 -c "
import sys
import re
for line in sys.stdin:
    # Convert #0x... hex immediates to decimal
    def hex_to_dec(match):
        val = int(match.group(1), 16)
        # Handle 32-bit sign extension
        if val >= 0x80000000:
            val = val - 0x100000000
        return '#' + str(val)
    line = re.sub(r'#0x([0-9a-fA-F]+)', hex_to_dec, line, flags=re.IGNORECASE)
    print(line, end='')
" 2>/dev/null || echo "$disasm_normalized")
        
        # Extract mnemonics (first word)
        local original_mnemonic=$(echo "$original_normalized" | cut -d' ' -f1)
        local disasm_mnemonic=$(echo "$disasm_normalized" | cut -d' ' -f1)
        
        # Compare mnemonics first
        if [ "$original_mnemonic" != "$disasm_mnemonic" ]; then
            echo "FAIL:$extension:$base_name[$((i+1))]:Mnemonic mismatch: '$original_normalized' -> '$disasm_normalized'"
            return 1
        fi
        
        # Check for operand mismatches
        local original_ops=$(echo "$original_normalized" | sed 's/^[^ ]* //')
        local disasm_ops=$(echo "$disasm_normalized" | sed 's/^[^ ]* //')
        
        # For instructions with labels, we can't do exact comparison on the label value
        if echo "$original_normalized" | grep -q "label"; then
            local original_op_count=$(echo "$original_ops" | tr ',' ' ' | wc -w | tr -d ' ')
            local disasm_op_count=$(echo "$disasm_ops" | tr ',' ' ' | wc -w | tr -d ' ')
            if [ "$original_op_count" != "$disasm_op_count" ]; then
                echo "FAIL:$extension:$base_name[$((i+1))]:Operand count mismatch: '$original_normalized' -> '$disasm_normalized'"
                return 1
            fi
        else
            local original_lower=$(echo "$original_normalized" | tr '[:upper:]' '[:lower:]')
            local disasm_lower=$(echo "$disasm_normalized" | tr '[:upper:]' '[:lower:]')
            if [ "$original_lower" != "$disasm_lower" ]; then
                echo "FAIL:$extension:$base_name[$((i+1))]:Instruction/operand mismatch: '$original_normalized' -> '$disasm_normalized'"
                return 1
            fi
        fi
    done
    
    echo "PASS:$extension:$base_name"
    return 0
}

# Export function and variables for parallel execution
export -f test_file
export GOODASM

# Test a directory of .asm files in parallel
test_directory() {
    local test_dir="$1"
    local extension="$2"
    
    echo ""
    echo "=========================================="
    echo "Testing $extension"
    echo "=========================================="
    
    if [ ! -d "$test_dir" ]; then
        echo "  WARNING: Directory $test_dir does not exist"
        return
    fi
    
    # Collect all .asm files
    local asm_files=()
    for asm_file in "$test_dir"/*.asm; do
        if [ -f "$asm_file" ]; then
            asm_files+=("$asm_file")
        fi
    done
    
    if [ ${#asm_files[@]} -eq 0 ]; then
        echo "  No test files found"
        return
    fi
    
    # Run tests in parallel using background jobs
    local results_file=$(mktemp)
    declare -a pids=()
    local job_count=0
    
    for asm_file in "${asm_files[@]}"; do
        # Wait if we've reached max jobs
        while [ $job_count -ge $MAX_JOBS ]; do
            # Wait for any job to complete
            declare -a new_pids=()
            for pid in "${pids[@]}"; do
                if [ -n "$pid" ] && [ "$pid" -gt 0 ] 2>/dev/null && kill -0 "$pid" 2>/dev/null; then
                    # Job still running
                    new_pids+=($pid)
                else
                    # Job completed
                    if [ -n "$pid" ] && [ "$pid" -gt 0 ] 2>/dev/null; then
                        wait "$pid" 2>/dev/null || true
                    fi
                    job_count=$((job_count - 1))
                fi
            done
            pids=("${new_pids[@]}")
            if [ $job_count -ge $MAX_JOBS ]; then
                sleep 0.1  # Small delay to avoid busy waiting
            fi
        done
        
        # Run test in background
        (test_file "$asm_file" "$extension" >> "$results_file" 2>&1) &
        local new_pid=$!
        if [ -n "$new_pid" ] && [ "$new_pid" -gt 0 ]; then
            pids+=($new_pid)
            job_count=$((job_count + 1))
        fi
    done
    
    # Wait for all remaining jobs
    for pid in "${pids[@]}"; do
        if [ -n "$pid" ] && [ "$pid" -gt 0 ] 2>/dev/null; then
            wait "$pid" 2>/dev/null || true
        fi
    done
    
    # Process results
    local dir_passed=0
    local dir_failed=0
    
    while IFS= read -r result; do
        if [ -z "$result" ]; then
            continue
        fi
        
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        
        if [[ "$result" =~ ^PASS: ]]; then
            local test_name=$(echo "$result" | cut -d: -f3)
            echo "  PASS: $test_name"
            PASSED_TESTS+=("$extension: $test_name")
            TOTAL_PASSED=$((TOTAL_PASSED + 1))
            dir_passed=$((dir_passed + 1))
        elif [[ "$result" =~ ^FAIL: ]]; then
            local test_name=$(echo "$result" | cut -d: -f3)
            local error_msg=$(echo "$result" | cut -d: -f4-)
            echo "  FAIL: $test_name"
            FAILED_TESTS+=("$extension: $test_name")
            ERRORS+=("$extension: $test_name - $error_msg")
            TOTAL_FAILED=$((TOTAL_FAILED + 1))
            dir_failed=$((dir_failed + 1))
        fi
    done < "$results_file"
    
    rm -f "$results_file"
    
    echo "  Summary: $dir_passed passed, $dir_failed failed"
}

# Run tests for each extension
test_directory "tests/riscv/rv32i" "RV32I (Base)"
test_directory "tests/riscv/c-extension" "C Extension"
test_directory "tests/riscv/m-extension" "M Extension"
test_directory "tests/riscv/a-extension" "A Extension"
test_directory "tests/riscv/f-extension" "F Extension"
test_directory "tests/riscv/d-extension" "D Extension"
test_directory "tests/riscv/zicsr-extension" "Zicsr Extension"

# Print summary
echo ""
echo "=========================================="
echo "Overall Test Summary"
echo "=========================================="
echo "Total tests: $TOTAL_TESTS"
echo "Passed: $TOTAL_PASSED"
echo "Failed: $TOTAL_FAILED"

if [ ${#FAILED_TESTS[@]} -gt 0 ]; then
    echo ""
    echo "Failed tests:"
    for test in "${FAILED_TESTS[@]}"; do
        echo "  - $test"
    done
    echo ""
    echo "Detailed errors:"
    for error in "${ERRORS[@]}"; do
        echo "  - $error"
    done
    exit 1
else
    echo ""
    echo "All tests passed!"
    exit 0
fi

