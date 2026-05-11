#!/bin/bash
#
# Full Range RISC-V Validation Test
# Tests every instruction with full value range coverage
# Includes boundary values (min, max, zero, mid-range)
# Runtime: ~2-3 minutes
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
normalize_values() {
    local input="$1"
    echo "$input" | python3 -c "
import sys
import re

line = sys.stdin.read().strip()

def hex_to_dec(match):
    hex_val = match.group(1)
    try:
        val = int(hex_val, 16)
        if val >= 0x80000000:
            val = val - 0x100000000
        return f'#{val}'
    except:
        return match.group(0)

result = re.sub(r'#0x([0-9a-fA-F]+)', hex_to_dec, line, flags=re.IGNORECASE)
print(result, end='')
"
}

# Test a single instruction roundtrip
test_instr() {
    local instr="$1"
    local category="$2"

    echo -e ".lang riscv32\n.org 0x0\n$instr" > /tmp/test_full.asm
    "$GOODASM" --riscv32 /tmp/test_full.asm -o /tmp/test_full.bin 2>/dev/null

    if [ ! -s /tmp/test_full.bin ]; then
        echo "  FAIL: $instr (assembly failed)"
        FAILED_TESTS+=("[$category] $instr - assembly failed")
        ((FAIL++))
        return 1
    fi

    result=$("$GOODASM" --riscv32 -d /tmp/test_full.bin 2>/dev/null | grep -v "^Colliding" | grep -v "^[0-9]*:  \"" | head -1)

    orig_norm=$(echo "$instr" | tr '[:upper:]' '[:lower:]' | sed 's/[[:space:]]\+/ /g' | sed 's/,\s*/, /g' | sed 's/[[:space:]]*$//')
    result_norm=$(echo "$result" | tr '[:upper:]' '[:lower:]' | sed 's/[[:space:]]\+/ /g' | sed 's/,\s*/, /g' | sed 's/[[:space:]]*$//')

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
echo "Full Range RISC-V Validation Test"
echo "=========================================="
echo "Testing every instruction with full value ranges..."
echo ""

# ============================================================================
# RV32I Base Integer Instructions
# ============================================================================

echo "=== R-type ALU (10 instructions) ==="
# Test with various register combinations including edge registers
for op in add sub and or xor sll srl sra slt sltu; do
    test_instr "$op zero, zero, zero" "R-$op"
    test_instr "$op t0, t1, t2" "R-$op"
    test_instr "$op a0, a1, a2" "R-$op"
    test_instr "$op s0, s1, s2" "R-$op"
    test_instr "$op ra, sp, gp" "R-$op"
    test_instr "$op t6, s11, a7" "R-$op"
done
echo "  R-type: $PASS passed"
R_PASS=$PASS

echo ""
echo "=== I-type ALU (6 instructions) ==="
# 12-bit signed immediate: -2048 to 2047
for op in addi andi ori xori slti sltiu; do
    test_instr "$op t0, t1, #0" "I-$op"
    test_instr "$op t0, t1, #1" "I-$op"
    test_instr "$op t0, t1, #-1" "I-$op"
    test_instr "$op t0, t1, #127" "I-$op"
    test_instr "$op t0, t1, #-128" "I-$op"
    test_instr "$op t0, t1, #255" "I-$op"
    test_instr "$op t0, t1, #1024" "I-$op"
    test_instr "$op t0, t1, #-1024" "I-$op"
    test_instr "$op t0, t1, #2047" "I-$op"
    test_instr "$op t0, t1, #-2048" "I-$op"
done
echo "  I-type ALU: $((PASS - R_PASS)) passed"
I_PASS=$PASS

echo ""
echo "=== Shift Immediate (3 instructions) ==="
# 5-bit shift amount: 0-31
for op in slli srli srai; do
    test_instr "$op t0, t1, #0" "Shift-$op"
    test_instr "$op t0, t1, #1" "Shift-$op"
    test_instr "$op t0, t1, #7" "Shift-$op"
    test_instr "$op t0, t1, #8" "Shift-$op"
    test_instr "$op t0, t1, #15" "Shift-$op"
    test_instr "$op t0, t1, #16" "Shift-$op"
    test_instr "$op t0, t1, #24" "Shift-$op"
    test_instr "$op t0, t1, #31" "Shift-$op"
done
echo "  Shift: $((PASS - I_PASS)) passed"
SHIFT_PASS=$PASS

echo ""
echo "=== Load (5 instructions) ==="
# 12-bit signed offset: -2048 to 2047
for op in lw lh lb lhu lbu; do
    test_instr "$op t0, (#0, t1)" "Load-$op"
    test_instr "$op t0, (#1, t1)" "Load-$op"
    test_instr "$op t0, (#-1, t1)" "Load-$op"
    test_instr "$op t0, (#127, t1)" "Load-$op"
    test_instr "$op t0, (#-128, t1)" "Load-$op"
    test_instr "$op t0, (#1024, t1)" "Load-$op"
    test_instr "$op t0, (#-1024, t1)" "Load-$op"
    test_instr "$op t0, (#2047, t1)" "Load-$op"
    test_instr "$op t0, (#-2048, t1)" "Load-$op"
done
echo "  Load: $((PASS - SHIFT_PASS)) passed"
LOAD_PASS=$PASS

echo ""
echo "=== Store (3 instructions) ==="
# 12-bit signed offset: -2048 to 2047
for op in sw sh sb; do
    test_instr "$op t0, (#0, t1)" "Store-$op"
    test_instr "$op t0, (#1, t1)" "Store-$op"
    test_instr "$op t0, (#-1, t1)" "Store-$op"
    test_instr "$op t0, (#127, t1)" "Store-$op"
    test_instr "$op t0, (#-128, t1)" "Store-$op"
    test_instr "$op t0, (#1024, t1)" "Store-$op"
    test_instr "$op t0, (#-1024, t1)" "Store-$op"
    test_instr "$op t0, (#2047, t1)" "Store-$op"
    test_instr "$op t0, (#-2048, t1)" "Store-$op"
done
echo "  Store: $((PASS - LOAD_PASS)) passed"
STORE_PASS=$PASS

echo ""
echo "=== U-type (2 instructions) ==="
# 20-bit upper immediate, must be multiple of 0x1000
for op in lui auipc; do
    test_instr "$op t0, #0x00000000" "U-$op"
    test_instr "$op t0, #0x00001000" "U-$op"
    test_instr "$op t0, #0x00010000" "U-$op"
    test_instr "$op t0, #0x00100000" "U-$op"
    test_instr "$op t0, #0x12345000" "U-$op"
    test_instr "$op t0, #0x7FFFF000" "U-$op"
    test_instr "$op t0, #0x80000000" "U-$op"
    test_instr "$op t0, #0xFFFFF000" "U-$op"
done
echo "  U-type: $((PASS - STORE_PASS)) passed"
U_PASS=$PASS

echo ""
echo "=== Branch (6 instructions) ==="
# 13-bit signed offset, must be even: -4096 to 4094
for op in beq bne blt bge bltu bgeu; do
    test_instr "$op t0, t1, #2" "Branch-$op"
    test_instr "$op t0, t1, #-2" "Branch-$op"
    test_instr "$op t0, t1, #4" "Branch-$op"
    test_instr "$op t0, t1, #-4" "Branch-$op"
    test_instr "$op t0, t1, #256" "Branch-$op"
    test_instr "$op t0, t1, #-256" "Branch-$op"
    test_instr "$op t0, t1, #2048" "Branch-$op"
    test_instr "$op t0, t1, #-2048" "Branch-$op"
    test_instr "$op t0, t1, #4094" "Branch-$op"
    test_instr "$op t0, t1, #-4096" "Branch-$op"
done
echo "  Branch: $((PASS - U_PASS)) passed"
BRANCH_PASS=$PASS

echo ""
echo "=== JAL (1 instruction) ==="
# 21-bit signed offset, must be even: -1048576 to 1048574
test_instr "jal ra, #2" "JAL"
test_instr "jal ra, #-2" "JAL"
test_instr "jal ra, #4" "JAL"
test_instr "jal ra, #-4" "JAL"
test_instr "jal t0, #1024" "JAL"
test_instr "jal t0, #-1024" "JAL"
test_instr "jal t0, #65536" "JAL"
test_instr "jal t0, #-65536" "JAL"
test_instr "jal zero, #1048574" "JAL"
test_instr "jal zero, #-1048576" "JAL"
echo "  JAL: $((PASS - BRANCH_PASS)) passed"
JAL_PASS=$PASS

echo ""
echo "=== JALR (1 instruction) ==="
# 12-bit signed offset: -2048 to 2047
test_instr "jalr ra, t0, #0" "JALR"
test_instr "jalr ra, t0, #1" "JALR"
test_instr "jalr ra, t0, #-1" "JALR"
test_instr "jalr ra, t0, #127" "JALR"
test_instr "jalr ra, t0, #-128" "JALR"
test_instr "jalr ra, t0, #1024" "JALR"
test_instr "jalr ra, t0, #-1024" "JALR"
test_instr "jalr t1, a0, #2047" "JALR"
test_instr "jalr t1, a0, #-2048" "JALR"
echo "  JALR: $((PASS - JAL_PASS)) passed"
JALR_PASS=$PASS

echo ""
echo "=== System (4 instructions) ==="
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
echo "=== M Extension (8 instructions) ==="
for op in mul mulh mulhsu mulhu div divu rem remu; do
    test_instr "$op t0, t1, t2" "M-$op"
    test_instr "$op a0, a1, a2" "M-$op"
    test_instr "$op s0, s1, s2" "M-$op"
done
echo "  M Extension: $((PASS - SYS_PASS)) passed"
M_PASS=$PASS

# ============================================================================
# A Extension - Atomics
# ============================================================================

echo ""
echo "=== A Extension (11 instructions) ==="
test_instr "lr.w t0, (t1)" "A-lr.w"
test_instr "lr.w a0, (a1)" "A-lr.w"
test_instr "lr.w s0, (s1)" "A-lr.w"
test_instr "sc.w t0, t2, (t1)" "A-sc.w"
test_instr "sc.w a0, a2, (a1)" "A-sc.w"
test_instr "sc.w s0, s2, (s1)" "A-sc.w"
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
echo "=== Zicsr Extension (6 instructions) ==="
# CSR address: 12-bit (0-4095), uimm: 5-bit (0-31)
for op in csrrw csrrs csrrc; do
    test_instr "$op t0, #0x000, t1" "Zicsr-$op"
    test_instr "$op t0, #0x300, t1" "Zicsr-$op"
    test_instr "$op a0, #0x341, a1" "Zicsr-$op"
    test_instr "$op a0, #0xFFF, a1" "Zicsr-$op"
done
for op in csrrwi csrrsi csrrci; do
    test_instr "$op t0, #0x000, #0" "Zicsr-$op"
    test_instr "$op t0, #0x300, #5" "Zicsr-$op"
    test_instr "$op a0, #0x341, #15" "Zicsr-$op"
    test_instr "$op a0, #0xFFF, #31" "Zicsr-$op"
done
echo "  Zicsr: $((PASS - A_PASS)) passed"
ZICSR_PASS=$PASS

# ============================================================================
# C Extension - Compressed Instructions
# ============================================================================

echo ""
echo "=== C Extension CR Format ==="
# c.add, c.mv: any register except x0 for rd
# c.jr, c.jalr: any register except x0 for rs1
test_instr "c.add ra, sp" "C-CR"
test_instr "c.add a0, a1" "C-CR"
test_instr "c.add s0, s1" "C-CR"
test_instr "c.add t6, s11" "C-CR"
test_instr "c.mv ra, sp" "C-CR"
test_instr "c.mv a0, a1" "C-CR"
test_instr "c.mv s0, s1" "C-CR"
test_instr "c.mv t6, s11" "C-CR"
test_instr "c.jr ra" "C-CR"
test_instr "c.jr a0" "C-CR"
test_instr "c.jr t6" "C-CR"
test_instr "c.jalr ra" "C-CR"
test_instr "c.jalr a0" "C-CR"
test_instr "c.jalr t6" "C-CR"
echo "  C-CR: $((PASS - ZICSR_PASS)) passed"
C_CR_PASS=$PASS

echo ""
echo "=== C Extension CI Format ==="
# c.addi: 6-bit signed non-zero: -32 to -1, 1 to 31
test_instr "c.addi a0, #1" "C-CI"
test_instr "c.addi a0, #-1" "C-CI"
test_instr "c.addi a0, #15" "C-CI"
test_instr "c.addi a0, #-16" "C-CI"
test_instr "c.addi a0, #31" "C-CI"
test_instr "c.addi a0, #-32" "C-CI"
test_instr "c.addi t0, #7" "C-CI"
test_instr "c.addi s0, #-7" "C-CI"

# c.li: 6-bit signed: -32 to 31
test_instr "c.li a0, #0" "C-CI"
test_instr "c.li a0, #1" "C-CI"
test_instr "c.li a0, #-1" "C-CI"
test_instr "c.li a0, #15" "C-CI"
test_instr "c.li a0, #-16" "C-CI"
test_instr "c.li a0, #31" "C-CI"
test_instr "c.li a0, #-32" "C-CI"

# c.slli: 5-bit shift (RV32): 1-31
test_instr "c.slli a0, #1" "C-CI"
test_instr "c.slli a0, #7" "C-CI"
test_instr "c.slli a0, #8" "C-CI"
test_instr "c.slli a0, #15" "C-CI"
test_instr "c.slli a0, #16" "C-CI"
test_instr "c.slli a0, #24" "C-CI"
test_instr "c.slli a0, #31" "C-CI"
test_instr "c.slli t0, #5" "C-CI"

# c.lui: 6-bit signed, scaled to bits [17:12]: ±0x1F000 (excluding 0)
test_instr "c.lui a0, #0x1000" "C-CI"
test_instr "c.lui a0, #0x2000" "C-CI"
test_instr "c.lui a0, #0xF000" "C-CI"
test_instr "c.lui a0, #0x10000" "C-CI"
test_instr "c.lui a0, #0x1F000" "C-CI"
test_instr "c.lui a0, #0xFFFFF000" "C-CI"
test_instr "c.lui t0, #0xFFFFE000" "C-CI"

# c.addi16sp: 10-bit signed, multiple of 16: -512 to 496
test_instr "c.addi16sp #16" "C-CI"
test_instr "c.addi16sp #-16" "C-CI"
test_instr "c.addi16sp #32" "C-CI"
test_instr "c.addi16sp #-32" "C-CI"
test_instr "c.addi16sp #128" "C-CI"
test_instr "c.addi16sp #-128" "C-CI"
test_instr "c.addi16sp #256" "C-CI"
test_instr "c.addi16sp #-256" "C-CI"
test_instr "c.addi16sp #496" "C-CI"
test_instr "c.addi16sp #-512" "C-CI"

# c.lwsp: 8-bit unsigned, scaled by 4: 0-252
test_instr "c.lwsp a0, #0" "C-CI"
test_instr "c.lwsp a0, #4" "C-CI"
test_instr "c.lwsp a0, #8" "C-CI"
test_instr "c.lwsp a0, #60" "C-CI"
test_instr "c.lwsp a0, #64" "C-CI"
test_instr "c.lwsp a0, #124" "C-CI"
test_instr "c.lwsp a0, #128" "C-CI"
test_instr "c.lwsp a0, #188" "C-CI"
test_instr "c.lwsp a0, #192" "C-CI"
test_instr "c.lwsp a1, #252" "C-CI"
echo "  C-CI: $((PASS - C_CR_PASS)) passed"
C_CI_PASS=$PASS

echo ""
echo "=== C Extension CSS Format ==="
# c.swsp: 8-bit unsigned, scaled by 4: 0-252
test_instr "c.swsp a0, #0" "C-CSS"
test_instr "c.swsp a0, #4" "C-CSS"
test_instr "c.swsp a0, #60" "C-CSS"
test_instr "c.swsp a0, #64" "C-CSS"
test_instr "c.swsp a0, #124" "C-CSS"
test_instr "c.swsp a0, #128" "C-CSS"
test_instr "c.swsp a0, #192" "C-CSS"
test_instr "c.swsp t0, #252" "C-CSS"
echo "  C-CSS: $((PASS - C_CI_PASS)) passed"
C_CSS_PASS=$PASS

echo ""
echo "=== C Extension CIW Format ==="
# c.addi4spn: 10-bit unsigned, scaled by 4, non-zero: 4-1020
test_instr "c.addi4spn s0, #4" "C-CIW"
test_instr "c.addi4spn s0, #8" "C-CIW"
test_instr "c.addi4spn s0, #16" "C-CIW"
test_instr "c.addi4spn s0, #64" "C-CIW"
test_instr "c.addi4spn s0, #128" "C-CIW"
test_instr "c.addi4spn s0, #256" "C-CIW"
test_instr "c.addi4spn s0, #512" "C-CIW"
test_instr "c.addi4spn a0, #1020" "C-CIW"
echo "  C-CIW: $((PASS - C_CSS_PASS)) passed"
C_CIW_PASS=$PASS

echo ""
echo "=== C Extension CL Format ==="
# c.lw: 7-bit unsigned, scaled by 4: 0-124
test_instr "c.lw s0, (#0, s1)" "C-CL"
test_instr "c.lw s0, (#4, s1)" "C-CL"
test_instr "c.lw s0, (#8, s1)" "C-CL"
test_instr "c.lw s0, (#16, s1)" "C-CL"
test_instr "c.lw a0, (#32, a1)" "C-CL"
test_instr "c.lw a0, (#60, a1)" "C-CL"
test_instr "c.lw a0, (#64, a1)" "C-CL"
test_instr "c.lw a0, (#96, a1)" "C-CL"
test_instr "c.lw a1, (#124, a0)" "C-CL"
echo "  C-CL: $((PASS - C_CIW_PASS)) passed"
C_CL_PASS=$PASS

echo ""
echo "=== C Extension CS Format ==="
# c.sw: 7-bit unsigned, scaled by 4: 0-124
test_instr "c.sw s0, (#0, s1)" "C-CS"
test_instr "c.sw s0, (#4, s1)" "C-CS"
test_instr "c.sw s0, (#8, s1)" "C-CS"
test_instr "c.sw s0, (#16, s1)" "C-CS"
test_instr "c.sw a0, (#32, a1)" "C-CS"
test_instr "c.sw a0, (#60, a1)" "C-CS"
test_instr "c.sw a0, (#64, a1)" "C-CS"
test_instr "c.sw a0, (#96, a1)" "C-CS"
test_instr "c.sw a1, (#124, a0)" "C-CS"
echo "  C-CS: $((PASS - C_CL_PASS)) passed"
C_CS_PASS=$PASS

echo ""
echo "=== C Extension CA Format ==="
# Compressed registers only: s0-s7 (x8-x15) or a0-a7 (x10-x17, overlaps)
for op in c.sub c.xor c.or c.and; do
    test_instr "$op s0, s1" "C-CA"
    test_instr "$op s1, s0" "C-CA"
    test_instr "$op a0, a1" "C-CA"
    test_instr "$op a1, a0" "C-CA"
    test_instr "$op s0, a0" "C-CA"
    test_instr "$op a1, s1" "C-CA"
done
echo "  C-CA: $((PASS - C_CS_PASS)) passed"
C_CA_PASS=$PASS

echo ""
echo "=== C Extension CB Format ==="
# c.beqz, c.bnez: 9-bit signed, must be even: -256 to 254
test_instr "c.beqz s0, #2" "C-CB"
test_instr "c.beqz s0, #-2" "C-CB"
test_instr "c.beqz s0, #4" "C-CB"
test_instr "c.beqz s0, #-4" "C-CB"
test_instr "c.beqz a0, #64" "C-CB"
test_instr "c.beqz a0, #-64" "C-CB"
test_instr "c.beqz a0, #128" "C-CB"
test_instr "c.beqz a0, #-128" "C-CB"
test_instr "c.beqz a0, #254" "C-CB"
test_instr "c.beqz a0, #-256" "C-CB"
test_instr "c.bnez s0, #2" "C-CB"
test_instr "c.bnez s0, #-2" "C-CB"
test_instr "c.bnez s0, #4" "C-CB"
test_instr "c.bnez s0, #-4" "C-CB"
test_instr "c.bnez a0, #64" "C-CB"
test_instr "c.bnez a0, #-64" "C-CB"
test_instr "c.bnez a0, #128" "C-CB"
test_instr "c.bnez a0, #-128" "C-CB"
test_instr "c.bnez a0, #254" "C-CB"
test_instr "c.bnez a0, #-256" "C-CB"

# c.srli, c.srai: 5-bit shift (RV32): 1-31
test_instr "c.srli s0, #1" "C-CB"
test_instr "c.srli s0, #7" "C-CB"
test_instr "c.srli s0, #8" "C-CB"
test_instr "c.srli a0, #15" "C-CB"
test_instr "c.srli a0, #16" "C-CB"
test_instr "c.srli a1, #24" "C-CB"
test_instr "c.srli a1, #31" "C-CB"
test_instr "c.srai s0, #1" "C-CB"
test_instr "c.srai s0, #7" "C-CB"
test_instr "c.srai s0, #8" "C-CB"
test_instr "c.srai a0, #15" "C-CB"
test_instr "c.srai a0, #16" "C-CB"
test_instr "c.srai a1, #24" "C-CB"
test_instr "c.srai a1, #31" "C-CB"

# c.andi: 6-bit signed: -32 to 31
test_instr "c.andi s0, #0" "C-CB"
test_instr "c.andi s0, #1" "C-CB"
test_instr "c.andi s0, #-1" "C-CB"
test_instr "c.andi s0, #15" "C-CB"
test_instr "c.andi s0, #-16" "C-CB"
test_instr "c.andi a0, #31" "C-CB"
test_instr "c.andi a0, #-32" "C-CB"
echo "  C-CB: $((PASS - C_CA_PASS)) passed"
C_CB_PASS=$PASS

echo ""
echo "=== C Extension CJ Format ==="
# c.j: 12-bit signed, must be even: -2048 to 2046
test_instr "c.j #2" "C-CJ"
test_instr "c.j #-2" "C-CJ"
test_instr "c.j #4" "C-CJ"
test_instr "c.j #-4" "C-CJ"
test_instr "c.j #64" "C-CJ"
test_instr "c.j #-64" "C-CJ"
test_instr "c.j #256" "C-CJ"
test_instr "c.j #-256" "C-CJ"
test_instr "c.j #1024" "C-CJ"
test_instr "c.j #-1024" "C-CJ"
test_instr "c.j #2046" "C-CJ"
test_instr "c.j #-2048" "C-CJ"
echo "  C-CJ: $((PASS - C_CB_PASS)) passed"
C_CJ_PASS=$PASS

echo ""
echo "=== C Extension System ==="
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
    echo "Full range validation complete!"
    echo "=========================================="
    exit 0
fi
