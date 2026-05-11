#!/bin/bash
#
# Quick RISC-V Validation Test
# Tests representative samples of each instruction category
# Runtime: ~5-10 seconds
#

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR/../.."
GOODASM="$(pwd)/build/goodasm"

if [ ! -f "$GOODASM" ]; then
    echo "Error: goodasm not found at $GOODASM"
    echo "Run 'make' first to build goodasm"
    exit 1
fi

PASS=0
FAIL=0
FAILED_TESTS=()

# Test a single instruction roundtrip
test_instr() {
    local instr="$1"
    local category="$2"

    echo -e ".lang riscv32\n.org 0x0\n$instr" > /tmp/test_quick.asm
    "$GOODASM" --riscv32 /tmp/test_quick.asm -o /tmp/test_quick.bin 2>/dev/null

    if [ ! -s /tmp/test_quick.bin ]; then
        echo "  FAIL: $instr (assembly failed)"
        FAILED_TESTS+=("[$category] $instr - assembly failed")
        ((FAIL++))
        return 1
    fi

    result=$("$GOODASM" --riscv32 -d /tmp/test_quick.bin 2>/dev/null | grep -v "^Colliding" | grep -v "^[0-9]*:  \"" | head -1)

    # Normalize for comparison (trim whitespace, lowercase, normalize spaces)
    orig_norm=$(echo "$instr" | tr '[:upper:]' '[:lower:]' | sed 's/[[:space:]]\+/ /g' | sed 's/,\s*/, /g' | sed 's/[[:space:]]*$//')
    result_norm=$(echo "$result" | tr '[:upper:]' '[:lower:]' | sed 's/[[:space:]]\+/ /g' | sed 's/,\s*/, /g' | sed 's/[[:space:]]*$//')

    # Normalize negative hex values (e.g., #0xfffffffb -> #-5)
    # This handles 32-bit signed representation
    normalize_hex() {
        echo "$1" | sed -E 's/#0x([0-9a-f]{8})/\n\1\n/g' | while IFS= read -r part; do
            if [[ "$part" =~ ^[0-9a-f]{8}$ ]]; then
                val=$((16#$part))
                if [ $val -ge $((0x80000000)) ]; then
                    val=$((val - 0x100000000))
                fi
                echo -n "#$val"
            else
                echo -n "$part"
            fi
        done
    }
    orig_norm=$(normalize_hex "$orig_norm")
    result_norm=$(normalize_hex "$result_norm")

    if [ "$orig_norm" = "$result_norm" ]; then
        echo "  PASS: $instr"
        ((PASS++))
        return 0
    else
        echo "  FAIL: $instr -> $result"
        FAILED_TESTS+=("[$category] $instr -> $result")
        ((FAIL++))
        return 1
    fi
}

echo "=========================================="
echo "Quick RISC-V Validation Test"
echo "=========================================="
echo ""

echo "=== RV32I Base Instructions ==="
test_instr "add t0, t1, t2" "R-type"
test_instr "sub t0, t1, t2" "R-type"
test_instr "and t0, t1, t2" "R-type"
test_instr "or t0, t1, t2" "R-type"
test_instr "xor t0, t1, t2" "R-type"
test_instr "sll t0, t1, t2" "R-type"
test_instr "srl t0, t1, t2" "R-type"
test_instr "sra t0, t1, t2" "R-type"
test_instr "slt t0, t1, t2" "R-type"
test_instr "sltu t0, t1, t2" "R-type"

echo ""
echo "=== I-type ALU ==="
test_instr "addi t0, t1, #5" "I-type"
test_instr "addi t0, t1, #-5" "I-type"
test_instr "andi t0, t1, #0x1F" "I-type"
test_instr "ori t0, t1, #0xF0" "I-type"
test_instr "xori t0, t1, #0xFF" "I-type"
test_instr "slti t0, t1, #10" "I-type"
test_instr "sltiu t0, t1, #10" "I-type"
test_instr "slli t0, t1, #5" "Shift"
test_instr "srli t0, t1, #5" "Shift"
test_instr "srai t0, t1, #5" "Shift"

echo ""
echo "=== Load/Store ==="
test_instr "lw a0, (#4, a1)" "Load"
test_instr "lw a0, (#-4, a1)" "Load"
test_instr "lh a0, (#2, a1)" "Load"
test_instr "lb a0, (#1, a1)" "Load"
test_instr "lhu a0, (#2, a1)" "Load"
test_instr "lbu a0, (#1, a1)" "Load"
test_instr "sw a0, (#4, a1)" "Store"
test_instr "sw a0, (#-4, a1)" "Store"
test_instr "sh a0, (#2, a1)" "Store"
test_instr "sb a0, (#1, a1)" "Store"

echo ""
echo "=== U-type ==="
test_instr "lui t0, #0x12345000" "U-type"
test_instr "auipc t0, #0x12345000" "U-type"

echo ""
echo "=== Branch ==="
test_instr "beq t0, t1, #4" "Branch"
test_instr "beq t0, t1, #-4" "Branch"
test_instr "bne t0, t1, #8" "Branch"
test_instr "blt t0, t1, #12" "Branch"
test_instr "bge t0, t1, #16" "Branch"
test_instr "bltu t0, t1, #20" "Branch"
test_instr "bgeu t0, t1, #24" "Branch"

echo ""
echo "=== JAL/JALR ==="
test_instr "jal ra, #4" "JAL"
test_instr "jal ra, #-4" "JAL"
test_instr "jalr ra, t0, #0" "JALR"
test_instr "jalr ra, t0, #4" "JALR"

echo ""
echo "=== System ==="
test_instr "ecall" "System"
test_instr "ebreak" "System"

echo ""
echo "=== M Extension ==="
test_instr "mul t0, t1, t2" "M-ext"
test_instr "mulh t0, t1, t2" "M-ext"
test_instr "mulhsu t0, t1, t2" "M-ext"
test_instr "mulhu t0, t1, t2" "M-ext"
test_instr "div t0, t1, t2" "M-ext"
test_instr "divu t0, t1, t2" "M-ext"
test_instr "rem t0, t1, t2" "M-ext"
test_instr "remu t0, t1, t2" "M-ext"

echo ""
echo "=== A Extension ==="
test_instr "lr.w t0, (t1)" "A-ext"
test_instr "sc.w t0, t2, (t1)" "A-ext"
test_instr "amoswap.w t0, t2, (t1)" "A-ext"
test_instr "amoadd.w t0, t2, (t1)" "A-ext"
test_instr "amoxor.w t0, t2, (t1)" "A-ext"
test_instr "amoand.w t0, t2, (t1)" "A-ext"
test_instr "amoor.w t0, t2, (t1)" "A-ext"

echo ""
echo "=== Zicsr Extension ==="
test_instr "csrrw t0, #0x300, t1" "Zicsr"
test_instr "csrrs t0, #0x300, t1" "Zicsr"
test_instr "csrrc t0, #0x300, t1" "Zicsr"
test_instr "csrrwi t0, #0x300, #5" "Zicsr"
test_instr "csrrsi t0, #0x300, #5" "Zicsr"
test_instr "csrrci t0, #0x300, #5" "Zicsr"

echo ""
echo "=== C Extension CR/CI Format ==="
test_instr "c.add a0, a1" "C-CR"
test_instr "c.mv a0, a1" "C-CR"
test_instr "c.jr ra" "C-CR"
test_instr "c.jr a0" "C-CR"
test_instr "c.jalr ra" "C-CR"
test_instr "c.jalr a0" "C-CR"
test_instr "c.addi a0, #5" "C-CI"
test_instr "c.addi a0, #-5" "C-CI"
test_instr "c.slli a0, #3" "C-CI"
test_instr "c.li a0, #10" "C-CI"
test_instr "c.lui a0, #0x12000" "C-CI"

echo ""
echo "=== C Extension CB/CA Format ==="
test_instr "c.sub s0, s1" "C-CA"
test_instr "c.and s0, s1" "C-CA"
test_instr "c.or s0, s1" "C-CA"
test_instr "c.xor s0, s1" "C-CA"
test_instr "c.beqz s0, #4" "C-CB"
test_instr "c.beqz s0, #-4" "C-CB"
test_instr "c.bnez s0, #6" "C-CB"
test_instr "c.andi s0, #7" "C-CB"
test_instr "c.srli s0, #2" "C-CB"
test_instr "c.srai s0, #2" "C-CB"

echo ""
echo "=== C Extension CIW/CL/CS/CSS Format ==="
test_instr "c.addi4spn s0, #4" "C-CIW"
test_instr "c.addi4spn s0, #256" "C-CIW"
test_instr "c.lw a0, (#4, a1)" "C-CL"
test_instr "c.sw a0, (#4, a1)" "C-CS"
test_instr "c.lwsp a0, #4" "C-CSS"
test_instr "c.swsp a0, #4" "C-CSS"

echo ""
echo "=== C Extension CJ Format ==="
test_instr "c.j #4" "C-CJ"
test_instr "c.j #-4" "C-CJ"

echo ""
echo "=== C Extension System ==="
test_instr "c.nop" "C-Sys"
test_instr "c.ebreak" "C-Sys"

echo ""
echo "=========================================="
echo "Test Summary"
echo "=========================================="
echo "Total: $((PASS + FAIL))"
echo "Passed: $PASS"
echo "Failed: $FAIL"

if [ ${#FAILED_TESTS[@]} -gt 0 ]; then
    echo ""
    echo "Failed tests:"
    for test in "${FAILED_TESTS[@]}"; do
        echo "  - $test"
    done
    exit 1
else
    echo ""
    echo "All tests passed!"
    exit 0
fi
