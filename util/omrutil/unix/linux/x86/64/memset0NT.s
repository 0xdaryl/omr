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
	cmpl	$0, %esi
	je	.L10
.L64byte:
	movq	%rdi, %rax
	andl	$63, %eax
	testq	%rax, %rax
	jne	.L3
	cmpl	$63, %esi
	jle	.L3
	vmovntpd %zmm0, (%rdi)
	addq	$64, %rdi
	subl	$64, %esi
	jmp	.Lwhile
.L32byte:
.L3:
	movq	%rdi, %rax
	andl	$31, %eax
	testq	%rax, %rax
	jne	.L5
	cmpl	$31, %esi
	jle	.L5
	vmovntpd %ymm0, (%rdi)
	addq	$32, %rdi
	subl	$32, %esi
	jmp	.Lwhile
.L16byte:
.L5:
	movq	%rdi, %rax
	andl	$15, %eax
	testq	%rax, %rax
	jne	.L6
	cmpl	$15, %esi
	jle	.L6
	vmovntpd %xmm0, (%rdi)
	addq	$16, %rdi
	subl	$16, %esi
	jmp	.Lwhile
.L8byte:
.L6:
	movq	%rdi, %rax
	andl	$7, %eax
	testq	%rax, %rax
	jne	.L7
	cmpl	$7, %esi
	jle	.L7
	movnti	%rcx, (%rdi)
	addq	$8, %rdi
	subl	$8, %esi
	jmp	.Lwhile
.L4byte:
.L7:
	cmpl	$4, %esi
	jne	.L8
	addq	$4, %rdi
	subl	$4, %esi
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
