.lang riscv32
.org 0x0

; c.lui uses 6-bit signed immediate nzimm[17:12]
; Valid positive range: 0x1000 to 0x1F000 (imm = 1 to 31)
; Valid negative range: 0xFFFE0000 to 0xFFFFF000 (imm = -32 to -1)

c.lui gp, #0x1000
c.lui gp, #0x2000
c.lui gp, #0x4000
c.lui gp, #0x8000
c.lui gp, #0xf000
c.lui gp, #0x1f000
c.lui tp, #0x1000
c.lui tp, #0x2000
c.lui tp, #0x4000
c.lui tp, #0x8000
c.lui tp, #0xf000
c.lui tp, #0x1f000
c.lui t0, #0x1000
c.lui t0, #0x2000
c.lui t0, #0x4000
c.lui t0, #0x8000
c.lui t0, #0xf000
c.lui t0, #0x1f000
c.lui t1, #0x1000
c.lui t1, #0x2000
c.lui t1, #0x4000
c.lui t1, #0x8000
c.lui t1, #0xf000
c.lui t1, #0x1f000
c.lui t2, #0x1000
c.lui t2, #0x2000
c.lui t2, #0x4000
c.lui t2, #0x8000
c.lui t2, #0xf000
c.lui t2, #0x1f000
c.lui t3, #0x1000
c.lui t3, #0x2000
c.lui t3, #0x4000
c.lui t3, #0x8000
c.lui t3, #0xf000
c.lui t3, #0x1f000
c.lui t4, #0x1000
c.lui t4, #0x2000
c.lui t4, #0x4000
c.lui t4, #0x8000
c.lui t4, #0xf000
c.lui t4, #0x1f000
c.lui t5, #0x1000
c.lui t5, #0x2000
c.lui t5, #0x4000
c.lui t5, #0x8000
c.lui t5, #0xf000
c.lui t5, #0x1f000
c.lui t6, #0x1000
c.lui t6, #0x2000
c.lui t6, #0x4000
c.lui t6, #0x8000
c.lui t6, #0xf000
c.lui t6, #0x1f000

