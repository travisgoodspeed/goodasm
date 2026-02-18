#!/bin/bash

# Test harness for RISC-V extensions
# Usage: ./test_riscv_extensions.sh [extension1] [extension2] ...
#   If no arguments provided, tests all extensions
#   Valid extensions: rv32i, c-extension, m-extension, a-extension, f-extension, d-extension, zicsr-extension
#   Example: ./test_riscv_extensions.sh rv32i
#   Example: ./test_riscv_extensions.sh rv32i m-extension

cd "$(dirname "$0")"
GOODASM="./build/goodasm"
ERRORS=()
FAILED_TESTS=()
PASSED_TESTS=()
FAILED_INSTRUCTIONS=()
TOTAL_TESTS=0
TOTAL_PASSED=0
TOTAL_FAILED=0

# Check if goodasm exists
if [ ! -f "$GOODASM" ]; then
    echo "Error: goodasm not found at $GOODASM"
    exit 1
fi

# Function to normalize immediate values for comparison
# Handles: negative decimals, hex values with different formatting (#0x0 vs #0x00 vs #0x000)
normalize_immediate() {
    local imm="$1"
    # Remove # prefix if present
    imm=$(echo "$imm" | sed 's/^#//')
    
    # Check if it's a hex value
    if echo "$imm" | grep -qE '^0x[0-9a-fA-F]+$'; then
        # Convert hex to decimal, then back to normalized hex
        local dec=$(printf "%d" "$imm" 2>/dev/null)
        if [ $? -eq 0 ]; then
            # Normalize to 32-bit signed, then format as hex
            if [ "$dec" -ge 2147483648 ]; then
                dec=$((dec - 4294967296))
            fi
            printf "#%d" "$dec"
        else
            echo "#$imm"
        fi
    elif echo "$imm" | grep -qE '^-?[0-9]+$'; then
        # It's already a decimal
        echo "#$imm"
    else
        # Return as-is (might be a register or label)
        echo "#$imm"
    fi
}

# Function to normalize an instruction string for comparison
normalize_instruction() {
    local instr="$1"
    # Replace hex immediates and normalize them
    # This is a simplified approach - we'll handle common cases
    echo "$instr" | sed -E 's/#0x([0-9a-fA-F]+)/#HEX\1/g' | \
        sed -E 's/#HEX([0-9a-fA-F]+)/$(normalize_hex \1)/g'
}

# Function to get extension directory and name
get_extension_info() {
    local ext="$1"
    case "$ext" in
        "rv32i")
            echo "tests/riscv/rv32i|RV32I (Base)"
            ;;
        "c-extension")
            echo "tests/riscv/c-extension|C Extension"
            ;;
        "m-extension")
            echo "tests/riscv/m-extension|M Extension"
            ;;
        "a-extension")
            echo "tests/riscv/a-extension|A Extension"
            ;;
        "f-extension")
            echo "tests/riscv/f-extension|F Extension"
            ;;
        "d-extension")
            echo "tests/riscv/d-extension|D Extension"
            ;;
        "zicsr-extension")
            echo "tests/riscv/zicsr-extension|Zicsr Extension"
            ;;
        *)
            echo ""
            ;;
    esac
}

# Determine which extensions to test
EXTENSIONS_TO_TEST=()
if [ $# -eq 0 ]; then
    # No arguments - test all extensions
    EXTENSIONS_TO_TEST=("rv32i" "c-extension" "m-extension" "a-extension" "f-extension" "d-extension" "zicsr-extension")
else
    # Test only specified extensions
    for ext in "$@"; do
        ext_info=$(get_extension_info "$ext")
        if [ -n "$ext_info" ]; then
            EXTENSIONS_TO_TEST+=("$ext")
        else
            echo "Warning: Unknown extension '$ext'. Valid extensions are:"
            echo "  - rv32i"
            echo "  - c-extension"
            echo "  - m-extension"
            echo "  - a-extension"
            echo "  - f-extension"
            echo "  - d-extension"
            echo "  - zicsr-extension"
        fi
    done
fi

if [ ${#EXTENSIONS_TO_TEST[@]} -eq 0 ]; then
    echo "Error: No valid extensions specified"
    exit 1
fi

# Function to test a single .asm file
test_file() {
    local asm_file="$1"
    local extension="$2"
    
    if [ ! -f "$asm_file" ]; then
        return 1
    fi
    
    local base_name=$(basename "$asm_file" .asm)
    local dir=$(dirname "$asm_file")
    local bin_file="$dir/${base_name}.bin"
    local disasm_file="$dir/${base_name}.disasm"
    
    # Assemble the file
    if ! "$GOODASM" --riscv32 "$asm_file" -o "$bin_file" > /dev/null 2>&1; then
        ERRORS+=("$extension: $base_name - Assembly failed")
        FAILED_INSTRUCTIONS+=("$base_name")
        return 1
    fi
    
    # Check if binary was created and is not empty
    if [ ! -s "$bin_file" ]; then
        ERRORS+=("$extension: $base_name - Empty binary file")
        FAILED_INSTRUCTIONS+=("$base_name")
        return 1
    fi
    
    # Disassemble the binary
    if ! "$GOODASM" --riscv32 -d "$bin_file" > "$disasm_file" 2>&1; then
        ERRORS+=("$extension: $base_name - Disassembly failed")
        FAILED_INSTRUCTIONS+=("$base_name")
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
        ERRORS+=("$extension: $base_name - Instruction count mismatch (${#original_instrs[@]} vs ${#disasm_instrs[@]})")
        FAILED_INSTRUCTIONS+=("$base_name")
        return 1
    fi
    
    # Compare each instruction pair
    for i in "${!original_instrs[@]}"; do
        local original_instr="${original_instrs[$i]}"
        local disasm_instr="${disasm_instrs[$i]}"
        
        # Normalize by removing extra whitespace, trailing spaces, and comments
        local original_normalized=$(echo "$original_instr" | sed 's/;.*$//' | sed 's/[[:space:]]\+/ /g' | sed 's/,[[:space:]]\+/, /g' | sed 's/[[:space:]]*$//')
        local disasm_normalized=$(echo "$disasm_instr" | sed 's/;.*$//' | sed 's/[[:space:]]\+/ /g' | sed 's/,[[:space:]]\+/, /g' | sed 's/[[:space:]]*$//')
        
        # Extract mnemonics (first word)
        local original_mnemonic=$(echo "$original_normalized" | cut -d' ' -f1)
        local disasm_mnemonic=$(echo "$disasm_normalized" | cut -d' ' -f1)
        
        # Compare mnemonics first
        if [ "$original_mnemonic" != "$disasm_mnemonic" ]; then
            ERRORS+=("$extension: $base_name[$((i+1))] - Mnemonic mismatch: '$original_normalized' -> '$disasm_normalized'")
            FAILED_INSTRUCTIONS+=("$base_name")
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
                ERRORS+=("$extension: $base_name[$((i+1))] - Operand count mismatch: '$original_normalized' -> '$disasm_normalized'")
                FAILED_INSTRUCTIONS+=("$base_name")
                return 1
            fi
        else
            # Normalize hex values to decimal for comparison
            # Convert hex immediates like #0xfffff801 to #-2047
            # Also normalize hex formatting (#0x0, #0x00, #0x000 -> same value)
            local original_normalized_hex=$(echo "$original_normalized" | python3 -c "
import sys
import re
line = sys.stdin.read()
# Convert hex to decimal
def hex_to_dec(match):
    hex_val = match.group(1)
    try:
        val = int(hex_val, 16)
        # Convert to 32-bit signed
        if val >= 0x80000000:
            val = val - 0x100000000
        return f'#{val}'
    except:
        return match.group(0)
line = re.sub(r'#0x([0-9a-fA-F]+)', hex_to_dec, line, flags=re.IGNORECASE)
# Normalize hex formatting differences (#0x0 vs #0x00 -> keep as decimal now)
print(line, end='')
")
            local disasm_normalized_hex=$(echo "$disasm_normalized" | python3 -c "
import sys
import re
line = sys.stdin.read()
# Convert hex to decimal
def hex_to_dec(match):
    hex_val = match.group(1)
    try:
        val = int(hex_val, 16)
        # Convert to 32-bit signed
        if val >= 0x80000000:
            val = val - 0x100000000
        return f'#{val}'
    except:
        return match.group(0)
line = re.sub(r'#0x([0-9a-fA-F]+)', hex_to_dec, line, flags=re.IGNORECASE)
print(line, end='')
")
            
            local original_lower=$(echo "$original_normalized_hex" | tr '[:upper:]' '[:lower:]')
            local disasm_lower=$(echo "$disasm_normalized_hex" | tr '[:upper:]' '[:lower:]')
            
            if [ "$original_lower" != "$disasm_lower" ]; then
                ERRORS+=("$extension: $base_name[$((i+1))] - Instruction/operand mismatch: '$original_normalized' -> '$disasm_normalized'")
                FAILED_INSTRUCTIONS+=("$base_name")
                return 1
            fi
        fi
    done
    
    return 0
}

# Test a directory of .asm files
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
    
    local dir_passed=0
    local dir_failed=0
    
    for asm_file in "$test_dir"/*.asm; do
        if [ ! -f "$asm_file" ]; then
            continue
        fi
        
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        local base_name=$(basename "$asm_file" .asm)
        
        if test_file "$asm_file" "$extension"; then
            echo "  PASS: $base_name"
            PASSED_TESTS+=("$extension: $base_name")
            TOTAL_PASSED=$((TOTAL_PASSED + 1))
            dir_passed=$((dir_passed + 1))
        else
            echo "  FAIL: $base_name"
            FAILED_TESTS+=("$extension: $base_name")
            TOTAL_FAILED=$((TOTAL_FAILED + 1))
            dir_failed=$((dir_failed + 1))
        fi
    done
    
    echo "  Summary: $dir_passed passed, $dir_failed failed"
}

# Run tests for specified extensions
for ext in "${EXTENSIONS_TO_TEST[@]}"; do
    ext_info=$(get_extension_info "$ext")
    if [ -n "$ext_info" ]; then
        IFS='|' read -r test_dir ext_name <<< "$ext_info"
        test_directory "$test_dir" "$ext_name"
    fi
done

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
    echo "Failed instructions (unique):"
    # Get unique failed instructions
    printf '%s\n' "${FAILED_INSTRUCTIONS[@]}" | sort -u | while read -r instr; do
        echo "  - $instr"
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

