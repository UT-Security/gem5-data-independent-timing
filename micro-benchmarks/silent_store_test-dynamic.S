
silent_store_test-dynamic:     file format elf64-littleaarch64


Disassembly of section .init:

00000000000006b8 <_init>:
 6b8:	d503201f 	nop
 6bc:	a9bf7bfd 	stp	x29, x30, [sp, #-16]!
 6c0:	910003fd 	mov	x29, sp
 6c4:	9400003c 	bl	7b4 <call_weak_fn>
 6c8:	a8c17bfd 	ldp	x29, x30, [sp], #16
 6cc:	d65f03c0 	ret

Disassembly of section .plt:

00000000000006d0 <.plt>:
 6d0:	a9bf7bf0 	stp	x16, x30, [sp, #-16]!
 6d4:	90000090 	adrp	x16, 10000 <__FRAME_END__+0xf550>
 6d8:	f947c611 	ldr	x17, [x16, #3976]
 6dc:	913e2210 	add	x16, x16, #0xf88
 6e0:	d61f0220 	br	x17
 6e4:	d503201f 	nop
 6e8:	d503201f 	nop
 6ec:	d503201f 	nop

00000000000006f0 <__libc_start_main@plt>:
 6f0:	90000090 	adrp	x16, 10000 <__FRAME_END__+0xf550>
 6f4:	f947ca11 	ldr	x17, [x16, #3984]
 6f8:	913e4210 	add	x16, x16, #0xf90
 6fc:	d61f0220 	br	x17

0000000000000700 <__cxa_finalize@plt>:
 700:	90000090 	adrp	x16, 10000 <__FRAME_END__+0xf550>
 704:	f947ce11 	ldr	x17, [x16, #3992]
 708:	913e6210 	add	x16, x16, #0xf98
 70c:	d61f0220 	br	x17

0000000000000710 <__printf_chk@plt>:
 710:	90000090 	adrp	x16, 10000 <__FRAME_END__+0xf550>
 714:	f947d211 	ldr	x17, [x16, #4000]
 718:	913e8210 	add	x16, x16, #0xfa0
 71c:	d61f0220 	br	x17

0000000000000720 <__stack_chk_fail@plt>:
 720:	90000090 	adrp	x16, 10000 <__FRAME_END__+0xf550>
 724:	f947d611 	ldr	x17, [x16, #4008]
 728:	913ea210 	add	x16, x16, #0xfa8
 72c:	d61f0220 	br	x17

0000000000000730 <__gmon_start__@plt>:
 730:	90000090 	adrp	x16, 10000 <__FRAME_END__+0xf550>
 734:	f947da11 	ldr	x17, [x16, #4016]
 738:	913ec210 	add	x16, x16, #0xfb0
 73c:	d61f0220 	br	x17

0000000000000740 <abort@plt>:
 740:	90000090 	adrp	x16, 10000 <__FRAME_END__+0xf550>
 744:	f947de11 	ldr	x17, [x16, #4024]
 748:	913ee210 	add	x16, x16, #0xfb8
 74c:	d61f0220 	br	x17

0000000000000750 <puts@plt>:
 750:	90000090 	adrp	x16, 10000 <__FRAME_END__+0xf550>
 754:	f947e211 	ldr	x17, [x16, #4032]
 758:	913f0210 	add	x16, x16, #0xfc0
 75c:	d61f0220 	br	x17

Disassembly of section .text:

0000000000000780 <_start>:
 780:	d503201f 	nop
 784:	d280001d 	mov	x29, #0x0                   	// #0
 788:	d280001e 	mov	x30, #0x0                   	// #0
 78c:	aa0003e5 	mov	x5, x0
 790:	f94003e1 	ldr	x1, [sp]
 794:	910023e2 	add	x2, sp, #0x8
 798:	910003e6 	mov	x6, sp
 79c:	90000080 	adrp	x0, 10000 <__FRAME_END__+0xf550>
 7a0:	f947f800 	ldr	x0, [x0, #4080]
 7a4:	d2800003 	mov	x3, #0x0                   	// #0
 7a8:	d2800004 	mov	x4, #0x0                   	// #0
 7ac:	97ffffd1 	bl	6f0 <__libc_start_main@plt>
 7b0:	97ffffe4 	bl	740 <abort@plt>

00000000000007b4 <call_weak_fn>:
 7b4:	90000080 	adrp	x0, 10000 <__FRAME_END__+0xf550>
 7b8:	f947f000 	ldr	x0, [x0, #4064]
 7bc:	b4000040 	cbz	x0, 7c4 <call_weak_fn+0x10>
 7c0:	17ffffdc 	b	730 <__gmon_start__@plt>
 7c4:	d65f03c0 	ret
 7c8:	d503201f 	nop
 7cc:	d503201f 	nop

00000000000007d0 <deregister_tm_clones>:
 7d0:	b0000080 	adrp	x0, 11000 <__data_start>
 7d4:	91004000 	add	x0, x0, #0x10
 7d8:	b0000081 	adrp	x1, 11000 <__data_start>
 7dc:	91004021 	add	x1, x1, #0x10
 7e0:	eb00003f 	cmp	x1, x0
 7e4:	540000c0 	b.eq	7fc <deregister_tm_clones+0x2c>  // b.none
 7e8:	90000081 	adrp	x1, 10000 <__FRAME_END__+0xf550>
 7ec:	f947e821 	ldr	x1, [x1, #4048]
 7f0:	b4000061 	cbz	x1, 7fc <deregister_tm_clones+0x2c>
 7f4:	aa0103f0 	mov	x16, x1
 7f8:	d61f0200 	br	x16
 7fc:	d65f03c0 	ret

0000000000000800 <register_tm_clones>:
 800:	b0000080 	adrp	x0, 11000 <__data_start>
 804:	91004000 	add	x0, x0, #0x10
 808:	b0000081 	adrp	x1, 11000 <__data_start>
 80c:	91004021 	add	x1, x1, #0x10
 810:	cb000021 	sub	x1, x1, x0
 814:	d37ffc22 	lsr	x2, x1, #63
 818:	8b810c41 	add	x1, x2, x1, asr #3
 81c:	9341fc21 	asr	x1, x1, #1
 820:	b40000c1 	cbz	x1, 838 <register_tm_clones+0x38>
 824:	90000082 	adrp	x2, 10000 <__FRAME_END__+0xf550>
 828:	f947fc42 	ldr	x2, [x2, #4088]
 82c:	b4000062 	cbz	x2, 838 <register_tm_clones+0x38>
 830:	aa0203f0 	mov	x16, x2
 834:	d61f0200 	br	x16
 838:	d65f03c0 	ret
 83c:	d503201f 	nop

0000000000000840 <__do_global_dtors_aux>:
 840:	a9be7bfd 	stp	x29, x30, [sp, #-32]!
 844:	910003fd 	mov	x29, sp
 848:	f9000bf3 	str	x19, [sp, #16]
 84c:	b0000093 	adrp	x19, 11000 <__data_start>
 850:	39404260 	ldrb	w0, [x19, #16]
 854:	35000140 	cbnz	w0, 87c <__do_global_dtors_aux+0x3c>
 858:	90000080 	adrp	x0, 10000 <__FRAME_END__+0xf550>
 85c:	f947ec00 	ldr	x0, [x0, #4056]
 860:	b4000080 	cbz	x0, 870 <__do_global_dtors_aux+0x30>
 864:	b0000080 	adrp	x0, 11000 <__data_start>
 868:	f9400400 	ldr	x0, [x0, #8]
 86c:	97ffffa5 	bl	700 <__cxa_finalize@plt>
 870:	97ffffd8 	bl	7d0 <deregister_tm_clones>
 874:	52800020 	mov	w0, #0x1                   	// #1
 878:	39004260 	strb	w0, [x19, #16]
 87c:	f9400bf3 	ldr	x19, [sp, #16]
 880:	a8c27bfd 	ldp	x29, x30, [sp], #32
 884:	d65f03c0 	ret
 888:	d503201f 	nop
 88c:	d503201f 	nop

0000000000000890 <frame_dummy>:
 890:	17ffffdc 	b	800 <register_tm_clones>

0000000000000894 <main>:
 894:	a9bd7bfd 	stp	x29, x30, [sp, #-48]!
 898:	910003fd 	mov	x29, sp
 89c:	90000080 	adrp	x0, 10000 <__FRAME_END__+0xf550>
 8a0:	f947f400 	ldr	x0, [x0, #4072]
 8a4:	f9400001 	ldr	x1, [x0]
 8a8:	f90017e1 	str	x1, [sp, #40]
 8ac:	d2800001 	mov	x1, #0x0                   	// #0
 8b0:	d2800000 	mov	x0, #0x0                   	// #0
 8b4:	d2800001 	mov	x1, #0x0                   	// #0
 8b8:	92607c00 	and	x0, x0, #0xffffffff00000000
 8bc:	d2800022 	mov	x2, #0x1                   	// #1
 8c0:	b3607c40 	bfi	x0, x2, #32, #32
 8c4:	d2800042 	mov	x2, #0x2                   	// #2
 8c8:	b3407c41 	bfxil	x1, x2, #0, #32
 8cc:	d2800062 	mov	x2, #0x3                   	// #3
 8d0:	b3607c41 	bfi	x1, x2, #32, #32
 8d4:	a90187e0 	stp	x0, x1, [sp, #24]
 8d8:	90000000 	adrp	x0, 0 <__abi_tag-0x278>
 8dc:	91260000 	add	x0, x0, #0x980
 8e0:	97ffff9c 	bl	750 <puts@plt>
 8e4:	52800000 	mov	w0, #0x0                   	// #0
 8e8:	52800023 	mov	w3, #0x1                   	// #1
 8ec:	52800042 	mov	w2, #0x2                   	// #2
 8f0:	52800061 	mov	w1, #0x3                   	// #3
 8f4:	b9001bff 	str	wzr, [sp, #24]
 8f8:	b9001fe3 	str	w3, [sp, #28]
 8fc:	b90023e2 	str	w2, [sp, #32]
 900:	b90027e1 	str	w1, [sp, #36]
 904:	b9001be0 	str	w0, [sp, #24]
 908:	b9001bff 	str	wzr, [sp, #24]
 90c:	11000400 	add	w0, w0, #0x1
 910:	710fa01f 	cmp	w0, #0x3e8
 914:	54ffff01 	b.ne	8f4 <main+0x60>  // b.any
 918:	b9401be2 	ldr	w2, [sp, #24]
 91c:	b9401fe3 	ldr	w3, [sp, #28]
 920:	b94023e4 	ldr	w4, [sp, #32]
 924:	b94027e5 	ldr	w5, [sp, #36]
 928:	90000001 	adrp	x1, 0 <__abi_tag-0x278>
 92c:	91268021 	add	x1, x1, #0x9a0
 930:	52800020 	mov	w0, #0x1                   	// #1
 934:	97ffff77 	bl	710 <__printf_chk@plt>
 938:	90000080 	adrp	x0, 10000 <__FRAME_END__+0xf550>
 93c:	f947f400 	ldr	x0, [x0, #4072]
 940:	f94017e2 	ldr	x2, [sp, #40]
 944:	f9400001 	ldr	x1, [x0]
 948:	eb010042 	subs	x2, x2, x1
 94c:	d2800001 	mov	x1, #0x0                   	// #0
 950:	54000081 	b.ne	960 <main+0xcc>  // b.any
 954:	52800000 	mov	w0, #0x0                   	// #0
 958:	a8c37bfd 	ldp	x29, x30, [sp], #48
 95c:	d65f03c0 	ret
 960:	97ffff70 	bl	720 <__stack_chk_fail@plt>

Disassembly of section .fini:

0000000000000964 <_fini>:
 964:	d503201f 	nop
 968:	a9bf7bfd 	stp	x29, x30, [sp, #-16]!
 96c:	910003fd 	mov	x29, sp
 970:	a8c17bfd 	ldp	x29, x30, [sp], #16
 974:	d65f03c0 	ret
