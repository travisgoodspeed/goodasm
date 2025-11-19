.lang riscv32
.org 0x0

; Basic test cases (from original failing tests - now FIXED)
c.swsp a0, #4
c.swsp a2, #12

; Edge cases: minimum and maximum immediate values
c.swsp t6, #0
c.swsp t1, #252

; Edge cases: various immediate values (must be multiples of 4)
c.swsp a1, #8
c.swsp a3, #16
c.swsp a4, #20
c.swsp a5, #24
c.swsp a6, #32
c.swsp a7, #64
c.swsp s0, #128
c.swsp s1, #248

; Edge cases: different register types
c.swsp ra, #4
c.swsp sp, #8
c.swsp gp, #12
c.swsp tp, #16
c.swsp t0, #20
c.swsp t2, #28
c.swsp s2, #36
c.swsp s3, #40
c.swsp s4, #44
c.swsp s5, #48
c.swsp s6, #52
c.swsp s7, #56
c.swsp s8, #60
c.swsp s9, #68
c.swsp s10, #72
c.swsp s11, #76
c.swsp t3, #80
c.swsp t4, #84
c.swsp t5, #88

; Edge cases: boundary immediate values and zero register
c.swsp zero, #0
c.swsp a0, #4
c.swsp a1, #252

