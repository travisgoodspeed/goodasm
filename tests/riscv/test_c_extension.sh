#!/bin/bash

# Script to test all C-type instruction unit tests
# For each .asm file, assemble it to .bin, disassemble to .disasm,
# and compare the instructions and operands

cd "$(dirname "$0")"
TEST_DIR="tests/riscv/c-extension"
GOODASM="./build/goodasm"
ERRORS=()
FAILED_INSTRUCTIONS=()

# Check if goodasm exists
if [ ! -f "$GOODASM" ]; then
    echo "Error: goodasm not found at $GOODASM"
    exit 1
fi

# Get all .asm files in the c-extension directory
for asm_file in "$TEST_DIR"/*.asm; do
    if [ ! -f "$asm_file" ]; then
        continue
    fi
    
    base_name=$(basename "$asm_file" .asm)
    bin_file="$TEST_DIR/${base_name}.bin"
    disasm_file="$TEST_DIR/${base_name}.disasm"
    
    echo "Testing: $base_name"
    
    # Assemble the file
    if ! "$GOODASM" --riscv32 "$asm_file" -o "$bin_file" > /dev/null 2>&1; then
        echo "  ERROR: Failed to assemble $asm_file"
        ERRORS+=("$base_name: Assembly failed")
        FAILED_INSTRUCTIONS+=("$base_name")
        continue
    fi
    
    # Check if binary was created and is not empty
    if [ ! -s "$bin_file" ]; then
        echo "  ERROR: Binary file is empty"
        ERRORS+=("$base_name: Empty binary file")
        FAILED_INSTRUCTIONS+=("$base_name")
        continue
    fi
    
    # Disassemble the binary
    if ! "$GOODASM" --riscv32 -d "$bin_file" > "$disasm_file" 2>&1; then
        echo "  ERROR: Failed to disassemble $bin_file"
        ERRORS+=("$base_name: Disassembly failed")
        FAILED_INSTRUCTIONS+=("$base_name")
        continue
    fi
    
    # Extract all instructions from the original .asm file (skip directives, blank lines, labels, and comments)
    original_instrs=()
    while IFS= read -r line; do
        # Skip empty lines, directives, labels, and comments
        if [[ "$line" =~ ^[[:space:]]*$ ]] || \
           [[ "$line" =~ ^[[:space:]]*\.lang ]] || \
           [[ "$line" =~ ^[[:space:]]*\.org ]] || \
           [[ "$line" =~ ^[[:space:]]*\; ]] || \
           [[ "$line" =~ ^[[:space:]]*[a-zA-Z_][a-zA-Z0-9_]*:[[:space:]]*$ ]]; then
            continue
        fi
        # Trim whitespace
        trimmed=$(echo "$line" | sed 's/^[[:space:]]*//' | sed 's/[[:space:]]*$//')
        if [ -n "$trimmed" ]; then
            original_instrs+=("$trimmed")
        fi
    done < "$asm_file"
    
    # Get all disassembled instructions (non-empty lines, excluding collision debug messages)
    disasm_instrs=()
    while IFS= read -r line; do
        trimmed=$(echo "$line" | sed 's/^[[:space:]]*//' | sed 's/[[:space:]]*$//')
        # Skip empty lines
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
        echo "  FAIL: Instruction count mismatch (${#original_instrs[@]} vs ${#disasm_instrs[@]})"
        echo "    Original instructions: ${#original_instrs[@]}"
        echo "    Disassembled instructions: ${#disasm_instrs[@]}"
        ERRORS+=("$base_name: Instruction count mismatch (${#original_instrs[@]} vs ${#disasm_instrs[@]})")
        FAILED_INSTRUCTIONS+=("$base_name")
        continue
    fi
    
    # Compare each instruction pair
    file_passed=true
    for i in "${!original_instrs[@]}"; do
        original_instr="${original_instrs[$i]}"
        disasm_instr="${disasm_instrs[$i]}"
        
        # Normalize by removing extra whitespace, trailing spaces, and comments
        original_normalized=$(echo "$original_instr" | sed 's/;.*$//' | sed 's/[[:space:]]\+/ /g' | sed 's/,[[:space:]]\+/, /g' | sed 's/[[:space:]]*$//')
        disasm_normalized=$(echo "$disasm_instr" | sed 's/;.*$//' | sed 's/[[:space:]]\+/ /g' | sed 's/,[[:space:]]\+/, /g' | sed 's/[[:space:]]*$//')
        
        # Extract mnemonics (first word)
        original_mnemonic=$(echo "$original_normalized" | cut -d' ' -f1)
        disasm_mnemonic=$(echo "$disasm_normalized" | cut -d' ' -f1)
        
        # Compare mnemonics first
        if [ "$original_mnemonic" != "$disasm_mnemonic" ]; then
            echo "  FAIL: Mnemonic mismatch for instruction $((i+1))"
            echo "    Original:  $original_normalized"
            echo "    Disasm:    $disasm_normalized"
            ERRORS+=("$base_name[$((i+1))]: Mnemonic mismatch - '$original_normalized' -> '$disasm_normalized'")
            file_passed=false
            continue
        fi
        
        # Check for operand mismatches
        # Extract operands (everything after mnemonic)
        original_ops=$(echo "$original_normalized" | sed 's/^[^ ]* //')
        disasm_ops=$(echo "$disasm_normalized" | sed 's/^[^ ]* //')
        
        # For instructions with labels, we can't do exact comparison on the label value
        if echo "$original_normalized" | grep -q "label"; then
            # Has label - check mnemonic matches and operand count
            original_op_count=$(echo "$original_ops" | tr ',' ' ' | wc -w | tr -d ' ')
            disasm_op_count=$(echo "$disasm_ops" | tr ',' ' ' | wc -w | tr -d ' ')
            
            if [ "$original_op_count" != "$disasm_op_count" ]; then
                echo "  FAIL: Operand count mismatch for instruction $((i+1))"
                echo "    Original:  $original_normalized"
                echo "    Disasm:    $disasm_normalized"
                ERRORS+=("$base_name[$((i+1))]: Operand count mismatch - '$original_normalized' -> '$disasm_normalized'")
                file_passed=false
            fi
            # Note: For labels, we accept the mnemonic match and operand count as pass
        else
            # No labels - do comparison including operands (case-insensitive for hex)
            # Normalize both strings to lowercase for comparison
            original_lower=$(echo "$original_normalized" | tr '[:upper:]' '[:lower:]')
            disasm_lower=$(echo "$disasm_normalized" | tr '[:upper:]' '[:lower:]')
            if [ "$original_lower" != "$disasm_lower" ]; then
                echo "  FAIL: Instruction/operand mismatch for instruction $((i+1))"
                echo "    Original:  $original_normalized"
                echo "    Disasm:    $disasm_normalized"
                ERRORS+=("$base_name[$((i+1))]: Instruction/operand mismatch - '$original_normalized' -> '$disasm_normalized'")
                file_passed=false
            fi
        fi
    done
    
    if [ "$file_passed" = true ]; then
        if [ ${#original_instrs[@]} -eq 1 ]; then
            echo "  PASS: ${original_instrs[0]}"
        else
            echo "  PASS: ${#original_instrs[@]} instructions"
        fi
    else
        FAILED_INSTRUCTIONS+=("$base_name")
    fi
done

# Print summary
echo ""
echo "=========================================="
echo "Test Summary"
echo "=========================================="
total_tests=$(ls "$TEST_DIR"/*.asm 2>/dev/null | wc -l | tr -d ' ')
echo "Total tests: $total_tests"
echo "Failed tests: ${#FAILED_INSTRUCTIONS[@]}"

if [ ${#FAILED_INSTRUCTIONS[@]} -gt 0 ]; then
    echo ""
    echo "Failed instructions:"
    for instr in "${FAILED_INSTRUCTIONS[@]}"; do
        echo "  - $instr"
    done
    echo ""
    echo "Detailed errors:"
    for error in "${ERRORS[@]}"; do
        echo "  - $error"
    done
    exit 1
else
    echo "All tests passed!"
    exit 0
fi
