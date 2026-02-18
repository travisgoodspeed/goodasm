#!/bin/bash
#
# Moderate RISC-V Validation Test
# Tests every instruction with a few representative operand combinations
# Covers edge cases (min/max values, different registers)
# Runtime: ~30-60 seconds
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

# Normalize hex to decimal for comparison
# Converts all #0xNNN patterns to decimal #NNN
normalize_values() {
    local input="$1"
    # Use Python for reliable hex-to-decimal conversion
    echo "$input" | python3 -c "
import sys
import re

line = sys.stdin.read().strip()

def hex_to_dec(match):
    hex_val = match.group(1)
    try:
        val = int(hex_val, 16)
        # Handle 32-bit signed conversion for large values
        if val >= 0x80000000:
            val = val - 0x100000000
        return f'#{val}'
    except:
        return match.group(0)

# Convert #0x... patterns to decimal
result = re.sub(r'#0x([0-9a-fA-F]+)', hex_to_dec, line, flags=re.IGNORECASE)
print(result, end='')
"
}

# Test a single instruction roundtrip
test_instr() {
    local instr="$1"
    local category="$2"

    echo -e ".lang riscv32\n.org 0x0\n$instr" > /tmp/test_mod.asm
    "$GOODASM" --riscv32 /tmp/test_mod.asm -o /tmp/test_mod.bin 2>/dev/null

    if [ ! -s /tmp/test_mod.bin ]; then
        echo "  FAIL: $instr (assembly failed)"
        FAILED_TESTS+=("[$category] $instr - assembly failed")
        ((FAIL++))
        return 1
    fi

    result=$("$GOODASM" --riscv32 -d /tmp/test_mod.bin 2>/dev/null | grep -v "^Colliding" | grep -v "^[0-9]*:  \"" | head -1)

    # Normalize for comparison (lowercase, normalize whitespace, trim)
    orig_norm=$(echo "$instr" | tr '[:upper:]' '[:lower:]' | sed 's/[[:space:]]\+/ /g' | sed 's/,\s*/, /g' | sed 's/[[:space:]]*$//')
    result_norm=$(echo "$result" | tr '[:upper:]' '[:lower:]' | sed 's/[[:space:]]\+/ /g' | sed 's/,\s*/, /g' | sed 's/[[:space:]]*$//')

    # Normalize hex values to decimal for comparison
    orig_norm=$(normalize_values "$orig_norm")
    result_norm=$(normalize_values "$result_norm")

    if [ "$orig_norm" = "$result_norm" ]; then
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
echo "Moderate RISC-V Validation Test"
echo "=========================================="
echo "Testing every instruction with representative operands..."
echo ""

# ============================================================================
# RV32I Base Integer Instructions
# ============================================================================

echo "=== R-type ALU (10 instructions × 3 variants = 30 tests) ==="
for op in add sub and or xor sll srl sra slt sltu; do
    test_instr "$op t0, t1, t2" "R-$op"
    test_instr "$op a0, a1, a2" "R-$op"
    test_instr "$op s0, s1, s2" "R-$op"
done
echo "  R-type: $PASS passed"
R_PASS=$PASS

echo ""
echo "=== I-type ALU (7 instructions × 4 variants = 28 tests) ==="
for op in addi andi ori xori slti sltiu; do
    test_instr "$op t0, t1, #0" "I-$op"
    test_instr "$op t0, t1, #1" "I-$op"
    test_instr "$op t0, t1, #-1" "I-$op"
    test_instr "$op a0, a1, #2047" "I-$op"
done
# Special case for addi with min value
test_instr "addi t0, t1, #-2048" "I-addi"
test_instr "andi t0, t1, #0x7FF" "I-andi"
test_instr "ori t0, t1, #0x7FF" "I-ori"
test_instr "xori t0, t1, #0x7FF" "I-xori"
echo "  I-type ALU: $((PASS - R_PASS)) passed"
I_PASS=$PASS

echo ""
echo "=== Shift Immediate (3 instructions × 4 variants = 12 tests) ==="
for op in slli srli srai; do
    test_instr "$op t0, t1, #0" "Shift-$op"
    test_instr "$op t0, t1, #1" "Shift-$op"
    test_instr "$op t0, t1, #15" "Shift-$op"
    test_instr "$op t0, t1, #31" "Shift-$op"
done
echo "  Shift: $((PASS - I_PASS)) passed"
SHIFT_PASS=$PASS

echo ""
echo "=== Load (6 instructions × 4 variants = 24 tests) ==="
for op in lw lh lb lhu lbu; do
    test_instr "$op t0, (#0, t1)" "Load-$op"
    test_instr "$op t0, (#4, t1)" "Load-$op"
    test_instr "$op t0, (#-4, t1)" "Load-$op"
    test_instr "$op a0, (#2047, a1)" "Load-$op"
done
test_instr "lw t0, (#-2048, t1)" "Load-lw"
test_instr "lh t0, (#-2048, t1)" "Load-lh"
test_instr "lb t0, (#-2048, t1)" "Load-lb"
test_instr "lhu t0, (#-2048, t1)" "Load-lhu"
echo "  Load: $((PASS - SHIFT_PASS)) passed"
LOAD_PASS=$PASS

echo ""
echo "=== Store (3 instructions × 4 variants = 12 tests) ==="
for op in sw sh sb; do
    test_instr "$op t0, (#0, t1)" "Store-$op"
    test_instr "$op t0, (#4, t1)" "Store-$op"
    test_instr "$op t0, (#-4, t1)" "Store-$op"
    test_instr "$op a0, (#2047, a1)" "Store-$op"
done
echo "  Store: $((PASS - LOAD_PASS)) passed"
STORE_PASS=$PASS

echo ""
echo "=== U-type (2 instructions × 4 variants = 8 tests) ==="
for op in lui auipc; do
    test_instr "$op t0, #0x00000000" "U-$op"
    test_instr "$op t0, #0x00001000" "U-$op"
    test_instr "$op t0, #0x12345000" "U-$op"
    test_instr "$op t0, #0xFFFFF000" "U-$op"
done
echo "  U-type: $((PASS - STORE_PASS)) passed"
U_PASS=$PASS

echo ""
echo "=== Branch (6 instructions × 4 variants = 24 tests) ==="
for op in beq bne blt bge bltu bgeu; do
    test_instr "$op t0, t1, #4" "Branch-$op"
    test_instr "$op t0, t1, #-4" "Branch-$op"
    test_instr "$op t0, t1, #256" "Branch-$op"
    test_instr "$op a0, a1, #-256" "Branch-$op"
done
echo "  Branch: $((PASS - U_PASS)) passed"
BRANCH_PASS=$PASS

echo ""
echo "=== JAL (1 instruction × 4 variants = 4 tests) ==="
test_instr "jal ra, #4" "JAL"
test_instr "jal ra, #-4" "JAL"
test_instr "jal t0, #1024" "JAL"
test_instr "jal zero, #-1024" "JAL"
echo "  JAL: $((PASS - BRANCH_PASS)) passed"
JAL_PASS=$PASS

echo ""
echo "=== JALR (1 instruction × 4 variants = 4 tests) ==="
test_instr "jalr ra, t0, #0" "JALR"
test_instr "jalr ra, t0, #4" "JALR"
test_instr "jalr ra, t0, #-4" "JALR"
test_instr "jalr t1, a0, #2047" "JALR"
echo "  JALR: $((PASS - JAL_PASS)) passed"
JALR_PASS=$PASS

echo ""
echo "=== System (4 instructions = 4 tests) ==="
test_instr "ecall" "System"
test_instr "ebreak" "System"
test_instr "fence" "System"
test_instr "fence.i" "System"
echo "  System: $((PASS - JALR_PASS)) passed"
SYS_PASS=$PASS

# ============================================================================
# M Extension - Multiply/Divide
# ============================================================================

echo ""
echo "=== M Extension (8 instructions × 2 variants = 16 tests) ==="
for op in mul mulh mulhsu mulhu div divu rem remu; do
    test_instr "$op t0, t1, t2" "M-$op"
    test_instr "$op a0, a1, a2" "M-$op"
done
echo "  M Extension: $((PASS - SYS_PASS)) passed"
M_PASS=$PASS

# ============================================================================
# A Extension - Atomics
# ============================================================================

echo ""
echo "=== A Extension (11 instructions × 2 variants = 22 tests) ==="
test_instr "lr.w t0, (t1)" "A-lr.w"
test_instr "lr.w a0, (a1)" "A-lr.w"
test_instr "sc.w t0, t2, (t1)" "A-sc.w"
test_instr "sc.w a0, a2, (a1)" "A-sc.w"
for op in amoswap.w amoadd.w amoxor.w amoand.w amoor.w amomin.w amomax.w amominu.w amomaxu.w; do
    test_instr "$op t0, t2, (t1)" "A-$op"
    test_instr "$op a0, a2, (a1)" "A-$op"
done
echo "  A Extension: $((PASS - M_PASS)) passed"
A_PASS=$PASS

# ============================================================================
# Zicsr Extension - CSR Instructions
# ============================================================================

echo ""
echo "=== Zicsr Extension (6 instructions × 2 variants = 12 tests) ==="
# Using numeric CSR addresses
for op in csrrw csrrs csrrc; do
    test_instr "$op t0, #0x300, t1" "Zicsr-$op"
    test_instr "$op a0, #0x341, a1" "Zicsr-$op"
done
for op in csrrwi csrrsi csrrci; do
    test_instr "$op t0, #0x300, #5" "Zicsr-$op"
    test_instr "$op a0, #0x341, #31" "Zicsr-$op"
done
echo "  Zicsr: $((PASS - A_PASS)) passed"
ZICSR_PASS=$PASS

# ============================================================================
# C Extension - Compressed Instructions
# ============================================================================

echo ""
echo "=== C Extension CR Format (4 instructions × 3 variants = 12 tests) ==="
test_instr "c.add a0, a1" "C-CR"
test_instr "c.add s0, s1" "C-CR"
test_instr "c.add a2, a3" "C-CR"
test_instr "c.mv a0, a1" "C-CR"
test_instr "c.mv s0, s1" "C-CR"
test_instr "c.mv a2, a3" "C-CR"
test_instr "c.jr ra" "C-CR"
test_instr "c.jr a0" "C-CR"
test_instr "c.jr a5" "C-CR"
test_instr "c.jalr ra" "C-CR"
test_instr "c.jalr a0" "C-CR"
test_instr "c.jalr a5" "C-CR"
echo "  C-CR: $((PASS - ZICSR_PASS)) passed"
C_CR_PASS=$PASS

echo ""
echo "=== C Extension CI Format (6 instructions × 4-5 variants = 25 tests) ==="
test_instr "c.addi a0, #1" "C-CI"
test_instr "c.addi a0, #-1" "C-CI"
test_instr "c.addi a0, #31" "C-CI"
test_instr "c.addi a0, #-32" "C-CI"
test_instr "c.li a0, #0" "C-CI"
test_instr "c.li a0, #1" "C-CI"
test_instr "c.li a0, #-1" "C-CI"
test_instr "c.li a0, #31" "C-CI"
test_instr "c.slli a0, #1" "C-CI"
test_instr "c.slli a0, #15" "C-CI"
test_instr "c.slli a0, #31" "C-CI"
test_instr "c.slli t0, #5" "C-CI"
test_instr "c.lui a0, #0x1000" "C-CI"
test_instr "c.lui a0, #0x12000" "C-CI"
test_instr "c.lui a0, #0x1F000" "C-CI"
test_instr "c.lui t0, #0x10000" "C-CI"
test_instr "c.addi16sp #16" "C-CI"
test_instr "c.addi16sp #-16" "C-CI"
test_instr "c.addi16sp #496" "C-CI"
test_instr "c.addi16sp #-512" "C-CI"
# c.lwsp: offset is 8-bit unsigned, scaled by 4: 0-252
test_instr "c.lwsp a0, #0" "C-CI"
test_instr "c.lwsp a0, #4" "C-CI"
test_instr "c.lwsp a0, #64" "C-CI"
test_instr "c.lwsp a0, #124" "C-CI"
test_instr "c.lwsp a1, #252" "C-CI"
echo "  C-CI: $((PASS - C_CR_PASS)) passed"
C_CI_PASS=$PASS

echo ""
echo "=== C Extension CSS Format (1 instruction × 4 variants = 4 tests) ==="
test_instr "c.swsp a0, #0" "C-CSS"
test_instr "c.swsp a0, #4" "C-CSS"
test_instr "c.swsp a0, #252" "C-CSS"
test_instr "c.swsp t0, #128" "C-CSS"
echo "  C-CSS: $((PASS - C_CI_PASS)) passed"
C_CSS_PASS=$PASS

echo ""
echo "=== C Extension CIW Format (1 instruction × 4 variants = 4 tests) ==="
test_instr "c.addi4spn s0, #4" "C-CIW"
test_instr "c.addi4spn s0, #8" "C-CIW"
test_instr "c.addi4spn s0, #256" "C-CIW"
test_instr "c.addi4spn a0, #1020" "C-CIW"
echo "  C-CIW: $((PASS - C_CSS_PASS)) passed"
C_CIW_PASS=$PASS

echo ""
echo "=== C Extension CL Format (1 instruction × 5 variants = 5 tests) ==="
# c.lw: 7-bit offset (5 bits scaled by 4), range 0-124
test_instr "c.lw s0, (#0, s1)" "C-CL"
test_instr "c.lw s0, (#4, s1)" "C-CL"
test_instr "c.lw a0, (#60, a1)" "C-CL"
test_instr "c.lw a0, (#64, a1)" "C-CL"
test_instr "c.lw a1, (#124, a0)" "C-CL"
echo "  C-CL: $((PASS - C_CIW_PASS)) passed"
C_CL_PASS=$PASS

echo ""
echo "=== C Extension CS Format (1 instruction × 5 variants = 5 tests) ==="
# c.sw: 7-bit offset (5 bits scaled by 4), range 0-124
test_instr "c.sw s0, (#0, s1)" "C-CS"
test_instr "c.sw s0, (#4, s1)" "C-CS"
test_instr "c.sw a0, (#60, a1)" "C-CS"
test_instr "c.sw a0, (#64, a1)" "C-CS"
test_instr "c.sw a1, (#124, a0)" "C-CS"
echo "  C-CS: $((PASS - C_CL_PASS)) passed"
C_CS_PASS=$PASS

echo ""
echo "=== C Extension CA Format (4 instructions × 2 variants = 8 tests) ==="
for op in c.sub c.xor c.or c.and; do
    test_instr "$op s0, s1" "C-CA"
    test_instr "$op a0, a1" "C-CA"
done
echo "  C-CA: $((PASS - C_CS_PASS)) passed"
C_CA_PASS=$PASS

echo ""
echo "=== C Extension CB Format (5 instructions × 4 variants = 20 tests) ==="
test_instr "c.beqz s0, #4" "C-CB"
test_instr "c.beqz s0, #-4" "C-CB"
test_instr "c.beqz a0, #254" "C-CB"
test_instr "c.beqz a0, #-256" "C-CB"
test_instr "c.bnez s0, #4" "C-CB"
test_instr "c.bnez s0, #-4" "C-CB"
test_instr "c.bnez a0, #254" "C-CB"
test_instr "c.bnez a0, #-256" "C-CB"
# c.srli/c.srai: 5-bit shift amount for RV32 (0-31)
test_instr "c.srli s0, #1" "C-CB"
test_instr "c.srli s0, #15" "C-CB"
test_instr "c.srli a0, #16" "C-CB"
test_instr "c.srli a1, #31" "C-CB"
test_instr "c.srai s0, #1" "C-CB"
test_instr "c.srai s0, #15" "C-CB"
test_instr "c.srai a0, #16" "C-CB"
test_instr "c.srai a1, #31" "C-CB"
test_instr "c.andi s0, #0" "C-CB"
test_instr "c.andi s0, #31" "C-CB"
test_instr "c.andi a0, #-1" "C-CB"
test_instr "c.andi a0, #-32" "C-CB"
echo "  C-CB: $((PASS - C_CA_PASS)) passed"
C_CB_PASS=$PASS

echo ""
echo "=== C Extension CJ Format (1 instruction × 4 variants = 4 tests) ==="
test_instr "c.j #4" "C-CJ"
test_instr "c.j #-4" "C-CJ"
test_instr "c.j #2046" "C-CJ"
test_instr "c.j #-2048" "C-CJ"
echo "  C-CJ: $((PASS - C_CB_PASS)) passed"
C_CJ_PASS=$PASS

echo ""
echo "=== C Extension System (2 instructions = 2 tests) ==="
test_instr "c.nop" "C-Sys"
test_instr "c.ebreak" "C-Sys"
echo "  C-System: $((PASS - C_CJ_PASS)) passed"
C_SYS_PASS=$PASS

# ============================================================================
# Summary
# ============================================================================

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
    echo ""
    echo "=========================================="
    echo "All known issues have been fixed!"
    echo "=========================================="
    exit 0
fi
