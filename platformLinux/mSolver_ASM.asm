;
; ASM implementations of mSolver code
;

; externs
extern mSolveCubic_c__FffffPf

; data
segment .data

; float constants
f_quarter dd 0.25
f_half dd 0.5
f_n_half dd -0.5
f_two dd 2.0
f_n_three dd -3.0
f_four dd 4.0
f_n_four dd -4.0
f_n_six dd -6.0
f_eight dd 8.0

; integer constants
i_n_one dd 1

; variables
i dd 0
nreal dd  0
w dd 0
b0 dd 0
b1 dd 0
b2 dd 0
d0 dd 0
d1 dd 0
h dd 0
t dd 0
z dd 0
px dd 0
cubeA dd 0
cubeB dd 0
cubeC dd 0
cubeD dd 0

p dd 0
q dd 0
dis dd 0
phi dd 0

segment .text

; functions

;
; U32 mSolveQuartic_ASM(F32 A, F32 B, F32 C, F32 D, F32 E, F32* x)
;

%define A [ebp+8]
%define B [ebp+12]
%define C [ebp+16]
%define D [ebp+20]
%define E [ebp+24]
%define x [ebp+28]

global mSolveQuartic_ASM

mSolveQuartic_ASM:

	push ebp
	mov ebp, esp

	;   if (A==0.0)
	;      return mSolveCubic(B, C, D, E, x);
	fld dword A
	fldz
	fcompp
	fnstsw ax
	and ah, 68
	xor ah, 64
	jne .L1

	push dword x
	push dword E
	push dword D
	push dword C
	push dword B
	call mSolveCubic_c__FffffPf
	add esp, byte 20
	jmp .L2

.L1:
	;   w = B/(4*A);			/* offset */
	fld dword A
	fmul dword [f_four]
	fld dword B
	fxch
	fdivp st1, st0
	fstp dword [w]

	;   b2 = -6*square(w) + C/A;		/* coeffs. of shifted polynomial */
	fld dword [w]
	fmul st0, st0
	fld dword [f_n_six]
	fmulp st1, st0
	fld dword C
	fld dword A
	fdivp st1, st0
	faddp st1, st0
	fstp dword [b2]

	;   b1 = (8*square(w) - 2*C/A)*w + D/A;
	fld dword [w]
	fmul st0, st0
	fmul dword [f_eight]
	fld dword C
	fld dword A
	fdivp st1, st0
	fmul dword [f_two]
	fsubp st1, st0
	fmul dword [w]
	fld dword D
	fld dword A
	fdivp st1, st0
	faddp st1, st0
	fstp dword [b1]

	;   b0 = ((-3*square(w) + C/A)*w - D/A)*w + E/A;
	fld dword [w]
	fmul st0, st0
	fmul dword [f_n_three]
	fld dword C
	fld dword A
	fdivp st1, st0
	faddp st1, st0
	fmul dword [w]
	fld dword D
	fld dword A
	fdivp st1, st0
	fsubp st1, st0
	fmul dword [w]
	fld dword E
	fld dword A
	fdivp st1, st0
	faddp st1, st0
	fstp dword [b0]

	;   // cubic resolvent
	;   F32 cubeA = 1.0;
	;   F32 cubeB = b2;
	;   F32 cubeC = -4 * b0;
	;   F32 cubeD = square(b1) - 4 * b0 * b2;
	fld1
	fstp dword [cubeA]
	fld dword [b2]
	fstp dword [cubeB]
	fld dword [b0]
	fmul dword [f_n_four]
	fstp dword [cubeC]
	fld dword [b1]
	fmul st0, st0
	fld dword [f_four]
	fmul dword [b0]
	fmul dword [b2]
	fsubp st1, st0
	fstp dword [cubeD]

	;   mSolveCubic(cubeA, cubeB, cubeC, cubeD, x);
	push dword x
	push dword [cubeD]
	push dword [cubeC]
	push dword [cubeB]
	push dword [cubeA]
	call mSolveCubic_c__FffffPf
	add esp, byte 20

	mov eax, dword x
	;   z = x[0]; 
	fld dword [eax+0]
	fstp dword [z]
	;   nreal = 0;
	xor dword [nreal], 0
	;   px = x;
	mov dword [px], eax
	;   t = mSqrt(0.25 * square(z) - b0);
	fld dword [z]
	fmul st0, st0
	fmul dword [f_quarter]
	fsub dword [b0]
	fsqrt
	fstp dword [t]

	;   for (i=-1; i<=1; i+=2) {
	mov ecx, dword [i_n_one]
.L3
	;      d0 = -0.5*z + i*t;			/* coeffs. of quadratic factor */
	fld dword [f_n_half]
	fmul dword [z]
	fld dword [i]
	fmul dword [t]
	faddp st1, st0
	fstp dword [d0]
	;      d1 = (t!=0.0)? -i*0.5*b1/t : i*mSqrt(-z - b2);
	fild dword [i]
	fld dword [t]
	fldz
	fcompp
	fnstsw ax
	;; if t == 0.0 jump to .L4
	;; if C3 is set, jump to .L4
	;; if ah & C3 is not-zero, jump to .L4
	and ah, 64
	jnz .L4

	fchs
	fmul dword [f_half]
	fmul dword [b1]
	fdiv dword [t]
	jmp .L5

.L4:
	fld dword [z]
	fchs
	fsub dword [b2]
	fsqrt
	fmulp st1, st0

.L5:
	fstp dword [d1]

	;      h = 0.25 * square(d1) - d0;
	fld dword [d1]
	fmul st0, st0
	fmul dword [f_quarter]
	fsub dword [d0]
	fst dword [h]

	;      if (h>=0.0) {
	;         h = mSqrt(h);
	;         nreal += 2;
	;         *px = -0.5*d1 - h - w;
	;	  px++;
	;         *px = -0.5*d1 + h - w;
	;	  px++;
	;      }
	fldz
	fcomp st1
	; if h < 0.0, jump to .L6
	; if C0 is 0, jump to .L6
	; if ( ah & 0x1 ) == 0.0, jump to .L6
	fnstsw ax
	and ah, 0x1
	jz .L6

	fsqrt
	fstp dword [h]
	add dword [nreal], 2

	fld dword [d1]
	fmul dword [f_n_half]
	fsub dword [h]
	fsub dword [w]
	fstp dword [px]
	mov eax, dword [px]
	add eax, 4
	mov dword [px], eax
	fld dword [d1]
	fmul dword [f_n_half]
	fadd dword [h]
	fsub dword [w]
	fstp dword [px]
	mov eax, dword [px]
	add eax, 4
	mov dword [px], eax

	;   }
.L6:
	add ecx, 2
	cmp ecx, 1
	jle near .L3

	;   // sort results in ascending order
	;   if (nreal == 4)
	;   {
	mov ecx, dword [nreal]
	cmp ecx, 4
	jnz near .L7

	mov ecx, x

	;      if (x[2] < x[0])
	fld dword [ecx]
	fld dword [ecx+8]
	fcom st1
	fnstsw ax
	and ah, 0x1
	jz .L8

	;         swap(x[0], x[2]);
	fstp dword [ecx]
	fstp dword [ecx+8]
	jmp .L9

.L8:
	fstp st0
	fstp st0

.L9:
	;      if (x[3] < x[1])
	fld dword [ecx+4]
	fld dword [ecx+12]
	fcom st1
	fnstsw ax
	and ah, 0x1
	jz .L10

	;         swap(x[1], x[3]);
	fstp dword [ecx+4]
	fstp dword [ecx+12]
	jmp .L11

.L10:
	fstp st0
	fstp st0

.L11:
	;      if (x[1] < x[0])
	fld dword [ecx]
	fld dword [ecx+4]
	fcom st1
	fnstsw ax
	and ah, 0x1
	jz .L12

	;         swap(x[0], x[1]);
	fstp dword [ecx]
	fstp dword [ecx+4]
	jmp .L13

.L12:
	fstp st0
	fstp st0

.L13:
	;      if (x[3] < x[2])
	fld dword [ecx+12]
	fld dword [ecx+8]
	fcom st1
	fnstsw ax
	and ah, 0x1
	jz .L14

	;         swap(x[2], x[3]);
	fstp dword [ecx+12]
	fstp dword [ecx+8]
	jmp .L15

.L14:
	fstp st0
	fstp st0

.L15:
	;      if (x[2] < x[1])
	fld dword [ecx+4]
	fld dword [ecx+8]
	fcom st1
	fnstsw ax
	and ah, 0x1
	jz .L16

	;         swap(x[1], x[2]);
	fstp dword [ecx+4]
	fstp dword [ecx+8]
	jmp .L17

.L16:
	fstp st0
	fstp st0

.L17
	;   }
	;

.L7
	;   return(nreal);
	mov eax, dword [nreal]

.L2:
	pop ebp
	ret
