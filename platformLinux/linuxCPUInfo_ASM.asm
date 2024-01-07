;
; CPU identification fun.
;

;
; void cpu_init_ASM( char* vendor, int* processor, int* properties )
;

%define in_vendor [ebp+8]
%define in_proc [ebp+12]
%define in_prop [ebp+16]

global cpu_init_ASM

cpu_init_ASM:

	push ebp
	mov ebp, esp

	push ebx
	push edx
	push ecx
	push esi

	pushfd
	pushfd
	pop eax

	mov ebx, eax
	xor eax, 0x200000
	push eax
	popfd
	pushfd
	pop eax
	cmp eax, ebx
	jz .L1

	xor eax, eax
	cpuid

	mov esi, in_vendor
	mov dword [esi], ebx
	mov dword [esi+4], edx
	mov dword [esi+8], ecx

	mov eax, 1
	cpuid

	and eax, 0x0ff0
	mov esi, in_proc
	mov [esi], eax
	mov esi, in_prop
	mov [esi], edx

.L1:
	popfd

	pop esi
	pop ecx
	pop edx
	pop ebx

	pop ebp
	ret