;
; Assembler FPU fun. Naive implementations
;
; TODO:
; - pipeline with fxch
; - remove redundant loads by leaving on stack
;

segment .data

; scratch array
array times 16 dd 0
; inverse of determinant
inverse_determinant dd 0


segment .text

;
; void m_quatF_set_matF_ASM( F32 x, F32 y, F32 z, F32 w, F32* m )
;

%define in_x [ebp+8]
%define in_y [ebp+12]
%define in_z [ebp+16]
%define in_w [ebp+20]
%define in_m [ebp+24]

%define xs [eax+0]
%define ys [eax+4]
%define zs [eax+8]
%define wx [eax+12]
%define wy [eax+16]
%define wz [eax+20]
%define xx [eax+24]
%define xy [eax+28]
%define xz [eax+32]
%define yy [eax+36]
%define yz [eax+40]
%define zz [eax+44]

global m_quatF_set_matF_ASM

m_quatF_set_matF_ASM:

	push ebp
	mov ebp, esp

	; f -> eax, m -> ecx
	mov eax, array
	mov ecx, in_m

	; xs = x * 2.0f
	fld dword in_x
	fadd st0, st0
	fstp dword xs

	; ys = y * 2.0f
	fld dword in_y
	fadd st0, st0
	fstp dword ys

	; zs = z * 2.0f
	fld dword in_z
	fadd st0, st0
	fstp dword zs

	; load w
	fld dword in_w

	; wx = w * xs
	fld dword xs
	fmul st0, st1
	fstp dword wx

	; wy = w * ys
	fld dword ys
	fmul st0, st1
	fstp dword wy

	; wz = w * zs
	fld dword zs
	fmul st0, st1
	fstp dword wz

	; pop w
	fstp st0

	; load x
	fld dword in_x

	; xx = x * xs
	fld dword xs
	fmul st0, st1
	fstp dword xx

	; xy = x * ys
	fld dword ys
	fmul st0, st1
	fstp dword xy

	; xz = x * zs
	fld dword zs
	fmul st0, st1
	fstp dword xz

	; pop x
	fstp st0

	; don't bother loading y once, the pop negates

	; yy = y * ys
	fld dword in_y
	fld dword ys
	fmulp st1, st0
	fstp dword yy

	; yz = y * zs
	fld dword in_y
	fld dword zs
	fmulp st1, st0
	fstp dword yz

	; and forget z

	; zz = z * zs
	fld dword in_z
	fld dword zs
	fmulp st1, st0
	fstp dword zz

	; m[0] = 1.0f - ( yy + zz )
	fld1 
	fld dword yy
	fld dword zz
	faddp st1, st0
	fsubp st1, st0
	fstp dword [ecx+0] 

	; m[4] = xy - wz 
	fld dword xy
	fld dword wz
	fsubp st1, st0
	fstp dword [ecx+16]

	; m[8] = xz + wy 
	fld dword xz
	fld dword wy
	faddp st1, st0
	fstp dword [ecx+32]

	; m[12] = 0.0f
	fldz 
	fstp dword [ecx+48]

	; m[1] = xy + wz 
	fld dword xy
	fld dword wz
	faddp st1, st0
	fstp dword [ecx+4]

	; m[5] = 1.0f - ( xx + zz )
	fld1 
	fld dword xx
	fld dword zz
	faddp st1, st0
	fsubp st1, st0
	fstp dword [ecx+20]

	; m[9] = yz - wx 
	fld dword yz
	fld dword wx
	fsubp st1, st0
	fstp dword [ecx+36]

	; m[13] = 0.0f
	fldz 
	fstp dword [ecx+52]

	; m[2] = xz - wy 
	fld dword xz
	fld dword wy
	fsubp st1, st0
	fstp dword [ecx+8]

	; m[6] = yz + wx 
	fld dword yz
	fld dword wx
	faddp st1, st0
	fstp dword [ecx+24]

	; m[10] = 1.0f - ( xx + yy )
	fld1
	fld dword xx
	fld dword yy
	faddp st1, st0
	fsubp st1, st0
	fstp dword [ecx+40]

	; m[14] = 0.0f 
	; m[3] = 0.0f
	; m[7] = 0.0f
	; m[11] = 0.0f
	fldz 
	fst dword [ecx+56]
	fst dword [ecx+12]
	fst dword [ecx+28]
	fstp dword [ecx+44]

	; m[15] = 1.0f
	fld1 
	fstp dword [ecx+60]

	pop ebp
	ret


;
; F32 m_matF_determinant_ASM( const F32* m )
;

global m_matF_determinant_ASM

m_matF_determinant_ASM:

%define in_m [ebp+8]

	push ebp
	mov ebp, esp

	mov eax, in_m

	fld dword [eax+20]
	fld dword [eax+40]
	fmulp st1, st0

	fld dword [eax+24]
	fld dword [eax+36]
	fmulp st1, st0

	fsubp st1, st0
	fld dword [eax+0]
	fmulp st1, st0

	fld dword [eax+8]
	fld dword [eax+36]
	fmulp st1, st0

	fld dword [eax+4]
	fld dword [eax+40]
	fmulp st1, st0

	fsubp st1, st0
	fld dword [eax+16]
	fmulp st1, st0

	fld dword [eax+4]
	fld dword [eax+24]
	fmulp st1, st0

	fld dword [eax+8]
	fld dword [eax+20]
	fmulp st1, st0

	fsubp st1, st0
	fld dword [eax+32]
	fmulp st1, st0

	faddp st1, st0
	faddp st1, st0

	; leave it on the FPU stack

	pop ebp
	ret


;
; void m_matF_adjoint_ASM( F32* dst, const F32* src, F32 invDet )
;

%define in_dst [ebp+8]
%define in_src [ebp+12]
%define in_invDet [ebp+16]

global m_matF_adjoint_ASM

m_matF_adjoint_ASM:

	push ebp
	mov ebp, esp

	mov edx, in_dst
	mov eax, in_src

	; dst[0] = (src[5] * src[10]- src[6] * src[9]) * invDet;
	fld dword [eax+20]
	fld dword [eax+40]
	fmulp st1, st0

	fld dword [eax+24]
	fld dword [eax+36]
	fmulp st1, st0

	fsubp st1, st0
	fmul dword in_invDet
	fstp dword [edx+0]
	
	; dst[1] = (src[9] * src[2] - src[10]* src[1]) * invDet;
	fld dword [eax+36]
	fld dword [eax+8]
	fmulp st1, st0

	fld dword [eax+40]
	fld dword [eax+4]
	fmulp st1, st0

	fsubp st1, st0
	fmul dword in_invDet
	fstp dword [edx+4]
	
	; dst[2] = (src[1] * src[6] - src[2] * src[5]) * invDet;
	fld dword [eax+4]
	fld dword [eax+24]
	fmulp st1, st0

	fld dword [eax+8]
	fld dword [eax+20]
	fmulp st1, st0

	fsubp st1, st0
	fmul dword in_invDet
	fstp dword [edx+8]
	

	; dst[4] = (src[6] * src[8] - src[4] * src[10])* invDet;
	fld dword [eax+24]
	fld dword [eax+32]
	fmulp st1, st0

	fld dword [eax+16]
	fld dword [eax+40]
	fmulp st1, st0

	fsubp st1, st0
	fmul dword in_invDet
	fstp dword [edx+16]

	; dst[5] = (src[10]* src[0] - src[8] * src[2]) * invDet;
	fld dword [eax+40]
	fld dword [eax+0]
	fmulp st1, st0

	fld dword [eax+32]
	fld dword [eax+8]
	fmulp st1, st0

	fsubp st1, st0
	fmul dword in_invDet
	fstp dword [edx+20]

	; dst[6] = (src[2] * src[4] - src[0] * src[6]) * invDet;
	fld dword [eax+8]
	fld dword [eax+16]
	fmulp st1, st0

	fld dword [eax+0]
	fld dword [eax+24]
	fmulp st1, st0

	fsubp st1, st0
	fmul dword in_invDet
	fstp dword [edx+24]


	; dst[8] = (src[4] * src[9] - src[5] * src[8]) * invDet;
	fld dword [eax+16]
	fld dword [eax+36]
	fmulp st1, st0

	fld dword [eax+20]
	fld dword [eax+32]
	fmulp st1, st0

	fsubp st1, st0
	fmul dword in_invDet
	fstp dword [edx+32]

	; dst[9] = (src[8] * src[1] - src[9] * src[0]) * invDet;
	fld dword [eax+32]
	fld dword [eax+4]
	fmulp st1, st0

	fld dword [eax+36]
	fld dword [eax+0]
	fmulp st1, st0

	fsubp st1, st0
	fmul dword in_invDet
	fstp dword [edx+36]

	; dst[10]= (src[0] * src[5] - src[1] * src[4]) * invDet;
	fld dword [eax+0]
	fld dword [eax+20]
	fmulp st1, st0

	fld dword [eax+4]
	fld dword [eax+16]
	fmulp st1, st0

	fsubp st1, st0
	fmul dword in_invDet
	fstp dword [edx+40]

	pop ebp
	ret

;
; m_matF_x_vectorF_ASM( const F32* m, const F32* v, F32* r )
;

%define in_m [ebp+8]
%define in_v [ebp+12]
%define in_r [ebp+16]

global m_matF_x_vectorF_ASM

m_matF_x_vectorF_ASM:

	push ebp
	mov ebp, esp

	mov eax, in_m
	mov ecx, in_v
	mov edx, in_r

	; vresult[0] = m[0]*v[0] + m[1]*v[1] + m[2]*v[2];   
	fld dword [eax]
	fld dword [eax+4]
	fmul dword [ecx+4]
	fxch
	fmul dword [ecx]
	fld dword [eax+8]
	fmul dword [ecx+8]
	fxch st2
	faddp st1, st0
	faddp st1, st0
	fstp dword [edx]

	; vresult[1] = m[4]*v[0] + m[5]*v[1] + m[6]*v[2];   
	fld dword [eax+16]
	fld dword [eax+20]
	fmul dword [ecx+4]
	fxch
	fmul dword [ecx]
	fld dword [eax+24]
	fmul dword [ecx+8]
	fxch st2
	faddp st1, st0
	faddp st1, st0
	fstp dword [edx+4]

	; vresult[2] = m[8]*v[0] + m[9]*v[1] + m[10]*v[2];   
	fld dword [eax+32]
	fld dword [eax+36]
	fmul dword [ecx+4]
	fxch
	fmul dword [ecx]
	fld dword [eax+40]
	fmul dword [ecx+8]
	fxch st2
	faddp st1, st0
	faddp st1, st0
	fstp dword [edx+8]

	pop ebp
	ret

;
; void m_matF_inverse_ASM( F32* m )
;

%define in_m [ebp+8]

global m_matF_inverse_ASM

m_matF_inverse_ASM:

	push ebp
	mov ebp, esp

	; calculate determinant
	push dword in_m
	call m_matF_determinant_ASM
	add esp, byte 4

	; calculate inverse of determinant
	fld1
	fxch
	fdivp st1, st0
	fstp dword [inverse_determinant]

	; calculate adjoint matrix
	push dword [inverse_determinant]
	push dword in_m
	push dword array
	call m_matF_adjoint_ASM
	add esp, byte 12

	mov eax, array
	mov edx, in_m

	; assign back
	fld dword [eax]
	fstp dword [edx]
	fld dword [eax+4]
	fstp dword [edx+4]
	fld dword [eax+8]
	fstp dword [edx+8]

	fld dword [eax+16]
	fstp dword [edx+16]
	fld dword [eax+20]
	fstp dword [edx+20]
	fld dword [eax+24]
	fstp dword [edx+24]

	fld dword [eax+32]
	fstp dword [edx+32]
	fld dword [eax+36]
	fstp dword [edx+36]
	fld dword [eax+40]
	fstp dword [edx+40]

	; invert the translation
	fld dword [edx+12]
	fchs
	fstp dword [eax]
	fld dword [edx+28]
	fchs
	fstp dword [eax+4]
	fld dword [edx+44]
	fchs
	fstp dword [eax+8]

	mov ecx, eax
	add ecx, 16
	push dword ecx
	push dword eax
	push edx
	call m_matF_x_vectorF_ASM
	add esp, byte 12
	mov eax, array
	mov edx, in_m

	fld dword [eax+16]
	fstp dword [edx+12]
	fld dword [eax+20]
	fstp dword [edx+28]
	fld dword [eax+24]
	fstp dword [edx+44]

	pop ebp
	ret

;
; void m_matF_affineInverse_ASM( F32* m )
;

%define in_m [ebp+8]

global m_matF_affineInverse_ASM

m_matF_affineInverse_ASM:

	push ebp
	mov ebp, esp

	mov eax, array
	mov edx, in_m

	; copy to temp
	fld dword [edx]
	fstp dword [eax]

	fld dword [edx+4]
	fstp dword [eax+4]

	fld dword [edx+8]
	fstp dword [eax+8]

	fld dword [edx+12]
	fstp dword [eax+12]

	fld dword [edx+16]
	fstp dword [eax+16]

	fld dword [edx+20]
	fstp dword [eax+20]

	fld dword [edx+24]
	fstp dword [eax+24]

	fld dword [edx+28]
	fstp dword [eax+28]

	fld dword [edx+32]
	fstp dword [eax+32]

	fld dword [edx+36]
	fstp dword [eax+36]

	fld dword [edx+40]
	fstp dword [eax+40]

	fld dword [edx+44]
	fstp dword [eax+44]

	fld dword [edx+48]
	fstp dword [eax+48]

	fld dword [edx+52]
	fstp dword [eax+52]

	fld dword [edx+56]
	fstp dword [eax+56]

	fld dword [edx+60]
	fstp dword [eax+60]

	; switcheroos
	fld dword [eax+16]
	fstp dword [edx+4]
	fld dword [eax+4]
	fstp dword [edx+16]
	fld dword [eax+32]
	fstp dword [edx+8]
	fld dword [eax+8]
	fstp dword [edx+32]
	fld dword [eax+36]
	fstp dword [edx+24]
	fld dword [eax+24]
	fstp dword [edx+36]

	; m[3]  = -(temp[0]*temp[3] + temp[4]*temp[7] + temp[8]*temp[11]);
	fld dword [eax+0]
	fld dword [eax+16]
	fmul dword [eax+28]
	fxch
	fmul dword [eax+12]
	fld dword [eax+32]
	fmul dword [eax+44]
	fxch st2
	faddp st1, st0
	faddp st1, st0
	fchs
	fstp dword [edx+12]

	; m[7]  = -(temp[1]*temp[3] + temp[5]*temp[7] + temp[9]*temp[11]);
	fld dword [eax+4]
	fld dword [eax+20]
	fmul dword [eax+28]
	fxch
	fmul dword [eax+12]
	fld dword [eax+36]
	fmul dword [eax+44]
	fxch st2
	faddp st1, st0
	faddp st1, st0
	fchs
	fstp dword [edx+28]

	; m[11] = -(temp[2]*temp[3] + temp[6]*temp[7] + temp[10]*temp[11]);
	fld dword [eax+8]
	fld dword [eax+24]
	fmul dword [eax+28]
	fxch
	fmul dword [eax+12]
	fld dword [eax+40]
	fmul dword [eax+44]
	fxch st2
	faddp st1, st0
	faddp st1, st0
	fchs
	fstp dword [edx+44]

	pop ebp
	ret

;
; void m_matF_x_matF_ASM( const F32* a, const F32* b, F32* mresult )
;

%define in_a [ebp+8]
%define in_b [ebp+12]
%define in_m [ebp+16]

global m_matF_x_matF_ASM

m_matF_x_matF_ASM:

	push ebp
	mov ebp, esp

	mov edx, in_a
	mov eax, in_b
	mov ecx, in_m

	; mresult[0] = a[0]*b[0] + a[1]*b[4] + a[2]*b[8] + a[3]*b[12];
	fld dword [edx]
	fld dword [edx+4]
	fxch
	fmul dword [eax]
	fxch
	fmul dword [eax+16]
	faddp st1, st0

	fld dword [edx+8]
	fmul dword [eax+32]
	faddp st1, st0

	fld dword [edx+12]
	fmul dword [eax+48]
	faddp st1, st0

	fstp dword [ecx]

	; mresult[1] = a[0]*b[1] + a[1]*b[5] + a[2]*b[9] + a[3]*b[13];
	fld dword [edx]
	fld dword [edx+4]
	fxch
	fmul dword [eax+4]
	fxch
	fmul dword [eax+20]
	faddp st1, st0

	fld dword [edx+8]
	fmul dword [eax+36]
	faddp st1, st0

	fld dword [edx+12]
	fmul dword [eax+52]
	faddp st1, st0

	fstp dword [ecx+4]

	; mresult[2] = a[0]*b[2] + a[1]*b[6] + a[2]*b[10] + a[3]*b[14];
	fld dword [edx]
	fld dword [edx+4]
	fxch
	fmul dword [eax+8]
	fxch
	fmul dword [eax+24]
	faddp st1, st0

	fld dword [edx+8]
	fmul dword [eax+40]
	faddp st1, st0

	fld dword [edx+12]
	fmul dword [eax+56]
	faddp st1, st0

	fstp dword [ecx+8]

	; mresult[3] = a[0]*b[3] + a[1]*b[7] + a[2]*b[11] + a[3]*b[15];
	fld dword [edx]
	fld dword [edx+4]
	fxch
	fmul dword [eax+12]
	fxch
	fmul dword [eax+28]
	faddp st1, st0

	fld dword [edx+8]
	fmul dword [eax+44]
	faddp st1, st0

	fld dword [edx+12]
	fmul dword [eax+60]
	faddp st1, st0

	fstp dword [ecx+12]

	; mresult[4] = a[4]*b[0] + a[5]*b[4] + a[6]*b[8] + a[7]*b[12];
	fld dword [edx+16]
	fld dword [edx+20]
	fxch
	fmul dword [eax]
	fxch
	fmul dword [eax+16]
	faddp st1, st0

	fld dword [edx+24]
	fmul dword [eax+32]
	faddp st1, st0

	fld dword [edx+28]
	fmul dword [eax+48]
	faddp st1, st0

	fstp dword [ecx+16]

	; mresult[5] = a[4]*b[1] + a[5]*b[5] + a[6]*b[9] + a[7]*b[13];
	fld dword [edx+16]
	fld dword [edx+20]
	fxch
	fmul dword [eax+4]
	fxch
	fmul dword [eax+20]
	faddp st1, st0

	fld dword [edx+24]
	fmul dword [eax+36]
	faddp st1, st0

	fld dword [edx+28]
	fmul dword [eax+52]
	faddp st1, st0

	fstp dword [ecx+20]

	; mresult[6] = a[4]*b[2] + a[5]*b[6] + a[6]*b[10] + a[7]*b[14];
	fld dword [edx+16]
	fld dword [edx+20]
	fxch
	fmul dword [eax+8]
	fxch
	fmul dword [eax+24]
	faddp st1, st0

	fld dword [edx+24]
	fmul dword [eax+40]
	faddp st1, st0

	fld dword [edx+28]
	fmul dword [eax+56]
	faddp st1, st0

	fstp dword [ecx+24]

	; mresult[7] = a[4]*b[3] + a[5]*b[7] + a[6]*b[11] + a[7]*b[15];
	fld dword [edx+16]
	fld dword [edx+20]
	fxch
	fmul dword [eax+12]
	fxch
	fmul dword [eax+28]
	faddp st1, st0

	fld dword [edx+24]
	fmul dword [eax+44]
	faddp st1, st0

	fld dword [edx+28]
	fmul dword [eax+60]
	faddp st1, st0

	fstp dword [ecx+28]

	; mresult[8] = a[8]*b[0] + a[9]*b[4] + a[10]*b[8] + a[11]*b[12];
	fld dword [edx+32]
	fld dword [edx+36]
	fxch
	fmul dword [eax]
	fxch
	fmul dword [eax+16]
	faddp st1, st0

	fld dword [edx+40]
	fmul dword [eax+32]
	faddp st1, st0

	fld dword [edx+44]
	fmul dword [eax+48]
	faddp st1, st0

	fstp dword [ecx+32]

	; mresult[9] = a[8]*b[1] + a[9]*b[5] + a[10]*b[9] + a[11]*b[13];
	fld dword [edx+32]
	fld dword [edx+36]
	fxch
	fmul dword [eax+4]
	fxch
	fmul dword [eax+20]
	faddp st1, st0

	fld dword [edx+40]
	fmul dword [eax+36]
	faddp st1, st0

	fld dword [edx+44]
	fmul dword [eax+52]
	faddp st1, st0

	fstp dword [ecx+36]

	; mresult[10]= a[8]*b[2] + a[9]*b[6] + a[10]*b[10] + a[11]*b[14];
	fld dword [edx+32]
	fld dword [edx+36]
	fxch
	fmul dword [eax+8]
	fxch
	fmul dword [eax+24]
	faddp st1, st0

	fld dword [edx+40]
	fmul dword [eax+40]
	faddp st1, st0

	fld dword [edx+44]
	fmul dword [eax+56]
	faddp st1, st0

	fstp dword [ecx+40]

	; mresult[11]= a[8]*b[3] + a[9]*b[7] + a[10]*b[11] + a[11]*b[15];
	fld dword [edx+32]
	fld dword [edx+36]
	fxch
	fmul dword [eax+12]
	fxch
	fmul dword [eax+28]
	faddp st1, st0

	fld dword [edx+40]
	fmul dword [eax+44]
	faddp st1, st0

	fld dword [edx+44]
	fmul dword [eax+60]
	faddp st1, st0

	fstp dword [ecx+44]

	; mresult[12]= a[12]*b[0] + a[13]*b[4] + a[14]*b[8] + a[15]*b[12];
	fld dword [edx+48]
	fld dword [edx+52]
	fxch
	fmul dword [eax]
	fxch
	fmul dword [eax+16]
	faddp st1, st0

	fld dword [edx+56]
	fmul dword [eax+32]
	faddp st1, st0

	fld dword [edx+60]
	fmul dword [eax+48]
	faddp st1, st0

	fstp dword [ecx+48]

	; mresult[13]= a[12]*b[1] + a[13]*b[5] + a[14]*b[9] + a[15]*b[13];
	fld dword [edx+48]
	fld dword [edx+52]
	fxch
	fmul dword [eax+4]
	fxch
	fmul dword [eax+20]
	faddp st1, st0

	fld dword [edx+56]
	fmul dword [eax+36]
	faddp st1, st0

	fld dword [edx+60]
	fmul dword [eax+52]
	faddp st1, st0

	fstp dword [ecx+52]

	; mresult[14]= a[12]*b[2] + a[13]*b[6] + a[14]*b[10] + a[15]*b[14];
	fld dword [edx+48]
	fld dword [edx+52]
	fxch
	fmul dword [eax+8]
	fxch
	fmul dword [eax+24]
	faddp st1, st0

	fld dword [edx+56]
	fmul dword [eax+40]
	faddp st1, st0

	fld dword [edx+60]
	fmul dword [eax+56]
	faddp st1, st0

	fstp dword [ecx+56]

	; mresult[15]= a[12]*b[3] + a[13]*b[7] + a[14]*b[11] + a[15]*b[15];
	fld dword [edx+48]
	fld dword [edx+52]
	fxch
	fmul dword [eax+12]
	fxch
	fmul dword [eax+28]
	faddp st1, st0

	fld dword [edx+56]
	fmul dword [eax+44]
	faddp st1, st0

	fld dword [edx+60]
	fmul dword [eax+60]
	faddp st1, st0

	fstp dword [ecx+60]

	pop ebp
	ret
