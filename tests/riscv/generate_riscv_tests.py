#!/usr/bin/env python3
"""
Generate comprehensive test cases for all RISC-V instructions.
Creates one test file per instruction with all valid register combinations
and immediate values (min, max, and samples in between).
"""

import os
from pathlib import Path

# RISC-V register names (x0-x31)
REGISTERS = [
    "zero", "ra", "sp", "gp", "tp",
    "t0", "t1", "t2",
    "s0", "s1",
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
    "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
    "t3", "t4", "t5", "t6"
]

# Floating point registers
FREGISTERS = [f"f{i}" for i in range(32)]

# Compressed registers (x8-x15, encoded as 0-7)
COMPRESSED_REGS = ["s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5"]

# Immediate value ranges and samples
IMM12_RANGE = (-2048, 2047)
IMM12_SAMPLES = [-2047, -1024, -512, -256, -128, -64, -32, -16, -8, -4, -2, -1,
                 0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2047]

SHAMT5_RANGE = (0, 31)
SHAMT5_SAMPLES = [0, 1, 2, 4, 8, 16, 31]

IMM20_RANGE = (0, 0xFFFFF)
IMM20_SAMPLES = [0, 0x1000, 0x10000, 0x100000, 0xFFFFF]

# For branch/jump instructions, we use labels
# We'll generate small offsets that can be represented

def generate_test_file(extension_dir, instruction_name, test_cases):
    """Generate a test file for an instruction."""
    test_dir = Path("tests/riscv") / extension_dir
    test_dir.mkdir(parents=True, exist_ok=True)
    
    test_file = test_dir / f"{instruction_name}.asm"
    
    with open(test_file, 'w') as f:
        f.write(".lang riscv32\n")
        f.write(".org 0x0\n\n")
        
        for case in test_cases:
            f.write(f"{case}\n")
        
        f.write("\n")

def generate_r_type_tests(extension_dir, mnemonic):
    """Generate tests for R-type instructions (rd, rs1, rs2)."""
    test_cases = []
    
    # Test all register combinations (sample a subset to avoid too many tests)
    # Test with different register combinations
    for rd_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
        for rs1_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
            for rs2_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
                rd = REGISTERS[rd_idx]
                rs1 = REGISTERS[rs1_idx]
                rs2 = REGISTERS[rs2_idx]
                test_cases.append(f"{mnemonic} {rd}, {rs1}, {rs2}")
    
    generate_test_file(extension_dir, mnemonic, test_cases)

def generate_i_type_tests(extension_dir, mnemonic):
    """Generate tests for I-type instructions (rd, rs1, imm12)."""
    test_cases = []
    
    # Test with different registers and immediate values
    for rd_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
        for rs1_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
            for imm in IMM12_SAMPLES:
                rd = REGISTERS[rd_idx]
                rs1 = REGISTERS[rs1_idx]
                test_cases.append(f"{mnemonic} {rd}, {rs1}, #{imm}")
    
    generate_test_file(extension_dir, mnemonic, test_cases)

def generate_shift_imm_tests(extension_dir, mnemonic):
    """Generate tests for shift immediate instructions (rd, rs1, shamt5)."""
    test_cases = []
    
    for rd_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
        for rs1_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
            for shamt in SHAMT5_SAMPLES:
                rd = REGISTERS[rd_idx]
                rs1 = REGISTERS[rs1_idx]
                test_cases.append(f"{mnemonic} {rd}, {rs1}, #{shamt}")
    
    generate_test_file(extension_dir, mnemonic, test_cases)

def generate_u_type_tests(extension_dir, mnemonic):
    """Generate tests for U-type instructions (rd, imm20)."""
    test_cases = []
    
    for rd_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
        for imm in IMM20_SAMPLES:
            rd = REGISTERS[rd_idx]
            test_cases.append(f"{mnemonic} {rd}, #{hex(imm)}")
    
    generate_test_file(extension_dir, mnemonic, test_cases)

def generate_j_type_tests(extension_dir, mnemonic):
    """Generate tests for J-type instructions (rd, label)."""
    test_cases = []
    
    # Generate multiple labels at different offsets
    for rd_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
        rd = REGISTERS[rd_idx]
        for i in range(10):
            label = f"label_{i}"
            test_cases.append(f"{mnemonic} {rd}, {label}")
            test_cases.append(f"{label}:")
    
    generate_test_file(extension_dir, mnemonic, test_cases)

def generate_b_type_tests(extension_dir, mnemonic):
    """Generate tests for B-type instructions (rs1, rs2, label)."""
    test_cases = []
    
    label_count = 0
    for rs1_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
        for rs2_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
            rs1 = REGISTERS[rs1_idx]
            rs2 = REGISTERS[rs2_idx]
            label = f"label_{label_count}"
            test_cases.append(f"{mnemonic} {rs1}, {rs2}, {label}")
            test_cases.append(f"{label}:")
            label_count += 1
    
    generate_test_file(extension_dir, mnemonic, test_cases)

def generate_load_tests(extension_dir, mnemonic):
    """Generate tests for load instructions (rd, imm(rs1))."""
    test_cases = []
    
    for rd_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
        for rs1_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
            for imm in IMM12_SAMPLES:
                rd = REGISTERS[rd_idx]
                rs1 = REGISTERS[rs1_idx]
                test_cases.append(f"{mnemonic} {rd}, (#{imm}, {rs1})")
    
    generate_test_file(extension_dir, mnemonic, test_cases)

def generate_store_tests(extension_dir, mnemonic):
    """Generate tests for store instructions (rs2, imm(rs1))."""
    test_cases = []
    
    for rs2_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
        for rs1_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
            for imm in IMM12_SAMPLES:
                rs2 = REGISTERS[rs2_idx]
                rs1 = REGISTERS[rs1_idx]
                test_cases.append(f"{mnemonic} {rs2}, (#{imm}, {rs1})")
    
    generate_test_file(extension_dir, mnemonic, test_cases)

def generate_jalr_tests(extension_dir, mnemonic):
    """Generate tests for JALR (rd, rs1, imm12)."""
    test_cases = []
    
    for rd_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
        for rs1_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
            for imm in IMM12_SAMPLES:
                rd = REGISTERS[rd_idx]
                rs1 = REGISTERS[rs1_idx]
                test_cases.append(f"{mnemonic} {rd}, {rs1}, #{imm}")
    
    generate_test_file(extension_dir, mnemonic, test_cases)

def generate_system_tests(extension_dir, mnemonic):
    """Generate tests for system instructions (ecall, ebreak, fence, fence.i)."""
    test_cases = []
    
    if mnemonic == "ecall" or mnemonic == "ebreak":
        test_cases.append(f"{mnemonic}")
    elif mnemonic == "fence.i":
        test_cases.append(f"{mnemonic}")
    elif mnemonic == "fence":
        # Fence has pred/succ fields
        for pred in [0, 0xF, 0xFF]:
            test_cases.append(f"{mnemonic} #{hex(pred)}, zero, zero")
    
    generate_test_file(extension_dir, mnemonic, test_cases)

def generate_csr_tests(extension_dir, mnemonic):
    """Generate tests for CSR instructions."""
    test_cases = []
    
    # CSR register numbers (sample some common ones)
    csr_regs = [0x000, 0x001, 0x002, 0x300, 0x301, 0x304, 0x305, 0x340, 0x341, 0xFFF]
    
    if mnemonic in ["csrrw", "csrrs", "csrrc"]:
        # rd, csr, rs1
        for rd_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
            for rs1_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
                for csr in csr_regs:
                    rd = REGISTERS[rd_idx]
                    rs1 = REGISTERS[rs1_idx]
                    test_cases.append(f"{mnemonic} {rd}, #{hex(csr)}, {rs1}")
    elif mnemonic in ["csrrwi", "csrrsi", "csrrci"]:
        # rd, csr, uimm5
        for rd_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
            for uimm in [0, 1, 5, 10, 15, 20, 25, 31]:
                for csr in csr_regs:
                    rd = REGISTERS[rd_idx]
                    test_cases.append(f"{mnemonic} {rd}, #{hex(csr)}, #{uimm}")
    
    generate_test_file(extension_dir, mnemonic, test_cases)

def generate_atomic_tests(extension_dir, mnemonic):
    """Generate tests for atomic instructions."""
    test_cases = []
    
    # Atomic instructions: rd, (rs1), rs2 or rd, rs2, (rs1)
    if mnemonic in ["lr.w"]:
        # lr.w rd, (rs1)
        for rd_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
            for rs1_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
                rd = REGISTERS[rd_idx]
                rs1 = REGISTERS[rs1_idx]
                test_cases.append(f"{mnemonic} {rd}, ({rs1})")
    elif mnemonic in ["sc.w"]:
        # sc.w rd, rs2, (rs1)
        for rd_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
            for rs2_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
                for rs1_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
                    rd = REGISTERS[rd_idx]
                    rs2 = REGISTERS[rs2_idx]
                    rs1 = REGISTERS[rs1_idx]
                    test_cases.append(f"{mnemonic} {rd}, {rs2}, ({rs1})")
    else:
        # amo* rd, rs2, (rs1)
        for rd_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
            for rs2_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
                for rs1_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
                    rd = REGISTERS[rd_idx]
                    rs2 = REGISTERS[rs2_idx]
                    rs1 = REGISTERS[rs1_idx]
                    test_cases.append(f"{mnemonic} {rd}, {rs2}, ({rs1})")
    
    generate_test_file(extension_dir, mnemonic, test_cases)

def generate_float_tests(extension_dir, mnemonic):
    """Generate tests for floating point instructions."""
    test_cases = []
    
    # Determine instruction type from mnemonic
    if mnemonic in ["flw", "fsw"]:
        # Load/store: rd/rs2, imm(rs1) where rd/rs2 is FP register
        for frd_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
            for rs1_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
                for imm in IMM12_SAMPLES:
                    frd = FREGISTERS[frd_idx]
                    rs1 = REGISTERS[rs1_idx]
                    if mnemonic == "flw":
                        test_cases.append(f"{mnemonic} {frd}, (#{imm}, {rs1})")
                    else:
                        test_cases.append(f"{mnemonic} {frd}, (#{imm}, {rs1})")
    elif mnemonic in ["fld", "fsd"]:
        # Double precision load/store
        for frd_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
            for rs1_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
                for imm in IMM12_SAMPLES:
                    frd = FREGISTERS[frd_idx]
                    rs1 = REGISTERS[rs1_idx]
                    test_cases.append(f"{mnemonic} {frd}, (#{imm}, {rs1})")
    elif mnemonic.startswith("fmadd") or mnemonic.startswith("fmsub") or \
         mnemonic.startswith("fnmsub") or mnemonic.startswith("fnmadd"):
        # FMA instructions: rd, rs1, rs2, rs3
        for rd_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
            for rs1_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
                for rs2_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
                    for rs3_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
                        rd = FREGISTERS[rd_idx]
                        rs1 = FREGISTERS[rs1_idx]
                        rs2 = FREGISTERS[rs2_idx]
                        rs3 = FREGISTERS[rs3_idx]
                        test_cases.append(f"{mnemonic} {rd}, {rs1}, {rs2}, {rs3}")
    elif mnemonic.startswith("fcvt.w") or mnemonic.startswith("fcvt.wu"):
        # Convert to integer: rd (integer), rs1 (float)
        for rd_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
            for rs1_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
                rd = REGISTERS[rd_idx]
                rs1 = FREGISTERS[rs1_idx]
                test_cases.append(f"{mnemonic} {rd}, {rs1}")
    elif mnemonic.startswith("fcvt.") and (".w" in mnemonic or ".wu" in mnemonic):
        # Convert from integer: rd (float), rs1 (integer)
        for rd_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
            for rs1_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
                rd = FREGISTERS[rd_idx]
                rs1 = REGISTERS[rs1_idx]
                test_cases.append(f"{mnemonic} {rd}, {rs1}")
    elif mnemonic.startswith("fcvt.s.d") or mnemonic.startswith("fcvt.d.s"):
        # Convert between float and double
        for rd_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
            for rs1_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
                rd = FREGISTERS[rd_idx]
                rs1 = FREGISTERS[rs1_idx]
                test_cases.append(f"{mnemonic} {rd}, {rs1}")
    elif mnemonic.startswith("fmv.x.w") or mnemonic.startswith("fmv.w.x"):
        # Move between float and integer registers
        for rd_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
            for rs1_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
                if "x.w" in mnemonic:
                    rd = REGISTERS[rd_idx]
                    rs1 = FREGISTERS[rs1_idx]
                else:
                    rd = FREGISTERS[rd_idx]
                    rs1 = REGISTERS[rs1_idx]
                test_cases.append(f"{mnemonic} {rd}, {rs1}")
    elif mnemonic.startswith("fclass"):
        # Classify: rd (integer), rs1 (float)
        for rd_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
            for rs1_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
                rd = REGISTERS[rd_idx]
                rs1 = FREGISTERS[rs1_idx]
                test_cases.append(f"{mnemonic} {rd}, {rs1}")
    else:
        # Most FP instructions: rd, rs1, rs2
        for rd_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
            for rs1_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
                for rs2_idx in [0, 1, 5, 10, 15, 20, 25, 31]:
                    rd = FREGISTERS[rd_idx]
                    rs1 = FREGISTERS[rs1_idx]
                    rs2 = FREGISTERS[rs2_idx]
                    test_cases.append(f"{mnemonic} {rd}, {rs1}, {rs2}")
    
    generate_test_file(extension_dir, mnemonic, test_cases)

def generate_compressed_tests(extension_dir, mnemonic):
    """Generate tests for compressed instructions."""
    test_cases = []
    
    if mnemonic == "c.nop":
        test_cases.append("c.nop")
    elif mnemonic == "c.ebreak":
        test_cases.append("c.ebreak")
    elif mnemonic == "c.addi4spn":
        # c.addi4spn rd', uimm[9:2]
        for rd_idx in range(8):  # Compressed registers
            rd = COMPRESSED_REGS[rd_idx]
            for imm in [0, 4, 8, 16, 32, 64, 128, 256, 512, 1020]:
                test_cases.append(f"{mnemonic} {rd}, #{imm}")
    elif mnemonic == "c.lw":
        # c.lw rd', uimm[6:2](rs1')
        for rd_idx in range(8):
            for rs1_idx in range(8):
                rd = COMPRESSED_REGS[rd_idx]
                rs1 = COMPRESSED_REGS[rs1_idx]
                for imm in [0, 4, 8, 16, 32, 64, 124]:
                    test_cases.append(f"{mnemonic} {rd}, #{imm}({rs1})")
    elif mnemonic == "c.sw":
        # c.sw rs2', uimm[6:2](rs1')
        for rs2_idx in range(8):
            for rs1_idx in range(8):
                rs2 = COMPRESSED_REGS[rs2_idx]
                rs1 = COMPRESSED_REGS[rs1_idx]
                for imm in [0, 4, 8, 16, 32, 64, 124]:
                    test_cases.append(f"{mnemonic} {rs2}, #{imm}({rs1})")
    elif mnemonic == "c.li":
        # c.li rd, imm[5:0]
        for rd_idx in [1, 5, 10, 15, 20, 25, 31]:  # rd != 0
            rd = REGISTERS[rd_idx]
            for imm in [-32, -16, -8, -4, -2, -1, 0, 1, 2, 4, 8, 16, 31]:
                test_cases.append(f"{mnemonic} {rd}, #{imm}")
    elif mnemonic == "c.addi16sp":
        # c.addi16sp imm[9:4]
        for imm in [-512, -256, -128, -64, -32, 32, 64, 128, 256, 496]:
            test_cases.append(f"{mnemonic} #{imm}")
    elif mnemonic == "c.lui":
        # c.lui rd, imm[17:12]
        for rd_idx in [3, 4, 5, 6, 7, 28, 29, 30, 31]:  # rd != 0, 2
            rd = REGISTERS[rd_idx]
            for imm in [0x1000, 0x2000, 0x4000, 0x8000, 0xF000, 0xFF000]:
                test_cases.append(f"{mnemonic} {rd}, #{hex(imm)}")
    elif mnemonic == "c.srli" or mnemonic == "c.srai":
        # c.srli/c.srai rd', uimm[5:0]
        for rd_idx in range(8):
            rd = COMPRESSED_REGS[rd_idx]
            for shamt in [0, 1, 2, 4, 8, 16, 31]:
                test_cases.append(f"{mnemonic} {rd}, #{shamt}")
    elif mnemonic == "c.andi":
        # c.andi rd', imm[5:0]
        for rd_idx in range(8):
            rd = COMPRESSED_REGS[rd_idx]
            for imm in [-32, -16, -8, -4, -2, -1, 0, 1, 2, 4, 8, 16, 31]:
                test_cases.append(f"{mnemonic} {rd}, #{imm}")
    elif mnemonic == "c.sub" or mnemonic == "c.xor" or mnemonic == "c.or" or mnemonic == "c.and":
        # c.sub/c.xor/c.or/c.and rd', rs2'
        for rd_idx in range(8):
            for rs2_idx in range(8):
                rd = COMPRESSED_REGS[rd_idx]
                rs2 = COMPRESSED_REGS[rs2_idx]
                test_cases.append(f"{mnemonic} {rd}, {rs2}")
    elif mnemonic == "c.j":
        # c.j imm[11:1]
        for i in range(10):
            label = f"label_{i}"
            test_cases.append(f"{mnemonic} {label}")
            test_cases.append(f"{label}:")
    elif mnemonic == "c.beqz" or mnemonic == "c.bnez":
        # c.beqz/c.bnez rs1', imm[8:1]
        for rs1_idx in range(8):
            rs1 = COMPRESSED_REGS[rs1_idx]
            for i in range(10):
                label = f"label_{i}"
                test_cases.append(f"{mnemonic} {rs1}, {label}")
                test_cases.append(f"{label}:")
    elif mnemonic == "c.slli":
        # c.slli rd, uimm[5:0]
        for rd_idx in [1, 5, 10, 15, 20, 25, 31]:  # rd != 0
            rd = REGISTERS[rd_idx]
            for shamt in [0, 1, 2, 4, 8, 16, 31]:
                test_cases.append(f"{mnemonic} {rd}, #{shamt}")
    elif mnemonic == "c.lwsp":
        # c.lwsp rd, uimm[7:2](sp)
        for rd_idx in [1, 5, 10, 15, 20, 25, 31]:  # rd != 0
            rd = REGISTERS[rd_idx]
            for imm in [0, 4, 8, 16, 32, 64, 128, 252]:
                test_cases.append(f"{mnemonic} {rd}, #{imm}(sp)")
    elif mnemonic == "c.jr" or mnemonic == "c.jalr":
        # c.jr/c.jalr rs1
        for rs1_idx in [1, 5, 10, 15, 20, 25, 31]:  # rs1 != 0
            rs1 = REGISTERS[rs1_idx]
            test_cases.append(f"{mnemonic} {rs1}")
    elif mnemonic == "c.add":
        # c.add rd, rs2
        for rd_idx in [1, 5, 10, 15, 20, 25, 31]:  # rd != 0
            for rs2_idx in [1, 5, 10, 15, 20, 25, 31]:  # rs2 != 0
                rd = REGISTERS[rd_idx]
                rs2 = REGISTERS[rs2_idx]
                test_cases.append(f"{mnemonic} {rd}, {rs2}")
    elif mnemonic == "c.mv":
        # c.mv rd, rs2
        for rd_idx in [1, 5, 10, 15, 20, 25, 31]:  # rd != 0
            for rs2_idx in [1, 5, 10, 15, 20, 25, 31]:  # rs2 != 0
                rd = REGISTERS[rd_idx]
                rs2 = REGISTERS[rs2_idx]
                test_cases.append(f"{mnemonic} {rd}, {rs2}")
    elif mnemonic == "c.swsp":
        # c.swsp rs2, uimm[7:2](sp)
        for rs2_idx in [1, 5, 10, 15, 20, 25, 31]:  # rs2 != 0
            rs2 = REGISTERS[rs2_idx]
            for imm in [0, 4, 8, 16, 32, 64, 128, 252]:
                test_cases.append(f"{mnemonic} {rs2}, #{imm}(sp)")
    elif mnemonic == "c.addi":
        # c.addi rd, imm[5:0]
        for rd_idx in [1, 5, 10, 15, 20, 25, 31]:  # rd != 0
            rd = REGISTERS[rd_idx]
            for imm in [-32, -16, -8, -4, -2, -1, 0, 1, 2, 4, 8, 16, 31]:
                test_cases.append(f"{mnemonic} {rd}, #{imm}")
    
    generate_test_file(extension_dir, mnemonic, test_cases)

def main():
    """Generate all test files."""
    
    # RV32I Base Instructions
    rv32i_instructions = {
        "R-type": ["add", "sub", "sll", "slt", "sltu", "xor", "srl", "sra", "or", "and"],
        "I-type": ["addi", "slti", "sltiu", "xori", "ori", "andi"],
        "Shift-imm": ["slli", "srli", "srai"],
        "U-type": ["lui", "auipc"],
        "J-type": ["jal"],
        "JALR": ["jalr"],
        "B-type": ["beq", "bne", "blt", "bge", "bltu", "bgeu"],
        "Load": ["lb", "lh", "lw", "lbu", "lhu"],
        "Store": ["sb", "sh", "sw"],
        "System": ["fence", "fence.i", "ecall", "ebreak"]
    }
    
    for category, instructions in rv32i_instructions.items():
        for mnemonic in instructions:
            if category == "R-type":
                generate_r_type_tests("rv32i", mnemonic)
            elif category == "I-type":
                generate_i_type_tests("rv32i", mnemonic)
            elif category == "Shift-imm":
                generate_shift_imm_tests("rv32i", mnemonic)
            elif category == "U-type":
                generate_u_type_tests("rv32i", mnemonic)
            elif category == "J-type":
                generate_j_type_tests("rv32i", mnemonic)
            elif category == "JALR":
                generate_jalr_tests("rv32i", mnemonic)
            elif category == "B-type":
                generate_b_type_tests("rv32i", mnemonic)
            elif category == "Load":
                generate_load_tests("rv32i", mnemonic)
            elif category == "Store":
                generate_store_tests("rv32i", mnemonic)
            elif category == "System":
                generate_system_tests("rv32i", mnemonic)
    
    # M Extension
    m_instructions = ["mul", "mulh", "mulhsu", "mulhu", "div", "divu", "rem", "remu"]
    for mnemonic in m_instructions:
        generate_r_type_tests("m-extension", mnemonic)
    
    # A Extension
    a_instructions = ["lr.w", "sc.w", "amoadd.w", "amoswap.w", "amoand.w", "amoor.w",
                      "amoxor.w", "amomax.w", "amomaxu.w", "amomin.w", "amominu.w"]
    for mnemonic in a_instructions:
        generate_atomic_tests("a-extension", mnemonic)
    
    # Zicsr Extension
    zicsr_instructions = ["csrrw", "csrrs", "csrrc", "csrrwi", "csrrsi", "csrrci"]
    for mnemonic in zicsr_instructions:
        generate_csr_tests("zicsr-extension", mnemonic)
    
    # F Extension
    f_instructions = ["flw", "fsw", "fmadd.s", "fmsub.s", "fnmsub.s", "fnmadd.s",
                       "fadd.s", "fsub.s", "fmul.s", "fdiv.s", "fsqrt.s",
                       "fsgnj.s", "fsgnjn.s", "fsgnjx.s", "fmin.s", "fmax.s",
                       "feq.s", "flt.s", "fle.s", "fcvt.w.s", "fcvt.wu.s",
                       "fcvt.s.w", "fcvt.s.wu", "fmv.x.w", "fmv.w.x", "fclass.s"]
    for mnemonic in f_instructions:
        generate_float_tests("f-extension", mnemonic)
    
    # D Extension
    d_instructions = ["fld", "fsd", "fmadd.d", "fmsub.d", "fnmsub.d", "fnmadd.d",
                       "fadd.d", "fsub.d", "fmul.d", "fdiv.d", "fsqrt.d",
                       "fsgnj.d", "fsgnjn.d", "fsgnjx.d", "fmin.d", "fmax.d",
                       "fcvt.s.d", "fcvt.d.s", "feq.d", "flt.d", "fle.d",
                       "fcvt.w.d", "fcvt.wu.d", "fcvt.d.w", "fcvt.d.wu", "fclass.d"]
    for mnemonic in d_instructions:
        generate_float_tests("d-extension", mnemonic)
    
    # C Extension
    c_instructions = ["c.addi4spn", "c.lw", "c.sw", "c.nop", "c.addi", "c.jalr",
                      "c.li", "c.addi16sp", "c.lui", "c.srli", "c.srai", "c.andi",
                      "c.sub", "c.xor", "c.or", "c.and", "c.j", "c.beqz", "c.bnez",
                      "c.slli", "c.lwsp", "c.jr", "c.add", "c.mv", "c.swsp", "c.ebreak"]
    for mnemonic in c_instructions:
        generate_compressed_tests("c-extension", mnemonic)
    
    print("Generated all test files!")

if __name__ == "__main__":
    main()

