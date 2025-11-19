.lang riscv32
.org 0x0

; I-type instruction tests
addi a0, a0, #1
slti a1, a2, #10
sltiu a3, a4, #20
xori a5, a6, #0xFF
ori a7, t0, #0xAA
andi t1, t2, #0xFF

; I-type shift immediate instruction tests
slli a0, a1, #0
slli a2, a3, #5
slli a4, a5, #31
srli a6, a7, #0
srli t0, t1, #10
srli t2, t3, #31
srai t4, t5, #0
srai t6, a0, #15
srai a1, a2, #31

; R-type instruction tests
add a0, a1, a2
sub a0, a1, a2
sll a0, a1, a2
slt a0, a1, a2
sltu a0, a1, a2
xor a0, a1, a2
srl a0, a1, a2
sra a0, a1, a2
or a0, a1, a2
and a0, a1, a2

; Control Transfer Instructions
; U-type instructions
lui a0, #0x12345
auipc a1, #0x67890

; J-type instructions
jal ra, label1
label1:

; I-type control transfer instructions
jalr ra, a1, #0
jalr a0, a2, #4

; B-type branch instructions
beq a0, a1, label2
bne a0, a1, label3
blt a0, a1, label4
bge a0, a1, label5
bltu a0, a1, label6
bgeu a0, a1, label7
label2:
label3:
label4:
label5:
label6:
label7:

; Load Instructions (I-type)
lb a0, (#4, a1)
lh a2, (#8, a3)
lw a4, (#12, a5)
lbu a6, (#16, a7)
lhu t0, (#20, t1)

; Store Instructions (S-type)
sb a0, (#4, a1)
sh a2, (#8, a3)
sw a4, (#12, a5)

; System Instructions
ecall
ebreak
fence.i
fence #0xFF, zero, zero

; M Extension Instructions (Multiplication and Division)
; Multiplication instructions
mul a0, a1, a2
mulh a3, a4, a5
mulhsu a6, a7, t0
mulhu t1, t2, t3

; Division instructions
div a0, a1, a2
divu a3, a4, a5

; Remainder instructions
rem a6, a7, t0
remu t1, t2, t3

; A Extension Instructions (Atomic Memory Operations)
; Load-Reserved and Store-Conditional
lr.w a0, (a1)
sc.w a2, a3, (a4)

; Atomic Memory Operations
amoswap.w a5, a6, (a7)
amoadd.w t0, t1, (t2)
amoxor.w t3, t4, (t5)
amoand.w t6, a0, (a1)
amoor.w a2, a3, (a4)
amomin.w a5, a6, (a7)
amomax.w t0, t1, (t2)
amominu.w t3, t4, (t5)
amomaxu.w t6, a0, (a1)

; ZICSR Extension Instructions (CSR Operations)
; Register-based CSR instructions
csrrw a0, #0x300, a1
csrrs a2, #0x301, a3
csrrc a4, #0x302, a5

; Immediate-based CSR instructions
csrrwi a6, #0x300, #0
csrrsi a7, #0x301, #15
csrrci t0, #0x302, #31

; F Extension Instructions (Single-Precision Floating-Point)
; Load/Store instructions
flw f0, (#4, a1)
flw f1, (#8, a2)
fsw f0, (#4, a1)
fsw f1, (#8, a2)

; Fused multiply-add instructions
fmadd.s f0, f1, f2, f3
fmsub.s f4, f5, f6, f7
fnmsub.s f8, f9, f10, f11
fnmadd.s f12, f13, f14, f15

; Arithmetic instructions
fadd.s f0, f1, f2
fsub.s f3, f4, f5
fmul.s f6, f7, f8
fdiv.s f9, f10, f11
fsqrt.s f12, f13

; Sign injection instructions
fsgnj.s f0, f1, f2
fsgnjn.s f3, f4, f5
fsgnjx.s f6, f7, f8

; Min/Max instructions
fmin.s f0, f1, f2
fmax.s f3, f4, f5

; Comparison instructions
feq.s a0, f1, f2
flt.s a1, f3, f4
fle.s a2, f5, f6

; Conversion instructions
fcvt.w.s a0, f1
fcvt.wu.s a1, f2
fcvt.s.w f0, a1
fcvt.s.wu f1, a2

; Move instructions
fmv.x.w a0, f1
fmv.w.x f0, a1

; Classify instruction
fclass.s a0, f1

; D Extension Instructions (Double-Precision Floating-Point)
; Load/Store instructions
fld f0, (#8, a1)
fld f1, (#16, a2)
fsd f0, (#8, a1)
fsd f1, (#16, a2)

; Fused multiply-add instructions
fmadd.d f0, f1, f2, f3
fmsub.d f4, f5, f6, f7
fnmsub.d f8, f9, f10, f11
fnmadd.d f12, f13, f14, f15

; Arithmetic instructions
fadd.d f0, f1, f2
fsub.d f3, f4, f5
fmul.d f6, f7, f8
fdiv.d f9, f10, f11
fsqrt.d f12, f13

; Sign injection instructions
fsgnj.d f0, f1, f2
fsgnjn.d f3, f4, f5
fsgnjx.d f6, f7, f8

; Min/Max instructions
fmin.d f0, f1, f2
fmax.d f3, f4, f5

; Comparison instructions
feq.d a0, f1, f2
flt.d a1, f3, f4
fle.d a2, f5, f6

; Conversion instructions
fcvt.s.d f0, f1
fcvt.d.s f1, f2
fcvt.w.d a0, f1
fcvt.wu.d a1, f2
fcvt.d.w f0, a1
fcvt.d.wu f1, a2

; Classify instruction
fclass.d a0, f1

; C Extension Instructions (Compressed 16-bit)
; Quadrant 0 instructions (rd' must be a0-a7 or s0-s1, which are x8-x15)
c.addi4spn a0, #16
c.lw a1, (#4, a2)
c.sw a3, (#4, a4)

; Quadrant 1 instructions
c.nop
c.addi a5, #5
c.li a0, #10
c.addi16sp #-32
c.lui a1, #1

; Quadrant 2 instructions (register-based) - rd' and rs2' must be compressed registers (x8-x15)
c.srli a0, #3
c.srai a1, #5
c.andi a2, #0x1F
c.sub a3, a4
c.xor a5, a0
c.or a1, a2
c.and a3, a4

; Control flow instructions
c.j label_c1
label_c1:

c.beqz a0, label_c2
label_c2:

c.bnez a1, label_c3
label_c3:

; Special instructions
c.slli a2, #2
c.lwsp a3, #8
c.jr a4
c.mv a5, a0
c.ebreak
c.jalr a6
c.add a0, a1
c.swsp a2, #12
