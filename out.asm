# MIPS gerado automaticamente (RARS/MARS friendly)
.data
tmp_space: .space 1024
str_0: .asciiz "maior!"
str_1: .asciiz "menor ou igual!"
var_y: .word 0
var_x: .word 0
.text
.globl main
main:
la $t9, tmp_space
li $t0, 0
sw $t0, var_x
lw $t0, var_x
sw $t0, 16($t9)
li $t0, 2
lw $t1, 16($t9)
addu $t0, $t1, $t0
sw $t0, var_y
lw $t0, var_y
sw $t0, 12($t9)
li $t0, 10
lw $t1, 12($t9)
slt $t2, $t0, $t1
move $t0, $t2
beq $t0, $zero, else_0
la $a0, str_0
li $v0, 4
syscall
j endif_1
else_0:
la $a0, str_1
li $v0, 4
syscall
endif_1:
li $v0, 10
syscall
