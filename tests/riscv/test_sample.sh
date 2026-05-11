#!/bin/bash

# Quick test script that samples a few test files from each extension
# to verify encoding/decoding works

cd "$(dirname "$0")/../.."
GOODASM="./build/goodasm"

if [ ! -f "$GOODASM" ]; then
    echo "Error: goodasm not found at $GOODASM"
    exit 1
fi

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
    
    # Assemble (suppress errors but capture them)
    "$GOODASM" --riscv32 "$asm_file" -o "$bin_file" > /dev/null 2>&1
    
    # Check if binary was created and is not empty
    if [ ! -s "$bin_file" ]; then
        echo "  FAIL: $base_name - Empty or missing binary"
        return 1
    fi
    
    # Disassemble
    "$GOODASM" --riscv32 -d "$bin_file" > "$disasm_file" 2>&1
    
    # Count instructions in original
    local orig_count=$(grep -vE '^\s*$|^\.lang|^\.org|^;|^\s*[a-zA-Z_][a-zA-Z0-9_]*:\s*$' "$asm_file" | wc -l | tr -d ' ')
    
    # Count instructions in disassembly (skip hex bytes and collision messages)
    local disasm_count=$(grep -vE '^\s*$|^Colliding matches:|^\s*[0-9]+:\s+\"|^\s*[0-9a-fA-F]{2}\s*$' "$disasm_file" | wc -l | tr -d ' ')
    
    if [ "$orig_count" -ne "$disasm_count" ]; then
        echo "  FAIL: $base_name - Count mismatch ($orig_count vs $disasm_count)"
        return 1
    fi
    
    echo "  PASS: $base_name ($orig_count instructions)"
    return 0
}

echo "Testing sample files from each extension..."
echo ""

# Test a few files from each extension
echo "RV32I Base:"
test_file "tests/riscv/rv32i/add.asm" "RV32I"
test_file "tests/riscv/rv32i/addi.asm" "RV32I"
test_file "tests/riscv/rv32i/beq.asm" "RV32I"
test_file "tests/riscv/rv32i/lw.asm" "RV32I"
test_file "tests/riscv/rv32i/sw.asm" "RV32I"

echo ""
echo "C Extension:"
test_file "tests/riscv/c-extension/c_add.asm" "C"
test_file "tests/riscv/c-extension/c_addi.asm" "C"
test_file "tests/riscv/c-extension/c_lw.asm" "C"

echo ""
echo "M Extension:"
test_file "tests/riscv/m-extension/mul.asm" "M"
test_file "tests/riscv/m-extension/div.asm" "M"

echo ""
echo "A Extension:"
test_file "tests/riscv/a-extension/lr_w.asm" "A"
test_file "tests/riscv/a-extension/amoadd_w.asm" "A"

echo ""
echo "F Extension:"
test_file "tests/riscv/f-extension/flw.asm" "F"
test_file "tests/riscv/f-extension/fadd_s.asm" "F"

echo ""
echo "D Extension:"
test_file "tests/riscv/d-extension/fld.asm" "D"
test_file "tests/riscv/d-extension/fadd_d.asm" "D"

echo ""
echo "Zicsr Extension:"
test_file "tests/riscv/zicsr-extension/csrrw.asm" "Zicsr"
test_file "tests/riscv/zicsr-extension/csrrwi.asm" "Zicsr"

echo ""
echo "Done!"

