.lang riscv32
.org 0x0

; Edge cases for c.beqz: testing different compressed registers and branch distances

; Test all compressed registers (x8-x15: s0, s1, a0-a5)
c.beqz a0, label1
c.beqz a1, label2
c.beqz a2, label3
c.beqz a3, label4
c.beqz a4, label5
c.beqz a5, label6
c.beqz s0, label7
c.beqz s1, label8

; Forward branches of different distances
label1:
c.beqz a0, label2
label2:
c.beqz a1, label3
label3:
c.beqz a2, label4
label4:
c.beqz a3, label5
label5:
c.beqz a4, label6
label6:
c.beqz a5, label7
label7:
c.beqz s0, label8
label8:
c.beqz s1, label9
label9:

; Backward branches
c.beqz a0, label1
c.beqz a1, label2
c.beqz a2, label3
c.beqz a3, label4
c.beqz a4, label5
c.beqz a5, label6
c.beqz s0, label7
c.beqz s1, label8

; Multiple branches to same label
c.beqz a0, common_label
c.beqz a1, common_label
c.beqz a2, common_label
common_label:

; Nested branches
c.beqz a0, outer_label
outer_label:
c.beqz a1, inner_label
inner_label:

