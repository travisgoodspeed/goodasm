.lang riscv32
.org 0x0

; Edge case tests for c.jalr
; c.jalr jumps to address in rs1 and stores return address in ra (x1)

; Test various rs1 registers
c.jalr a0
c.jalr a1
c.jalr a2
c.jalr a3
c.jalr a4
c.jalr a5
c.jalr a6
c.jalr a7
c.jalr s0
c.jalr s1
c.jalr t0
c.jalr t1
c.jalr t2
c.jalr ra
c.jalr sp
c.jalr gp
c.jalr tp

; Note: c.jalr with rs1=zero (x0) is reserved and should encode as c.ebreak
; This is handled by the assembler/encoder, not by the instruction definition

