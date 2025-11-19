#include "goodasm.h"

#include "galangriscv.h"

#include "gamnemonic.h"

#define mnem new GAMnemonic

// https://riscv.org/specifications/ratified/
// https://msyksphinz-self.github.io/riscv-isadoc/
// Follow RISC-V Instruction Organization from the ISA Manual

GALangRISCV::GALangRISCV() {
    endian = LITTLE;
    name = "riscv";
    maxbytes = 4; // Maximum instruction length (32-bit or 16-bit compressed)

    // Register names.
    regnames.clear();
    regnames
        <<"zero"<<"ra"<<"sp"<<"gp"<<"tp"
        <<"t0"<<"t1"<<"t2"
        <<"s0"<<"s1"
        <<"a0"<<"a1"<<"a2"<<"a3"<<"a4"<<"a5"<<"a6"<<"a7"
        <<"s2"<<"s3"<<"s4"<<"s5"<<"s6"<<"s7"<<"s8"<<"s9"<<"s10"<<"s11"
        <<"t3"<<"t4"<<"t5"<<"t6"
        // Floating point registers f0-f31
        <<"f0"<<"f1"<<"f2"<<"f3"<<"f4"<<"f5"<<"f6"<<"f7"
        <<"f8"<<"f9"<<"f10"<<"f11"<<"f12"<<"f13"<<"f14"<<"f15"
        <<"f16"<<"f17"<<"f18"<<"f19"<<"f20"<<"f21"<<"f22"<<"f23"
        <<"f24"<<"f25"<<"f26"<<"f27"<<"f28"<<"f29"<<"f30"<<"f31";
    
    threshold = 0.0;

    /* Integer Computational Instructions (RV32I Base) */

    /* R-Type Instructions */

    /*
        * Example: ADD rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x00000033  (funct7=0x00, funct3=000 + opcode=0x33)
        */
    insert(
        mnem("add", 4,
             "\x33\x00\x00\x00",    // pattern 32-bit LE: 0x00000033
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Add: rd = rs1 + rs2")
        ->example("add a0, a1, a2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7] - bit 7 in byte 0, bits 8-11 in byte 1
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15] - bit 15 in byte 1, bits 16-19 in byte 2
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0] - bits 20-24 in bytes 2-3
    
    /*
        * Example: SUB rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x40000033  (funct7=0x20=bit30, funct3=000 + opcode=0x33)
        */
    insert(
        mnem("sub", 4,
             "\x33\x00\x00\x40",    // pattern 32-bit LE: 0x40000033
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Subtract: rd = rs1 - rs2")
        ->example("sub a0, a1, a2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7] - bit 7 in byte 0, bits 8-11 in byte 1
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15] - bit 15 in byte 1, bits 16-19 in byte 2
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0] - bits 20-24 in bytes 2-3
    
    /*
        * Example: SLL rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x00100033  (funct7=0x00, funct3=001 + opcode=0x33)
        */
    insert(
        mnem("sll", 4,
             "\x33\x10\x00\x00",    // pattern 32-bit LE: 0x00100033
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Shift Left Logical: rd = rs1 << rs2[4:0]")
        ->example("sll a0, a1, a2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7] - bit 7 in byte 0, bits 8-11 in byte 1
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15] - bit 15 in byte 1, bits 16-19 in byte 2
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0] - bits 20-24 in bytes 2-3
    
    /*
        * Example: SLT rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x00200033  (funct7=0x00, funct3=010 + opcode=0x33)
        */
    insert(
        mnem("slt", 4,
             "\x33\x20\x00\x00",    // pattern 32-bit LE: 0x00200033
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Set Less Than: rd = (rs1 < rs2) ? 1 : 0 (signed)")
        ->example("slt a0, a1, a2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7] - bit 7 in byte 0, bits 8-11 in byte 1
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15] - bit 15 in byte 1, bits 16-19 in byte 2
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0] - bits 20-24 in bytes 2-3
    
    /*
        * Example: SLTU rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x00300033  (funct7=0x00, funct3=011 + opcode=0x33)
        */
    insert(
        mnem("sltu", 4,
             "\x33\x30\x00\x00",    // pattern 32-bit LE: 0x00300033
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Set Less Than Unsigned: rd = (rs1 < rs2) ? 1 : 0 (unsigned)")
        ->example("sltu a0, a1, a2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7] - bit 7 in byte 0, bits 8-11 in byte 1
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15] - bit 15 in byte 1, bits 16-19 in byte 2
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0] - bits 20-24 in bytes 2-3
    
    /*
        * Example: XOR rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x00400033  (funct7=0x00, funct3=100 + opcode=0x33)
        */
    insert(
        mnem("xor", 4,
             "\x33\x40\x00\x00",    // pattern 32-bit LE: 0x00400033
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Bitwise XOR: rd = rs1 ^ rs2")
        ->example("xor a0, a1, a2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7] - bit 7 in byte 0, bits 8-11 in byte 1
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15] - bit 15 in byte 1, bits 16-19 in byte 2
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0] - bits 20-24 in bytes 2-3
    
    /*
        * Example: SRL rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x00500033  (funct7=0x00, funct3=101 + opcode=0x33)
        */
    insert(
        mnem("srl", 4,
             "\x33\x50\x00\x00",    // pattern 32-bit LE: 0x00500033
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Shift Right Logical: rd = rs1 >> rs2[4:0]")
        ->example("srl a0, a1, a2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7] - bit 7 in byte 0, bits 8-11 in byte 1
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15] - bit 15 in byte 1, bits 16-19 in byte 2
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0] - bits 20-24 in bytes 2-3
    
    /*
        * Example: SRA rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x40500033  (funct7=0x20=bit30, funct3=101 + opcode=0x33)
        */
    insert(
        mnem("sra", 4,
             "\x33\x50\x00\x40",    // pattern 32-bit LE: 0x40500033
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Shift Right Arithmetic: rd = rs1 >> rs2[4:0] (sign-extended)")
        ->example("sra a0, a1, a2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7] - bit 7 in byte 0, bits 8-11 in byte 1
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15] - bit 15 in byte 1, bits 16-19 in byte 2
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0] - bits 20-24 in bytes 2-3
    
    /*
        * Example: OR rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x00600033  (funct7=0x00, funct3=110 + opcode=0x33)
        */
    insert(
        mnem("or", 4,
             "\x33\x60\x00\x00",    // pattern 32-bit LE: 0x00600033
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Bitwise OR: rd = rs1 | rs2")
        ->example("or a0, a1, a2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7] - bit 7 in byte 0, bits 8-11 in byte 1
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15] - bit 15 in byte 1, bits 16-19 in byte 2
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0] - bits 20-24 in bytes 2-3
    
    /*
        * Example: AND rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x00700033  (funct7=0x00, funct3=111 + opcode=0x33)
        */
    insert(
        mnem("and", 4,
             "\x33\x70\x00\x00",    // pattern 32-bit LE: 0x00700033
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Bitwise AND: rd = rs1 & rs2")
        ->example("and a0, a1, a2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7] - bit 7 in byte 0, bits 8-11 in byte 1
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15] - bit 15 in byte 1, bits 16-19 in byte 2
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0] - bits 20-24 in bytes 2-3
    
    /* I-Type Instructions */

    /*
        * Example: ADDI rd, rs1, imm12
        * 
        * Encoding:
        *   imm[11:0] rs1 funct3 rd opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00000013  (funct3=000 + opcode=0x13)
        */
    insert(
        mnem("addi", 4,
             "\x13\x00\x00\x00",    // pattern 32-bit LE: 0x00000013
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("Add immediate: rd = rs1 + imm")
        ->example("addi a0, a0, #1")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7] - bit 7 in byte 0, bits 8-11 in byte 1
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15] - bit 15 in byte 1, bits 16-19 in byte 2
        ->insert(new GAParameterRiscvImm12("\x00\x00\xF0\xFF")); // imm[11:0] bits [31:20] - bit 20-23 in byte 2, bits 24-31 in byte 3
    
    /*
        * Example: SLTI rd, rs1, imm12
        * 
        * Encoding:
        *   imm[11:0] rs1 funct3 rd opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00002013  (funct3=010 + opcode=0x13)
        */
    insert(
        mnem("slti", 4,
             "\x13\x20\x00\x00",    // pattern 32-bit LE: 0x00002013
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("Set Less Than Immediate: rd = (rs1 < imm) ? 1 : 0 (signed)")
        ->example("slti a0, a1, #10")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7] - bit 7 in byte 0, bits 8-11 in byte 1
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15] - bit 15 in byte 1, bits 16-19 in byte 2
        ->insert(new GAParameterRiscvImm12("\x00\x00\xF0\xFF")); // imm[11:0] bits [31:20] - bit 20-23 in byte 2, bits 24-31 in byte 3
    
    /*
        * Example: SLTIU rd, rs1, imm12
        * 
        * Encoding:
        *   imm[11:0] rs1 funct3 rd opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00003013  (funct3=011 + opcode=0x13)
        */
    insert(
        mnem("sltiu", 4,
             "\x13\x30\x00\x00",    // pattern 32-bit LE: 0x00003013
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("Set Less Than Immediate Unsigned: rd = (rs1 < imm) ? 1 : 0 (unsigned)")
        ->example("sltiu a0, a1, #10")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7] - bit 7 in byte 0, bits 8-11 in byte 1
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15] - bit 15 in byte 1, bits 16-19 in byte 2
        ->insert(new GAParameterRiscvImm12("\x00\x00\xF0\xFF")); // imm[11:0] bits [31:20] - bit 20-23 in byte 2, bits 24-31 in byte 3
    
    /*
        * Example: XORI rd, rs1, imm12
        * 
        * Encoding:
        *   imm[11:0] rs1 funct3 rd opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00004013  (funct3=100 + opcode=0x13)
        */
    insert(
        mnem("xori", 4,
             "\x13\x40\x00\x00",    // pattern 32-bit LE: 0x00004013
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("XOR Immediate: rd = rs1 ^ imm")
        ->example("xori a0, a1, #0xFF")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7] - bit 7 in byte 0, bits 8-11 in byte 1
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15] - bit 15 in byte 1, bits 16-19 in byte 2
        ->insert(new GAParameterRiscvImm12("\x00\x00\xF0\xFF")); // imm[11:0] bits [31:20] - bit 20-23 in byte 2, bits 24-31 in byte 3
    
    /*
        * Example: ORI rd, rs1, imm12
        * 
        * Encoding:
        *   imm[11:0] rs1 funct3 rd opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00006013  (funct3=110 + opcode=0x13)
        */
    insert(
        mnem("ori", 4,
             "\x13\x60\x00\x00",    // pattern 32-bit LE: 0x00006013
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("OR Immediate: rd = rs1 | imm")
        ->example("ori a0, a1, #0xFF")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7] - bit 7 in byte 0, bits 8-11 in byte 1
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15] - bit 15 in byte 1, bits 16-19 in byte 2
        ->insert(new GAParameterRiscvImm12("\x00\x00\xF0\xFF")); // imm[11:0] bits [31:20] - bit 20-23 in byte 2, bits 24-31 in byte 3
    
    /*
        * Example: ANDI rd, rs1, imm12
        * 
        * Encoding:
        *   imm[11:0] rs1 funct3 rd opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00007013  (funct3=111 + opcode=0x13)
        */
    insert(
        mnem("andi", 4,
             "\x13\x70\x00\x00",    // pattern 32-bit LE: 0x00007013
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("AND Immediate: rd = rs1 & imm")
        ->example("andi a0, a1, #0xFF")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7] - bit 7 in byte 0, bits 8-11 in byte 1
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15] - bit 15 in byte 1, bits 16-19 in byte 2
        ->insert(new GAParameterRiscvImm12("\x00\x00\xF0\xFF")); // imm[11:0] bits [31:20] - bit 20-23 in byte 2, bits 24-31 in byte 3

    /* I-type Shift Immediate Instructions */

    /*
        * Example: SLLI rd, rs1, shamt5
        * 
        * Encoding:
        *   shamt[4:0] rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x00100013  (funct7=0x00, funct3=001 + opcode=0x13)
        * 
        * shamt[4:0] is in bits [24:20]
        */
    insert(
        mnem("slli", 4,
             "\x13\x10\x00\x00",    // pattern 32-bit LE: 0x00100013
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Shift Left Logical Immediate: rd = rs1 << shamt")
        ->example("slli a0, a1, #5")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvShamt5("\x00\x00\xF0\x01")); // shamt[4:0] bits [24:20]

    /*
        * Example: SRLI rd, rs1, shamt5
        * 
        * Encoding:
        *   shamt[4:0] rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x00500013  (funct7=0x00, funct3=101 + opcode=0x13)
        */
    insert(
        mnem("srli", 4,
             "\x13\x50\x00\x00",    // pattern 32-bit LE: 0x00500013
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Shift Right Logical Immediate: rd = rs1 >> shamt")
        ->example("srli a0, a1, #5")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvShamt5("\x00\x00\xF0\x01")); // shamt[4:0] bits [24:20]

    /*
        * Example: SRAI rd, rs1, shamt5
        * 
        * Encoding:
        *   shamt[4:0] rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x40500013  (funct7=0x20, funct3=101 + opcode=0x13)
        */
    insert(
        mnem("srai", 4,
             "\x13\x50\x00\x40",    // pattern 32-bit LE: 0x40500013
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Shift Right Arithmetic Immediate: rd = rs1 >> shamt (sign-extended)")
        ->example("srai a0, a1, #5")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvShamt5("\x00\x00\xF0\x01")); // shamt[4:0] bits [24:20]
    
    /* Control Transfer Instructions */

    /* U-Type Control Transfer Instructions */

    /*
        * Example: LUI rd, imm20
        * 
        * Encoding:
        *   imm[31:12] rd opcode
        *
        * Mask  :   0x0000007F  (mask opcode)
        * Value :   0x00000037  (opcode=0x37)
        */
    insert(
        mnem("lui", 4,
             "\x37\x00\x00\x00",    // pattern 32-bit LE: 0x00000037
             "\x7F\x00\x00\x00"))   // mask: opcode only
        ->help("Load Upper Immediate: rd = imm << 12")
        ->example("lui a0, #0x12345000")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvUtypeImm20("\x00\xF0\xFF\xFF")); // imm[31:12] bits [31:12]

    /*
        * Example: AUIPC rd, imm20
        * 
        * Encoding:
        *   imm[31:12] rd opcode
        *
        * Mask  :   0x0000007F  (mask opcode)
        * Value :   0x00000017  (opcode=0x17)
        */
    insert(
        mnem("auipc", 4,
             "\x17\x00\x00\x00",    // pattern 32-bit LE: 0x00000017
             "\x7F\x00\x00\x00"))   // mask: opcode only
        ->help("Add Upper Immediate to PC: rd = PC + (imm << 12)")
        ->example("auipc a0, #0x12345000")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvUtypeImm20("\x00\xF0\xFF\xFF")); // imm[31:12] bits [31:12]

    /* J-Type Control Transfer Instructions */

    /*
        * Example: JAL rd, imm21
        * 
        * Encoding:
        *   imm[20|10:1|11|19:12] rd opcode
        *
        * Mask  :   0x0000007F  (mask opcode)
        * Value :   0x0000006F  (opcode=0x6F)
        */
    insert(
        mnem("jal", 4,
             "\x6F\x00\x00\x00",    // pattern 32-bit LE: 0x0000006F
             "\x7F\x00\x00\x00"))   // mask: opcode only
        ->help("Jump and Link: rd = PC + 4; PC += imm")
        ->example("jal ra, #8")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvJtypeImm21("\x00\xF0\xFF\xFF")); // imm[20:1] in bits [31:12]

    /* I-Type Control Transfer Instructions */

    /*
        * Example: JALR rd, rs1, imm12
        * 
        * Encoding:
        *   imm[11:0] rs1 funct3 rd opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00000067  (funct3=000 + opcode=0x67)
        */
    insert(
        mnem("jalr", 4,
             "\x67\x00\x00\x00",    // pattern 32-bit LE: 0x00000067
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("Jump and Link Register: rd = PC + 4; PC = (rs1 + imm) & ~1")
        ->example("jalr ra, a1, #0")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvImm12("\x00\x00\xF0\xFF")); // imm[11:0] bits [31:20]

    /* B-Type Control Transfer Instructions */

    /*
        * Example: BEQ rs1, rs2, imm13
        * 
        * Encoding:
        *   imm[12|10:5] rs2 rs1 funct3 imm[4:1|11] opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00000063  (funct3=000 + opcode=0x63)
        */
    insert(
        mnem("beq", 4,
             "\x63\x00\x00\x00",    // pattern 32-bit LE: 0x00000063
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("Branch if Equal: if (rs1 == rs2) PC += imm")
        ->example("beq a0, a1, #8")
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]
        ->insert(new GAParameterRiscvBtypeImm13("\x80\x0F\x00\xFE")); // imm[12:1] split encoding

    /*
        * Example: BNE rs1, rs2, imm13
        * 
        * Encoding:
        *   imm[12|10:5] rs2 rs1 funct3 imm[4:1|11] opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00001063  (funct3=001 + opcode=0x63)
        */
    insert(
        mnem("bne", 4,
             "\x63\x10\x00\x00",    // pattern 32-bit LE: 0x00100063
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("Branch if Not Equal: if (rs1 != rs2) PC += imm")
        ->example("bne a0, a1, #8")
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]
        ->insert(new GAParameterRiscvBtypeImm13("\x80\x0F\x00\xFE")); // imm[12:1] split encoding

    /*
        * Example: BLT rs1, rs2, imm13
        * 
        * Encoding:
        *   imm[12|10:5] rs2 rs1 funct3 imm[4:1|11] opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00004063  (funct3=100 + opcode=0x63)
        */
    insert(
        mnem("blt", 4,
             "\x63\x40\x00\x00",    // pattern 32-bit LE: 0x00004063
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("Branch if Less Than: if (rs1 < rs2) PC += imm (signed)")
        ->example("blt a0, a1, #8")
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]
        ->insert(new GAParameterRiscvBtypeImm13("\x80\x0F\x00\xFE")); // imm[12:1] split encoding

    /*
        * Example: BGE rs1, rs2, imm13
        * 
        * Encoding:
        *   imm[12|10:5] rs2 rs1 funct3 imm[4:1|11] opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00005063  (funct3=101 + opcode=0x63)
        */
    insert(
        mnem("bge", 4,
             "\x63\x50\x00\x00",    // pattern 32-bit LE: 0x00005063
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("Branch if Greater or Equal: if (rs1 >= rs2) PC += imm (signed)")
        ->example("bge a0, a1, #8")
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]
        ->insert(new GAParameterRiscvBtypeImm13("\x80\x0F\x00\xFE")); // imm[12:1] split encoding

    /*
        * Example: BLTU rs1, rs2, imm13
        * 
        * Encoding:
        *   imm[12|10:5] rs2 rs1 funct3 imm[4:1|11] opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00006063  (funct3=110 + opcode=0x63)
        */
    insert(
        mnem("bltu", 4,
             "\x63\x60\x00\x00",    // pattern 32-bit LE: 0x00006063
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("Branch if Less Than Unsigned: if (rs1 < rs2) PC += imm (unsigned)")
        ->example("bltu a0, a1, #8")
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]
        ->insert(new GAParameterRiscvBtypeImm13("\x80\x0F\x00\xFE")); // imm[12:1] split encoding

    /*
        * Example: BGEU rs1, rs2, imm13
        * 
        * Encoding:
        *   imm[12|10:5] rs2 rs1 funct3 imm[4:1|11] opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00007063  (funct3=111 + opcode=0x63)
        */
    insert(
        mnem("bgeu", 4,
             "\x63\x70\x00\x00",    // pattern 32-bit LE: 0x00007063
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("Branch if Greater or Equal Unsigned: if (rs1 >= rs2) PC += imm (unsigned)")
        ->example("bgeu a0, a1, #8")
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]
        ->insert(new GAParameterRiscvBtypeImm13("\x80\x0F\x00\xFE")); // imm[12:1] split encoding

    /* Load Instructions */

    /*
        * Example: LB rd, imm(rs1)
        * 
        * Encoding:
        *   imm[11:0] rs1 funct3 rd opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00000003  (funct3=000 + opcode=0x03)
        */
    insert(
        mnem("lb", 4,
             "\x03\x00\x00\x00",    // pattern 32-bit LE: 0x00000003
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("Load Byte (signed): rd = sign-extend(M[rs1 + imm][7:0])")
        ->example("lb a0, (#4, a1)")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->group('(') // imm(rs1) group
        ->insert(new GAParameterRiscvImm12("\x00\x00\xF0\xFF")) // imm[11:0] bits [31:20]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: LH rd, imm(rs1)
        * 
        * Encoding:
        *   imm[11:0] rs1 funct3 rd opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00001003  (funct3=001 + opcode=0x03)
        */
    insert(
        mnem("lh", 4,
             "\x03\x10\x00\x00",    // pattern 32-bit LE: 0x00001003
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("Load Halfword (signed): rd = sign-extend(M[rs1 + imm][15:0])")
        ->example("lh a0, (#4, a1)")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->group('(') // imm(rs1) group
        ->insert(new GAParameterRiscvImm12("\x00\x00\xF0\xFF")) // imm[11:0] bits [31:20]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: LW rd, imm(rs1)
        * 
        * Encoding:
        *   imm[11:0] rs1 funct3 rd opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00002003  (funct3=010 + opcode=0x03)
        */
    insert(
        mnem("lw", 4,
             "\x03\x20\x00\x00",    // pattern 32-bit LE: 0x00002003
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("Load Word: rd = M[rs1 + imm][31:0]")
        ->example("lw a0, (#4, a1)")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->group('(') // imm(rs1) group
        ->insert(new GAParameterRiscvImm12("\x00\x00\xF0\xFF")) // imm[11:0] bits [31:20]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: LBU rd, imm(rs1)
        * 
        * Encoding:
        *   imm[11:0] rs1 funct3 rd opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00004003  (funct3=100 + opcode=0x03)
        */
    insert(
        mnem("lbu", 4,
             "\x03\x40\x00\x00",    // pattern 32-bit LE: 0x00004003
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("Load Byte Unsigned: rd = zero-extend(M[rs1 + imm][7:0])")
        ->example("lbu a0, (#4, a1)")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->group('(') // imm(rs1) group
        ->insert(new GAParameterRiscvImm12("\x00\x00\xF0\xFF")) // imm[11:0] bits [31:20]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: LHU rd, imm(rs1)
        * 
        * Encoding:
        *   imm[11:0] rs1 funct3 rd opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00005003  (funct3=101 + opcode=0x03)
        */
    insert(
        mnem("lhu", 4,
             "\x03\x50\x00\x00",    // pattern 32-bit LE: 0x00005003
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("Load Halfword Unsigned: rd = zero-extend(M[rs1 + imm][15:0])")
        ->example("lhu a0, (#4, a1)")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->group('(') // imm(rs1) group
        ->insert(new GAParameterRiscvImm12("\x00\x00\xF0\xFF")) // imm[11:0] bits [31:20]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /* Store Instructions */

    /*
        * Example: SB rs2, imm(rs1)
        * 
        * Encoding:
        *   imm[11:5] rs2 rs1 funct3 imm[4:0] opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00000023  (funct3=000 + opcode=0x23)
        */
    insert(
        mnem("sb", 4,
             "\x23\x00\x00\x00",    // pattern 32-bit LE: 0x00000023
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("Store Byte: M[rs1 + imm] = rs2[7:0]")
        ->example("sb a1, (#4, a0)")
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]
        ->group('(') // imm(rs1) group
        ->insert(new GAParameterRiscvStypeImm12("\x80\x0F\x00\xFE")) // imm[11:0] split encoding
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: SH rs2, imm(rs1)
        * 
        * Encoding:
        *   imm[11:5] rs2 rs1 funct3 imm[4:0] opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00001023  (funct3=001 + opcode=0x23)
        */
    insert(
        mnem("sh", 4,
             "\x23\x10\x00\x00",    // pattern 32-bit LE: 0x00001023
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("Store Halfword: M[rs1 + imm] = rs2[15:0]")
        ->example("sh a1, (#4, a0)")
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]
        ->group('(') // imm(rs1) group
        ->insert(new GAParameterRiscvStypeImm12("\x80\x0F\x00\xFE")) // imm[11:0] split encoding
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: SW rs2, imm(rs1)
        * 
        * Encoding:
        *   imm[11:5] rs2 rs1 funct3 imm[4:0] opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00002023  (funct3=010 + opcode=0x23)
        */
    insert(
        mnem("sw", 4,
             "\x23\x20\x00\x00",    // pattern 32-bit LE: 0x00002023
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("Store Word: M[rs1 + imm] = rs2[31:0]")
        ->example("sw a1, (#4, a0)")
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]
        ->group('(') // imm(rs1) group
        ->insert(new GAParameterRiscvStypeImm12("\x80\x0F\x00\xFE")) // imm[11:0] split encoding
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]
    
    /* System Instructions */

    /* Memory Ordering Instructions (RV32I Base) */

    /*
        * Example: FENCE pred, succ
        * 
        * Encoding:
        *   imm[11:0] rs1 funct3 rd opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x0000000F  (funct3=000 + opcode=0x0F)
        * 
        * pred[3:0] in imm[3:0] (bits [23:20])
        * succ[3:0] in imm[7:4] (bits [27:24])
        * rs1 and rd are typically zero but can be non-zero
        */
    // Simple fence (no arguments) - defaults to fence iorw, iorw (full barrier)
    // Encoding: 0x0FF0000F - pred=1111, succ=1111, rs1=x0, rd=x0
    insert(
        mnem("fence", 4,
             "\x0F\x00\xF0\x0F",    // pattern 32-bit LE: 0x0FF0000F (fence iorw, iorw)
             "\xFF\xFF\xFF\xFF"))  // exact match - no variable bits
        ->help("Memory Ordering Fence (full barrier): fence iorw, iorw")
        ->example("fence");

    // Fence with explicit pred/succ and registers
    insert(
        mnem("fence", 4,
             "\x0F\x00\x00\x00",    // pattern 32-bit LE: 0x0000000F
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("Memory Ordering Fence: orders memory operations")
        ->example("fence #0xFF, zero, zero")
        ->insert(new GAParameterRiscvFencePredSucc("\x00\x00\xF0\x0F")) // pred/succ in imm[7:0]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15] (typically zero)
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")); // RD: bits [11:7] (typically zero)

    /*
        * Example: FENCE.I
        * 
        * Encoding:
        *   imm[11:0] rs1 funct3 rd opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x0000100F  (funct3=001 + opcode=0x0F)
        * 
        * All fields except funct3 and opcode are zero
        */
    insert(
        mnem("fence.i", 4,
             "\x0F\x10\x00\x00",    // pattern 32-bit LE: 0x0000100F
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("Instruction Fence: synchronizes instruction and data streams")
        ->example("fence.i");
    
    /* Environment Call and Breakpoints */

    /*
        * Example: ECALL
        * 
        * Encoding:
        *   imm[11:0] rs1 funct3 rd opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00000073  (funct3=000 + opcode=0x73)
        * 
        * All fields except funct3 and opcode are zero
        * Note: imm[0]=0, so no mask needed for byte 2 (unlike EBREAK)
        */
    insert(
        mnem("ecall", 4,
             "\x73\x00\x00\x00",    // pattern 32-bit LE: 0x00000073
             "\x7F\x70\x10\x00"))   // mask: opcode + funct3 + imm[0] (bit 20 = byte 2 bit 4, must be 0)
        ->help("Environment Call: makes a request to the execution environment")
        ->example("ecall");

    /*
        * Example: EBREAK
        * 
        * Encoding:
        *   imm[11:0] rs1 funct3 rd opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00100073  (funct3=000 + opcode=0x73 + imm[0]=1)
        * 
        * imm[0]=1 (bit 20), all other fields except funct3 and opcode are zero
        */
    insert(
        mnem("ebreak", 4,
             "\x73\x00\x10\x00",    // pattern 32-bit LE: 0x00100073 (imm[0]=1 in bit 20)
             "\x7F\x70\x10\x00"))   // mask: opcode + funct3 + imm[0] (bit 20 = byte 2 bit 4)
        ->help("Environment Break: causes a breakpoint exception")
        ->example("ebreak");

    /* M Extension Instructions */

    /*
        * Example: MUL rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x02000033  (funct7=0x01, funct3=000 + opcode=0x33)
        */
    insert(
        mnem("mul", 4,
             "\x33\x00\x00\x02",    // pattern 32-bit LE: 0x02000033
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Multiply: rd = (rs1 * rs2)[XLEN-1:0] (lower XLEN bits)")
        ->example("mul a0, a1, a2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - only bit 0 of byte 3 (bit 24)

    /*
        * Example: MULH rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x02001033  (funct7=0x01, funct3=001 + opcode=0x33)
        */
    insert(
        mnem("mulh", 4,
             "\x33\x10\x00\x02",    // pattern 32-bit LE: 0x02001033
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Multiply High (signed × signed): rd = (rs1 * rs2)[2*XLEN-1:XLEN]")
        ->example("mulh a0, a1, a2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - only bit 0 of byte 3 (bit 24)

    /*
        * Example: MULHSU rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x02002033  (funct7=0x01, funct3=010 + opcode=0x33)
        */
    insert(
        mnem("mulhsu", 4,
             "\x33\x20\x00\x02",    // pattern 32-bit LE: 0x02002033
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Multiply High (signed × unsigned): rd = (rs1 * rs2)[2*XLEN-1:XLEN]")
        ->example("mulhsu a0, a1, a2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - only bit 0 of byte 3 (bit 24)

    /*
        * Example: MULHU rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x02003033  (funct7=0x01, funct3=011 + opcode=0x33)
        */
    insert(
        mnem("mulhu", 4,
             "\x33\x30\x00\x02",    // pattern 32-bit LE: 0x02003033
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Multiply High (unsigned × unsigned): rd = (rs1 * rs2)[2*XLEN-1:XLEN]")
        ->example("mulhu a0, a1, a2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - only bit 0 of byte 3 (bit 24)

    /*
        * Example: DIV rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x02004033  (funct7=0x01, funct3=100 + opcode=0x33)
        */
    insert(
        mnem("div", 4,
             "\x33\x40\x00\x02",    // pattern 32-bit LE: 0x02004033
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Divide (signed): rd = rs1 / rs2")
        ->example("div a0, a1, a2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - only bit 0 of byte 3 (bit 24)

    /*
        * Example: DIVU rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x02005033  (funct7=0x01, funct3=101 + opcode=0x33)
        */
    insert(
        mnem("divu", 4,
             "\x33\x50\x00\x02",    // pattern 32-bit LE: 0x02005033
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Divide (unsigned): rd = rs1 / rs2")
        ->example("divu a0, a1, a2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - only bit 0 of byte 3 (bit 24)

    /*
        * Example: REM rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x02006033  (funct7=0x01, funct3=110 + opcode=0x33)
        */
    insert(
        mnem("rem", 4,
             "\x33\x60\x00\x02",    // pattern 32-bit LE: 0x02006033
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Remainder (signed): rd = rs1 % rs2")
        ->example("rem a0, a1, a2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - only bit 0 of byte 3 (bit 24)

    /*
        * Example: REMU rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x02007033  (funct7=0x01, funct3=111 + opcode=0x33)
        */
    insert(
        mnem("remu", 4,
             "\x33\x70\x00\x02",    // pattern 32-bit LE: 0x02007033
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Remainder (unsigned): rd = rs1 % rs2")
        ->example("remu a0, a1, a2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - only bit 0 of byte 3 (bit 24)

    /* Atomic Memory Instructions Extension (RV32A) */

    /*
        * Example: LR.W rd, (rs1)
        * 
        * Encoding:
        *   funct5 aq rl rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xF8F0707F  (mask funct5 + aq + rl + funct3 + opcode, rs2 must be 0)
        * Value :   0x1000202F  (funct5=00010, aq=0, rl=0, rs2=00000, funct3=010, opcode=0x2F)
        * 
        * funct5[4:0] = 00010 (0x02) in bits [31:27]
        * aq = 0 in bit [26]
        * rl = 0 in bit [25]
        * rs2 = 00000 (reserved, must be zero) in bits [24:20]
        */
    insert(
        mnem("lr.w", 4,
             "\x2F\x20\x00\x10",    // pattern 32-bit LE: 0x1000202F
             "\x7F\x70\x00\xF8"))   // mask: opcode + funct3 + funct5 (aq/rl default to 0)
        ->help("Load Reserved Word: rd = M[rs1]; reserve address")
        ->example("lr.w a0, (a1)")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->group('(') // (rs1) group
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: SC.W rd, rs2, (rs1)
        * 
        * Encoding:
        *   funct5 aq rl rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xF8F0707F  (mask funct5 + aq + rl + funct3 + opcode)
        * Value :   0x1800202F  (funct5=00011, aq=0, rl=0, funct3=010, opcode=0x2F)
        */
    insert(
        mnem("sc.w", 4,
             "\x2F\x20\x00\x18",    // pattern 32-bit LE: 0x1800202F
             "\x7F\x70\x00\xF8"))   // mask: opcode + funct3 + funct5
        ->help("Store Conditional Word: if reserved, M[rs1] = rs2, rd = 0; else rd != 0")
        ->example("sc.w a0, a2, (a1)")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20]
        ->group('(') // (rs1) group
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: AMOSWAP.W rd, rs2, (rs1)
        * 
        * Encoding:
        *   funct5 aq rl rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xF8F0707F  (mask funct5 + aq + rl + funct3 + opcode)
        * Value :   0x0800202F  (funct5=00001, aq=0, rl=0, funct3=010, opcode=0x2F)
        */
    insert(
        mnem("amoswap.w", 4,
             "\x2F\x20\x00\x08",    // pattern 32-bit LE: 0x0800202F
             "\x7F\x70\x00\xF8"))   // mask: opcode + funct3 + funct5
        ->help("Atomic Memory Swap Word: temp = M[rs1]; M[rs1] = rs2; rd = temp")
        ->example("amoswap.w a0, a2, (a1)")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20]
        ->group('(') // (rs1) group
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: AMOADD.W rd, rs2, (rs1)
        * 
        * Encoding:
        *   funct5 aq rl rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xF8F0707F  (mask funct5 + aq + rl + funct3 + opcode)
        * Value :   0x0000202F  (funct5=00000, aq=0, rl=0, funct3=010, opcode=0x2F)
        */
    insert(
        mnem("amoadd.w", 4,
             "\x2F\x20\x00\x00",    // pattern 32-bit LE: 0x0000202F
             "\x7F\x70\x00\xF8"))   // mask: opcode + funct3 + funct5
        ->help("Atomic Memory Add Word: temp = M[rs1]; M[rs1] = temp + rs2; rd = temp")
        ->example("amoadd.w a0, a2, (a1)")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20]
        ->group('(') // (rs1) group
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: AMOXOR.W rd, rs2, (rs1)
        * 
        * Encoding:
        *   funct5 aq rl rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xF8F0707F  (mask funct5 + aq + rl + funct3 + opcode)
        * Value :   0x2000202F  (funct5=00100, aq=0, rl=0, funct3=010, opcode=0x2F)
        */
    insert(
        mnem("amoxor.w", 4,
             "\x2F\x20\x00\x20",    // pattern 32-bit LE: 0x2000202F
             "\x7F\x70\x00\xF8"))   // mask: opcode + funct3 + funct5
        ->help("Atomic Memory XOR Word: temp = M[rs1]; M[rs1] = temp ^ rs2; rd = temp")
        ->example("amoxor.w a0, a2, (a1)")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20]
        ->group('(') // (rs1) group
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: AMOAND.W rd, rs2, (rs1)
        * 
        * Encoding:
        *   funct5 aq rl rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xF8F0707F  (mask funct5 + aq + rl + funct3 + opcode)
        * Value :   0x6000202F  (funct5=01100, aq=0, rl=0, funct3=010, opcode=0x2F)
        */
    insert(
        mnem("amoand.w", 4,
             "\x2F\x20\x00\x60",    // pattern 32-bit LE: 0x6000202F
             "\x7F\x70\x00\xF8"))   // mask: opcode + funct3 + funct5
        ->help("Atomic Memory AND Word: temp = M[rs1]; M[rs1] = temp & rs2; rd = temp")
        ->example("amoand.w a0, a2, (a1)")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20]
        ->group('(') // (rs1) group
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: AMOOR.W rd, rs2, (rs1)
        * 
        * Encoding:
        *   funct5 aq rl rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xF8F0707F  (mask funct5 + aq + rl + funct3 + opcode)
        * Value :   0x4000202F  (funct5=01000, aq=0, rl=0, funct3=010, opcode=0x2F)
        */
    insert(
        mnem("amoor.w", 4,
             "\x2F\x20\x00\x40",    // pattern 32-bit LE: 0x4000202F
             "\x7F\x70\x00\xF8"))   // mask: opcode + funct3 + funct5
        ->help("Atomic Memory OR Word: temp = M[rs1]; M[rs1] = temp | rs2; rd = temp")
        ->example("amoor.w a0, a2, (a1)")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20]
        ->group('(') // (rs1) group
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: AMOMIN.W rd, rs2, (rs1)
        * 
        * Encoding:
        *   funct5 aq rl rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xF8F0707F  (mask funct5 + aq + rl + funct3 + opcode)
        * Value :   0x8000202F  (funct5=10000, aq=0, rl=0, funct3=010, opcode=0x2F)
        */
    insert(
        mnem("amomin.w", 4,
             "\x2F\x20\x00\x80",    // pattern 32-bit LE: 0x8000202F
             "\x7F\x70\x00\xF8"))   // mask: opcode + funct3 + funct5
        ->help("Atomic Memory Minimum Word (signed): temp = M[rs1]; M[rs1] = min(temp, rs2); rd = temp")
        ->example("amomin.w a0, a2, (a1)")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20]
        ->group('(') // (rs1) group
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: AMOMAX.W rd, rs2, (rs1)
        * 
        * Encoding:
        *   funct5 aq rl rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xF8F0707F  (mask funct5 + aq + rl + funct3 + opcode)
        * Value :   0xA000202F  (funct5=10100, aq=0, rl=0, funct3=010, opcode=0x2F)
        */
    insert(
        mnem("amomax.w", 4,
             "\x2F\x20\x00\xA0",    // pattern 32-bit LE: 0xA000202F
             "\x7F\x70\x00\xF8"))   // mask: opcode + funct3 + funct5
        ->help("Atomic Memory Maximum Word (signed): temp = M[rs1]; M[rs1] = max(temp, rs2); rd = temp")
        ->example("amomax.w a0, a2, (a1)")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20]
        ->group('(') // (rs1) group
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: AMOMINU.W rd, rs2, (rs1)
        * 
        * Encoding:
        *   funct5 aq rl rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xF8F0707F  (mask funct5 + aq + rl + funct3 + opcode)
        * Value :   0xC000202F  (funct5=11000, aq=0, rl=0, funct3=010, opcode=0x2F)
        */
    insert(
        mnem("amominu.w", 4,
             "\x2F\x20\x00\xC0",    // pattern 32-bit LE: 0xC000202F
             "\x7F\x70\x00\xF8"))   // mask: opcode + funct3 + funct5
        ->help("Atomic Memory Minimum Word (unsigned): temp = M[rs1]; M[rs1] = min(temp, rs2); rd = temp")
        ->example("amominu.w a0, a2, (a1)")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20]
        ->group('(') // (rs1) group
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: AMOMAXU.W rd, rs2, (rs1)
        * 
        * Encoding:
        *   funct5 aq rl rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xF8F0707F  (mask funct5 + aq + rl + funct3 + opcode)
        * Value :   0xE000202F  (funct5=11100, aq=0, rl=0, funct3=010, opcode=0x2F)
        */
    insert(
        mnem("amomaxu.w", 4,
             "\x2F\x20\x00\xE0",    // pattern 32-bit LE: 0xE000202F
             "\x7F\x70\x00\xF8"))   // mask: opcode + funct3 + funct5
        ->help("Atomic Memory Maximum Word (unsigned): temp = M[rs1]; M[rs1] = max(temp, rs2); rd = temp")
        ->example("amomaxu.w a0, a2, (a1)")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20]
        ->group('(') // (rs1) group
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /* ZICSR Instructions Extension */

    /*
        * Example: CSRRW rd, csr, rs1
        * 
        * Encoding:
        *   csr[11:0] rs1 funct3 rd opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00001073  (funct3=001 + opcode=0x73)
        * 
        * Atomic Read/Write CSR: rd = CSR; CSR = rs1
        */
    insert(
        mnem("csrrw", 4,
             "\x73\x10\x00\x00",    // pattern 32-bit LE: 0x00001073
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("CSR Read/Write: rd = CSR; CSR = rs1")
        ->example("csrrw a0, #0x300, a1")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvCsr12("\x00\x00\xF0\xFF")) // CSR: bits [31:20]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: CSRRS rd, csr, rs1
        * 
        * Encoding:
        *   csr[11:0] rs1 funct3 rd opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00002073  (funct3=010 + opcode=0x73)
        * 
        * Atomic Read and Set Bits in CSR: rd = CSR; CSR = CSR | rs1
        */
    insert(
        mnem("csrrs", 4,
             "\x73\x20\x00\x00",    // pattern 32-bit LE: 0x00002073
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("CSR Read and Set Bits: rd = CSR; CSR = CSR | rs1")
        ->example("csrrs a0, #0x300, a1")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvCsr12("\x00\x00\xF0\xFF")) // CSR: bits [31:20]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: CSRRC rd, csr, rs1
        * 
        * Encoding:
        *   csr[11:0] rs1 funct3 rd opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00003073  (funct3=011 + opcode=0x73)
        * 
        * Atomic Read and Clear Bits in CSR: rd = CSR; CSR = CSR & ~rs1
        */
    insert(
        mnem("csrrc", 4,
             "\x73\x30\x00\x00",    // pattern 32-bit LE: 0x00003073
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("CSR Read and Clear Bits: rd = CSR; CSR = CSR & ~rs1")
        ->example("csrrc a0, #0x300, a1")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvCsr12("\x00\x00\xF0\xFF")) // CSR: bits [31:20]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: CSRRWI rd, csr, uimm5
        * 
        * Encoding:
        *   csr[11:0] uimm[4:0] funct3 rd opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00005073  (funct3=101 + opcode=0x73)
        * 
        * Atomic Read/Write CSR Immediate: rd = CSR; CSR = uimm[4:0]
        */
    insert(
        mnem("csrrwi", 4,
             "\x73\x50\x00\x00",    // pattern 32-bit LE: 0x00005073
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("CSR Read/Write Immediate: rd = CSR; CSR = uimm[4:0]")
        ->example("csrrwi a0, #0x300, #5")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvCsr12("\x00\x00\xF0\xFF")) // CSR: bits [31:20]
        ->insert(new GAParameterRiscvUimm5("\x00\x80\x0F\x00")); // uimm[4:0]: bits [19:15]

    /*
        * Example: CSRRSI rd, csr, uimm5
        * 
        * Encoding:
        *   csr[11:0] uimm[4:0] funct3 rd opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00006073  (funct3=110 + opcode=0x73)
        * 
        * Atomic Read and Set Bits in CSR Immediate: rd = CSR; CSR = CSR | uimm[4:0]
        */
    insert(
        mnem("csrrsi", 4,
             "\x73\x60\x00\x00",    // pattern 32-bit LE: 0x00006073
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("CSR Read and Set Bits Immediate: rd = CSR; CSR = CSR | uimm[4:0]")
        ->example("csrrsi a0, #0x300, #5")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvCsr12("\x00\x00\xF0\xFF")) // CSR: bits [31:20]
        ->insert(new GAParameterRiscvUimm5("\x00\x80\x0F\x00")); // uimm[4:0]: bits [19:15]

    /*
        * Example: CSRRCI rd, csr, uimm5
        * 
        * Encoding:
        *   csr[11:0] uimm[4:0] funct3 rd opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00007073  (funct3=111 + opcode=0x73)
        * 
        * Atomic Read and Clear Bits in CSR Immediate: rd = CSR; CSR = CSR & ~uimm[4:0]
        */
    insert(
        mnem("csrrci", 4,
             "\x73\x70\x00\x00",    // pattern 32-bit LE: 0x00007073
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("CSR Read and Clear Bits Immediate: rd = CSR; CSR = CSR & ~uimm[4:0]")
        ->example("csrrci a0, #0x300, #5")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvCsr12("\x00\x00\xF0\xFF")) // CSR: bits [31:20]
        ->insert(new GAParameterRiscvUimm5("\x00\x80\x0F\x00")); // uimm[4:0]: bits [19:15]

    /* F Extension Instructions */

    /*
        * Example: FLW rd, imm12(rs1)
        * 
        * Encoding:
        *   imm[11:0] rs1 funct3 rd opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00002007  (funct3=010 + opcode=0x07)
        */
    insert(
        mnem("flw", 4,
             "\x07\x20\x00\x00",    // pattern 32-bit LE: 0x00002007
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("Floating-Point Load Word: rd = M[rs1 + imm12]")
        ->example("flw f0, (#4, a1)")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->group('(') // (rs1) group
        ->insert(new GAParameterRiscvImm12("\x00\x00\xF0\xFF")) // imm[11:0]: bits [31:20]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: FSW rs2, imm12(rs1)
        * 
        * Encoding:
        *   imm[11:5] rs2 rs1 funct3 imm[4:0] opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00002027  (funct3=010 + opcode=0x27)
        */
    insert(
        mnem("fsw", 4,
             "\x27\x20\x00\x00",    // pattern 32-bit LE: 0x00002027
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("Floating-Point Store Word: M[rs1 + imm12] = rs2")
        ->example("fsw f0, (#4, a1)")
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]
        ->group('(') // (rs1) group
        ->insert(new GAParameterRiscvStypeImm12("\x00\x00\xF0\xFE")) // imm[11:0]: split encoding
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: FMADD.S rd, rs1, rs2, rs3
        * 
        * Encoding:
        *   rs3 funct2 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0x6000707F  (mask funct2 + funct3 + opcode)
        * Value :   0x00000043  (funct2=00, funct3=000 + opcode=0x43)
        */
    insert(
        mnem("fmadd.s", 4,
             "\x43\x00\x00\x00",    // pattern 32-bit LE: 0x00000043
             "\x7F\x70\x00\x06"))   // mask: opcode + funct3 + funct2
        ->help("Floating-Point Fused Multiply-Add: rd = rs1 * rs2 + rs3")
        ->example("fmadd.s f0, f1, f2, f3")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]
        ->insert(new GAParameterRiscvReg("\x00\x00\x00\xF8")); // RS3: bits [31:27] - byte 3 bits [7:3]

    /*
        * Example: FMSUB.S rd, rs1, rs2, rs3
        * 
        * Encoding:
        *   rs3 funct2 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0x6000707F  (mask funct2 + funct3 + opcode)
        * Value :   0x00000047  (funct2=00, funct3=000 + opcode=0x47)
        */
    insert(
        mnem("fmsub.s", 4,
             "\x47\x00\x00\x00",    // pattern 32-bit LE: 0x00000047
             "\x7F\x70\x00\x06"))   // mask: opcode + funct3 + funct2
        ->help("Floating-Point Fused Multiply-Subtract: rd = rs1 * rs2 - rs3")
        ->example("fmsub.s f0, f1, f2, f3")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]
        ->insert(new GAParameterRiscvReg("\x00\x00\x00\xF8")); // RS3: bits [31:27] - byte 3 bits [7:3]

    /*
        * Example: FNMSUB.S rd, rs1, rs2, rs3
        * 
        * Encoding:
        *   rs3 funct2 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0x6000707F  (mask funct2 + funct3 + opcode)
        * Value :   0x0000004B  (funct2=00, funct3=000 + opcode=0x4B)
        */
    insert(
        mnem("fnmsub.s", 4,
             "\x4B\x00\x00\x00",    // pattern 32-bit LE: 0x0000004B
             "\x7F\x70\x00\x06"))   // mask: opcode + funct3 + funct2
        ->help("Floating-Point Negative Fused Multiply-Subtract: rd = -(rs1 * rs2) + rs3")
        ->example("fnmsub.s f0, f1, f2, f3")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]
        ->insert(new GAParameterRiscvReg("\x00\x00\x00\xF8")); // RS3: bits [31:27] - byte 3 bits [7:3]

    /*
        * Example: FNMADD.S rd, rs1, rs2, rs3
        * 
        * Encoding:
        *   rs3 funct2 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0x6000707F  (mask funct2 + funct3 + opcode)
        * Value :   0x0000004F  (funct2=00, funct3=000 + opcode=0x4F)
        */
    insert(
        mnem("fnmadd.s", 4,
             "\x4F\x00\x00\x00",    // pattern 32-bit LE: 0x0000004F
             "\x7F\x70\x00\x06"))   // mask: opcode + funct3 + funct2
        ->help("Floating-Point Negative Fused Multiply-Add: rd = -(rs1 * rs2) - rs3")
        ->example("fnmadd.s f0, f1, f2, f3")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]
        ->insert(new GAParameterRiscvReg("\x00\x00\x00\xF8")); // RS3: bits [31:27] - byte 3 bits [7:3]

    /*
        * Example: FADD.S rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x00000053  (funct7=0x00, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fadd.s", 4,
             "\x53\x00\x00\x00",    // pattern 32-bit LE: 0x00000053
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Floating-Point Add: rd = rs1 + rs2")
        ->example("fadd.s f0, f1, f2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]

    /*
        * Example: FSUB.S rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x08000053  (funct7=0x08, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fsub.s", 4,
             "\x53\x00\x00\x08",    // pattern 32-bit LE: 0x08000053
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Floating-Point Subtract: rd = rs1 - rs2")
        ->example("fsub.s f0, f1, f2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]

    /*
        * Example: FMUL.S rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x10000053  (funct7=0x10, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fmul.s", 4,
             "\x53\x00\x00\x10",    // pattern 32-bit LE: 0x10000053
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Floating-Point Multiply: rd = rs1 * rs2")
        ->example("fmul.s f0, f1, f2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]

    /*
        * Example: FDIV.S rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x18000053  (funct7=0x18, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fdiv.s", 4,
             "\x53\x00\x00\x18",    // pattern 32-bit LE: 0x18000053
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Floating-Point Divide: rd = rs1 / rs2")
        ->example("fdiv.s f0, f1, f2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]

    /*
        * Example: FSQRT.S rd, rs1
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFFF0707F  (mask funct7 + funct3 + opcode + rs2)
        * Value :   0x58000053  (funct7=0x2C, rs2=0, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fsqrt.s", 4,
             "\x53\x00\x00\x58",    // pattern 32-bit LE: 0x58000053
             "\x7F\x70\x00\xFF"))   // mask: opcode + funct3 + funct7 + rs2
        ->help("Floating-Point Square Root: rd = sqrt(rs1)")
        ->example("fsqrt.s f0, f1")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: FSGNJ.S rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x20000053  (funct7=0x20, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fsgnj.s", 4,
             "\x53\x00\x00\x20",    // pattern 32-bit LE: 0x20000053
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Floating-Point Sign Injection: rd = {rs2[31], rs1[30:0]}")
        ->example("fsgnj.s f0, f1, f2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]

    /*
        * Example: FSGNJN.S rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x20001053  (funct7=0x20, funct3=001 + opcode=0x53)
        */
    insert(
        mnem("fsgnjn.s", 4,
             "\x53\x10\x00\x20",    // pattern 32-bit LE: 0x20001053
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Floating-Point Sign Injection Negative: rd = {~rs2[31], rs1[30:0]}")
        ->example("fsgnjn.s f0, f1, f2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]

    /*
        * Example: FSGNJX.S rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x20002053  (funct7=0x20, funct3=010 + opcode=0x53)
        */
    insert(
        mnem("fsgnjx.s", 4,
             "\x53\x20\x00\x20",    // pattern 32-bit LE: 0x20002053
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Floating-Point Sign Injection XOR: rd = {rs2[31]^rs1[31], rs1[30:0]}")
        ->example("fsgnjx.s f0, f1, f2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]

    /*
        * Example: FMIN.S rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x28000053  (funct7=0x28, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fmin.s", 4,
             "\x53\x00\x00\x28",    // pattern 32-bit LE: 0x28000053
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Floating-Point Minimum: rd = min(rs1, rs2)")
        ->example("fmin.s f0, f1, f2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]

    /*
        * Example: FMAX.S rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x28001053  (funct7=0x28, funct3=001 + opcode=0x53)
        */
    insert(
        mnem("fmax.s", 4,
             "\x53\x10\x00\x28",    // pattern 32-bit LE: 0x28001053
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Floating-Point Maximum: rd = max(rs1, rs2)")
        ->example("fmax.s f0, f1, f2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]

    /*
        * Example: FEQ.S rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0xA0002053  (funct7=0x50, funct3=010 + opcode=0x53)
        */
    insert(
        mnem("feq.s", 4,
             "\x53\x20\x00\xA0",    // pattern 32-bit LE: 0xA0002053
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Floating-Point Equal Compare: rd = (rs1 == rs2) ? 1 : 0")
        ->example("feq.s a0, f1, f2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]

    /*
        * Example: FLT.S rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0xA0001053  (funct7=0x50, funct3=001 + opcode=0x53)
        */
    insert(
        mnem("flt.s", 4,
             "\x53\x10\x00\xA0",    // pattern 32-bit LE: 0xA0001053
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Floating-Point Less Than Compare: rd = (rs1 < rs2) ? 1 : 0")
        ->example("flt.s a0, f1, f2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]

    /*
        * Example: FLE.S rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0xA0000053  (funct7=0x50, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fle.s", 4,
             "\x53\x00\x00\xA0",    // pattern 32-bit LE: 0xA0000053
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Floating-Point Less Than or Equal Compare: rd = (rs1 <= rs2) ? 1 : 0")
        ->example("fle.s a0, f1, f2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]

    /*
        * Example: FCVT.W.S rd, rs1
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFFF0707F  (mask funct7 + funct3 + opcode + rs2)
        * Value :   0xC0000053  (funct7=0x60, rs2=0, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fcvt.w.s", 4,
             "\x53\x00\x00\xC0",    // pattern 32-bit LE: 0xC0000053
             "\x7F\x70\x00\xFF"))   // mask: opcode + funct3 + funct7 + rs2
        ->help("Floating-Point Convert to Word: rd = (int32_t)rs1")
        ->example("fcvt.w.s a0, f1")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: FCVT.WU.S rd, rs1
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFFF0707F  (mask funct7 + funct3 + opcode + rs2)
        * Value :   0xC0001053  (funct7=0x60, rs2=1, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fcvt.wu.s", 4,
             "\x53\x10\x00\xC0",    // pattern 32-bit LE: 0xC0001053
             "\x7F\x70\x00\xFF"))   // mask: opcode + funct3 + funct7 + rs2
        ->help("Floating-Point Convert to Word Unsigned: rd = (uint32_t)rs1")
        ->example("fcvt.wu.s a0, f1")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: FCVT.S.W rd, rs1
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFFF0707F  (mask funct7 + funct3 + opcode + rs2)
        * Value :   0xD0000053  (funct7=0x68, rs2=0, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fcvt.s.w", 4,
             "\x53\x00\x00\xD0",    // pattern 32-bit LE: 0xD0000053
             "\x7F\x70\x00\xFF"))   // mask: opcode + funct3 + funct7 + rs2
        ->help("Floating-Point Convert from Word: rd = (float)rs1")
        ->example("fcvt.s.w f0, a1")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: FCVT.S.WU rd, rs1
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFFF0707F  (mask funct7 + funct3 + opcode + rs2)
        * Value :   0xD0001053  (funct7=0x68, rs2=1, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fcvt.s.wu", 4,
             "\x53\x10\x00\xD0",    // pattern 32-bit LE: 0xD0001053
             "\x7F\x70\x00\xFF"))   // mask: opcode + funct3 + funct7 + rs2
        ->help("Floating-Point Convert from Word Unsigned: rd = (float)(uint32_t)rs1")
        ->example("fcvt.s.wu f0, a1")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: FMV.X.W rd, rs1
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFFF0707F  (mask funct7 + funct3 + opcode + rs2)
        * Value :   0xE0000053  (funct7=0x70, rs2=0, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fmv.x.w", 4,
             "\x53\x00\x00\xE0",    // pattern 32-bit LE: 0xE0000053
             "\x7F\x70\x00\xFF"))   // mask: opcode + funct3 + funct7 + rs2
        ->help("Floating-Point Move to Integer: rd = rs1 (bitwise copy)")
        ->example("fmv.x.w a0, f1")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: FMV.W.X rd, rs1
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFFF0707F  (mask funct7 + funct3 + opcode + rs2)
        * Value :   0xF0000053  (funct7=0x78, rs2=0, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fmv.w.x", 4,
             "\x53\x00\x00\xF0",    // pattern 32-bit LE: 0xF0000053
             "\x7F\x70\x00\xFF"))   // mask: opcode + funct3 + funct7 + rs2
        ->help("Floating-Point Move from Integer: rd = rs1 (bitwise copy)")
        ->example("fmv.w.x f0, a1")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: FCLASS.S rd, rs1
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFFF0707F  (mask funct7 + funct3 + opcode + rs2)
        * Value :   0xE0001053  (funct7=0x70, rs2=1, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fclass.s", 4,
             "\x53\x10\x00\xE0",    // pattern 32-bit LE: 0xE0001053
             "\x7F\x70\x00\xFF"))   // mask: opcode + funct3 + funct7 + rs2
        ->help("Floating-Point Classify: rd = classification bits for rs1")
        ->example("fclass.s a0, f1")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /* D Extension Instructions (Double-Precision Floating-Point) */

    /*
        * Example: FLD rd, imm12(rs1)
        * 
        * Encoding:
        *   imm[11:0] rs1 funct3 rd opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00003007  (funct3=011 + opcode=0x07)
        */
    insert(
        mnem("fld", 4,
             "\x07\x30\x00\x00",    // pattern 32-bit LE: 0x00003007
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("Floating-Point Load Double: rd = M[rs1 + imm12]")
        ->example("fld f0, (#8, a1)")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->group('(') // (rs1) group
        ->insert(new GAParameterRiscvImm12("\x00\x00\xF0\xFF")) // imm[11:0]: bits [31:20]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: FSD rs2, imm12(rs1)
        * 
        * Encoding:
        *   imm[11:5] rs2 rs1 funct3 imm[4:0] opcode
        *
        * Mask  :   0x0000707F  (mask funct3 + opcode)
        * Value :   0x00003027  (funct3=011 + opcode=0x27)
        */
    insert(
        mnem("fsd", 4,
             "\x27\x30\x00\x00",    // pattern 32-bit LE: 0x00003027
             "\x7F\x70\x00\x00"))   // mask: opcode + funct3
        ->help("Floating-Point Store Double: M[rs1 + imm12] = rs2")
        ->example("fsd f0, (#8, a1)")
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20] - byte 2 bits [7:4] + byte 3 bit [0]
        ->group('(') // (rs1) group
        ->insert(new GAParameterRiscvStypeImm12("\x00\x00\xF0\xFE")) // imm[11:0]: split encoding
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: FMADD.D rd, rs1, rs2, rs3
        * 
        * Encoding:
        *   rs3 funct2 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0x6000707F  (mask funct2 + funct3 + opcode)
        * Value :   0x04000043  (funct2=01, funct3=000 + opcode=0x43)
        */
    insert(
        mnem("fmadd.d", 4,
             "\x43\x00\x00\x04",    // pattern 32-bit LE: 0x04000043
             "\x7F\x70\x00\x06"))   // mask: opcode + funct3 + funct2
        ->help("Floating-Point Fused Multiply-Add Double: rd = rs1 * rs2 + rs3")
        ->example("fmadd.d f0, f1, f2, f3")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20]
        ->insert(new GAParameterRiscvReg("\x00\x00\x00\xF8")); // RS3: bits [31:27]

    /*
        * Example: FMSUB.D rd, rs1, rs2, rs3
        * 
        * Encoding:
        *   rs3 funct2 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0x6000707F  (mask funct2 + funct3 + opcode)
        * Value :   0x04000047  (funct2=01, funct3=000 + opcode=0x47)
        */
    insert(
        mnem("fmsub.d", 4,
             "\x47\x00\x00\x04",    // pattern 32-bit LE: 0x04000047
             "\x7F\x70\x00\x06"))   // mask: opcode + funct3 + funct2
        ->help("Floating-Point Fused Multiply-Subtract Double: rd = rs1 * rs2 - rs3")
        ->example("fmsub.d f0, f1, f2, f3")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20]
        ->insert(new GAParameterRiscvReg("\x00\x00\x00\xF8")); // RS3: bits [31:27]

    /*
        * Example: FNMSUB.D rd, rs1, rs2, rs3
        * 
        * Encoding:
        *   rs3 funct2 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0x6000707F  (mask funct2 + funct3 + opcode)
        * Value :   0x0400004B  (funct2=01, funct3=000 + opcode=0x4B)
        */
    insert(
        mnem("fnmsub.d", 4,
             "\x4B\x00\x00\x04",    // pattern 32-bit LE: 0x0400004B
             "\x7F\x70\x00\x06"))   // mask: opcode + funct3 + funct2
        ->help("Floating-Point Negative Fused Multiply-Subtract Double: rd = -(rs1 * rs2) + rs3")
        ->example("fnmsub.d f0, f1, f2, f3")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20]
        ->insert(new GAParameterRiscvReg("\x00\x00\x00\xF8")); // RS3: bits [31:27]

    /*
        * Example: FNMADD.D rd, rs1, rs2, rs3
        * 
        * Encoding:
        *   rs3 funct2 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0x6000707F  (mask funct2 + funct3 + opcode)
        * Value :   0x0400004F  (funct2=01, funct3=000 + opcode=0x4F)
        */
    insert(
        mnem("fnmadd.d", 4,
             "\x4F\x00\x00\x04",    // pattern 32-bit LE: 0x0400004F
             "\x7F\x70\x00\x06"))   // mask: opcode + funct3 + funct2
        ->help("Floating-Point Negative Fused Multiply-Add Double: rd = -(rs1 * rs2) - rs3")
        ->example("fnmadd.d f0, f1, f2, f3")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")) // RS2: bits [24:20]
        ->insert(new GAParameterRiscvReg("\x00\x00\x00\xF8")); // RS3: bits [31:27]

    /*
        * Example: FADD.D rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x02000053  (funct7=0x01, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fadd.d", 4,
             "\x53\x00\x00\x02",    // pattern 32-bit LE: 0x02000053
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Floating-Point Add Double: rd = rs1 + rs2")
        ->example("fadd.d f0, f1, f2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20]

    /*
        * Example: FSUB.D rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x0A000053  (funct7=0x05, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fsub.d", 4,
             "\x53\x00\x00\x0A",    // pattern 32-bit LE: 0x0A000053
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Floating-Point Subtract Double: rd = rs1 - rs2")
        ->example("fsub.d f0, f1, f2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20]

    /*
        * Example: FMUL.D rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x12000053  (funct7=0x09, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fmul.d", 4,
             "\x53\x00\x00\x12",    // pattern 32-bit LE: 0x12000053
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Floating-Point Multiply Double: rd = rs1 * rs2")
        ->example("fmul.d f0, f1, f2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20]

    /*
        * Example: FDIV.D rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x1A000053  (funct7=0x0D, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fdiv.d", 4,
             "\x53\x00\x00\x1A",    // pattern 32-bit LE: 0x1A000053
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Floating-Point Divide Double: rd = rs1 / rs2")
        ->example("fdiv.d f0, f1, f2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20]

    /*
        * Example: FSQRT.D rd, rs1
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFFF0707F  (mask funct7 + funct3 + opcode + rs2)
        * Value :   0x5A000053  (funct7=0x2D, rs2=0, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fsqrt.d", 4,
             "\x53\x00\x00\x5A",    // pattern 32-bit LE: 0x5A000053
             "\x7F\x70\x00\xFF"))   // mask: opcode + funct3 + funct7 + rs2
        ->help("Floating-Point Square Root Double: rd = sqrt(rs1)")
        ->example("fsqrt.d f0, f1")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: FSGNJ.D rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x22000053  (funct7=0x11, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fsgnj.d", 4,
             "\x53\x00\x00\x22",    // pattern 32-bit LE: 0x22000053
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Floating-Point Sign Injection Double: rd = {rs2[63], rs1[62:0]}")
        ->example("fsgnj.d f0, f1, f2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20]

    /*
        * Example: FSGNJN.D rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x22001053  (funct7=0x11, funct3=001 + opcode=0x53)
        */
    insert(
        mnem("fsgnjn.d", 4,
             "\x53\x10\x00\x22",    // pattern 32-bit LE: 0x22001053
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Floating-Point Sign Injection Negative Double: rd = {~rs2[63], rs1[62:0]}")
        ->example("fsgnjn.d f0, f1, f2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20]

    /*
        * Example: FSGNJX.D rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x22002053  (funct7=0x11, funct3=010 + opcode=0x53)
        */
    insert(
        mnem("fsgnjx.d", 4,
             "\x53\x20\x00\x22",    // pattern 32-bit LE: 0x22002053
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Floating-Point Sign Injection XOR Double: rd = {rs2[63]^rs1[63], rs1[62:0]}")
        ->example("fsgnjx.d f0, f1, f2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20]

    /*
        * Example: FMIN.D rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x2A000053  (funct7=0x15, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fmin.d", 4,
             "\x53\x00\x00\x2A",    // pattern 32-bit LE: 0x2A000053
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Floating-Point Minimum Double: rd = min(rs1, rs2)")
        ->example("fmin.d f0, f1, f2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20]

    /*
        * Example: FMAX.D rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0x2A001053  (funct7=0x15, funct3=001 + opcode=0x53)
        */
    insert(
        mnem("fmax.d", 4,
             "\x53\x10\x00\x2A",    // pattern 32-bit LE: 0x2A001053
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Floating-Point Maximum Double: rd = max(rs1, rs2)")
        ->example("fmax.d f0, f1, f2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20]

    /*
        * Example: FCVT.S.D rd, rs1
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFFF0707F  (mask funct7 + funct3 + opcode + rs2)
        * Value :   0x40000053  (funct7=0x20, rs2=0, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fcvt.s.d", 4,
             "\x53\x00\x00\x40",    // pattern 32-bit LE: 0x40000053
             "\x7F\x70\x00\xFF"))   // mask: opcode + funct3 + funct7 + rs2
        ->help("Floating-Point Convert Single from Double: rd = (float)rs1")
        ->example("fcvt.s.d f0, f1")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: FCVT.D.S rd, rs1
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFFF0707F  (mask funct7 + funct3 + opcode + rs2)
        * Value :   0x42000053  (funct7=0x21, rs2=0, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fcvt.d.s", 4,
             "\x53\x00\x00\x42",    // pattern 32-bit LE: 0x42000053
             "\x7F\x70\x00\xFF"))   // mask: opcode + funct3 + funct7 + rs2
        ->help("Floating-Point Convert Double from Single: rd = (double)rs1")
        ->example("fcvt.d.s f0, f1")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: FEQ.D rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0xA2002053  (funct7=0x51, funct3=010 + opcode=0x53)
        */
    insert(
        mnem("feq.d", 4,
             "\x53\x20\x00\xA2",    // pattern 32-bit LE: 0xA2002053
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Floating-Point Equal Compare Double: rd = (rs1 == rs2) ? 1 : 0")
        ->example("feq.d a0, f1, f2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20]

    /*
        * Example: FLT.D rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0xA2001053  (funct7=0x51, funct3=001 + opcode=0x53)
        */
    insert(
        mnem("flt.d", 4,
             "\x53\x10\x00\xA2",    // pattern 32-bit LE: 0xA2001053
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Floating-Point Less Than Compare Double: rd = (rs1 < rs2) ? 1 : 0")
        ->example("flt.d a0, f1, f2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20]

    /*
        * Example: FLE.D rd, rs1, rs2
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFE00707F  (mask funct7 + funct3 + opcode)
        * Value :   0xA2000053  (funct7=0x51, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fle.d", 4,
             "\x53\x00\x00\xA2",    // pattern 32-bit LE: 0xA2000053
             "\x7F\x70\x00\xFE"))   // mask: opcode + funct3 + funct7
        ->help("Floating-Point Less Than or Equal Compare Double: rd = (rs1 <= rs2) ? 1 : 0")
        ->example("fle.d a0, f1, f2")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")) // RS1: bits [19:15]
        ->insert(new GAParameterRiscvReg("\x00\x00\xF0\x01")); // RS2: bits [24:20]

    /*
        * Example: FCVT.W.D rd, rs1
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFFF0707F  (mask funct7 + funct3 + opcode + rs2)
        * Value :   0xC2000053  (funct7=0x61, rs2=0, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fcvt.w.d", 4,
             "\x53\x00\x00\xC2",    // pattern 32-bit LE: 0xC2000053
             "\x7F\x70\x00\xFF"))   // mask: opcode + funct3 + funct7 + rs2
        ->help("Floating-Point Convert to Word from Double: rd = (int32_t)rs1")
        ->example("fcvt.w.d a0, f1")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: FCVT.WU.D rd, rs1
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFFF0707F  (mask funct7 + funct3 + opcode + rs2)
        * Value :   0xC2001053  (funct7=0x61, rs2=1, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fcvt.wu.d", 4,
             "\x53\x10\x00\xC2",    // pattern 32-bit LE: 0xC2001053
             "\x7F\x70\x00\xFF"))   // mask: opcode + funct3 + funct7 + rs2
        ->help("Floating-Point Convert to Word Unsigned from Double: rd = (uint32_t)rs1")
        ->example("fcvt.wu.d a0, f1")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: FCVT.D.W rd, rs1
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFFF0707F  (mask funct7 + funct3 + opcode + rs2)
        * Value :   0xD2000053  (funct7=0x69, rs2=0, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fcvt.d.w", 4,
             "\x53\x00\x00\xD2",    // pattern 32-bit LE: 0xD2000053
             "\x7F\x70\x00\xFF"))   // mask: opcode + funct3 + funct7 + rs2
        ->help("Floating-Point Convert from Word to Double: rd = (double)rs1")
        ->example("fcvt.d.w f0, a1")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: FCVT.D.WU rd, rs1
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFFF0707F  (mask funct7 + funct3 + opcode + rs2)
        * Value :   0xD2001053  (funct7=0x69, rs2=1, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fcvt.d.wu", 4,
             "\x53\x10\x00\xD2",    // pattern 32-bit LE: 0xD2001053
             "\x7F\x70\x00\xFF"))   // mask: opcode + funct3 + funct7 + rs2
        ->help("Floating-Point Convert from Word Unsigned to Double: rd = (double)(uint32_t)rs1")
        ->example("fcvt.d.wu f0, a1")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /*
        * Example: FCLASS.D rd, rs1
        * 
        * Encoding:
        *   funct7 rs2 rs1 funct3 rd opcode
        *
        * Mask  :   0xFFF0707F  (mask funct7 + funct3 + opcode + rs2)
        * Value :   0xE2001053  (funct7=0x71, rs2=1, funct3=000 + opcode=0x53)
        */
    insert(
        mnem("fclass.d", 4,
             "\x53\x10\x00\xE2",    // pattern 32-bit LE: 0xE2001053
             "\x7F\x70\x00\xFF"))   // mask: opcode + funct3 + funct7 + rs2
        ->help("Floating-Point Classify Double: rd = classification bits for rs1")
        ->example("fclass.d a0, f1")
        ->insert(new GAParameterRiscvReg("\x80\x0F\x00\x00")) // RD: bits [11:7]
        ->insert(new GAParameterRiscvReg("\x00\x80\x0F\x00")); // RS1: bits [19:15]

    /* C-Extension - Compressed Instructions (16-bit) */
    
    // Quadrant 0 (opcode [1:0] = 00)
    
    /*
     * C.ADDI4SPN - Add Immediate to Stack Pointer (4-byte aligned)
     * Format: CIW
     * Encoding: 000|imm[9:2]|rd'[4:2]|00
     * Opcode: 0x00 (bits [1:0] = 00, bits [15:13] = 000)
     */
    insert(
        mnem("c.addi4spn", 2,
             "\x00\x00",        // pattern 16-bit LE: bits [1:0]=00, [15:13]=000
             "\x03\xE0"))       // mask: opcode bits
        ->help("Compressed Add Immediate to SP: rd' = sp + (uimm << 2)")
        ->example("c.addi4spn a0, #16")
        ->insert(new GAParameterRiscvCompReg("\x1C\x00")) // rd': bits [4:2] in byte 0
        ->insert(new GAParameterRiscvCIWimm9("\xE0\x1F")); // imm[9:2]: bits [7:5] in byte 0, bits [4:0] in byte 1 = instruction bits [12:5]
    
    /*
     * C.LW - Compressed Load Word
     * Format: CL
     * Encoding: 010|imm[5|4:3|8:6]|rs1'[2:0]|imm[2|6]|rd'[2:0]|00
     * Opcode: 0x4000 (bits [1:0] = 00, bits [15:13] = 010)
     */
    insert(
        mnem("c.lw", 2,
             "\x00\x40",        // pattern 16-bit LE: bits [1:0]=00, [15:13]=010
             "\x03\xE0"))       // mask: opcode bits
        ->help("Compressed Load Word: rd' = M[rs1' + (uimm << 2)][31:0]")
        ->example("c.lw a0, (#4, a1)")
        ->insert(new GAParameterRiscvCompReg("\x1C\x00")) // rd': bits [4:2] in byte 0
        ->group('(') // imm(rs1) group
        ->insert(new GAParameterRiscvCLimm5("\x60\x1C")) // imm[6:2]: imm[6] in bit 5, imm[2] in bit 6, imm[5:3] in bits [12:10]
        ->insert(new GAParameterRiscvCompReg("\x80\x03")); // rs1': bits [9:7] - bit [7] from byte 0, bits [9:8] from byte 1
    
    /*
     * C.SW - Compressed Store Word
     * Format: CS
     * Encoding: 110|imm[5|4:3|8:6]|rs1'[2:0]|imm[2|6]|rs2'[2:0]|00
     * Opcode: 0xC000 (bits [1:0] = 00, bits [15:13] = 110)
     */
    insert(
        mnem("c.sw", 2,
             "\x00\xC0",        // pattern 16-bit LE: bits [1:0]=00, [15:13]=110
             "\x03\xE0"))       // mask: opcode bits
        ->help("Compressed Store Word: M[rs1' + (uimm << 2)] = rs2'[31:0]")
        ->example("c.sw a0, (#4, a1)")
        ->insert(new GAParameterRiscvCompReg("\x1C\x00")) // rs2': bits [4:2] in byte 0
        ->group('(') // imm(rs1) group
        ->insert(new GAParameterRiscvCSimm5("\x60\x1C")) // imm[6:2]: imm[6] in bit 5, imm[2] in bit 6, imm[5:3] in bits [12:10]
        ->insert(new GAParameterRiscvCompReg("\x80\x03")); // rs1': bits [9:7] - bit [7] from byte 0, bits [9:8] from byte 1
    
    // Quadrant 1 (opcode [1:0] = 01)
    
    /*
     * C.NOP - Compressed No Operation / C.ADDI
     * Format: CI
     * Encoding: 000|imm[5:0]|rd[4:0]|01
     * Opcode: 0x0001 (bits [1:0] = 01, bits [15:13] = 000, rd=0 for NOP)
     * Note: Inserted before c.addi so it's checked first (more specific pattern with rd=0)
     */
    insert(
        mnem("c.nop", 2,
             "\x01\x00",        // pattern 16-bit LE: bits [1:0]=01, [15:13]=000, rd=0
             "\x83\xEF"))       // mask: opcode + rd (byte0: bits [7] and [1:0]=0x83, byte1: bits [15:13] and [11:8]=0xEF)
        ->help("Compressed No Operation")
        ->example("c.nop");
    
    /*
     * C.ADDI - Compressed Add Immediate
     * Format: CI
     * Encoding: 000|imm[5:0]|rd[4:0]|01
     * Opcode: 0x0001 (bits [1:0] = 01, bits [15:13] = 000, rd != 0)
     * Note: Uses rejectWhenZero to check rd != 0, eliminating collision with c.nop
     */
    insert(
        mnem("c.addi", 2,
             "\x01\x00",        // pattern 16-bit LE: bits [1:0]=01, [15:13]=000
             "\x03\xE0"))       // mask: opcode bits only
        ->rejectWhenZero("\x80\x0F") // rd (bits [11:7]) must be non-zero
        ->help("Compressed Add Immediate: rd = rd + imm")
        ->example("c.addi a0, #5")
        ->insert(new GAParameterRiscvReg("\x80\x0F")) // rd: bits [11:7] - bit [7] from byte 0, bits [11:8] from byte 1
        ->insert(new GAParameterRiscvCIimm6("\x7C\x10")); // imm[5:0]: split encoding - bits [6:2] from byte 0, bit [12] from byte 1
    
    /*
     * C.LI - Compressed Load Immediate
     * Format: CI
     * Encoding: 010|imm[5:0]|rd[4:0]|01
     * Opcode: 0x4001 (bits [1:0] = 01, bits [15:13] = 010)
     */
    insert(
        mnem("c.li", 2,
             "\x01\x40",        // pattern 16-bit LE: bits [1:0]=01, [15:13]=010
             "\x03\xE0"))       // mask: opcode bits
        ->help("Compressed Load Immediate: rd = imm (rd != 0, rd != 2)")
        ->example("c.li a0, #5")
        ->insert(new GAParameterRiscvReg("\x80\x0F")) // rd: bits [11:7] - bit [7] from byte 0, bits [11:8] from byte 1
        ->insert(new GAParameterRiscvCIimm6("\x7C\x10")); // imm[5:0]: split encoding - bits [6:2] from byte 0, bit [12] from byte 1
    
    /*
     * C.ADDI16SP - Compressed Add Immediate to SP (16-byte aligned)
     * Format: CI
     * Encoding: 011|imm[9|4|6|8:7|5]|01010|01
     * Opcode: 0x6101 (bits [1:0] = 01, bits [15:13] = 011, rd = 2 (sp))
     * Note: Pattern includes rd=2 (bits [11:7]=00010) to distinguish from c.lui
     *       rd[4:1]=0001 in bits [11:8], rd[0]=0 in bit [7]
     *       Inserted before c.lui so it's checked first (more specific pattern)
     */
    insert(
        mnem("c.addi16sp", 2,
             "\x01\x61",        // pattern 16-bit LE: bits [1:0]=01, [15:13]=011, rd=2 (bits [11:8]=0001, bit [7]=0)
             "\x83\xEF"))       // mask: opcode + rd (byte0: bits [7] and [1:0]=0x83, byte1: bits [15:13] and [11:8]=0xEF)
        ->help("Compressed Add Immediate to SP: sp = sp + (imm << 4)")
        ->example("c.addi16sp #-32")
        ->insert(new GAParameterRiscvCIimm6("\x7C\x10")); // imm[9|4|6|8:7|5]: split encoding (special format) - same mask as CI but different encoding logic
    
    /*
     * C.LUI - Compressed Load Upper Immediate
     * Format: CI
     * Encoding: 011|imm[17:12]|rd[4:0]|01
     * Opcode: 0x6001 (bits [1:0] = 01, bits [15:13] = 011, rd != 0, rd != 2)
     * Note: Inserted after c.addi16sp so c.addi16sp (more specific) is checked first
     */
    insert(
        mnem("c.lui", 2,
             "\x01\x60",        // pattern 16-bit LE: bits [1:0]=01, [15:13]=011 (rd != 0 and != 2 is checked in parameter decode to distinguish from c.addi16sp)
             "\x03\xE0"))       // mask: opcode only (rd != 0 and != 2 is checked in parameter decode to distinguish from c.addi16sp)
        ->help("Compressed Load Upper Immediate: rd = (imm << 12)")
        ->example("c.lui a0, #0x10000")
        ->insert(new GAParameterRiscvReg("\x80\x0F")) // rd: bits [11:7] - bit [7] from byte 0, bits [11:8] from byte 1 (must be != 0 and != 2)
        ->insert(new GAParameterRiscvCIimm6("\x7C\x10")); // imm[17:12]: encoded in imm[5:0] position - same mask as CI but represents imm[17:12]
    
    /*
     * C.SRAI - Compressed Shift Right Arithmetic Immediate
     * Format: CB
     * Encoding: 100|imm[5:0]|rd'[2:0]|01
     * Opcode: 0x8401 (bits [1:0] = 01, bits [15:13] = 100, bits [11:10] = 01)
     * Note: C.SRAI is in quadrant 01, distinct from C.MV which is in quadrant 10
     */
    {
        auto m = mnem("c.srai", 2,
                     "\x01\x84",        // pattern 16-bit LE: bits [1:0]=01, [15:13]=100, [11:10]=01 (only fixed opcode bits)
                     "\x03\xEC");       // mask: bits [1:0] + [15:13] + [11:10] = 0x03 in byte 0, 0xEC in byte 1 (exclude bit [6] and [12] - they're data!)
        m->help("Compressed Shift Right Arithmetic Immediate: rd' = rd' >> uimm (arithmetic)");
        m->example("c.srai a0, #3");
        m->insert(new GAParameterRiscvCompReg("\x80\x03")); // rd': bits [9:7] - bit [7] from byte 0, bits [9:8] from byte 1
        m->insert(new GAParameterRiscvCIUimm6("\x7C\x10")); // imm[5:0]: split encoding (unsigned) - bits [6:2] from byte 0, bit [12] from byte 1
        insert(m);
    }
    
    // Quadrant 2 (opcode [1:0] = 10)
    
    /*
     * C.EBREAK - Compressed Environment Break
     * Format: CR
     * Encoding: 100|00000|00000|10
     * Opcode: 0x9002 (bits [1:0] = 10, bits [15:13] = 100, bits [12:10] = 001, rd=0, rs1=0)
     * Note: Inserted first to ensure it's checked first (has most specific mask 0xFFFF)
     */
    insert(
        mnem("c.ebreak", 2,
             "\x02\x90",        // pattern 16-bit LE: bits [1:0]=10, [15:13]=100, [12:10]=001, rd=0, rs1=0
             "\xFF\xFF"))       // mask: all bits (0xFFFF) - c.ebreak is an exact encoding with rd=0, rs2=0
        // Removed priority - full mask 0xFFFF ensures c.ebreak matches only its exact encoding 0x9002
        ->help("Compressed Environment Break");
    
    /*
     * C.ANDI - Compressed AND Immediate
     * Format: CB
     * Encoding: 100|imm[5]|10|rd'[2:0]|imm[4:0]|01
     * Opcode: 0x8801 (bits [1:0] = 01, bits [15:13] = 100, bits [11:10] = 10)
     * Note: bits [12] and [6:2] are part of immediate data, so they are NOT in the mask
     */
    // Using standard mnemonic - c.andi is in quadrant 01 (bits[1:0]=01),
    // distinct from c.jr/c.jalr which are in quadrant 10 (bits[1:0]=10)
    // The mask already distinguishes them by checking bits[1:0] and bits[11:10]=10
    insert(
        mnem("c.andi", 2,
             "\x01\x88",        // pattern 16-bit LE: bits [1:0]=01, [15:13]=100, [11:10]=10
             "\x03\xEC"))       // mask: bits [1:0] in byte 0, bits [15:13] + [11:10] in byte 1
        ->help("Compressed AND Immediate: rd' = rd' & imm")
        ->example("c.andi a0, #7")
        ->insert(new GAParameterRiscvCompReg("\x80\x03")) // rd': bits [9:7] - bit [7] from byte 0, bits [9:8] from byte 1
        ->insert(new GAParameterRiscvCBimm6Andi("\x7C\x10")); // imm[5:0]: split encoding - bits [6:2] from byte 0, bit [12] from byte 1
    
    /*
     * C.SRLI - Compressed Shift Right Logical Immediate
     * Format: CB
     * Encoding: 100|imm[5]|00|rd'[2:0]|imm[4:0]|01
     * Opcode: bits [1:0] = 01 (quadrant 1), bits [15:13] = 100, bits [11:10] = 00
     * Note: bit [12] is imm[5], bits [6:2] are imm[4:0] (parameters)
     * Pattern: 0x8001 (bits [1:0]=01, [15:13]=100, [11:10]=00)
     */
    // Using standard mnemonic - c.srli is in quadrant 01 (bits[1:0]=01),
    // distinct from c.jr/c.jalr/c.mv which are in quadrant 10 (bits[1:0]=10)
    // The mask checks bits[11:10]=00 to distinguish from c.srai (01) and c.andi (10)
    insert(
        mnem("c.srli", 2,
             "\x01\x80",        // pattern 16-bit LE: bits [1:0]=01, [15:13]=100, [11:10]=00
             "\x03\xEC"))       // mask: bits [1:0] in byte 0, bits [15:13] + [11:10] in byte 1
        ->help("Compressed Shift Right Logical Immediate: rd' = rd' >> uimm")
        ->example("c.srli a0, #3")
        ->insert(new GAParameterRiscvCompReg("\x80\x03")) // rd': bits [9:7] - bit [7] from byte 0, bits [9:8] from byte 1
        ->insert(new GAParameterRiscvCIUimm6("\x7C\x10")); // imm[5:0]: split encoding (unsigned) - bits [6:2] from byte 0, bit [12] from byte 1
    
    /*
     * C.JR - Compressed Jump Register
     * Format: CR
     * Encoding: 100|rs1[4:0]|00000|10
     * Opcode: bits [15:13] = 100, bits [12:10] = 000, bits [6:2] = 00000 (rd=0), bits [1:0] = 10
     * Note: [11:10] are part of rs1 register field, not opcode bits, so mask doesn't check them
     * Note: rs1 must be non-zero (rs1=0 with rd=0 is reserved)
     */
    insert(
        mnem("c.jr", 2,
             "\x02\x80",        // pattern 16-bit LE: bits [1:0]=10, [15:13]=100, [12]=0 (funct4), [6:2]=00000 (rs2=0)
             "\x7F\xF0"))       // mask: bits [1:0] + [6:2] in byte 0 (0x7F), bits [15:12] in byte 1 (0xF0)
        ->rejectWhenZero("\x80\x0F") // rs1 (bits [11:7]) must be non-zero
        ->help("Compressed Jump Register: pc = rs1")
        ->example("c.jr a0")
        ->insert(new GAParameterRiscvReg("\x80\x0F")); // rs1: bits [11:7] - bit [7] from byte 0, bits [11:8] from byte 1
    
    /*
     * C.JALR - Compressed Jump and Link Register
     * Format: CR
     * Encoding: 1001|rs1[4:0]|00000|10
     * Opcode: bits [15:12] = 1001 (funct4), bits [6:2] = 00000 (rs2=0), bits [1:0] = 10
     * Note: rd is implicitly ra (x1), rs1 must be non-zero (rs1=0 with rs2=0 is c.ebreak)
     */
    insert(
        mnem("c.jalr", 2,
             "\x02\x90",        // pattern 16-bit LE: bits [1:0]=10, [15:12]=1001 (funct4), [6:2]=00000 (rs2=0)
             "\x7F\xF0"))       // mask: bits [6:0] in byte 0 (check rs2=0, op=10), bits [15:12] in byte 1 (check funct4)
        ->rejectWhenZero("\x80\x0F") // rs1 (bits [11:7]) must be non-zero
        ->help("Compressed Jump and Link Register: ra = pc + 2; pc = rs1")
        ->example("c.jalr a0")
        ->insert(new GAParameterRiscvReg("\x80\x0F")); // rs1: bits [11:7] - bit [7] from byte 0, bits [11:8] from byte 1
    
    /*
     * C.SUB - Compressed Subtract
     * Format: CA
     * Encoding: 100|rs2'[2:0]|rs2'[2:0]|11|rd'[2:0]|00|10
     * Opcode: 0x8C02 (bits [1:0] = 10, bits [15:13] = 100, bits [12:10] = 111, bits [6:5] = 00)
     */
    insert(
        mnem("c.sub", 2,
             "\x02\x8C",        // pattern 16-bit LE: bits [1:0]=10, [15:13]=100, [12:10]=111, [6:5]=00
             "\x63\xFC"))       // mask: bits [1:0] + [6:5] + [15:13] + [12:10] = 0x63 in byte 0, 0xFC in byte 1
        // Removed priority - mask correctly checks [6:5]=00 to distinguish from other CA format instructions
        ->help("Compressed Subtract: rd' = rd' - rs2'")
        ->example("c.sub a0, a1")
        ->insert(new GAParameterRiscvCompReg("\x80\x03")) // rd': bits [9:7] - bit [7] from byte 0, bits [9:8] from byte 1
        ->insert(new GAParameterRiscvCompReg("\x1C\x00")); // rs2': bits [4:2] in byte 0
    
    /*
     * C.XOR - Compressed XOR
     * Format: CA
     * Encoding: 100|rs2'[2:0]|rs2'[2:0]|11|rd'[2:0]|01|10
     * Opcode: 0x8C22 (bits [1:0] = 10, bits [15:13] = 100, bits [12:10] = 111, bits [6:5] = 01)
     */
    insert(
        mnem("c.xor", 2,
             "\x22\x8C",        // pattern 16-bit LE: bits [1:0]=10, [15:13]=100, [12:10]=111, [6:5]=01
             "\x63\xFC"))       // mask: bits [1:0] + [6:5] + [15:13] + [12:10] = 0x63 in byte 0, 0xFC in byte 1
        // Removed priority - mask correctly checks [6:5]=01 to distinguish from other CA format instructions
        ->help("Compressed XOR: rd' = rd' ^ rs2'")
        ->example("c.xor a0, a1")
        ->insert(new GAParameterRiscvCompReg("\x80\x03")) // rd': bits [9:7] - bit [7] from byte 0, bits [9:8] from byte 1
        ->insert(new GAParameterRiscvCompReg("\x1C\x00")); // rs2': bits [4:2] in byte 0
    
    /*
     * C.OR - Compressed OR
     * Format: CA
     * Encoding: 100|rs2'[2:0]|rs2'[2:0]|11|rd'[2:0]|10|10
     * Opcode: 0x8C42 (bits [1:0] = 10, bits [15:13] = 100, bits [12:10] = 111, bits [6:5] = 10)
     */
    insert(
        mnem("c.or", 2,
             "\x42\x8C",        // pattern 16-bit LE: bits [1:0]=10, [15:13]=100, [12:10]=111, [6:5]=10
             "\x63\xFC"))       // mask: bits [1:0] + [6:5] + [15:13] + [12:10] = 0x63 in byte 0, 0xFC in byte 1
        // Removed priority - mask correctly checks [6:5]=10 to distinguish from other CA format instructions
        ->help("Compressed OR: rd' = rd' | rs2'")
        ->example("c.or a0, a1")
        ->insert(new GAParameterRiscvCompReg("\x80\x03")) // rd': bits [9:7] - bit [7] from byte 0, bits [9:8] from byte 1
        ->insert(new GAParameterRiscvCompReg("\x1C\x00")); // rs2': bits [4:2] in byte 0
    
    /*
     * C.AND - Compressed AND
     * Format: CA
     * Encoding: 100|rs2'[2:0]|rs2'[2:0]|11|rd'[2:0]|11|10
     * Opcode: 0x8C62 (bits [1:0] = 10, bits [15:13] = 100, bits [12:10] = 111, bits [6:5] = 11)
     */
    insert(
        mnem("c.and", 2,
             "\x62\x8C",        // pattern 16-bit LE: bits [1:0]=10, [15:13]=100, [12:10]=111, [6:5]=11
             "\x63\xFC"))       // mask: bits [1:0] + [6:5] + [15:13] + [12:10] = 0x63 in byte 0, 0xFC in byte 1
        // Removed priority - mask correctly checks [6:5]=11 to distinguish from other CA format instructions
        ->help("Compressed AND: rd' = rd' & rs2'")
        ->example("c.and a0, a1")
        ->insert(new GAParameterRiscvCompReg("\x80\x03")) // rd': bits [9:7] - bit [7] from byte 0, bits [9:8] from byte 1
        ->insert(new GAParameterRiscvCompReg("\x1C\x00")); // rs2': bits [4:2] in byte 0
    
    /*
     * C.J - Compressed Jump
     * Format: CJ
     * Encoding: 101|imm[11|4|9:8|10|6|7|3:1|5]|01
     * Opcode: 0xA001 (bits [1:0] = 01, bits [15:13] = 101)
     */
    insert(
        mnem("c.j", 2,
             "\x01\xA0",        // pattern 16-bit LE: bits [1:0]=01, [15:13]=101
             "\x03\xE0"))       // mask: opcode bits
        ->help("Compressed Jump: pc = pc + imm")
        ->example("c.j #8")
        ->insert(new GAParameterRiscvCJimm12("\xFC\x1F")); // imm[11:1]: split encoding - bits [7:2] from byte 0, bits [12:8] from byte 1
    
    /*
     * C.BEQZ - Compressed Branch if Equal to Zero
     * Format: CB
     * Encoding: 110|imm[8|4:3|7:6|2:1|5]|rs1'[2:0]|01
     * Opcode: 0xC001 (bits [1:0] = 01, bits [15:13] = 110)
     * Note: bits [9:7] contain rs1' (compressed register), not regular rs2
     * Note: Distinguished from c.swsp by op field: c.beqz has [1:0]=01, c.swsp has [1:0]=10
     */
    insert(
        mnem("c.beqz", 2,
             "\x01\xC0",        // pattern 16-bit LE: bits [1:0]=01, [15:13]=110
             "\x03\xE0"))       // mask: bits [1:0] + [15:13] = 0x03 in byte 0, 0xE0 in byte 1
        ->help("Compressed Branch if Equal to Zero: if (rs1' == 0) pc = pc + imm")
        ->example("c.beqz a0, #8")
        ->insert(new GAParameterRiscvCompReg("\x80\x03")) // rs1': bits [9:7] - bit [7] from byte 0, bits [9:8] from byte 1
        ->insert(new GAParameterRiscvCBimm9("\x7C\x1C")); // imm[8:1]: split encoding - bits [6:2] from byte 0, bits [12] and [11:10] from byte 1
    
    /*
     * C.BNEZ - Compressed Branch if Not Equal to Zero
     * Format: CB
     * Encoding: 111|imm[8|4:3|7:6|2:1|5]|rs1'[2:0]|01
     * Opcode: 0xE001 (bits [1:0] = 01, bits [15:13] = 111)
     */
    insert(
        mnem("c.bnez", 2,
             "\x01\xE0",        // pattern 16-bit LE: bits [1:0]=01, [15:13]=111
             "\x03\xE0"))       // mask: bits [1:0] + [15:13] = 0x03 in byte 0, 0xE0 in byte 1
        ->help("Compressed Branch if Not Equal to Zero: if (rs1' != 0) pc = pc + imm")
        ->example("c.bnez a0, #8")
        ->insert(new GAParameterRiscvCompReg("\x80\x03")) // rs1': bits [9:7] - bit [7] from byte 0, bits [9:8] from byte 1
        ->insert(new GAParameterRiscvCBimm9("\x7C\x1C")); // imm[8:1]: split encoding - bits [6:2] from byte 0, bits [12] and [11:10] from byte 1
    
    // Quadrant 2 special instructions (rd != 0)
    
    /*
     * C.SLLI - Compressed Shift Left Logical Immediate
     * Format: CI
     * Encoding: 000|imm[5:0]|rd[4:0]|10
     * Opcode: 0x0002 (bits [1:0] = 10, bits [15:13] = 000, rd != 0)
     */
    insert(
        mnem("c.slli", 2,
             "\x02\x00",        // pattern 16-bit LE: bits [1:0]=10, [15:13]=000
             "\x03\xE0"))       // mask: opcode bits
        ->help("Compressed Shift Left Logical Immediate: rd = rd << uimm")
        ->example("c.slli a0, #3")
        ->insert(new GAParameterRiscvReg("\x80\x0F")) // rd: bits [11:7] - bit [7] from byte 0, bits [11:8] from byte 1
        ->insert(new GAParameterRiscvCIUimm6("\x7C\x10")); // imm[5:0]: split encoding (unsigned) - bits [6:2] from byte 0, bit [12] from byte 1
    
    /*
     * C.LWSP - Compressed Load Word from Stack Pointer
     * Format: CI
     * Encoding: 010|imm[7:2]|rd[4:0]|10
     * Opcode: 0x4002 (bits [1:0] = 10, bits [15:13] = 010, rd != 0)
     */
    insert(
        mnem("c.lwsp", 2,
             "\x02\x40",        // pattern 16-bit LE: bits [1:0]=10, [15:13]=010
             "\x03\xE0"))       // mask: opcode bits
        ->help("Compressed Load Word from SP: rd = M[sp + (uimm << 2)][31:0]")
        ->example("c.lwsp a0, #4")
        ->insert(new GAParameterRiscvReg("\x80\x0F")) // rd: bits [11:7] - bit [7] from byte 0, bits [11:8] from byte 1
        ->insert(new GAParameterRiscvCIimm7("\x7C\x10")); // imm[7:2]: imm[5] in bit [12], imm[4:2] in bits [6:4], imm[7:6] in bits [3:2]
    
    /*
     * C.MV - Compressed Move
     * Format: CR
     * Encoding: 1000|rs2[4:0]|rd[4:0]|10
     * Opcode: funct4 = 1000, so bits [15:12] = 1000, bits [1:0] = 10
     * Note: bit [12] = 0 distinguishes from c.add (funct4 = 1001, bit [12] = 1)
     * Note: C.MV is in quadrant 10, distinct from C.SRAI which is in quadrant 01
     * Note: both rd != 0 and rs2 != 0 required (rs2=0 would be c.jr, rd=0 invalid)
     */
    insert(
        mnem("c.mv", 2,
             "\x02\x80",        // pattern 16-bit LE: bits [1:0]=10, [15:12]=1000 (funct4)
             "\x03\xF0"))       // mask: bits [1:0] + [15:12] = 0x03 in byte 0, 0xF0 in byte 1
        ->rejectWhenZero("\x80\x0F") // rd (bits [11:7]) must be non-zero
        ->rejectWhenZero("\x7C\x00") // rs2 (bits [6:2]) must be non-zero
        ->help("Compressed Move: rd = rs2")
        ->example("c.mv a0, a1")
        ->insert(new GAParameterRiscvReg("\x80\x0F")) // rd: bits [11:7] - bit [7] from byte 0, bits [11:8] from byte 1
        ->insert(new GAParameterRiscvReg("\x7C\x00")); // rs2: bits [6:2] - bits [6:2] in byte 0
    
    /*
     * C.ADD - Compressed Add
     * Format: CR
     * Encoding: 100|rs2[4:0]/rs1[4:0]|rd[4:0]|10
     * Opcode: 0x9002 (bits [1:0] = 10, bits [15:13] = 100, bits [12:10] = 000, rd != 0, rd != 1)
     */
    insert(
        mnem("c.add", 2,
             "\x02\x90",        // pattern 16-bit LE: bits [1:0]=10, [15:12]=1001 (funct4), rd=0 (template)
             "\x03\xF0"))       // mask: bits [1:0] + [15:12] = 0x03 in byte 0, 0xF0 in byte 1 (don't check rd or rs2)
        // Removed priority - c.ebreak (inserted before this) has more specific mask (0x03FF) and will match first
        ->help("Compressed Add: rd = rd + rs2")
        ->example("c.add a0, a1")
        ->insert(new GAParameterRiscvReg("\x80\x0F")) // rd: bits [11:7] - bit [7] from byte 0, bits [11:8] from byte 1
        ->insert(new GAParameterRiscvReg("\x7C\x00")); // rs2: bits [6:2] - bits [6:2] in byte 0
    
    /*
     * C.SWSP - Compressed Store Word to Stack Pointer
     * Format: CSS
     * Encoding: 110|imm[7:2]|rs2[4:0]|10
     * Opcode: 0xC002 (bits [1:0] = 10, bits [15:13] = 110)
     * Note: bits [6:2] contain regular rs2 register, not compressed rs1'
     * Note: Distinguished from c.beqz by op field: c.swsp has [1:0]=10, c.beqz has [1:0]=01
     */
    insert(
        mnem("c.swsp", 2,
             "\x02\xC0",        // pattern 16-bit LE: bits [1:0]=10, [15:13]=110
             "\x03\xE0"))       // mask: bits [1:0] + [15:13] = 0x03 in byte 0, 0xE0 in byte 1
        ->help("Compressed Store Word to SP: M[sp + (uimm << 2)] = rs2[31:0]")
        ->example("c.swsp a0, #4")
        ->insert(new GAParameterRiscvReg("\x7C\x00")) // rs2: bits [6:2] - bits [6:2] in byte 0
        ->insert(new GAParameterRiscvCSSimm7("\x80\x1F")); // imm[7:2]: bits [12:7] - bit [7] from byte 0, bits [12:8] from byte 1
}

/* RISC-V Parameter Types */

// RISC-V Reg Parameter

GAParameterRiscvReg::GAParameterRiscvReg(const char* mask){
    setMask(mask);
}

int GAParameterRiscvReg::match(GAParserOperand *op, int len){
    if(op->prefix!=this->prefix)
        return 0;

    // Check if it is a valid RISC-V register name.
    QString name = op->value;
    if(name == "zero" || name == "ra" || name == "sp" || name == "gp" || name == "tp" ||
        name == "t0" || name == "t1" || name == "t2" ||
        name == "s0" || name == "s1" ||
        name == "a0" || name == "a1" || name == "a2" || name == "a3" || 
        name == "a4" || name == "a5" || name == "a6" || name == "a7" ||
        name == "s2" || name == "s3" || name == "s4" || name == "s5" ||
        name == "s6" || name == "s7" || name == "s8" || name == "s9" ||
        name == "s10" || name == "s11" ||
        name == "t3" || name == "t4" || name == "t5" || name == "t6" ||
        // Floating point registers f0-f31
        name == "f0" || name == "f1" || name == "f2" || name == "f3" ||
        name == "f4" || name == "f5" || name == "f6" || name == "f7" ||
        name == "f8" || name == "f9" || name == "f10" || name == "f11" ||
        name == "f12" || name == "f13" || name == "f14" || name == "f15" ||
        name == "f16" || name == "f17" || name == "f18" || name == "f19" ||
        name == "f20" || name == "f21" || name == "f22" || name == "f23" ||
        name == "f24" || name == "f25" || name == "f26" || name == "f27" ||
        name == "f28" || name == "f29" || name == "f30" || name == "f31") {
         return 1;
     }
    return 0;
}

QString GAParameterRiscvReg::decode(GALanguage *lang, uint64_t adr, const char *bytes, int inslen){
    uint64_t regnum = rawdecode(lang, adr, bytes, inslen);
    // Clamp register number to valid range (0-31 for both integer and floating point)
    if(regnum > 31) {
        return QString::asprintf("r%llu", (unsigned long long) regnum);
    }
    
    // Check if this is a C-Extension instruction (16-bit, mask "\x80\x0F" for bits [11:7])
    // For 16-bit instructions, mask is 2 bytes; for 32-bit, it's 4 bytes
    bool isCExtension = (inslen == 2 && (unsigned char)mask[0] == 0x80 && (unsigned char)mask[1] == 0x0F);
    
    // Determine which register field this is by checking the mask
    // For 32-bit instructions:
    // RD: bits [11:7] - mask "\x80\x0F\x00\x00"
    // RS1: bits [19:15] - mask "\x00\x80\x0F\x00"
    // RS2: bits [24:20] - mask "\x00\x00\xF0\x01" or "\x00\x00\xF0\x1F"
    // RS3: bits [31:27] - mask "\x00\x00\x00\xF8"
    // For 16-bit C-Extension:
    // RD/RS1/RS2: bits [11:7] - mask "\x80\x0F"
    bool isRD = false;
    bool isRS1 = false;
    bool isRS2 = false;
    bool isRS3 = false;
    
    if(isCExtension) {
        // For C-Extension, bits [11:7] can be RD, RS1, or RS2 depending on instruction
        // We can't determine from mask alone, so assume it's RD (most common)
        isRD = true;
    } else {
        isRD = ((unsigned char)mask[0] == 0x80 && (unsigned char)mask[1] == 0x0F && (unsigned char)mask[2] == 0x00 && (unsigned char)mask[3] == 0x00);
        isRS1 = ((unsigned char)mask[0] == 0x00 && (unsigned char)mask[1] == 0x80 && (unsigned char)mask[2] == 0x0F && (unsigned char)mask[3] == 0x00);
        isRS2 = ((unsigned char)mask[0] == 0x00 && (unsigned char)mask[1] == 0x00 && ((unsigned char)mask[2] == 0xF0) && ((unsigned char)mask[3] == 0x01 || (unsigned char)mask[3] == 0x1F));
        isRS3 = ((unsigned char)mask[0] == 0x00 && (unsigned char)mask[1] == 0x00 && (unsigned char)mask[2] == 0x00 && (unsigned char)mask[3] == 0xF8);
    }
    
    // Check instruction encoding to determine register type for F extension instructions
    bool useFPReg = false;
    if(inslen >= 4) {
        uint8_t opcode = bytes[0] & 0x7F; // Opcode is in bits [6:0] of first byte
        
        if(opcode == 0x07 || opcode == 0x27) {
            // FLW, FSW: rd/rs2 are FP, rs1 is integer (base address)
            useFPReg = (isRD || isRS2);
        } else if(opcode == 0x43 || opcode == 0x47 || opcode == 0x4B || opcode == 0x4F) {
            // FMADD/FMSUB/FNMSUB/FNMADD: all registers are FP
            useFPReg = true;
        } else if(opcode == 0x53) {
            // F extension arithmetic/comparison/conversion instructions
            // Extract funct7 from byte 3 (bits [31:25] = byte 3 bits [7:1])
            // funct7 is 7 bits, so we shift right by 1 and mask with 0x7F
            uint8_t funct7 = (bytes[3] >> 1) & 0x7F;
            uint8_t funct3 = (bytes[1] >> 4) & 0x07; // funct3 is in bits [14:12] = byte 1 bits [6:4]
            
            if(funct7 == 0x60) { // fcvt.w.s, fcvt.wu.s
                useFPReg = isRS1; // rs1 is FP, rd is integer
            } else if(funct7 == 0x68) { // fcvt.s.w, fcvt.s.wu
                useFPReg = isRD; // rd is FP, rs1 is integer
            } else if(funct7 == 0x70) { // fmv.x.w, fclass.s
                useFPReg = isRS1; // rs1 is FP, rd is integer
            } else if(funct7 == 0x78) { // fmv.w.x
                useFPReg = isRD; // rd is FP, rs1 is integer
            } else if(funct7 == 0x50) { // feq.s, flt.s, fle.s
                useFPReg = (isRS1 || isRS2); // rs1 and rs2 are FP, rd is integer
            } else if(funct7 == 0x51) { // feq.d, flt.d, fle.d
                useFPReg = (isRS1 || isRS2); // rs1 and rs2 are FP, rd is integer
            } else if(funct7 == 0x61) { // fcvt.w.d, fcvt.wu.d
                useFPReg = isRS1; // rs1 is FP, rd is integer
            } else if(funct7 == 0x69) { // fcvt.d.w, fcvt.d.wu
                useFPReg = isRD; // rd is FP, rs1 is integer
            } else if(funct7 == 0x71) { // fclass.d
                useFPReg = isRS1; // rs1 is FP, rd is integer
            } else {
                // All other F/D extension instructions: all registers are FP
                useFPReg = true;
            }
        }
    }
    
    // Use the determined register type
    if(useFPReg && regnum + 32 < lang->regnames.size()) {
        QString fpname = lang->regnames[regnum + 32];
        if(fpname.startsWith("f")) {
            return fpname;
        }
    }
    
    // Prefer integer registers for non-FP or when useFPReg is false
    if(regnum < 32 && regnum < lang->regnames.size()) {
        QString intname = lang->regnames[regnum];
        // If it's not a floating point register name, use it
        if(!intname.startsWith("f")) {
            return intname;
        }
    }
    
    // Fallback to floating point register if integer register wasn't found
    if(regnum + 32 < lang->regnames.size()) {
        QString fpname = lang->regnames[regnum + 32];
        if(fpname.startsWith("f")) {
            return fpname;
        }
    }
    
    // Final fallback
    if(regnum < lang->regnames.size()) {
        return lang->regnames[regnum];
    }
    return QString::asprintf("r%llu", (unsigned long long) regnum);
}

void GAParameterRiscvReg::encode(GALanguage *lang,
            uint64_t adr, QByteArray &bytes,
            GAParserOperand op,
            int inslen
            ){
    int regnum = lang->regnames.indexOf(op.value);
    if(regnum == -1) {
        op.goodasm->error("Unknown RISC-V register: " + op.value);
        return;
    }
    // Floating point registers f0-f31 are at indices 32-63, but encode as 0-31
    if(regnum >= 32 && regnum < 64) {
        regnum -= 32;
    }
    rawencode(lang, adr, bytes, op, inslen, regnum);
}

// RISC-V Imm12 (imm[11:0]) parameter

GAParameterRiscvImm12::GAParameterRiscvImm12(const char* mask){
    setMask(mask);
    prefix = "#";
}

int GAParameterRiscvImm12::match(GAParserOperand *op, int len){
    int64_t val=op->int64(false); //False on a match, int64 handles negative prefix.

    // must fit signed 12-bit immediate
    if (val < -2048 || val > 2047) {
        op->goodasm->error("RISC-V imm12 is out of range (-2048 to 2047)");
        return 0;
    }

    return 1; // valid
}

QString GAParameterRiscvImm12::decode(GALanguage *lang, uint64_t adr, const char *bytes, int inslen){
    uint64_t raw = rawdecode(lang,adr,bytes,inslen);
    int32_t val = (raw & 0x800) ? (raw | ~0xFFF) : (raw & 0xFFF);
    // Format as hex if value is >= 0x10 or negative, otherwise decimal
    if (val < 0 || (uint32_t)val >= 0x10) {
        return prefix + QString::asprintf("0x%x", (uint32_t)val);
    }
    return prefix + QString::number(val);
}

void GAParameterRiscvImm12::encode(GALanguage *lang,
            uint64_t adr, QByteArray &bytes,
            GAParserOperand op,
            int inslen
            ){
    int64_t val = op.int64(true);

    if (val < -2048 || val > 2047) {
        op.goodasm->error("RISC-V imm12 is out of range (-2048 to 2047).");
        return;
    }

    rawencode(lang,adr,bytes,op,inslen,val);
}

// RISC-V U-type Imm20 (imm[31:12]) parameter

GAParameterRiscvUtypeImm20::GAParameterRiscvUtypeImm20(const char* mask){
    setMask(mask);
    prefix = "#";
}

int GAParameterRiscvUtypeImm20::match(GAParserOperand *op, int len){
    // Try parsing as unsigned first, then signed if that fails
    uint64_t val=op->uint64(false); // False on a match
    
    // If uint64 parsing failed, try int64 (for negative values, though U-type shouldn't have them)
    if (val == (uint64_t)-1) {
        int64_t sval = op->int64(false);
        if (sval == -1) {
            return 0; // Parsing failed, let other parameter types try
        }
        // If negative, reject (U-type immediates are unsigned)
        if (sval < 0) {
            return 0;
        }
        val = (uint64_t)sval;
    }

    // U-type immediate: the value is the result value (what should be in rd)
    // Must be <= 0xFFFFF000 and a multiple of 0x1000 (lower 12 bits must be zero)
    
    // Check if value is out of range
    if (val > 0xFFFFF000) {
        op->goodasm->error("RISC-V U-type instruction imm20 is out of range (0 to 0xFFFFF000, must be multiple of 0x1000)");
        return 0;
    }
    
    // Check if value is a multiple of 0x1000
    if ((val & 0xFFF) != 0) {
        op->goodasm->error("RISC-V U-type instruction imm20 must be a multiple of 0x1000 (lower 12 bits must be zero)");
        return 0;
    }
    
    return 1; // valid value
}

QString GAParameterRiscvUtypeImm20::decode(GALanguage *lang, uint64_t adr, const char *bytes, int inslen){
    uint64_t raw = rawdecode(lang,adr,bytes,inslen);
    // rawdecode extracts bits [31:12] which is the immediate field
    // lui sets rd = imm[31:12] << 12, so we need to left-shift to show the result value
    uint32_t imm_field = raw & 0xFFFFF;
    uint64_t result = ((uint64_t)imm_field) << 12;
    return prefix + QString::asprintf("0x%llx", (unsigned long long)result);
}

void GAParameterRiscvUtypeImm20::encode(GALanguage *lang,
            uint64_t adr, QByteArray &bytes,
            GAParserOperand op,
            int inslen
            ){
    int64_t val = op.int64(true);
    uint32_t imm_field;

    // lui/auipc: rd = imm[31:12] << 12
    // The immediate value provided is the result value (what should be in rd)
    // Extract the immediate field: imm[31:12] = val >> 12
    
    // Check if value is out of range (max 32-bit value with lower 12 bits zero)
    if (val > 0xFFFFF000) {
        op.goodasm->error("RISC-V U-type imm20 is out of range (0 to 0xFFFFF000, must be multiple of 0x1000).");
        return;
    }
    
    // Check if value is a multiple of 0x1000 (lower 12 bits must be zero)
    if ((val & 0xFFF) != 0) {
        op.goodasm->error("RISC-V U-type imm20 must be a multiple of 0x1000 (lower 12 bits must be zero).");
        return;
    }

    // Right-shift by 12 to get the immediate field value that goes in bits [31:12]
    imm_field = (val >> 12) & 0xFFFFF;
    
    rawencode(lang,adr,bytes,op,inslen,imm_field);
}

// RISC-V B-type Imm13 (imm[12:1]) parameter
// Encoding: imm[12] in bit [31], imm[11] in bit [7], imm[10:5] in bits [30:25], imm[4:1] in bits [11:8]
// Mask: "\x80\x8F\xF0\xBF" - combined from: imm[12] (0x80 in byte 3), imm[11] (0x80 in byte 1), imm[10:5] (0xF0 in byte 2, 0x3F in byte 3), imm[4:1] (0x0F in byte 1)

GAParameterRiscvBtypeImm13::GAParameterRiscvBtypeImm13(const char* mask){
    setMask(mask);
    prefix = "#";
}

int GAParameterRiscvBtypeImm13::match(GAParserOperand *op, int len){
    int64_t val=op->int64(false); // False on a match

    // must fit signed 13-bit immediate, and must be even (imm[0] = 0)
    if (val < -4096 || val > 4094) {
        op->goodasm->error("RISC-V B-type imm13 is out of range (-4096 to 4094)");
        return 0;
    }
    if ((val & 1) != 0) {
        op->goodasm->error("RISC-V B-type imm13 must be even (target address must be 2-byte aligned)");
        return 0;
    }

    return 1; // valid
}

QString GAParameterRiscvBtypeImm13::decode(GALanguage *lang, uint64_t adr, const char *bytes, int inslen){
    uint64_t raw = rawdecode(lang,adr,bytes,inslen);
    // rawdecode packs bits in mask order: bit7(imm[11]), bits8-11(imm[4:1]), bits25-30(imm[10:5]), bit31(imm[12])
    int32_t imm11 = raw & 1;
    int32_t imm4_1 = (raw >> 1) & 0xF;
    int32_t imm10_5 = (raw >> 5) & 0x3F;
    int32_t imm12 = (raw >> 11) & 1;
    int32_t imm = (imm12 << 12) | (imm11 << 11) | (imm10_5 << 5) | (imm4_1 << 1);
    // Sign extend from 13 bits
    if (imm & 0x1000) {
        imm |= ~0x1FFF;
    }
    return prefix + QString::number(imm);
}

void GAParameterRiscvBtypeImm13::encode(GALanguage *lang,
            uint64_t adr, QByteArray &bytes,
            GAParserOperand op,
            int inslen
            ){
    int64_t val = op.int64(true);

    if (val < -4096 || val > 4094) {
        op.goodasm->error("RISC-V B-type imm13 is out of range (-4096 to 4094).");
        return;
    }
    if ((val & 1) != 0) {
        op.goodasm->error("RISC-V B-type imm13 must be even (target address must be 2-byte aligned).");
        return;
    }

    // Pack imm[12:1] in mask order: imm[11], imm[4:1], imm[10:5], imm[12]
    uint32_t imm = val & 0x1FFE;
    uint32_t raw = ((imm >> 11) & 1) |
                   ((imm >> 1) & 0xF) << 1 |
                   ((imm >> 5) & 0x3F) << 5 |
                   ((imm >> 12) & 1) << 11;

    rawencode(lang,adr,bytes,op,inslen,raw);
}

// RISC-V J-type Imm21 (imm[20:1]) parameter
// Encoding: imm[20] in bit [31], imm[19:12] in bits [19:12], imm[11] in bit [20], imm[10:1] in bits [30:21]
// Mask: "\x00\xFF\x10\xFF" - combined from: imm[20] (0x80 in byte 3), imm[19:12] (0xFF in byte 1), imm[11] (0x10 in byte 2), imm[10:1] (0x7F in byte 3)

GAParameterRiscvJtypeImm21::GAParameterRiscvJtypeImm21(const char* mask){
    setMask(mask);
    prefix = "#";
}

int GAParameterRiscvJtypeImm21::match(GAParserOperand *op, int len){
    int64_t val=op->int64(false); // False on a match

    // must fit signed 21-bit immediate, and must be even (imm[0] = 0)
    if (val < -1048576 || val > 1048574) {
        op->goodasm->error("RISC-V J-type imm21 is out of range (-1048576 to 1048574)");
        return 0;
    }
    if ((val & 1) != 0) {
        op->goodasm->error("RISC-V J-type imm21 must be even (target address must be 2-byte aligned)");
        return 0;
    }

    return 1; // valid
}

QString GAParameterRiscvJtypeImm21::decode(GALanguage *lang, uint64_t adr, const char *bytes, int inslen){
    uint64_t raw = rawdecode(lang,adr,bytes,inslen);
    // Reconstruct imm[20:1] from mask-ordered bits
    // Mask "\x00\xF0\xFF\xFF" extracts bits in this order:
    // raw[3:0] = inst[15:12] = imm[19:16]
    // raw[7:4] = inst[19:16] = imm[15:12]
    // raw[8] = inst[20] = imm[11]
    // raw[11:9] = inst[23:21] = imm[10:8]
    // raw[18:12] = inst[30:24] = imm[7:1]
    // raw[19] = inst[31] = imm[20]
    int32_t imm20 = (raw >> 19) & 1;
    int32_t imm19_16 = raw & 0xF;
    int32_t imm15_12 = (raw >> 4) & 0xF;
    int32_t imm11 = (raw >> 8) & 1;
    int32_t imm10_8 = (raw >> 9) & 0x7;
    int32_t imm7_1 = (raw >> 12) & 0x7F;
    int32_t imm = (imm20 << 20) | (imm19_16 << 16) | (imm15_12 << 12) | (imm11 << 11) | (imm10_8 << 8) | (imm7_1 << 1);
    // Sign extend from 21 bits
    if (imm & 0x100000) {
        imm |= ~0x1FFFFF;
    }
    return prefix + QString::number(imm);
}

void GAParameterRiscvJtypeImm21::encode(GALanguage *lang,
            uint64_t adr, QByteArray &bytes,
            GAParserOperand op,
            int inslen
            ){
    int64_t val = op.int64(true);

    if (val < -1048576 || val > 1048574) {
        op.goodasm->error("RISC-V J-type imm21 is out of range (-1048576 to 1048574).");
        return;
    }
    if ((val & 1) != 0) {
        op.goodasm->error("RISC-V J-type imm21 must be even (target address must be 2-byte aligned).");
        return;
    }

    // Pack imm[20:1] into mask order
    // raw[3:0] = imm[19:16]
    // raw[7:4] = imm[15:12]
    // raw[8] = imm[11]
    // raw[11:9] = imm[10:8]
    // raw[18:12] = imm[7:1]
    // raw[19] = imm[20]
    uint32_t imm = val & 0x1FFFFE; // imm[20:1], clear imm[0]
    uint32_t raw = (((imm >> 20) & 1) << 19) |      // imm[20] -> raw[19]
                   (((imm >> 1) & 0x7F) << 12) |    // imm[7:1] -> raw[18:12]
                   (((imm >> 8) & 0x7) << 9) |      // imm[10:8] -> raw[11:9]
                   (((imm >> 11) & 1) << 8) |       // imm[11] -> raw[8]
                   (((imm >> 12) & 0xF) << 4) |     // imm[15:12] -> raw[7:4]
                   ((imm >> 16) & 0xF);             // imm[19:16] -> raw[3:0]

    rawencode(lang,adr,bytes,op,inslen,raw);
}

// RISC-V S-type Imm12 (imm[11:0]) parameter
// Encoding: imm[11:5] in bits [31:25], imm[4:0] in bits [11:7]
// Mask: "\x80\x0F\x00\xFE" - imm[4:0] in bits [11:7] (byte 0 bit 7 + byte 1 bits 0-3), imm[11:5] in bits [31:25] (byte 3 bits 1-7)

GAParameterRiscvStypeImm12::GAParameterRiscvStypeImm12(const char* mask){
    setMask(mask);
    prefix = "#";
    isSigned = true;  // 12-bit signed immediate
}

int GAParameterRiscvStypeImm12::match(GAParserOperand *op, int len){
    int64_t val=op->int64(false); // False on a match

    // must fit signed 12-bit immediate
    if (val < -2048 || val > 2047) {
        op->goodasm->error("RISC-V S-type imm12 is out of range (-2048 to 2047)");
        return 0;
    }

    return 1; // valid
}

QString GAParameterRiscvStypeImm12::decode(GALanguage *lang, uint64_t adr, const char *bytes, int inslen){
    uint64_t raw = rawdecode(lang,adr,bytes,inslen);
    // rawdecode already extracts and packs imm[11:0] from the split encoding via the mask
    int32_t imm = raw & 0xFFF;
    // Sign extend from 12 bits
    if (imm & 0x800) {
        imm |= ~0xFFF;
    }
    return prefix + QString::number(imm);
}

void GAParameterRiscvStypeImm12::encode(GALanguage *lang,
            uint64_t adr, QByteArray &bytes,
            GAParserOperand op,
            int inslen
            ){
    int64_t val = op.int64(true);

    if (val < -2048 || val > 2047) {
        op.goodasm->error("RISC-V S-type imm12 is out of range (-2048 to 2047).");
        return;
    }

    // rawencode expects the immediate value directly (in mask bit order, not instruction bit positions)
    // The mask will distribute the bits to the correct instruction positions
    uint32_t imm = val & 0xFFF; // imm[11:0] - mask to 12 bits
    rawencode(lang, adr, bytes, op, inslen, imm);
}

// RISC-V FENCE pred/succ parameter
// Encoding: pred[3:0] in imm[3:0] (bits [23:20], byte 2 bits [7:4])
//           succ[3:0] in imm[7:4] (bits [27:24], byte 3 bits [3:0])
// Mask: "\x00\x00\xF0\x0F" - pred in byte 2 bits [7:4], succ in byte 3 bits [3:0]

GAParameterRiscvFencePredSucc::GAParameterRiscvFencePredSucc(const char* mask){
    setMask(mask);
    prefix = "#";
}

int GAParameterRiscvFencePredSucc::match(GAParserOperand *op, int len){
    int64_t val=op->uint64(false); // False on a match

    // must fit 8-bit value (0-255) where lower 4 bits are pred and upper 4 bits are succ
    if (val < 0 || val > 255) {
        op->goodasm->error("RISC-V FENCE pred/succ is out of range (0 to 255)");
        return 0;
    }

    return 1; // valid
}

QString GAParameterRiscvFencePredSucc::decode(GALanguage *lang, uint64_t adr, const char *bytes, int inslen){
    // Extract pred[3:0] from imm[3:0] (bits [23:20], byte 2 bits [7:4])
    // Extract succ[3:0] from imm[7:4] (bits [27:24], byte 3 bits [3:0])
    // For little-endian: byte 2 is bytes[2], byte 3 is bytes[3]
    uint32_t pred = (bytes[2] >> 4) & 0x0F;  // byte 2 bits [7:4] -> pred[3:0]
    uint32_t succ = bytes[3] & 0x0F;          // byte 3 bits [3:0] -> succ[3:0]
    uint32_t val = (succ << 4) | pred;
    return QString::asprintf("#0x%02X", val);
}

void GAParameterRiscvFencePredSucc::encode(GALanguage *lang,
            uint64_t adr, QByteArray &bytes,
            GAParserOperand op,
            int inslen
            ){
    int64_t val = op.uint64(true);

    if (val < 0 || val > 255) {
        op.goodasm->error("RISC-V FENCE pred/succ is out of range (0 to 255).");
        return;
    }

    // Split into pred[3:0] and succ[3:0]
    uint32_t pred = val & 0x0F;        // lower 4 bits
    uint32_t succ = (val >> 4) & 0x0F; // upper 4 bits
    
    // Encode directly into bytes: pred in byte 2 bits [7:4], succ in byte 3 bits [3:0]
    // For little-endian: byte 2 is bytes[2], byte 3 is bytes[3]
    bytes[2] = (bytes[2] & 0x0F) | (pred << 4);  // Set byte 2 bits [7:4] to pred
    bytes[3] = (bytes[3] & 0xF0) | succ;          // Set byte 3 bits [3:0] to succ
}

// RISC-V Shamt5 (shamt[4:0]) parameter
// Encoding: shamt[4:0] in bits [24:20]
// Mask: "\x00\x00\xF0\x01" - bits [24:20] in bytes 2-3 (byte 2 bits 7-4, byte 3 bit 0)

GAParameterRiscvShamt5::GAParameterRiscvShamt5(const char* mask){
    setMask(mask);
    prefix = "#";
}

int GAParameterRiscvShamt5::match(GAParserOperand *op, int len){
    int64_t val=op->uint64(false); // False on a match

    // must fit 5-bit shift amount (0 to 31)
    if (val < 0 || val > 31) {
        op->goodasm->error("RISC-V shamt5 is out of range (0 to 31)");
        return 0;
    }

    return 1; // valid
}

QString GAParameterRiscvShamt5::decode(GALanguage *lang, uint64_t adr, const char *bytes, int inslen){
    uint64_t raw = rawdecode(lang,adr,bytes,inslen);
    // rawdecode already extracts bits [24:20] into the value, so just return it
    uint32_t shamt = raw & 0x1F; // Mask to 5 bits just to be safe
    return prefix + QString::number(shamt);
}

void GAParameterRiscvShamt5::encode(GALanguage *lang,
            uint64_t adr, QByteArray &bytes,
            GAParserOperand op,
            int inslen
            ){
    int64_t val = op.int64(true);

    if (val < 0 || val > 31) {
        op.goodasm->error("RISC-V shamt5 is out of range (0 to 31).");
        return;
    }

    // Encode shamt[4:0] into bits [24:20]
    rawencode(lang,adr,bytes,op,inslen,val);
}

// RISC-V CSR12 (csr[11:0]) parameter
// Encoding: csr[11:0] in bits [31:20]
// Mask: "\x00\x00\xF0\xFF" - bits [31:20] in bytes 2-3

GAParameterRiscvCsr12::GAParameterRiscvCsr12(const char* mask){
    setMask(mask);
    prefix = "#";
}

int GAParameterRiscvCsr12::match(GAParserOperand *op, int len){
    int64_t val=op->uint64(false); // False on a match

    // must fit unsigned 12-bit CSR address (0 to 4095)
    if (val < 0 || val > 4095) {
        op->goodasm->error("RISC-V CSR12 is out of range (0 to 4095)");
        return 0;
    }

    return 1; // valid
}

QString GAParameterRiscvCsr12::decode(GALanguage *lang, uint64_t adr, const char *bytes, int inslen){
    uint64_t raw = rawdecode(lang,adr,bytes,inslen);
    // rawdecode already extracts bits [31:20] into the lower bits
    uint32_t csr = raw & 0xFFF;
    return QString::asprintf("#0x%03X", csr);
}

void GAParameterRiscvCsr12::encode(GALanguage *lang,
            uint64_t adr, QByteArray &bytes,
            GAParserOperand op,
            int inslen
            ){
    int64_t val = op.uint64(true);

    if (val < 0 || val > 4095) {
        op.goodasm->error("RISC-V CSR12 is out of range (0 to 4095).");
        return;
    }

    // Encode csr[11:0] into bits [31:20]
    rawencode(lang,adr,bytes,op,inslen,val);
}

// RISC-V Uimm5 (uimm[4:0]) parameter
// Encoding: uimm[4:0] in bits [19:15]
// Mask: "\x00\x80\x0F\x00" - bits [19:15] in byte 1-2

GAParameterRiscvUimm5::GAParameterRiscvUimm5(const char* mask){
    setMask(mask);
    prefix = "#";
}

int GAParameterRiscvUimm5::match(GAParserOperand *op, int len){
    int64_t val=op->uint64(false); // False on a match

    // must fit 5-bit unsigned immediate (0 to 31)
    if (val < 0 || val > 31) {
        op->goodasm->error("RISC-V uimm5 is out of range (0 to 31)");
        return 0;
    }

    return 1; // valid
}

QString GAParameterRiscvUimm5::decode(GALanguage *lang, uint64_t adr, const char *bytes, int inslen){
    uint64_t raw = rawdecode(lang,adr,bytes,inslen);
    // rawdecode already extracts bits [19:15] into the lower bits
    uint32_t uimm = raw & 0x1F;
    return prefix + QString::number(uimm);
}

void GAParameterRiscvUimm5::encode(GALanguage *lang,
            uint64_t adr, QByteArray &bytes,
            GAParserOperand op,
            int inslen
            ){
    int64_t val = op.uint64(true);

    if (val < 0 || val > 31) {
        op.goodasm->error("RISC-V uimm5 is out of range (0 to 31).");
        return;
    }

    // Encode uimm[4:0] into bits [19:15]
    rawencode(lang,adr,bytes,op,inslen,val);
}

// C Extension parameter types for compressed 16-bit instructions

// RISC-V Compressed Register (rd'/rs1'/rs2') parameter
// Encoding: 3-bit field for registers x8-x15 (encoded as 0-7)
// rd' in bits [4:2] (CIW format) or bits [9:7] (most formats)
// rs1' in bits [9:7]
// rs2' in bits [4:2]
// Mask depends on specific format

GAParameterRiscvCompReg::GAParameterRiscvCompReg(const char* mask){
    setMask(mask);
}

int GAParameterRiscvCompReg::match(GAParserOperand *op, int len){
    if(op->prefix!=this->prefix)
        return 0;

    // Check if it is a valid RISC-V register name.
    QString name = op->value;
    // Compressed registers are x8-x15 (s0-s1, a0-a7)
    if(name == "s0" || name == "s1" ||
        name == "a0" || name == "a1" || name == "a2" || name == "a3" || 
        name == "a4" || name == "a5" || name == "a6" || name == "a7") {
         return 1;
     }
    return 0;
}

QString GAParameterRiscvCompReg::decode(GALanguage *lang, uint64_t adr, const char *bytes, int inslen){
    uint64_t regnum = rawdecode(lang, adr, bytes, inslen);
    // Compressed register encoding: 0-7 maps to x8-x15
    if(regnum > 7) {
        return QString::asprintf("r%llu", (unsigned long long)(regnum + 8));
    }
    // Map 0-7 to x8-x15 (s0, s1, a0-a7)
    int regnames_idx = 8 + regnum; // x8 = s0, x9 = s1, x10 = a0, etc.
    if(regnames_idx < lang->regnames.size()) {
        return lang->regnames[regnames_idx];
    }
    return QString::asprintf("r%llu", (unsigned long long)(regnum + 8));
}

void GAParameterRiscvCompReg::encode(GALanguage *lang,
            uint64_t adr, QByteArray &bytes,
            GAParserOperand op,
            int inslen
            ){
    int regnum = lang->regnames.indexOf(op.value);
    if(regnum == -1) {
        op.goodasm->error("Unknown RISC-V register: " + op.value);
        return;
    }
    // Compressed registers must be x8-x15
    if(regnum < 8 || regnum > 15) {
        op.goodasm->error("RISC-V compressed register must be x8-x15 (s0, s1, a0-a7): " + op.value);
        return;
    }
    // Map x8-x15 (8-15) to compressed encoding (0-7)
    int compressed = regnum - 8;
    rawencode(lang, adr, bytes, op, inslen, compressed);
}

// RISC-V CI Format Immediate (imm[5:0]) parameter
// Encoding: imm[5] in bit [12], imm[4:0] in bits [6:2]
// Mask depends on specific instruction

GAParameterRiscvCIimm6::GAParameterRiscvCIimm6(const char* mask){
    setMask(mask);
    prefix = "#";
}

int GAParameterRiscvCIimm6::match(GAParserOperand *op, int len){
    int64_t val=op->int64(false); // False on a match

    // Standard CI format: must fit signed 6-bit immediate (-32 to 31)
    if (val >= -32 && val <= 31) {
        return 1; // valid for standard CI
    }
    
    // c.lui: accepts values 0-63 (imm[17:12]) or shifted values (imm << 12)
    // where imm[17:12] is in range 0-63
    // This means values that are multiples of 0x1000 in range 0x0 to 0x3F000
    if (val >= 0) {
        if (val <= 63) {
            return 1; // valid for c.lui (direct imm[17:12])
        }
        // Check if it's a valid shifted value for c.lui
        if ((val & 0xFFF) == 0) { // Must be multiple of 0x1000
            uint32_t imm_upper = (val >> 12) & 0x3F;
            if (imm_upper >= 1 && imm_upper <= 63) { // c.lui cannot be 0 (reserved for c.addi16sp)
                return 1; // valid for c.lui (shifted value)
            }
        }
    }
    
    // c.addi16sp: accepts multiples of 16 in range -512 to 496
    if ((val & 0xF) == 0) { // Must be multiple of 16
        if (val >= -512 && val <= 496) {
            return 1; // valid for c.addi16sp
        }
    }

    // Value doesn't fit any CI format
    op->goodasm->error("RISC-V CI imm6 is out of range (-32 to 31 for standard CI, 0-63 or multiples of 0x1000 up to 0x3F000 for c.lui, or multiples of 16 in -512 to 496 for c.addi16sp)");
    return 0;
}

QString GAParameterRiscvCIimm6::decode(GALanguage *lang, uint64_t adr, const char *bytes, int inslen){
    uint64_t raw = rawdecode(lang,adr,bytes,inslen);
    
    // Check if this is c.addi16sp (rd=2, bits [1:0]=01, [15:13]=011) or c.lui (bits [1:0]=01, [15:13]=011, rd!=0,2)
    // rd is the definitive distinguisher - c.addi16sp has rd=2, c.lui has rd != 0 and rd != 2
    bool is_addi16sp = false;
    bool is_lui = false;
    if(inslen >= 2) {
        uint8_t byte0 = bytes[0];
        uint8_t byte1 = bytes[1];
        uint8_t opcode_low = byte0 & 0x03; // bits [1:0]
        uint8_t opcode_high = (byte1 >> 5) & 0x07; // bits [15:13]
        
        // Extract rd from bits [11:7]:
        // bit [7] is in byte0 bit 7
        // bits [11:8] are in byte1 bits [3:0]
        // So rd = (bits [11:8] << 1) | bit [7]
        uint8_t rd_high = (byte1 & 0x0F);  // bits [11:8]
        uint8_t rd_low = (byte0 >> 7) & 0x01;  // bit [7]
        uint8_t rd = (rd_high << 1) | rd_low;
        
        if(opcode_low == 0x01 && opcode_high == 0x03) { // bits [1:0]=01, [15:13]=011
            if(rd == 2) {
                is_addi16sp = true;  // rd=2 definitively indicates c.addi16sp
            } else if(rd != 0 && rd != 2) {
                is_lui = true;  // rd != 0,2 definitively indicates c.lui
            }
            // rd == 0 is invalid for both c.addi16sp and c.lui (reserved)
        }
    }
    
    if(is_addi16sp) {
        // c.addi16sp: nzimm[9:4] encoded in bits [12|6|5:2]
        // According to RISC-V spec and rawencode order:
        //   rawencode processes mask 0x7C10 in order:
        //   val[0]->bit[2] (nzimm[6])
        //   val[1]->bit[3] (nzimm[7])
        //   val[2]->bit[4] (nzimm[8])
        //   val[3]->bit[5] (nzimm[9])
        //   val[4]->bit[6] (nzimm[4])
        //   val[5]->bit[12] (nzimm[5])
        // So rawdecode extracts in the same order:
        //   raw[0] = nzimm[6]
        //   raw[1] = nzimm[7]
        //   raw[2] = nzimm[8]
        //   raw[3] = nzimm[9]
        //   raw[4] = nzimm[4]
        //   raw[5] = nzimm[5]
        // The instruction does: sp = sp + sext(nzimm[9:4] << 4)
        if(inslen >= 2) {
            // Extract individual bits from raw value (already decoded by rawdecode)
            uint32_t nzimm6 = (raw >> 0) & 1;  // raw[0] = nzimm[6]
            uint32_t nzimm7 = (raw >> 1) & 1;  // raw[1] = nzimm[7]
            uint32_t nzimm8 = (raw >> 2) & 1;  // raw[2] = nzimm[8]
            uint32_t nzimm9 = (raw >> 3) & 1;  // raw[3] = nzimm[9]
            uint32_t nzimm4 = (raw >> 4) & 1;  // raw[4] = nzimm[4]
            uint32_t nzimm5 = (raw >> 5) & 1;  // raw[5] = nzimm[5]
            // Reconstruct nzimm[9:4] (6 bits): nzimm[9] is MSB, nzimm[4] is LSB
            uint32_t nzimm_6bit = (nzimm9 << 5) | (nzimm8 << 4) | (nzimm7 << 3) | (nzimm6 << 2) | (nzimm5 << 1) | nzimm4;
            // Sign extend from 6 bits (nzimm[9] is the sign bit)
            // Convert to signed 6-bit value: if bit 5 is set, it's negative
            int32_t signed_nzimm_6bit;
            if(nzimm_6bit & 0x20) {  // Check bit 5 (nzimm[9] when in position) - negative
                signed_nzimm_6bit = (int32_t)(nzimm_6bit | 0xFFFFFFC0);  // Sign extend to 32 bits
            } else {
                signed_nzimm_6bit = (int32_t)nzimm_6bit;  // Positive, no sign extension needed
            }
            // The instruction does: sp = sp + sext(nzimm[9:4] << 4)
            int32_t result = signed_nzimm_6bit << 4;
            return prefix + QString::number(result);
        }
        return prefix + QString::number(0);
    } else if(is_lui) {
        // c.lui: imm[17:12] in bits [12|6:2] (same positions as CI format)
        // The instruction does: rd = imm << 12
        // Extract directly from bytes
        if(inslen >= 2) {
            uint8_t byte0 = bytes[0];
            uint8_t byte1 = bytes[1];
            uint32_t imm17 = (byte1 >> 4) & 1;      // bit [12] = byte1 bit 4
            uint32_t imm16_12 = (byte0 >> 2) & 0x1F; // bits [6:2] = byte0 bits 6:2
            uint32_t imm_upper = (imm17 << 5) | imm16_12; // imm[17:12]
            // Return the full shifted value (imm << 12) for display
            uint32_t imm_full = imm_upper << 12;
            return prefix + QString::asprintf("0x%x", imm_full);
        }
        return QString::number(0);
    }
    
    // Standard CI format: rawdecode packs bits in mask order
    // Mask 0x7C10: bits [6:2] -> raw[4:0], bit [12] -> raw[5]
    uint32_t imm4_0 = raw & 0x1F;       // raw[4:0] = imm[4:0]
    uint32_t imm5 = (raw >> 5) & 1;     // raw[5] = imm[5]
    int32_t imm = (imm5 << 5) | imm4_0;
    // Sign extend from 6 bits
    if (imm & 0x20) {
        imm |= ~0x3F;
    }
    return prefix + QString::number(imm);
}

void GAParameterRiscvCIimm6::encode(GALanguage *lang,
            uint64_t adr, QByteArray &bytes,
            GAParserOperand op,
            int inslen
            ){
    int64_t val = op.int64(true);
    
    // Check if this might be c.addi16sp (value is multiple of 16, range -512 to 496)
    // or c.lui (value represents upper 6 bits, typically 0-63 but represents imm << 12)
    // We detect by checking opcode, rd value, and value constraints
    bool is_addi16sp = false;
    bool is_lui = false;
    if(inslen >= 2 && bytes.length() >= 2) {
        uint8_t byte0 = bytes[0];
        uint8_t byte1 = bytes[1];
        uint8_t opcode_low = byte0 & 0x03;
        uint8_t opcode_high = (byte1 >> 5) & 0x07;
        
        if(opcode_low == 0x01 && opcode_high == 0x03) {
            // Extract rd from bits [11:7]:
            // bit [7] is in byte0 bit 7
            // bits [11:8] are in byte1 bits [3:0]
            // So rd = (bits [11:8] << 1) | bit [7]
            uint8_t rd_high = (byte1 & 0x0F);  // bits [11:8]
            uint8_t rd_low = (byte0 >> 7) & 0x01;  // bit [7]
            uint8_t rd = (rd_high << 1) | rd_low;
            
            // Check value constraints
            bool val_is_multiple_of_16 = ((val & 0xF) == 0) && (val >= -512) && (val <= 496);
            bool val_is_lui_range = (val >= 0 && val <= 63) || ((val & 0xFFF) == 0 && (val >> 12) >= 1 && (val >> 12) <= 63);
            
            // Check value constraints first - if value can only be one instruction, use that
            if(val_is_lui_range && !val_is_multiple_of_16) {
                // Value only matches c.lui range
                is_lui = true;
            } else if(val_is_multiple_of_16 && !val_is_lui_range) {
                // Value only matches c.addi16sp range
                is_addi16sp = true;
            } else {
                // Value matches both or neither - use rd as definitive distinguisher
                if(rd == 2) {
                    is_addi16sp = true;  // rd=2 definitively indicates c.addi16sp
                } else if(rd != 0 && rd != 2) {
                    is_lui = true;  // rd != 0,2 definitively indicates c.lui
                } else {
                    // rd is 0 or not yet set - value matches both ranges
                    // If value is a multiple of 0x1000 (c.lui format), prefer c.lui
                    if((val & 0xFFF) == 0 && (val >> 12) >= 1 && (val >> 12) <= 63) {
                        is_lui = true;
                    } else if(val_is_multiple_of_16) {
                        // Value is multiple of 16 but not 0x1000 - prefer c.addi16sp
                        is_addi16sp = true;
                    }
                    // If neither matches, neither will be set and standard CI encoding will be used
                }
            }
        }
    }
    
    if(is_addi16sp) {
        // c.addi16sp: encode imm[9|4|6|8:7|5] where imm is the actual value (not shifted)
        // The instruction does: sp = sp + (imm << 4), so we need to divide by 16
        // Also ensure rd=2 (sp) is set in the instruction
        if((val & 0xF) != 0) {
            op.goodasm->error("RISC-V c.addi16sp immediate must be a multiple of 16.");
            return;
        }
        if(val < -512 || val > 496) {
            op.goodasm->error("RISC-V c.addi16sp immediate is out of range (-512 to 496).");
            return;
        }
        // Set rd=2 (sp) in the instruction: rd bits [11:7] = 2 = 00010
        // bit [7] is in byte0 bit 7, bits [11:8] are in byte1 bits [3:0]
        if(inslen >= 2 && bytes.length() >= 2) {
            bytes[0] = (bytes[0] & ~0x80) | (0 << 7);  // bit [7] = 0 (from rd bit 0)
            bytes[1] = (bytes[1] & ~0x0F) | (1 << 0);  // bits [11:8] = 0001 (from rd bits [4:1])
        }
        int32_t imm_shifted = (int32_t)(val >> 4); // Divide by 16 (signed)
        // Convert to 6-bit two's complement for encoding
        // Mask to 6 bits - this automatically handles two's complement for negative values
        uint32_t nzimm_6bit = (uint32_t)(imm_shifted & 0x3F); // Get lower 6 bits (handles negative correctly)
        // Now extract the individual bits for the scrambled encoding
        // According to RISC-V spec:
        //   nzimm[5] goes to bit [12]
        //   nzimm[4] goes to bit [6]
        //   nzimm[9:6] go to bits [5:2]
        uint32_t nzimm9 = (nzimm_6bit >> 5) & 1;  // nzimm[9] -> bit [5]
        uint32_t nzimm8 = (nzimm_6bit >> 4) & 1;  // nzimm[8] -> bit [4]
        uint32_t nzimm7 = (nzimm_6bit >> 3) & 1;  // nzimm[7] -> bit [3]
        uint32_t nzimm6 = (nzimm_6bit >> 2) & 1;  // nzimm[6] -> bit [2]
        uint32_t nzimm5 = (nzimm_6bit >> 1) & 1;  // nzimm[5] -> bit [12]
        uint32_t nzimm4 = (nzimm_6bit >> 0) & 1;  // nzimm[4] -> bit [6]
        // Encode according to c.addi16sp format
        // rawencode processes mask 0x7C10 (little-endian) in this order:
        // Byte 0: bits [6:2] = 0x7C, processes j=2,3,4,5,6 in order
        // Byte 1: bit [12] = 0x10, processes j=4 in byte1
        // So the order is: val[0]->bit[2], val[1]->bit[3], val[2]->bit[4], val[3]->bit[5], val[4]->bit[6], val[5]->bit[12]
        // Which means: nzimm[6]->bit[2], nzimm[7]->bit[3], nzimm[8]->bit[4], nzimm[9]->bit[5], nzimm[4]->bit[6], nzimm[5]->bit[12]
        uint32_t raw = (nzimm6 << 0) |    // val[0] -> bit [2] (nzimm[6])
                       (nzimm7 << 1) |    // val[1] -> bit [3] (nzimm[7])
                       (nzimm8 << 2) |    // val[2] -> bit [4] (nzimm[8])
                       (nzimm9 << 3) |    // val[3] -> bit [5] (nzimm[9])
                       (nzimm4 << 4) |    // val[4] -> bit [6] (nzimm[4])
                       (nzimm5 << 5);     // val[5] -> bit [12] (nzimm[5])
        rawencode(lang,adr,bytes,op,inslen,raw);
    } else if(is_lui) {
        // c.lui: encode imm[17:12] where the value represents the upper 6 bits
        // The instruction does: rd = imm << 12
        // If val is already the shifted value (like 0x10000), extract upper bits
        // Otherwise, if val is 0-63, treat it as imm[17:12] directly
        uint32_t imm_upper;
        if(val >= 0 && val <= 63) {
            imm_upper = val; // Direct value 0-63
        } else {
            imm_upper = (val >> 12) & 0x3F; // Extract upper 6 bits from shifted value
        }
        if(imm_upper == 0) {
            op.goodasm->error("RISC-V c.lui immediate cannot be 0 (reserved for c.addi16sp).");
            return;
        }
        // Note: imm_upper == 1 is technically reserved per spec, but some implementations allow it
        // rawencode processes mask 0x7C10 in order: val[0]->bit[2], val[1]->bit[3], val[2]->bit[4], val[3]->bit[5], val[4]->bit[6], val[5]->bit[12]
        // For c.lui: imm[17:12] maps to bits [12|6:2]
        // So: imm[12]->bit[2], imm[13]->bit[3], imm[14]->bit[4], imm[15]->bit[5], imm[16]->bit[6], imm[17]->bit[12]
        uint32_t imm12 = (imm_upper >> 0) & 1;
        uint32_t imm13 = (imm_upper >> 1) & 1;
        uint32_t imm14 = (imm_upper >> 2) & 1;
        uint32_t imm15 = (imm_upper >> 3) & 1;
        uint32_t imm16 = (imm_upper >> 4) & 1;
        uint32_t imm17 = (imm_upper >> 5) & 1;
        uint32_t raw = (imm12 << 0) |   // val[0] -> bit [2]
                       (imm13 << 1) |   // val[1] -> bit [3]
                       (imm14 << 2) |   // val[2] -> bit [4]
                       (imm15 << 3) |   // val[3] -> bit [5]
                       (imm16 << 4) |   // val[4] -> bit [6]
                       (imm17 << 5);    // val[5] -> bit [12]
        rawencode(lang,adr,bytes,op,inslen,raw);
    } else {
        // Standard CI format
        if (val < -32 || val > 31) {
            op.goodasm->error("RISC-V CI imm6 is out of range (-32 to 31).");
            return;
        }

        // rawencode packs bits in mask order: raw[4:0]->inst[6:2], raw[5]->inst[12]
        // This matches imm[4:0] and imm[5] directly
        uint32_t raw = val & 0x3F; // imm[5:0] directly

        rawencode(lang,adr,bytes,op,inslen,raw);
    }
}

// RISC-V CI Format Immediate Unsigned (uimm[5:0]) parameter
// Encoding: Same as CI format but unsigned (0-31)
// Used in C.SLLI, C.SRLI, C.SRAI

GAParameterRiscvCIUimm6::GAParameterRiscvCIUimm6(const char* mask){
    setMask(mask);
    prefix = "#";
}

int GAParameterRiscvCIUimm6::match(GAParserOperand *op, int len){
    int64_t val=op->uint64(false); // False on a match

    // must fit unsigned 6-bit immediate (0 to 31)
    if (val < 0 || val > 31) {
        op->goodasm->error("RISC-V CI uimm6 is out of range (0 to 31)");
        return 0;
    }

    return 1; // valid
}

QString GAParameterRiscvCIUimm6::decode(GALanguage *lang, uint64_t adr, const char *bytes, int inslen){
    uint64_t raw = rawdecode(lang,adr,bytes,inslen);
    // rawdecode extracts bits in order: [12] (byte 1 bit 4), then [6:2] (byte 0 bits [6:2])
    // So the result has: [12] at bit 5, [6] at bit 4, [5] at bit 3, [4] at bit 2, [3] at bit 1, [2] at bit 0
    // This gives us: imm[5]|imm[4]|imm[3]|imm[2]|imm[1]|imm[0]
    uint32_t imm5 = (raw >> 5) & 1;   // imm[5] from bit [12] (at position 5 in raw)
    uint32_t imm4 = (raw >> 4) & 1;   // imm[4] from bit [6] (at position 4 in raw)
    uint32_t imm3_0 = raw & 0xF;      // imm[3:0] from bits [5:2] (at positions [3:0] in raw)
    uint32_t imm = (imm5 << 5) | (imm4 << 4) | imm3_0;
    return prefix + QString::number(imm);
}

void GAParameterRiscvCIUimm6::encode(GALanguage *lang,
            uint64_t adr, QByteArray &bytes,
            GAParserOperand op,
            int inslen
            ){
    int64_t val = op.uint64(true);

    if (val < 0 || val > 31) {
        op.goodasm->error("RISC-V CI uimm6 is out of range (0 to 31).");
        return;
    }

    // Encode uimm[5] into bit [12] and uimm[4:0] into bits [6:2]
    // rawencode expects the value in the same order as rawdecode extracts it:
    // - imm[5] at bit position 5 (encodes to [12])
    // - imm[4] at bit position 4 (encodes to [6])
    // - imm[3:0] at bit positions [3:0] (encodes to [5:2])
    uint32_t imm = val & 0x3F; // uimm[5:0]
    uint32_t raw = ((imm >> 5) & 1) << 5 |  // uimm[5] -> position 5 (encodes to bit [12])
                   ((imm >> 4) & 1) << 4 |  // uimm[4] -> position 4 (encodes to bit [6])
                   ((imm >> 0) & 0xF);      // uimm[3:0] -> positions [3:0] (encodes to bits [5:2])

    rawencode(lang,adr,bytes,op,inslen,raw);
}

// RISC-V CI Format Immediate Unsigned (uimm[5:0]) parameter for C.SRAI
// Encoding: Same as CIUimm6 but bit [6] is part of opcode (must be 1)
// Used in C.SRAI only (C.SLLI and C.SRLI use GAParameterRiscvCIUimm6)

GAParameterRiscvCIUimm6Srai::GAParameterRiscvCIUimm6Srai(const char* mask){
    setMask(mask);
    prefix = "#";
}

int GAParameterRiscvCIUimm6Srai::match(GAParserOperand *op, int len){
    int64_t val=op->uint64(false); // False on a match

    // must fit unsigned 6-bit immediate (0 to 31)
    if (val < 0 || val > 31) {
        op->goodasm->error("RISC-V CI uimm6 is out of range (0 to 31)");
        return 0;
    }

    return 1; // valid
}

QString GAParameterRiscvCIUimm6Srai::decode(GALanguage *lang, uint64_t adr, const char *bytes, int inslen){
    // Use the same logic as GAParameterRiscvCIUimm6 - the encoding is the same
    uint64_t raw = rawdecode(lang,adr,bytes,inslen);
    // rawdecode extracts bits in order: [12] (byte 1 bit 4), then [6:2] (byte 0 bits [6:2])
    // So the result has: [12] at bit 5, [6] at bit 4, [5] at bit 3, [4] at bit 2, [3] at bit 1, [2] at bit 0
    uint32_t imm5 = (raw >> 5) & 1;   // imm[5] from bit [12] (at position 5 in raw)
    uint32_t imm4 = (raw >> 4) & 1;   // imm[4] from bit [6] (at position 4 in raw)
    uint32_t imm3_0 = raw & 0xF;      // imm[3:0] from bits [5:2] (at positions [3:0] in raw)
    uint32_t imm = (imm5 << 5) | (imm4 << 4) | imm3_0;
    return prefix + QString::number(imm);
}

void GAParameterRiscvCIUimm6Srai::encode(GALanguage *lang,
            uint64_t adr, QByteArray &bytes,
            GAParserOperand op,
            int inslen
            ){
    int64_t val = op.uint64(true);

    if (val < 0 || val > 31) {
        op.goodasm->error("RISC-V CI uimm6 is out of range (0 to 31).");
        return;
    }

    // Encode uimm[5] into bit [12] and uimm[4:0] into bits [6:2]
    // rawencode expects the value in the same order as rawdecode extracts it:
    // - imm[5] at bit position 5 (encodes to [12])
    // - imm[4] at bit position 4 (encodes to [6])
    // - imm[3:0] at bit positions [3:0] (encodes to [5:2])
    uint32_t imm = val & 0x3F; // uimm[5:0]
    uint32_t raw = ((imm >> 5) & 1) << 5 |  // uimm[5] -> position 5 (encodes to bit [12])
                   ((imm >> 4) & 1) << 4 |  // uimm[4] -> position 4 (encodes to bit [6])
                   ((imm >> 0) & 0xF);      // uimm[3:0] -> positions [3:0] (encodes to bits [5:2])

    // Clear bits [6:5] first since they're part of the opcode pattern but also encode imm[4:3]
    // The mask 0x7C covers bits [6:2], so we need to clear bits [6:5] before encoding
    bytes[0] &= ~0x60;  // Clear bits [6:5] in byte 0
    
    rawencode(lang,adr,bytes,op,inslen,raw);
}

// RISC-V CI Format Immediate (imm[7:2]) parameter for c.lwsp
// Encoding: imm[5] in bit [12], imm[4:2] in bits [6:4], imm[7:6] in bits [3:2]
// Used in C.LWSP

GAParameterRiscvCIimm7::GAParameterRiscvCIimm7(const char* mask){
    setMask(mask);
    prefix = "#";
}

int GAParameterRiscvCIimm7::match(GAParserOperand *op, int len){
    int64_t val=op->uint64(false); // False on a match

    // must fit unsigned 8-bit immediate, 4-byte aligned (imm[1:0] = 0)
    // Range: 0, 4, 8, 12, ..., 252 (must be multiple of 4, 0-252)
    if (val < 0 || val > 252 || (val & 3) != 0) {
        op->goodasm->error("RISC-V CI imm7 is out of range (0 to 252, must be multiple of 4)");
        return 0;
    }

    return 1; // valid
}

QString GAParameterRiscvCIimm7::decode(GALanguage *lang, uint64_t adr, const char *bytes, int inslen){
    uint64_t raw = rawdecode(lang,adr,bytes,inslen);
    // Mask "\x7C\x10" captures instruction bits [6:4], [3:2], [12]
    // rawdecode packs bits sequentially by mask position:
    // raw bit 0 = instruction bit 2 = imm[6]
    // raw bit 1 = instruction bit 3 = imm[7]
    // raw bit 2 = instruction bit 4 = imm[2]
    // raw bit 3 = instruction bit 5 = imm[3]
    // raw bit 4 = instruction bit 6 = imm[4]
    // raw bit 5 = instruction bit 12 = imm[5]
    uint32_t imm6 = (raw >> 0) & 1;
    uint32_t imm7 = (raw >> 1) & 1;
    uint32_t imm2 = (raw >> 2) & 1;
    uint32_t imm3 = (raw >> 3) & 1;
    uint32_t imm4 = (raw >> 4) & 1;
    uint32_t imm5 = (raw >> 5) & 1;
    uint32_t imm = (imm7 << 7) | (imm6 << 6) | (imm5 << 5) | (imm4 << 4) | (imm3 << 3) | (imm2 << 2);
    return prefix + QString::number(imm);
}

void GAParameterRiscvCIimm7::encode(GALanguage *lang,
            uint64_t adr, QByteArray &bytes,
            GAParserOperand op,
            int inslen
            ){
    int64_t val = op.uint64(true);

    if (val < 0 || val > 252 || (val & 3) != 0) {
        op.goodasm->error("RISC-V CI imm7 is out of range (0 to 252, must be multiple of 4).");
        return;
    }

    // Encode imm[7:2] into sequential raw bits (matching mask bit order)
    // raw bit 0 = imm[6]
    // raw bit 1 = imm[7]
    // raw bit 2 = imm[2]
    // raw bit 3 = imm[3]
    // raw bit 4 = imm[4]
    // raw bit 5 = imm[5]
    uint32_t imm = val;
    uint32_t raw = ((imm >> 6) & 1) << 0 |   // imm[6] -> raw bit 0
                   ((imm >> 7) & 1) << 1 |   // imm[7] -> raw bit 1
                   ((imm >> 2) & 1) << 2 |   // imm[2] -> raw bit 2
                   ((imm >> 3) & 1) << 3 |   // imm[3] -> raw bit 3
                   ((imm >> 4) & 1) << 4 |   // imm[4] -> raw bit 4
                   ((imm >> 5) & 1) << 5;    // imm[5] -> raw bit 5

    rawencode(lang,adr,bytes,op,inslen,raw);
}

// RISC-V CIW Format Immediate (imm[9:2]) parameter
// Encoding: imm[9:2] in bits [10:7|12:11|5|6] (split across instruction)
// Used in C.ADDI4SPN

GAParameterRiscvCIWimm9::GAParameterRiscvCIWimm9(const char* mask){
    setMask(mask);
    prefix = "#";
}

int GAParameterRiscvCIWimm9::match(GAParserOperand *op, int len){
    int64_t val=op->uint64(false); // False on a match

    // must fit unsigned 9-bit immediate, 4-byte aligned (imm[1:0] = 0)
    // Range: 0 to 1020 (must be multiple of 4)
    if (val < 0 || val > 1020 || (val & 3) != 0) {
        op->goodasm->error("RISC-V CIW imm9 is out of range (0 to 1020, must be multiple of 4)");
        return 0;
    }

    return 1; // valid
}

QString GAParameterRiscvCIWimm9::decode(GALanguage *lang, uint64_t adr, const char *bytes, int inslen){
    uint64_t raw = rawdecode(lang,adr,bytes,inslen);
    // Extract imm[9:2] from split encoding
    // Mask "\xE0\x1F" captures instruction bits [12:5]
    // rawdecode packs bits sequentially:
    // raw bit 0 = instruction bit 5 = nzuimm[3]
    // raw bit 1 = instruction bit 6 = nzuimm[2]
    // raw bit 2 = instruction bit 7 = nzuimm[6]
    // raw bit 3 = instruction bit 8 = nzuimm[7]
    // raw bit 4 = instruction bit 9 = nzuimm[8]
    // raw bit 5 = instruction bit 10 = nzuimm[9]
    // raw bit 6 = instruction bit 11 = nzuimm[4]
    // raw bit 7 = instruction bit 12 = nzuimm[5]
    uint32_t imm3 = (raw >> 0) & 1;   // raw bit 0
    uint32_t imm2 = (raw >> 1) & 1;   // raw bit 1
    uint32_t imm6 = (raw >> 2) & 1;   // raw bit 2
    uint32_t imm7 = (raw >> 3) & 1;   // raw bit 3
    uint32_t imm8 = (raw >> 4) & 1;   // raw bit 4
    uint32_t imm9 = (raw >> 5) & 1;   // raw bit 5
    uint32_t imm4 = (raw >> 6) & 1;   // raw bit 6
    uint32_t imm5 = (raw >> 7) & 1;   // raw bit 7
    uint32_t imm = (imm9 << 9) | (imm8 << 8) | (imm7 << 7) | (imm6 << 6) |
                   (imm5 << 5) | (imm4 << 4) | (imm3 << 3) | (imm2 << 2);
    return prefix + QString::number(imm);
}

void GAParameterRiscvCIWimm9::encode(GALanguage *lang,
            uint64_t adr, QByteArray &bytes,
            GAParserOperand op,
            int inslen
            ){
    int64_t val = op.uint64(true);

    if (val < 0 || val > 1020 || (val & 3) != 0) {
        op.goodasm->error("RISC-V CIW imm9 is out of range (0 to 1020, must be multiple of 4).");
        return;
    }

    // Encode imm[9:2] into sequential raw bits (matching mask bit order)
    // raw bit 0 = nzuimm[3]
    // raw bit 1 = nzuimm[2]
    // raw bit 2 = nzuimm[6]
    // raw bit 3 = nzuimm[7]
    // raw bit 4 = nzuimm[8]
    // raw bit 5 = nzuimm[9]
    // raw bit 6 = nzuimm[4]
    // raw bit 7 = nzuimm[5]
    uint32_t imm = val;
    uint32_t raw = ((imm >> 3) & 1) << 0 |   // nzuimm[3] -> raw bit 0
                   ((imm >> 2) & 1) << 1 |   // nzuimm[2] -> raw bit 1
                   ((imm >> 6) & 1) << 2 |   // nzuimm[6] -> raw bit 2
                   ((imm >> 7) & 1) << 3 |   // nzuimm[7] -> raw bit 3
                   ((imm >> 8) & 1) << 4 |   // nzuimm[8] -> raw bit 4
                   ((imm >> 9) & 1) << 5 |   // nzuimm[9] -> raw bit 5
                   ((imm >> 4) & 1) << 6 |   // nzuimm[4] -> raw bit 6
                   ((imm >> 5) & 1) << 7;    // nzuimm[5] -> raw bit 7

    rawencode(lang,adr,bytes,op,inslen,raw);
}

// RISC-V CL Format Immediate (imm[5:2]) parameter
// Encoding: imm[5] in bit [12], imm[4:2] in bits [6:5|2]
// Used in C.LW

GAParameterRiscvCLimm5::GAParameterRiscvCLimm5(const char* mask){
    setMask(mask);
    prefix = "#";
}

int GAParameterRiscvCLimm5::match(GAParserOperand *op, int len){
    int64_t val=op->uint64(false); // False on a match

    // must fit unsigned 4-bit immediate, 4-byte aligned (imm[1:0] = 0)
    // Range: 0, 4, 8, 12, ..., 124 (must be multiple of 4, 0-124)
    if (val < 0 || val > 124 || (val & 3) != 0) {
        op->goodasm->error("RISC-V CL imm5 is out of range (0 to 124, must be multiple of 4)");
        return 0;
    }

    return 1; // valid
}

QString GAParameterRiscvCLimm5::decode(GALanguage *lang, uint64_t adr, const char *bytes, int inslen){
    uint64_t raw = rawdecode(lang,adr,bytes,inslen);
    // Mask "\x60\x1C" captures instruction bits [6:5], [12:10]
    // rawdecode packs bits sequentially by mask position:
    // raw bit 0 = instruction bit 5 = imm[6]
    // raw bit 1 = instruction bit 6 = imm[2]
    // raw bit 2 = instruction bit 10 = imm[3]
    // raw bit 3 = instruction bit 11 = imm[4]
    // raw bit 4 = instruction bit 12 = imm[5]
    uint32_t imm6 = (raw >> 0) & 1;
    uint32_t imm2 = (raw >> 1) & 1;
    uint32_t imm3 = (raw >> 2) & 1;
    uint32_t imm4 = (raw >> 3) & 1;
    uint32_t imm5 = (raw >> 4) & 1;
    uint32_t imm = (imm6 << 6) | (imm5 << 5) | (imm4 << 4) | (imm3 << 3) | (imm2 << 2);
    return prefix + QString::number(imm);
}

void GAParameterRiscvCLimm5::encode(GALanguage *lang,
            uint64_t adr, QByteArray &bytes,
            GAParserOperand op,
            int inslen
            ){
    int64_t val = op.uint64(true);

    if (val < 0 || val > 124 || (val & 3) != 0) {
        op.goodasm->error("RISC-V CL imm5 is out of range (0 to 124, must be multiple of 4).");
        return;
    }

    // Encode imm[6:2] into sequential raw bits (matching mask bit order)
    // raw bit 0 = imm[6]
    // raw bit 1 = imm[2]
    // raw bit 2 = imm[3]
    // raw bit 3 = imm[4]
    // raw bit 4 = imm[5]
    uint32_t imm = val;
    uint32_t raw = ((imm >> 6) & 1) << 0 |   // imm[6] -> raw bit 0
                   ((imm >> 2) & 1) << 1 |   // imm[2] -> raw bit 1
                   ((imm >> 3) & 1) << 2 |   // imm[3] -> raw bit 2
                   ((imm >> 4) & 1) << 3 |   // imm[4] -> raw bit 3
                   ((imm >> 5) & 1) << 4;    // imm[5] -> raw bit 4

    rawencode(lang,adr,bytes,op,inslen,raw);
}

// RISC-V CS Format Immediate (imm[5:2]) parameter
// Encoding: Same as CL format
// Used in C.SW

GAParameterRiscvCSimm5::GAParameterRiscvCSimm5(const char* mask){
    setMask(mask);
    prefix = "#";
}

int GAParameterRiscvCSimm5::match(GAParserOperand *op, int len){
    int64_t val=op->uint64(false); // False on a match

    // must fit unsigned 4-bit immediate, 4-byte aligned (imm[1:0] = 0)
    // Range: 0, 4, 8, 12, ..., 124 (must be multiple of 4, 0-124)
    if (val < 0 || val > 124 || (val & 3) != 0) {
        op->goodasm->error("RISC-V CS imm5 is out of range (0 to 124, must be multiple of 4)");
        return 0;
    }

    return 1; // valid
}

QString GAParameterRiscvCSimm5::decode(GALanguage *lang, uint64_t adr, const char *bytes, int inslen){
    uint64_t raw = rawdecode(lang,adr,bytes,inslen);
    // Mask "\x60\x1C" captures instruction bits [6:5], [12:10]
    // rawdecode packs bits sequentially by mask position:
    // raw bit 0 = instruction bit 5 = imm[6]
    // raw bit 1 = instruction bit 6 = imm[2]
    // raw bit 2 = instruction bit 10 = imm[3]
    // raw bit 3 = instruction bit 11 = imm[4]
    // raw bit 4 = instruction bit 12 = imm[5]
    uint32_t imm6 = (raw >> 0) & 1;
    uint32_t imm2 = (raw >> 1) & 1;
    uint32_t imm3 = (raw >> 2) & 1;
    uint32_t imm4 = (raw >> 3) & 1;
    uint32_t imm5 = (raw >> 4) & 1;
    uint32_t imm = (imm6 << 6) | (imm5 << 5) | (imm4 << 4) | (imm3 << 3) | (imm2 << 2);
    return prefix + QString::number(imm);
}

void GAParameterRiscvCSimm5::encode(GALanguage *lang,
            uint64_t adr, QByteArray &bytes,
            GAParserOperand op,
            int inslen
            ){
    int64_t val = op.uint64(true);

    if (val < 0 || val > 124 || (val & 3) != 0) {
        op.goodasm->error("RISC-V CS imm5 is out of range (0 to 124, must be multiple of 4).");
        return;
    }

    // Encode imm[6:2] into sequential raw bits (matching mask bit order)
    // raw bit 0 = imm[6]
    // raw bit 1 = imm[2]
    // raw bit 2 = imm[3]
    // raw bit 3 = imm[4]
    // raw bit 4 = imm[5]
    uint32_t imm = val;
    uint32_t raw = ((imm >> 6) & 1) << 0 |   // imm[6] -> raw bit 0
                   ((imm >> 2) & 1) << 1 |   // imm[2] -> raw bit 1
                   ((imm >> 3) & 1) << 2 |   // imm[3] -> raw bit 2
                   ((imm >> 4) & 1) << 3 |   // imm[4] -> raw bit 3
                   ((imm >> 5) & 1) << 4;    // imm[5] -> raw bit 4

    rawencode(lang,adr,bytes,op,inslen,raw);
}

// RISC-V CSS Format Immediate (imm[7:2]) parameter
// Encoding: imm[7:2] in bits [12:7]
// Used in C.SWSP

GAParameterRiscvCSSimm7::GAParameterRiscvCSSimm7(const char* mask){
    setMask(mask);
    prefix = "#";
}

int GAParameterRiscvCSSimm7::match(GAParserOperand *op, int len){
    int64_t val=op->uint64(false); // False on a match

    // must fit unsigned 6-bit immediate, 4-byte aligned (imm[1:0] = 0)
    // Range: 0, 4, 8, 12, ..., 252 (must be multiple of 4, 0-252)
    if (val < 0 || val > 252 || (val & 3) != 0) {
        op->goodasm->error("RISC-V CSS imm7 is out of range (0 to 252, must be multiple of 4)");
        return 0;
    }

    return 1; // valid
}

QString GAParameterRiscvCSSimm7::decode(GALanguage *lang, uint64_t adr, const char *bytes, int inslen){
    uint64_t raw = rawdecode(lang,adr,bytes,inslen);
    // rawdecode with mask 0x80 0x1F extracts bits [12:7] in the order:
    // Byte 1 (last): bits [12:8] (mask 0x1F = bits [4:0] of byte 1)
    // Byte 0 (first): bit 7 (mask 0x80 = bit 7 of byte 0)
    // rawdecode processes bits from highest byte to lowest, highest bit to lowest bit:
    // Byte 1, bit 4 (bit 12) → raw bit 5 (MSB of imm[7:2])
    // Byte 1, bit 3 (bit 11) → raw bit 4
    // Byte 1, bit 2 (bit 10) → raw bit 3
    // Byte 1, bit 1 (bit 9) → raw bit 2
    // Byte 1, bit 0 (bit 8) → raw bit 1
    // Byte 0, bit 7 (bit 7) → raw bit 0 (LSB of imm[7:2])
    // So raw already contains imm[7:2] in the correct order!
    uint32_t imm = raw << 2; // Shift left by 2 (imm[1:0] are always 0)
    return prefix + QString::number(imm);
}

void GAParameterRiscvCSSimm7::encode(GALanguage *lang,
            uint64_t adr, QByteArray &bytes,
            GAParserOperand op,
            int inslen
            ){
    int64_t val = op.uint64(true);

    if (val < 0 || val > 252 || (val & 3) != 0) {
        op.goodasm->error("RISC-V CSS imm7 is out of range (0 to 252, must be multiple of 4).");
        return;
    }

    // Encode imm[7:2] into bits [12:7]
    // rawencode processes bits from lowest bit to highest, placing them into mask positions
    // in order (highest byte first, highest bit first):
    // raw bit 0 → Byte 0, bit 7 (bit 7) - LSB
    // raw bit 1 → Byte 1, bit 0 (bit 8)
    // raw bit 2 → Byte 1, bit 1 (bit 9)
    // raw bit 3 → Byte 1, bit 2 (bit 10)
    // raw bit 4 → Byte 1, bit 3 (bit 11)
    // raw bit 5 → Byte 1, bit 4 (bit 12) - MSB
    // So raw should be imm[7:2] directly (no rearrangement needed)
    uint32_t imm = (val >> 2) & 0x3F; // imm[7:2]
    rawencode(lang,adr,bytes,op,inslen,imm);
}

// RISC-V CB Format Immediate (imm[8|4:3|7:6|2:1|5]) parameter
// Encoding: Split across instruction bits
// Used in C.BEQZ, C.BNEZ

GAParameterRiscvCBimm9::GAParameterRiscvCBimm9(const char* mask){
    setMask(mask);
    prefix = "#";
}

int GAParameterRiscvCBimm9::match(GAParserOperand *op, int len){
    // Accept any label or integer - range check will happen in encode with PC-relative calculation
    // This allows labels to be matched and resolved during encoding
    return 1; // valid
}

QString GAParameterRiscvCBimm9::decode(GALanguage *lang, uint64_t adr, const char *bytes, int inslen){
    uint64_t raw = rawdecode(lang,adr,bytes,inslen);
    // Mask 0x7C1C: inst[6:2] -> raw[4:0], inst[12:10] -> raw[7:5]
    // CB format immediate mapping in instruction bits:
    // inst[2]=imm[5], inst[3]=imm[1], inst[4]=imm[2], inst[5]=imm[6]
    // inst[6]=imm[7], inst[10]=imm[3], inst[11]=imm[4], inst[12]=imm[8]
    // So in raw:
    // raw[0]=imm[5], raw[1]=imm[1], raw[2]=imm[2], raw[3]=imm[6]
    // raw[4]=imm[7], raw[5]=imm[3], raw[6]=imm[4], raw[7]=imm[8]
    int32_t imm1 = (raw >> 1) & 1;
    int32_t imm2 = (raw >> 2) & 1;
    int32_t imm3 = (raw >> 5) & 1;
    int32_t imm4 = (raw >> 6) & 1;
    int32_t imm5 = raw & 1;
    int32_t imm6 = (raw >> 3) & 1;
    int32_t imm7 = (raw >> 4) & 1;
    int32_t imm8 = (raw >> 7) & 1;
    int32_t imm = (imm8 << 8) | (imm7 << 7) | (imm6 << 6) | (imm5 << 5) |
                  (imm4 << 4) | (imm3 << 3) | (imm2 << 2) | (imm1 << 1);
    // Sign extend from 9 bits
    if (imm & 0x100) {
        imm |= ~0x1FF;
    }
    return prefix + QString::number(imm);
}

void GAParameterRiscvCBimm9::encode(GALanguage *lang,
            uint64_t adr, QByteArray &bytes,
            GAParserOperand op,
            int inslen
            ){
    // PC-relative addressing: calculate offset from current address
    int64_t target = op.int64(true);
    int64_t val = target - adr; // PC-relative offset

    // Range check: -256 to 254 bytes (signed 9-bit, even addresses only)
    if (val < -256 || val > 254) {
        op.goodasm->error("RISC-V CB imm9 is out of range (-256 to 254 bytes from PC).");
        return;
    }
    if ((val & 1) != 0) {
        op.goodasm->error("RISC-V CB imm9 must be even (target address must be 2-byte aligned).");
        return;
    }

    // Pack into raw in mask order:
    // raw[0]=imm[5], raw[1]=imm[1], raw[2]=imm[2], raw[3]=imm[6]
    // raw[4]=imm[7], raw[5]=imm[3], raw[6]=imm[4], raw[7]=imm[8]
    uint32_t imm = val & 0x1FE; // imm[8:1], clear imm[0]
    uint32_t raw = (((imm >> 5) & 1) << 0) |  // imm[5] -> raw[0]
                   (((imm >> 1) & 1) << 1) |  // imm[1] -> raw[1]
                   (((imm >> 2) & 1) << 2) |  // imm[2] -> raw[2]
                   (((imm >> 6) & 1) << 3) |  // imm[6] -> raw[3]
                   (((imm >> 7) & 1) << 4) |  // imm[7] -> raw[4]
                   (((imm >> 3) & 1) << 5) |  // imm[3] -> raw[5]
                   (((imm >> 4) & 1) << 6) |  // imm[4] -> raw[6]
                   (((imm >> 8) & 1) << 7);   // imm[8] -> raw[7]

    rawencode(lang,adr,bytes,op,inslen,raw);
}

// RISC-V CJ Format Immediate (imm[11|4|9:8|10|6|7|3:1|5]) parameter
// Encoding: Split across instruction bits
// Used in C.J

GAParameterRiscvCJimm12::GAParameterRiscvCJimm12(const char* mask){
    setMask(mask);
    prefix = "#";
}

int GAParameterRiscvCJimm12::match(GAParserOperand *op, int len){
    // Accept any label or integer - range check will happen in encode with PC-relative calculation
    // This allows labels to be matched and resolved during encoding
    return 1; // valid
}

QString GAParameterRiscvCJimm12::decode(GALanguage *lang, uint64_t adr, const char *bytes, int inslen){
    uint64_t raw = rawdecode(lang,adr,bytes,inslen);
    // Mask 0xFC1F: inst[7:2] -> raw[5:0], inst[12:8] -> raw[10:6]
    // CJ format immediate mapping in instruction bits:
    // inst[2]=imm[5], inst[3]=imm[1], inst[4]=imm[2], inst[5]=imm[3]
    // inst[6]=imm[7], inst[7]=imm[6], inst[8]=imm[10], inst[9]=imm[9]
    // inst[10]=imm[8], inst[11]=imm[4], inst[12]=imm[11]
    // So in raw:
    // raw[0]=imm[5], raw[1]=imm[1], raw[2]=imm[2], raw[3]=imm[3]
    // raw[4]=imm[7], raw[5]=imm[6], raw[6]=imm[10], raw[7]=imm[9]
    // raw[8]=imm[8], raw[9]=imm[4], raw[10]=imm[11]
    int32_t imm1 = (raw >> 1) & 1;
    int32_t imm2 = (raw >> 2) & 1;
    int32_t imm3 = (raw >> 3) & 1;
    int32_t imm4 = (raw >> 9) & 1;
    int32_t imm5 = raw & 1;
    int32_t imm6 = (raw >> 5) & 1;
    int32_t imm7 = (raw >> 4) & 1;
    int32_t imm8 = (raw >> 8) & 1;
    int32_t imm9 = (raw >> 7) & 1;
    int32_t imm10 = (raw >> 6) & 1;
    int32_t imm11 = (raw >> 10) & 1;
    int32_t imm = (imm11 << 11) | (imm10 << 10) | (imm9 << 9) | (imm8 << 8) |
                  (imm7 << 7) | (imm6 << 6) | (imm5 << 5) | (imm4 << 4) |
                  (imm3 << 3) | (imm2 << 2) | (imm1 << 1);
    // Sign extend from 12 bits
    if (imm & 0x800) {
        imm |= ~0xFFF;
    }
    return prefix + QString::number(imm);
}

void GAParameterRiscvCJimm12::encode(GALanguage *lang,
            uint64_t adr, QByteArray &bytes,
            GAParserOperand op,
            int inslen
            ){
    // PC-relative addressing: calculate offset from current address
    int64_t target = op.int64(true);
    int64_t val = target - adr; // PC-relative offset

    // Range check: -2048 to 2046 bytes (signed 12-bit, even addresses only)
    if (val < -2048 || val > 2046) {
        op.goodasm->error("RISC-V CJ imm12 is out of range (-2048 to 2046 bytes from PC).");
        return;
    }
    if ((val & 1) != 0) {
        op.goodasm->error("RISC-V CJ imm12 must be even (target address must be 2-byte aligned).");
        return;
    }

    // Pack into raw in mask order:
    // raw[0]=imm[5], raw[1]=imm[1], raw[2]=imm[2], raw[3]=imm[3]
    // raw[4]=imm[7], raw[5]=imm[6], raw[6]=imm[10], raw[7]=imm[9]
    // raw[8]=imm[8], raw[9]=imm[4], raw[10]=imm[11]
    uint32_t imm = val & 0xFFE; // imm[11:1], clear imm[0]
    uint32_t raw = (((imm >> 5) & 1) << 0) |   // imm[5] -> raw[0]
                   (((imm >> 1) & 1) << 1) |   // imm[1] -> raw[1]
                   (((imm >> 2) & 1) << 2) |   // imm[2] -> raw[2]
                   (((imm >> 3) & 1) << 3) |   // imm[3] -> raw[3]
                   (((imm >> 7) & 1) << 4) |   // imm[7] -> raw[4]
                   (((imm >> 6) & 1) << 5) |   // imm[6] -> raw[5]
                   (((imm >> 10) & 1) << 6) |  // imm[10] -> raw[6]
                   (((imm >> 9) & 1) << 7) |   // imm[9] -> raw[7]
                   (((imm >> 8) & 1) << 8) |   // imm[8] -> raw[8]
                   (((imm >> 4) & 1) << 9) |   // imm[4] -> raw[9]
                   (((imm >> 11) & 1) << 10);  // imm[11] -> raw[10]

    rawencode(lang,adr,bytes,op,inslen,raw);
}

// RISC-V CB Format Immediate for C.ANDI (imm[5:0]) parameter
// Encoding: Standard CB format: imm[5] in bit [12], imm[4:0] in bits [6:2]
// - imm[5] in bit [12]
// - imm[4:0] in bits [6:2]
// Note: bits [6:5] are part of the immediate encoding, not the opcode pattern
// Range: -32 to 31 (6-bit signed)

GAParameterRiscvCBimm6Andi::GAParameterRiscvCBimm6Andi(const char* mask){
    setMask(mask);
    prefix = "#";
}

int GAParameterRiscvCBimm6Andi::match(GAParserOperand *op, int len){
    int64_t val = op->int64(false); // False on a match

    // must fit signed 6-bit immediate (-32 to 31)
    if (val < -32 || val > 31) {
        op->goodasm->error("RISC-V CB imm6 (c.andi) is out of range (-32 to 31)");
        return 0;
    }

    return 1; // valid
}

QString GAParameterRiscvCBimm6Andi::decode(GALanguage *lang, uint64_t adr, const char *bytes, int inslen){
    uint64_t raw = rawdecode(lang,adr,bytes,inslen);
    
    // rawdecode extracts bits according to mask 0x7C10:
    // - bits [6:2] from byte 0 → bits 0-4 of raw = imm[4:0]
    // - bit [12] from byte 1 → bit 5 of raw = imm[5]
    // So raw already has: imm[4:0] in bits 0-4, imm[5] in bit 5
    uint32_t imm5 = (raw >> 5) & 1;      // bit 5 of raw = imm[5]
    uint32_t imm4_0 = raw & 0x1F;        // bits 0-4 of raw = imm[4:0]
    
    // Reconstruct imm[5:0] = imm[5] << 5 | imm[4:0]
    uint32_t imm = (imm5 << 5) | imm4_0;
    
    // Sign extend from 6 bits
    if (imm & 0x20) {
        imm |= ~0x3F;
    }
    int32_t signed_imm = (int32_t)imm;
    return prefix + QString::number(signed_imm);
}

void GAParameterRiscvCBimm6Andi::encode(GALanguage *lang,
            uint64_t adr, QByteArray &bytes,
            GAParserOperand op,
            int inslen
            ){
    int64_t val = op.int64(true);
    
    if (val < -32 || val > 31) {
        op.goodasm->error("RISC-V CB imm6 (c.andi) is out of range (-32 to 31).");
        return;
    }

    // Convert to 6-bit signed value
    int32_t imm_signed = val & 0x3F;
    if (imm_signed & 0x20) {
        imm_signed |= ~0x3F; // Sign extend
    }
    
    // Extract imm[5] and imm[4:0] as unsigned for encoding
    // Mask 0x7C10 encodes: imm[4:0] in bits [6:2], imm[5] in bit [12]
    // rawencode processes mask bits in order: first bits [6:2] (5 bits), then bit [12] (1 bit)
    // So the raw value should have: imm[4:0] in bits 0-4, imm[5] in bit 5
    uint32_t imm = (uint32_t)(imm_signed & 0x3F);  // Get 6-bit value as unsigned
    uint32_t imm5 = (imm >> 5) & 1;
    uint32_t imm4_0 = imm & 0x1F;
    
    // rawencode processes mask 0x7C10 in order:
    // - Byte 0 bits [6:2] (5 bits) = imm[4:0]
    // - Byte 1 bit [12] (1 bit) = imm[5]
    // So raw value: imm[4:0] in lower 5 bits, imm[5] in bit 5
    uint32_t raw = imm4_0 | (imm5 << 5);
    
    rawencode(lang,adr,bytes,op,inslen,raw);
}
