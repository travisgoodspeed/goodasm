#!/bin/bash

# Test Harness for RV32I (Base)
# Performs round-trip assemble/disassemble comparison for all .asm files
# Generates fresh .bin and .disasm files each time

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
# Navigate to project root (from tests/riscv/test_harnesses/ to root)
cd "$SCRIPT_DIR/../../.."
PROJECT_ROOT="$(pwd)"
GOODASM="$PROJECT_ROOT/build/goodasm"

# Extension directory
EXTENSION_DIR="$PROJECT_ROOT/tests/riscv/rv32i"
EXTENSION_NAME="RV32I (Base)"

# Arrays for tracking results
ERRORS=()
FAILED_TESTS=()
PASSED_TESTS=()
TOTAL_TESTS=0
TOTAL_PASSED=0
TOTAL_FAILED=0

# Check if goodasm exists
if [ ! -f "$GOODASM" ]; then
    echo "Error: goodasm not found at $GOODASM"
    exit 1
fi

# Function to test a single .asm file
test_file() {
    local asm_file="$1"
    base_name="$2"
    
    if [ ! -f "$asm_file" ]; then
        return 1
    fi
    
    local bin_file="$EXTENSION_DIR/${base_name}.bin"
    local disasm_file="$EXTENSION_DIR/${base_name}.disasm"
    
    # Assemble the file (generates fresh binary)
    if ! "$GOODASM" --riscv32 "$asm_file" -o "$bin_file" > /dev/null 2>&1; then
        ERRORS+=("$base_name - Assembly failed")
        return 1
    fi
    
    # Check if binary was created and is not empty
    if [ ! -s "$bin_file" ]; then
        ERRORS+=("$base_name - Empty binary file")
        return 1
    fi
    
    # Disassemble the binary (generates fresh disassembly)
    if ! "$GOODASM" --riscv32 -d "$bin_file" > "$disasm_file" 2>&1; then
        ERRORS+=("$base_name - Disassembly failed")
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
        # - Lines matching pattern "^[0-9]+:  \"" (numbered collision examples)
        if [[ "$trimmed" =~ ^Colliding\ matches: ]] || [[ "$trimmed" =~ ^[0-9]+:\ \ \" ]]; then
            continue
        fi
        # Skip hex byte lines (lines with only whitespace and hex digits)
        if [[ "$trimmed" =~ ^[[:space:]]*[0-9a-fA-F]{2}[[:space:]]*$ ]]; then
            continue
        fi
        disasm_instrs+=("$trimmed")
    done < "$disasm_file"
    
    # Check if counts match
    if [ ${#original_instrs[@]} -ne ${#disasm_instrs[@]} ]; then
        ERRORS+=("$base_name - Instruction count mismatch (${#original_instrs[@]} vs ${#disasm_instrs[@]})")
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
            ERRORS+=("$base_name[$((i+1))] - Mnemonic mismatch: '$original_normalized' -> '$disasm_normalized'")
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
                ERRORS+=("$base_name[$((i+1))] - Operand count mismatch: '$original_normalized' -> '$disasm_normalized'")
                return 1
            fi
        else
            # Normalize hex values to decimal for comparison
            # Convert hex immediates like #0xfffff801 to #-2047
            # This handles format differences between decimal and hex representation
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
                ERRORS+=("$base_name[$((i+1))] - Instruction/operand mismatch: '$original_normalized' -> '$disasm_normalized'")
                return 1
            fi
        fi
    done
    
    return 0
}

# Main test execution
echo "=========================================="
echo "Test Harness: $EXTENSION_NAME"
echo "=========================================="
echo "Extension directory: $EXTENSION_DIR"
echo ""

# Check if extension directory exists
if [ ! -d "$EXTENSION_DIR" ]; then
    echo "Error: Extension directory not found: $EXTENSION_DIR"
    exit 1
fi

# Find all .asm files in the extension directory
for asm_file in "$EXTENSION_DIR"/*.asm; do
    if [ ! -f "$asm_file" ]; then
        continue
    fi
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    base_name=$(basename "$asm_file" .asm)
    
    if test_file "$asm_file" "$base_name"; then
        echo "  PASS: $base_name"
        PASSED_TESTS+=("$base_name")
        TOTAL_PASSED=$((TOTAL_PASSED + 1))
    else
        echo "  FAIL: $base_name"
        FAILED_TESTS+=("$base_name")
        TOTAL_FAILED=$((TOTAL_FAILED + 1))
    fi
done

# Print summary
echo ""
echo "=========================================="
echo "Test Summary: $EXTENSION_NAME"
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

