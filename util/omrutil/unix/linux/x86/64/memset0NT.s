	.file	"align.c"
	.text
	.globl	memset0NT
	.type	memset0NT, @function
memset0NT:
.LFB0:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6

#	int3

	vpxorq	%zmm0, %zmm0, %zmm0
	xor	%ecx, %ecx
.Lwhile:
	testq	%rsi, %rsi
	je	.L10
.L64byte:
	movq	%rdi, %rax
	andl	$63, %eax
	testq	%rax, %rax
	jne	.L3
	cmpq	$63, %rsi
	jle	.L3
	vmovntpd %zmm0, (%rdi)
	addq	$64, %rdi
	subq	$64, %rsi
	jmp	.Lwhile
.L32byte:
.L3:
	movq	%rdi, %rax
	andl	$31, %eax
	testq	%rax, %rax
	jne	.L5
	cmpq	$31, %rsi
	jle	.L5
	vmovntpd %ymm0, (%rdi)
	addq	$32, %rdi
	subq	$32, %rsi
	jmp	.Lwhile
.L16byte:
.L5:
	movq	%rdi, %rax
	andl	$15, %eax
	testq	%rax, %rax
	jne	.L6
	cmpq	$15, %rsi
	jle	.L6
	vmovntpd %xmm0, (%rdi)
	addq	$16, %rdi
	subq	$16, %rsi
	jmp	.Lwhile
.L8byte:
.L6:
	movq	%rdi, %rax
	andl	$7, %eax
	testq	%rax, %rax
	jne	.L7
	cmpq	$7, %rsi
	jle	.L7
	movnti	%rcx, (%rdi)
	addq	$8, %rdi
	subq	$8, %rsi
	jmp	.Lwhile
.L4byte:
.L7:
	cmpq	$4, %rsi
	jne	.L8
	addq	$4, %rdi
	subq	$4, %rsi
	movnti	%ecx, (%rdi)
	jmp	.Lwhile
.L8:
	int3
	jmp	.Lwhile
.L10:
	nop
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	memset0NT, .-memset0NT
	.ident	"GCC: (Ubuntu 9.4.0-1ubuntu1~20.04.2) 9.4.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	 1f - 0f
	.long	 4f - 1f
	.long	 5
0:
	.string	 "GNU"
1:
	.align 8
	.long	 0xc0000002
	.long	 3f - 2f
2:
	.long	 0x3
3:
	.align 8
4:
