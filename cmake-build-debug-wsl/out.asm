global _start
_start:
	mov rax, 6
	push rax
	mov rax, 3
	push rax
	mov rax, 3
	push rax
	pop rbx
	pop rax
	div rbx
	push rax
	pop rbx
	pop rax
	add rax, rbx
	push rax
	mov rax, 4
	push rax
	push qword [rsp + 8]
	push qword [rsp + 8]
	pop rbx
	pop rax
	add rax, rbx
	push rax
	push qword [rsp + 16]
	mov rax, 60
	pop rdi
	syscall
	mov rax, 60
	mov rdi, 0
	syscall