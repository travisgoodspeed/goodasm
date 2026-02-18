.lang riscv32
.org 0x0

; FSQRT.S - Floating-point square root (single precision)
; Format: fsqrt.s rd, rs1
; Computes sqrt(rs1) and stores result in rd

; Same register (rd == rs1)
fsqrt.s f0, f0
fsqrt.s f1, f1
fsqrt.s f5, f5
fsqrt.s f10, f10
fsqrt.s f15, f15
fsqrt.s f20, f20
fsqrt.s f25, f25
fsqrt.s f31, f31

; Different registers - various rd values with f0 as source
fsqrt.s f1, f0
fsqrt.s f5, f0
fsqrt.s f10, f0
fsqrt.s f15, f0
fsqrt.s f20, f0
fsqrt.s f25, f0
fsqrt.s f31, f0

; Different registers - various rd values with f1 as source
fsqrt.s f0, f1
fsqrt.s f5, f1
fsqrt.s f10, f1
fsqrt.s f15, f1
fsqrt.s f20, f1
fsqrt.s f25, f1
fsqrt.s f31, f1

; Different registers - various rd values with f10 as source
fsqrt.s f0, f10
fsqrt.s f1, f10
fsqrt.s f5, f10
fsqrt.s f15, f10
fsqrt.s f20, f10
fsqrt.s f25, f10
fsqrt.s f31, f10

; Different registers - various rd values with f31 as source
fsqrt.s f0, f31
fsqrt.s f1, f31
fsqrt.s f5, f31
fsqrt.s f10, f31
fsqrt.s f15, f31
fsqrt.s f20, f31
fsqrt.s f25, f31

; Edge cases - adjacent registers
fsqrt.s f0, f1
fsqrt.s f1, f2
fsqrt.s f2, f3
fsqrt.s f30, f31

; Mixed register combinations
fsqrt.s f2, f29
fsqrt.s f3, f28
fsqrt.s f4, f27
fsqrt.s f12, f19
fsqrt.s f16, f8
