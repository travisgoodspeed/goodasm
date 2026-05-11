.lang riscv32
.org 0x0

; Edge cases for c.mv: Move register to register
; Testing all register combinations, including edge cases

; Basic test - already in c_mv.asm
c.mv a0, a1

; Test all compressed register combinations (x8-x15: s0, s1, a0-a5)
; These are the only registers that c.mv can use (according to RISC-V spec, c.mv uses regular 5-bit register encoding)

; Testing all destination registers with a1 as source
; Note: c.mv with rd=0 is reserved (use c.jr instead), so we skip zero
c.mv ra, a1
c.mv sp, a1
c.mv gp, a1
c.mv tp, a1
c.mv t0, a1
c.mv t1, a1
c.mv t2, a1
c.mv s0, a1
c.mv s1, a1
c.mv a0, a1
c.mv a1, a1    ; Move to itself (legal but effectively nop)
c.mv a2, a1
c.mv a3, a1
c.mv a4, a1
c.mv a5, a1

; Testing all source registers with a0 as destination
; Note: c.mv with rs2=zero is invalid (reserved encoding - use c.jr instead)
c.mv a0, ra
c.mv a0, sp
c.mv a0, gp
c.mv a0, tp
c.mv a0, t0
c.mv a0, t1
c.mv a0, t2
c.mv a0, s0
c.mv a0, s1
c.mv a0, a0    ; Move to itself (legal but effectively nop)
c.mv a0, a1
c.mv a0, a2
c.mv a0, a3
c.mv a0, a4
c.mv a0, a5

; Test all argument registers (a0-a7) combinations
c.mv a0, a1
c.mv a0, a2
c.mv a0, a3
c.mv a0, a4
c.mv a0, a5
c.mv a1, a0
c.mv a1, a2
c.mv a1, a3
c.mv a1, a4
c.mv a1, a5
c.mv a2, a0
c.mv a2, a1
c.mv a2, a3
c.mv a2, a4
c.mv a2, a5
c.mv a3, a0
c.mv a3, a1
c.mv a3, a2
c.mv a3, a4
c.mv a3, a5
c.mv a4, a0
c.mv a4, a1
c.mv a4, a2
c.mv a4, a3
c.mv a4, a5
c.mv a5, a0
c.mv a5, a1
c.mv a5, a2
c.mv a5, a3
c.mv a5, a4

; Test saved registers (s0, s1) combinations
c.mv s0, s1
c.mv s1, s0
c.mv s0, a0
c.mv s0, a1
c.mv s1, a0
c.mv s1, a1
c.mv a0, s0
c.mv a0, s1
c.mv a1, s0
c.mv a1, s1

; Test temporary registers (t0-t2) combinations
c.mv t0, t1
c.mv t0, t2
c.mv t1, t0
c.mv t1, t2
c.mv t2, t0
c.mv t2, t1
c.mv a0, t0
c.mv a0, t1
c.mv a0, t2
c.mv t0, a0
c.mv t1, a0
c.mv t2, a0

; Test special registers as source
; Note: c.mv with rs2=zero is invalid (reserved encoding - use c.jr or c.add with zero instead)
c.mv a0, ra     ; Move return address
c.mv a0, sp     ; Move stack pointer
c.mv a0, gp     ; Move global pointer
c.mv a0, tp     ; Move thread pointer
; Note: c.mv with zero as destination (rd=0) is reserved - use c.jr instead
c.mv ra, a0     ; Move to return address
c.mv sp, a0     ; Move to stack pointer
c.mv gp, a0     ; Move to global pointer
c.mv tp, a0     ; Move to thread pointer

; Note: According to RISC-V spec, c.mv with rd=0 is reserved and should be encoded as c.jr instead.
; Therefore, we don't test c.mv with zero as destination here - that's tested in c_jr.asm
