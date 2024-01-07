;
; NASM implementation for terrain/blender.cc
;

segment .data

delta_a times 2 dd 0
delta_b times 2 dd 0
delta_c times 2 dd 0
delta_d times 2 dd 0

alpha_a0 times 2 dd 0
alpha_b0 times 2 dd 0
alpha_c0 times 2 dd 0
alpha_d0 times 2 dd 0
alpha_a1 times 2 dd 0
alpha_b1 times 2 dd 0
alpha_c1 times 2 dd 0
alpha_d1 times 2 dd 0
alpha_a2 times 2 dd 0
alpha_b2 times 2 dd 0
alpha_c2 times 2 dd 0
alpha_d2 times 2 dd 0
alpha_a3 times 2 dd 0
alpha_b3 times 2 dd 0
alpha_c3 times 2 dd 0
alpha_d3 times 2 dd 0

ldelt_a times 2 dd 0
ldelt_b times 2 dd 0
ldelt_c times 2 dd 0
ldelt_d times 2 dd 0
rdelt_a times 2 dd 0
rdelt_b times 2 dd 0
rdelt_c times 2 dd 0
rdelt_d times 2 dd 0

zero times 2 dd 0

; FIXME: get back to this
; redLightMask times 2 dd 0xf800000000000000
; greenLightMask times 2 dd 0x07c0000000000000
; blueLightMask times 2 dd 0x003e000000000000

; bluePackMask times 2 dd 0x003e0000000000000
; greenPackMask times 2 dd 0x07c0000000000000
; redPackMask times 2 dd 0x00f8000000000000

redLightMask times 2 dd 0
greenLightMask times 2 dd 0
blueLightMask times 2 dd 0

bluePackMask times 2 dd 0
greenPackMask times 2 dd 0
redPackMask times 2 dd 0

rdeltq times 2 dd 0
ldeltq times 2 dd 0

ix dw 0
iy dw 0

lpoints times 4 dd 0
texelsPerLumelShift dd 0
texelsPerLumel dd 0
texelsPerLumelDiv2 dd 0

segment .text

; global macros
%define dst [ebp+8]
%define sq_shift [ebp+12]
%define aoff [ebp+16]
%define bmp_ptrs [ebp+20]
%define alpha_ptrs [ebp+24]

;
; void doSquare4( U32 *dst,
;                 int sq_shift,
;                 int *aoff,
;                 U32 **bmp_ptrs, 
;                 U8 **alpha_ptrs )
;

global doSquare4

doSquare4:

	; prologue
	push ebp
	mov ebp, esp

	; setup ix, iy
	mov eax, 1
	mov cl, sq_shift
	shl eax, cl
	mov dword [iy], eax

	shr eax, 1
	mov dword [ix], eax

	; actual code from blender.cc
        movd    mm1, sq_shift

        ; get alpha values for the corners of the square for each texture type.
        ;  replicate the values into 4 words of the qwords.  Also calc vertical
        ;  stepping values for the alpha values on left and right edges.
        ; load alpha value into bh to mul by 256 for precision. then
        ; punpcklwd mm0, mm0 followed by punpckldq mm0, mm0 
        ;  to replicate the low word into all words of mm0.
        ; shift down difference by sqshift to divide by pixels per square to get
        ;  increment.
        mov     esi, aoff
        mov     edi, alpha_ptrs
        mov     eax, [edi]
        mov     edx, eax
        add     eax, [esi]
        xor     ebx,ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm2, ebx
        punpcklwd mm2, mm2
        add     eax, [esi+8]
        punpckldq mm2, mm2
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm0, ebx
        punpcklwd mm0, mm0
        punpckldq mm0, mm0
        movq    [alpha_a0], mm2
        psubw   mm0, mm2
        add     eax, [esi+4]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm4, ebx
        punpcklwd mm4, mm4
        add     eax, [esi+12]
        punpckldq mm4, mm4
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        movd    mm3, ebx
        movq    [alpha_a1], mm4
        punpcklwd mm3, mm3
        punpckldq mm3, mm3
        psraw   mm0, mm1
        psubw   mm3, mm4
        movq    [ldelt_a], mm0
        psraw   mm3, mm1
        movq    [rdelt_a], mm3

        mov     eax, [edi+4]
        mov     edx, eax
        add     eax, [esi]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm2, ebx
        punpcklwd mm2, mm2
        add     eax, [esi+8]
        punpckldq mm2, mm2
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm0, ebx
        punpcklwd mm0, mm0
        punpckldq mm0, mm0
        movq    [alpha_b0], mm2
        psubw   mm0, mm2
        add     eax, [esi+4]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm4, ebx
        punpcklwd mm4, mm4
        add     eax, [esi+12]
        punpckldq mm4, mm4
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        movd    mm3, ebx
        movq    [alpha_b1], mm4
        punpcklwd mm3, mm3
        punpckldq mm3, mm3
        psraw   mm0, mm1
        psubw   mm3, mm4
        movq    [ldelt_b], mm0
        psraw   mm3, mm1
        movq    [rdelt_b], mm3

        mov     eax, [edi+8]
        mov     edx, eax
        add     eax, [esi]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm2, ebx
        punpcklwd mm2, mm2
        add     eax, [esi+8]
        punpckldq mm2, mm2
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm0, ebx
        punpcklwd mm0, mm0
        punpckldq mm0, mm0
        movq    [alpha_c0], mm2
        movq    [alpha_c2], mm0
        psubw   mm0, mm2
        add     eax, [esi+4]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm4, ebx
        punpcklwd mm4, mm4
        add     eax, [esi+12]
        punpckldq mm4, mm4
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        movd    mm3, ebx
        movq    [alpha_c1], mm4
        punpcklwd mm3, mm3
        punpckldq mm3, mm3
        psraw   mm0, mm1
        psubw   mm3, mm4
        movq    [ldelt_c], mm0
        psraw   mm3, mm1
        movq    [rdelt_c], mm3

        mov     eax, [edi+12]
        mov     edx, eax
        add     eax, [esi]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm2, ebx
        punpcklwd mm2, mm2
        add     eax, [esi+8]
        punpckldq mm2, mm2
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm0, ebx
        punpcklwd mm0, mm0
        punpckldq mm0, mm0
        movq    [alpha_d0], mm2
        movq    [alpha_d2], mm0
        psubw   mm0, mm2
        add     eax, [esi+4]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm4, ebx
        punpcklwd mm4, mm4
        add     eax, [esi+12]
        punpckldq mm4, mm4
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        movd    mm3, ebx
        movq    [alpha_d1], mm4
        punpcklwd mm3, mm3
        punpckldq mm3, mm3
        psraw   mm0, mm1
        psubw   mm3, mm4
        movq    [ldelt_d], mm0
        psraw   mm3, mm1
        movq    [rdelt_d], mm3

        mov     esi, bmp_ptrs
        mov     eax, [esi]
        mov     ebx, [esi+4]
        mov     ecx, [esi+8]
        mov     edx, [esi+12]

        movq    mm0, [alpha_a1]
        movq    mm2, [alpha_b1]
        movq    mm3, [alpha_c1]
        movq    mm4, [alpha_a0]
        movq    mm5, [alpha_b0]
        movq    mm6, [alpha_c0]
        movq    mm7, [alpha_d0]
        mov     edi, dst

yloop4:
        ; mm1 should be sq_shift at this point

        ; calculate alpha step increments...word-size steps are replicated
        ;  to fill qword.
        psubw   mm0, mm4
        psraw   mm0, mm1            ;mm0 = (right-left) >> sq_shift
        movq    [delta_a], mm0         ;delta = ainc ainc ainc ainc
        
        psubw   mm2, mm5
        psraw   mm2, mm1            ;mm0 = (right-left) >> sq_shift
        movq    [delta_b], mm2         ;delta = ainc ainc ainc ainc
        
        psubw   mm3, mm6
        psraw   mm3, mm1            ;mm0 = (right-left) >> sq_shift
        movq    [delta_c], mm3         ;delta = ainc ainc ainc ainc
        
        movq    mm0, [alpha_d1]
        psubw   mm0, mm7
        psraw   mm0, mm1            ;mm0 = (right-left) >> sq_shift
        movq    [delta_d], mm0         ;delta = ainc ainc ainc ainc

        mov     esi, [ix]
        pxor    mm2, mm2

xloop4:
        movq    mm0, [eax]
        movq    mm1, mm0
        punpcklbw mm0, mm2
        pmulhw  mm0, mm4
        paddw   mm4, [delta_a]
        punpckhbw mm1, mm2
        pmulhw  mm1, mm4
        paddw   mm4, [delta_a]
        packuswb mm0, mm1

        movq    mm3, [ebx]
        movq    mm1, mm3
        punpcklbw mm3, mm2
        pmulhw  mm3, mm5
        paddw   mm5, [delta_b]
        punpckhbw mm1, mm2
        pmulhw  mm1, mm5
        paddw   mm5, [delta_b]
        packuswb mm3, mm1
        paddb   mm0, mm3

        movq    mm3, [ecx]
        movq    mm1, mm3
        punpcklbw mm3, mm2
        pmulhw  mm3, mm6
        paddw   mm6, [delta_c]
        punpckhbw mm1, mm2
        pmulhw  mm1, mm6
        paddw   mm6, [delta_c]
        packuswb mm3, mm1
        paddb   mm0, mm3

        movq    mm3, [edx]
        movq    mm1, mm3
        punpcklbw mm3, mm2
        pmulhw  mm3, mm7
        paddw   mm7, [delta_d]
        punpckhbw mm1, mm2
        pmulhw  mm1, mm7
        paddw   mm7, [delta_d]
        packuswb mm3, mm1
        paddb   mm0, mm3

        ; double result, to make up for alpha vals being signed (max = 127)
        ; so our math turns out a bit short, example:
        ;  (0x7f00 * 0xff) >> 16 = 0x7e....* 2 = 252...not quite 255
        ; would have been (0xff00 * 0xff) >> 16 = 0xfe = 254, 
        ;  if I could do an unsigned pmulhw...
        ; pmulhuw is in an intel document I found, but doesn't compile....
        paddb   mm0, mm0        

        movq    [edi], mm0

        add     eax, 8
        add     ebx, 8
        add     ecx, 8
        add     edx, 8
        add     edi, 8

        dec     esi
        jnz     near xloop4

        movq    mm4, [alpha_a0]
        paddw   mm4, [ldelt_a]
        movq    [alpha_a0], mm4

        movq    mm5, [alpha_b0]
        paddw   mm5, [ldelt_b]
        movq    [alpha_b0], mm5

        movq    mm6, [alpha_c0]
        paddw   mm6, [ldelt_c]
        movq    [alpha_c0], mm6

        movq    mm7, [alpha_d0]
        paddw   mm7, [ldelt_d]
        movq    [alpha_d0], mm7

        movq    mm0, [alpha_d1]
        paddw   mm0, [rdelt_d]
        movq    [alpha_d1], mm0

        movq    mm2, [alpha_b1]
        paddw   mm2, [rdelt_b]
        movq    [alpha_b1], mm2

        movq    mm3, [alpha_c1]
        paddw   mm3, [rdelt_c]
        movq    [alpha_c1], mm3

        movq    mm0, [alpha_a1]
        paddw   mm0, [rdelt_a]
        movq    [alpha_a1], mm0

        movd    mm1, sq_shift       ; top of loop expects this

        dec     dword [iy]
        jnz     near yloop4

        emms	

	; epilogue
	pop ebp
	ret

;
; void doSquare3( U32 *dst,
;		  int sq_shift,
;		  int *aoff,
;		  U32 **bmp_ptrs, 
;		  U8 **alpha_ptrs )
;

global doSquare3

doSquare3:

	; prologue
	push ebp
	mov ebp, esp

	; setup ix, iy
	mov eax, 1
	mov cl, sq_shift
	shl eax, cl
	mov dword [iy], eax

	shr eax, 1
	mov dword [ix], eax

        movd    mm1, sq_shift
        ; get alpha values for the corners of the square for each texture type.
        ;  replicate the values into 4 words of the qwords.  Also calc vertical
        ;  stepping values for the alpha values on left and right edges.
        ; load alpha value into bh to mul by 256 for precision. then
        ; punpcklwd mm0, mm0 followed by punpckldq mm0, mm0 
        ;  to replicate the low word into all words of mm0.
        ; shift down difference by sqshift to divide by pixels per square to get
        ;  increment.

        mov     esi, aoff
        mov     edi, alpha_ptrs
        mov     eax, [edi]
        mov     edx, eax
        add     eax, [esi]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm2, ebx
        punpcklwd mm2, mm2
        add     eax, [esi+8]
        punpckldq mm2, mm2
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm0, ebx
        punpcklwd mm0, mm0
        punpckldq mm0, mm0
        movq    [alpha_a0], mm2
        psubw   mm0, mm2
        add     eax, [esi+4]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm4, ebx
        punpcklwd mm4, mm4
        add     eax, [esi+12]
        punpckldq mm4, mm4
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        movd    mm3, ebx
        movq    [alpha_a1], mm4
        punpcklwd mm3, mm3
        punpckldq mm3, mm3
        psraw   mm0, mm1
        psubw   mm3, mm4
        movq    [ldelt_a], mm0
        psraw   mm3, mm1
        movq    [rdelt_a], mm3

        mov     eax, [edi+4]
        mov     edx, eax
        add     eax, [esi]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm2, ebx
        punpcklwd mm2, mm2
        add     eax, [esi+8]
        punpckldq mm2, mm2
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm0, ebx
        punpcklwd mm0, mm0
        punpckldq mm0, mm0
        movq    [alpha_b0], mm2
        psubw   mm0, mm2
        add     eax, [esi+4]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm4, ebx
        punpcklwd mm4, mm4
        add     eax, [esi+12]
        punpckldq mm4, mm4
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        movd    mm3, ebx
        movq    [alpha_b1], mm4
        punpcklwd mm3, mm3
        punpckldq mm3, mm3
        psraw   mm0, mm1
        psubw   mm3, mm4
        movq    [ldelt_b], mm0
        psraw   mm3, mm1
        movq    [rdelt_b], mm3

        mov     eax, [edi+8]
        mov     edx, eax
        add     eax, [esi]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm2, ebx
        punpcklwd mm2, mm2
        add     eax, [esi+8]
        punpckldq mm2, mm2
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm0, ebx
        punpcklwd mm0, mm0
        punpckldq mm0, mm0
        movq    [alpha_c0], mm2
        movq    [alpha_c2], mm0
        psubw   mm0, mm2
        add     eax, [esi+4]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm4, ebx
        punpcklwd mm4, mm4
        add     eax, [esi+12]
        punpckldq mm4, mm4
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        movd    mm3, ebx
        movq    [alpha_c1], mm4
        punpcklwd mm3, mm3
        punpckldq mm3, mm3
        psraw   mm0, mm1
        psubw   mm3, mm4
        movq    [ldelt_c], mm0
        psraw   mm3, mm1
        movq    [rdelt_c], mm3

        mov     esi, bmp_ptrs
        mov     eax, [esi]
        mov     ebx, [esi+4]
        mov     ecx, [esi+8]

        movq    mm0, [alpha_a1]
        movq    mm2, [alpha_b1]
        movq    mm3, [alpha_c1]
        movq    mm4, [alpha_a0]
        movq    mm5, [alpha_b0]
        movq    mm6, [alpha_c0]
        mov     edi, dst

yloop3:
        ; mm1 should be sq_shift at this point
        ; mm0 should be [alpha_a1]
        ; mm2 should be [alpha_b1]
        ; mm3 should be [alpha_c1]

        ; calculate alpha step increments...word-size steps are replicated
        ;  to fill qword.
        psubw   mm0, mm4
        psraw   mm0, mm1            ;mm0 = (right-left) >> sq_shift
        movq    [delta_a], mm0         ;delta = ainc ainc ainc ainc
        
        psubw   mm2, mm5
        psraw   mm2, mm1            ;mm0 = (right-left) >> sq_shift
        movq    [delta_b], mm2         ;delta = ainc ainc ainc ainc
        
        psubw   mm3, mm6
        psraw   mm3, mm1            ;mm0 = (right-left) >> sq_shift
        movq    [delta_c], mm3         ;delta = ainc ainc ainc ainc
        
        mov     esi, ix
        pxor    mm2, mm2

        movq    mm7, [delta_a]
xloop3:
        movq    mm0, [eax]
        movq    mm1, mm0
        punpcklbw mm0, mm2
        pmulhw  mm0, mm4
        paddw   mm4, mm7
        punpckhbw mm1, mm2
        pmulhw  mm1, mm4
        paddw   mm4, mm7
        packuswb mm0, mm1

        movq    mm3, [ebx]
        movq    mm1, mm3
        punpcklbw mm3, mm2
        pmulhw  mm3, mm5
        paddw   mm5, [delta_b]
        punpckhbw mm1, mm2
        pmulhw  mm1, mm5
        paddw   mm5, [delta_b]
        packuswb mm3, mm1
        paddb   mm0, mm3

        movq    mm3, [ecx]
        movq    mm1, mm3
        punpcklbw mm3, mm2
        pmulhw  mm3, mm6
        paddw   mm6, [delta_c]
        punpckhbw mm1, mm2
        pmulhw  mm1, mm6
        paddw   mm6, [delta_c]
        packuswb mm3, mm1
        paddb   mm0, mm3
        paddb   mm0, mm0

        movq    [edi], mm0

        add     eax, 8
        add     ebx, 8
        add     ecx, 8
        add     edi, 8

        dec     esi
        jnz     near xloop3

        movq    mm4, [alpha_a0]
        paddw   mm4, [ldelt_a]
        movq    [alpha_a0], mm4

        movq    mm5, [alpha_b0]
        paddw   mm5, [ldelt_b]
        movq    [alpha_b0], mm5

        movq    mm6, [alpha_c0]
        paddw   mm6, [ldelt_c]
        movq    [alpha_c0], mm6

        movq    mm2, [alpha_b1]
        paddw   mm2, [rdelt_b]
        movq    [alpha_b1], mm2

        movq    mm3, [alpha_c1]
        paddw   mm3, [rdelt_c]
        movq    [alpha_c1], mm3

        movq    mm0, [alpha_a1]
        paddw   mm0, [rdelt_a]
        movq    [alpha_a1], mm0

        movd    mm1, sq_shift       ; top of loop expects this

        dec     dword [iy]
        jnz     near yloop3

        emms

	; epilogue
	pop ebp
	ret

;
; void doSquare2( U32 *dst,
;		  int sq_shift,
;		  int *aoff,
;		  U32 **bmp_ptrs, 
;                 U8 **alpha_ptrs )
;

global doSquare2

doSquare2:

	; prologue
	push ebp
	mov ebp, esp

	; setup ix, iy
	mov eax, 1
	mov cl, sq_shift
	shl eax, cl
	mov dword [iy], eax

	shr eax, 1
	mov dword [ix], eax

        movd    mm1, sq_shift
        ; get alpha values for the corners of the square for each texture type.
        ;  replicate the values into 4 words of the qwords.  Also calc vertical
        ;  stepping values for the alpha values on left and right edges.
        ; punpcklwd mm0, mm0 followed by punpckldq mm0, mm0 
        ;  to replicate the low word into all words of mm0.
        ; shift down difference by sqshift to divide by pixels per square to get
        ;  increment.

        mov     esi, aoff
        mov     edi, alpha_ptrs
        mov     eax, [edi]
        mov     edx, eax
        add     eax, [esi]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm2, ebx
        punpcklwd mm2, mm2
        add     eax, [esi+8]
        punpckldq mm2, mm2
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm0, ebx
        punpcklwd mm0, mm0
        punpckldq mm0, mm0
        movq    [alpha_a0], mm2
        psubw   mm0, mm2
        add     eax, [esi+4]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm4, ebx
        punpcklwd mm4, mm4
        add     eax, [esi+12]
        punpckldq mm4, mm4
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        movd    mm3, ebx
        movq    [alpha_a1], mm4
        punpcklwd mm3, mm3
        punpckldq mm3, mm3
        psraw   mm0, mm1
        psubw   mm3, mm4
        movq    [ldelt_a], mm0
        psraw   mm3, mm1
        movq    [rdelt_a], mm3

        mov     eax, [edi+4]
        mov     edx, eax
        add     eax, [esi]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm2, ebx
        punpcklwd mm2, mm2
        add     eax, [esi+8]
        punpckldq mm2, mm2
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm0, ebx
        punpcklwd mm0, mm0
        punpckldq mm0, mm0
        movq    [alpha_b0], mm2
        psubw   mm0, mm2
        add     eax, [esi+4]
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        mov     eax, edx
        movd    mm4, ebx
        punpcklwd mm4, mm4
        add     eax, [esi+12]
        punpckldq mm4, mm4
        xor     ebx, ebx
        mov     bl, [eax]
        shl     ebx, 7
        movd    mm3, ebx
        movq    [alpha_b1], mm4
        punpcklwd mm3, mm3
        punpckldq mm3, mm3
        psraw   mm0, mm1
        psubw   mm3, mm4
        movq    [ldelt_b], mm0
        psraw   mm3, mm1
        movq    [rdelt_b], mm3

        mov     esi, bmp_ptrs
        mov     eax, [esi]
        mov     ebx, [esi+4]

        movq    mm0, [alpha_a1]
        movq    mm2, [alpha_b1]
        movq    mm4, [alpha_a0]
        movq    mm5, [alpha_b0]
        mov     edi, dst

yloop2:
        ; mm1 should be sq_shift at this point
        ; mm0 should be [alpha_a1]
        ; mm2 should be [alpha_b1]

        ; calculate alpha step increments...word-size steps are replicated
        ;  to fill qword.
        psubw   mm0, mm4
        psraw   mm0, mm1            ;mm0 = (right-left) >> sq_shift
        movq    [delta_a], mm0         ;delta = ainc ainc ainc ainc
        
        psubw   mm2, mm5
        psraw   mm2, mm1            ;mm0 = (right-left) >> sq_shift
        movq    [delta_b], mm2         ;delta = ainc ainc ainc ainc
        
        mov     esi, ix
        pxor    mm2, mm2

        movq    mm6, [delta_a]
        movq    mm7, [delta_b]

xloop2:
        movq    mm0, [eax]
        movq    mm3, [ebx]

        movq    mm1, mm0
        punpcklbw mm0, mm2
        pmulhw  mm0, mm4
        paddw   mm4, mm6
        punpckhbw mm1, mm2
        pmulhw  mm1, mm4
        paddw   mm4, mm6
        packuswb mm0, mm1

        movq    mm1, mm3
        punpcklbw mm3, mm2
        pmulhw  mm3, mm5
        paddw   mm5, mm7
        punpckhbw mm1, mm2
        pmulhw  mm1, mm5
        paddw   mm5, mm7
        packuswb mm3, mm1
        paddb   mm0, mm3
        paddb   mm0, mm0

        movq    [edi], mm0

        add     edi, 8
        add     eax, 8
        add     ebx, 8

        dec     esi
        jnz     xloop2

        movq    mm4, [alpha_a0]
        paddw   mm4, [ldelt_a]
        movq    [alpha_a0], mm4

        movq    mm5, [alpha_b0]
        paddw   mm5, [ldelt_b]
        movq    [alpha_b0], mm5

        movq    mm2, [alpha_b1]
        paddw   mm2, [rdelt_b]
        movq    [alpha_b1], mm2

        movq    mm0, [alpha_a1]
        paddw   mm0, [rdelt_a]
        movq    [alpha_a1], mm0

        movd    mm1, sq_shift       ; top of loop expects this

        dec     dword [iy]
        jnz     near yloop2

        emms

	; epilogue
	pop ebp
	ret

global setupLumel

setupLumel:

	; prologue
	push ebp
	mov ebp, esp

	; we only need to load the high bits up, they're already 0
	; in the low bits
	mov dword [redLightMask], 0xf8000000
	mov dword [greenLightMask], 0x07c0000
	mov dword [blueLightMask], 0x003e0000

	mov dword [bluePackMask], 0x003e0000
	mov dword [greenPackMask], 0x07c00000
	mov dword [redPackMask], 0x00f80000

	; epilogue
	pop ebp
	ret

;
; void doLumel( U16 *dstptr,
;		U32 *srcptr,
;		int nextdstrow,
;		int nextsrcrow )
;

%define dstptr [ebp+8]
%define srcptr [ebp+12]
%define nextdstrow [ebp+16]
%define nextsrcrow [ebp+20]

global doLumel

doLumel:

	; prologue
	push ebp
	mov ebp, esp

        movd    mm7, [texelsPerLumelShift]

        movd    mm0, [lpoints]
        movq    mm4, mm0
        pand    mm0, [redLightMask]
        movq    mm5, mm4
        pand    mm4, [greenLightMask]
        psllq   mm0, 31
        pand    mm5, [blueLightMask]
        psllq   mm4, 20
        paddw   mm0, mm4
        psllq   mm5, 9
        paddw   mm0, mm5        ; mm0 = 0000rrrrggggbbbb qword for lp[0]

        movd    mm1, [lpoints+8]    ; get lp2
        movq    mm4, mm1
        pand    mm1, [redLightMask]
        movq    mm5, mm4
        pand    mm4, [greenLightMask]
        psllq   mm1, 31
        pand    mm5, [blueLightMask]
        psllq   mm4, 20
        paddw   mm1, mm4
        psllq   mm5, 9
        paddw   mm1, mm5        ; mm1 = 0000rrrrggggbbbb qword for lp[2]

        psubw   mm1, mm0
        psraw   mm1, mm7
        movq    [ldeltq], mm1

        movd    mm2, [lpoints+4]    ; get lp[1]
        movq    mm4, mm2
        pand    mm2, [redLightMask]
        movq    mm5, mm4
        pand    mm4, [greenLightMask]
        psllq   mm2, 31
        pand    mm5, [blueLightMask]
        psllq   mm4, 20
        paddw   mm2, mm4
        psllq   mm5, 9
        paddw   mm2, mm5        ; mm2 = 0000rrrrggggbbbb qword for lp[1]

        movd    mm3, [lpoints+12]    ; get lp3
        movq    mm4, mm3
        pand    mm3, [redLightMask]
        movq    mm5, mm4
        pand    mm4, [greenLightMask]
        psllq   mm3, 31
        pand    mm5, [blueLightMask]
        psllq   mm4, 20
        paddw   mm3, mm4
        psllq   mm5, 9
        paddw   mm3, mm5        ; mm3 = 0000rrrrggggbbbb qword for lp[3]

        psubw   mm3, mm2
        psraw   mm3, mm7
        movq    [rdeltq], mm3

        mov     edi, dstptr
        mov     esi, srcptr
        pxor    mm6, mm6

        mov     eax, [texelsPerLumel] ; yloop count
        cmp     eax, 1
        jne     not_special


        ; special case for 1x1 lumel
        movd    mm4, [esi]
        punpcklbw mm4, mm6      ; mm6 is expected to be 0 here
        pmulhw  mm4, mm0
        paddw   mm4, mm4

        movq    mm7, mm4
        movq    mm6, mm4
        psrlq   mm4, 34
        pand    mm7, [redPackMask]
        psrlq   mm6, 13
; MASKALPHA
;        pand    mm4, [bluePackMask]
        psllq   mm7, 8
        pand    mm6, [greenPackMask]
        paddw   mm4, mm7
        paddw   mm4, mm6
        movd    eax, mm4
        mov     [edi],ax
        jmp     done

not_special:

        ; mm0 = left at loop start
        ; mm2 = right
yloopL:
        movq    mm1, mm0        ;mm1 = start
        movq    mm3, mm2

        psubw   mm3, mm0
        psraw   mm3, mm7        ; mm3 = delta

        mov     ebx, [texelsPerLumelDiv2] ; loop count

xloopL:
        movq    mm4, [esi]
        movq    mm5, mm4
        punpcklbw mm4, mm6      ; mm6 is expected to be 0 here
        pmulhw  mm4, mm1
        paddw   mm1, mm3
        punpckhbw mm5, mm6
        pmulhw  mm5, mm1
        paddw   mm1, mm3
        paddw   mm4, mm4
        paddw   mm5, mm5

        movq    mm7, mm4
        movq    mm6, mm4
        psrlq   mm4, 34
        pand    mm7, [redPackMask]
        psrlq   mm6, 13
; MASKALPHA
;        pand    mm4, [bluePackMask]
        psllq   mm7, 8
        pand    mm6, [greenPackMask]
        paddw   mm4, mm7
        paddw   mm4, mm6

        movq    mm7, mm5
        movq    mm6, mm5
        psrlq   mm5, 34
        pand    mm7, [redPackMask]
        psrlq   mm6, 13
; MASKALPHA
;        pand    mm4, [bluePackMask]
        psllq   mm7, 8
        pand    mm6, [greenPackMask]
        paddw   mm5, mm7
        paddw   mm5, mm6
        psllq   mm5, 16         ; lazy, I reused code above and must now shift
        paddw   mm4, mm5

        ; write 2 16-bit pixels.  I'd consider doing 4 at a time, but 
        ;  loop count can be as small as 1 at lowest detail already.
        ;  could do separate loop I guess.
        movd    [edi], mm4
        pxor    mm6, mm6
        add     edi, 4
        add     esi, 8
        dec     ebx
        jnz     near xloopL

        movd    mm7, [texelsPerLumelShift]
        paddw   mm0, [ldeltq]
        paddw   mm2, [rdeltq]
        add     edi, nextdstrow
        add     esi, nextsrcrow
        dec     eax
        jnz     near yloopL

done:
        emms

	; epilogue
	pop ebp
	ret
