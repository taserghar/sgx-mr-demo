;
;    ZeroTrace: Oblivious Memory Primitives from Intel SGX
;    Copyright (C) 2018  Sajin (sshsshy)
;
;    This program is free software: you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation, version 3 of the License.
;
;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program.  If not, see <https://www.gnu.org/licenses/>.
;

BITS 64

section .text
	;global_start
	global oswap_buffer


oswap_buffer:
		; Take inputs,  1 ptr to dest_buffer, 2 ptr to source_buffer, 3 buffer_size, 4 flag
		; Linux : 	rdi,rsi,rdx,rcx->rbp

		; Callee-saved : RBP, RBX, and R12–R15

		push rbx
		push rbp
		push r9
		push r12
		push r13
		push r14
		push r15

		; Move ptr to data from serialized_dest_block and serialized_source_blk
		mov r10, rdi
		mov r11, rsi

		;RCX will be lost for loop, store flag from rcx to rbp (1 byte , so bpl)
		mov bpl, cl

		; Oblivious evaluation of flag
		cmp bpl, 1

		;Set loop parameters
		mov ax, dx
		xor rdx, rdx
		mov bx, 8
		div bx
		mov cx, ax

		; Loop to fetch iter & res chunks till blk_size
		loopstart5:
			cmp bpl, 1
			mov r14, qword [r10]		; r14 <- dest
			mov r15, qword [r11]		;r15 <- src
			cmovz r9, r14
			cmovz r14, r15 				;r14 / r15 based on the compare
			cmovz r15, r9
			mov qword [r10], r14
			mov qword [r11], r15
			add r10, 8
			add r11, 8
			loop loopstart5

		pop r15
		pop r14
		pop r13
		pop r12
		pop r9
		pop rbp
		pop rbx

		ret
