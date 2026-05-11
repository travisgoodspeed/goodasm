.lang riscv32
.org 0x0

; Edge case tests for c.lui instruction
; c.lui: rd = imm << 12, where imm[17:12] is in range 1-63 (imm[17:12]=0 is reserved for c.addi16sp)

; Test minimum immediate value (imm[17:12] = 1, so imm = 0x1000)
c.lui a0, #0x1000

; Test various immediate values
c.lui a0, #0x2000
c.lui a0, #0x3000
c.lui a0, #0x4000
c.lui a0, #0x5000
c.lui a0, #0x6000
c.lui a0, #0x7000
c.lui a0, #0x8000
c.lui a0, #0x9000
c.lui a0, #0xA000
c.lui a0, #0xB000
c.lui a0, #0xC000
c.lui a0, #0xD000
c.lui a0, #0xE000
c.lui a0, #0xF000

; Test maximum positive immediate value (imm[17:12] = 31, so imm = 0x1F000)
c.lui a0, #0x1F000

; Test with different registers (rd != 0 and rd != 2)
c.lui a1, #0x10000
c.lui a2, #0x10000
c.lui a3, #0x10000
c.lui a4, #0x10000
c.lui a5, #0x10000
c.lui a6, #0x10000
c.lui a7, #0x10000
c.lui t0, #0x10000
c.lui t1, #0x10000
c.lui t2, #0x10000
c.lui s0, #0x10000
c.lui s1, #0x10000

; Test with various immediate values and different registers
; Note: c.lui uses 6-bit signed imm[17:12], so max positive is 0x1F000, not 0x20000+
c.lui a0, #0x1F000
c.lui a1, #0x1E000
c.lui a2, #0x1D000
c.lui a3, #0x1000
c.lui a4, #0x2000
c.lui a5, #0x1F000

