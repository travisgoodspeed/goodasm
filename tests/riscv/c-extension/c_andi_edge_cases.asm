.lang riscv32
.org 0x0

; Edge cases for c.andi: signed 6-bit immediate range (-32 to 31)
; Testing minimum, maximum, zero, and various values

; Minimum value
c.andi a0, #-32

; Maximum value
c.andi a1, #31

; Zero
c.andi a2, #0

; Negative edge cases
c.andi a3, #-1
c.andi a4, #-16
c.andi a5, #-8
c.andi s0, #-4
c.andi s1, #-2

; Positive edge cases
c.andi s0, #1
c.andi s1, #2
c.andi a0, #4
c.andi a1, #8
c.andi a2, #16
c.andi a3, #15
c.andi a4, #30

; Testing different compressed registers (x8-x15: s0, s1, a0-a5)
c.andi a0, #-32
c.andi a1, #-32
c.andi a2, #-32
c.andi a3, #-32
c.andi a4, #-32
c.andi a5, #-32
c.andi s0, #-32
c.andi s1, #-32

c.andi a0, #31
c.andi a1, #31
c.andi a2, #31
c.andi a3, #31
c.andi a4, #31
c.andi a5, #31
c.andi s0, #31
c.andi s1, #31

