;
; NASM version of optimized funcs in mMath_C
;

; The following funcs are included:
;  m_ceil_ASM, m_ceilD_ASM, m_floor_ASM, m_floorD_ASM
;  m_fmod_ASM, m_fmodD_ASM, m_mulDivS32_ASM, m_mulDivU32_ASM
;  m_sincos_ASM, m_sincosD_ASM

; The other funcs from mMath_C were determined to compile into fast
;  code using MSVC --Paul Bowman


segment .data


temp_int64			dq		0.0
const_0pt5_D		dq		0.4999999999995
temp_int32			dd		0
const_0pt5			dd		0.49999995
const_neg1			dd		-1.0


segment .text


%define rnd_adjD	qword [const_0pt5_D]
%define rnd_adj		dword [const_0pt5]


%define val		dword [esp+4]
%define val64	qword [esp+4]
;
; static F32 m_ceil_ASM(F32 val)
;
%ifdef __linux
global m_ceil_ASM
m_ceil_ASM:
%else
global _m_ceil_ASM
_m_ceil_ASM:
%endif
    fld		val
    fadd	rnd_adj
    fistp	qword [temp_int64]
    fild	qword [temp_int64]
	ret

;
; static F64 m_ceilD_ASM(F64 val64)
;
%ifdef __linux
global m_ceilD_ASM
m_ceilD_ASM:
%else
global _m_ceilD_ASM
_m_ceilD_ASM:
%endif
    fld		val64
    fadd	rnd_adjD
    fistp	qword [temp_int64]
    fild	qword [temp_int64]
	ret

; 
; static F32 m_floor_ASM(F32 val)
; 
%ifdef __linux
global m_floor_ASM
m_floor_ASM:
%else
global _m_floor_ASM
_m_floor_ASM:
%endif
    fld		val
    fsub	rnd_adj
    fistp	qword [temp_int64]
    fild	qword [temp_int64] 
	ret


;
; static F32 m_floorD_ASM( F64 val64 )
;
%ifdef __linux
global m_floorD_ASM
m_floorD_ASM:
%else
global _m_floorD_ASM
_m_floorD_ASM:
%endif
    fld		val64
    fsub	rnd_adjD
    fistp	qword [temp_int64]
    fild	qword [temp_int64] 
	ret



%define arg_a		dword [esp+4]
%define arg_b		dword [esp+8]
%define arg_c		dword [esp+12]

;
; static S32 m_mulDivS32_ASM( S32 a, S32 b, S32 c )
;
;    // Note: this returns different (but correct) values than the C
;    //  version.  C code must be overflowing...returns -727
;    //  if a b and c are 1 million, for instance.  This version returns
;    //  1 million.
; return (S32) ((S64)a*(S64)b) / (S64)c;
;
%ifdef __linux
global m_mulDivS32_ASM
m_mulDivS32_ASM:
%else
global _m_mulDivS32_ASM
_m_mulDivS32_ASM:
%endif
    mov     eax, arg_a
    imul    arg_b
    idiv    arg_c
	ret

;
; static U32 m_mulDivU32_ASM( U32 a, U32 b, U32 c )
;
;    // Note: again, C version overflows
;
%ifdef __linux
global m_mulDivU32_ASM
m_mulDivU32_ASM:
%else
global _m_mulDivU32_ASM
_m_mulDivU32_ASM:
%endif
    mov     eax, arg_a
    mul     arg_b
    div     arg_c
	ret



; val is already defined above to be esp+4
%define		modulo	dword [esp+8]


;
; static F32 m_fmod_ASM(F32 val, F32 modulo)
;
%ifdef __linux
global m_fmod_ASM
m_fmod_ASM:
%else
global _m_fmod_ASM
_m_fmod_ASM:
%endif
    mov     eax, val
    fld     modulo
    fabs
    fld     val
    fabs
    fdiv    st0, st1
    fld     st0
    fsub	rnd_adj
    fistp   qword [temp_int64]
    fild    qword [temp_int64]
    fsubp   st1, st0
    fmulp   st1, st0

;    // sign bit can be read as integer high bit, 
;    //  as long as # isn't 0x80000000
    cmp     eax, 0x80000000
    jbe     notneg

    fmul    dword [const_neg1]

notneg:
	ret


%define val64hi		dword [esp+8]
%define val64		qword [esp+4]
%define modulo64	qword [esp+12]

;
; static F32 m_fmodD_ASM(F64 val, F64 modulo)
;
%ifdef __linux
global m_fmodD_ASM
m_fmodD_ASM:
%else
global _m_fmodD_ASM
_m_fmodD_ASM:
%endif
    mov     eax, val64hi
    fld     modulo64
    fabs
    fld     val64
    fabs
    fdiv    st0, st1
    fld     st0
    fsub	rnd_adjD
    fistp   qword [temp_int64]
    fild    qword [temp_int64]
    fsubp   st1, st0
    fmulp   st1, st0

;    // sign bit can be read as integer high bit, 
;    //  as long as # isn't 0x80000000
    cmp     eax, 0x80000000
    jbe     notnegD

    fmul    dword [const_neg1]

notnegD:
	ret

	 

%define angle		dword [esp+4]
%define res_sin		dword [esp+8]
%define res_cos		dword [esp+12]

;
;static void m_sincos_ASM( F32 angle, F32 *s, F32 *c )
;
%ifdef __linux
global m_sincos_ASM
m_sincos_ASM:
%else
global _m_sincos_ASM
_m_sincos_ASM:
%endif
    mov     eax, res_cos
    fld     angle
    fsincos
    fstp    dword [eax]
    mov     eax, res_sin
    fstp    dword [eax]
	ret



%define angle64		qword [esp+4]
%define res_sin64	dword [esp+12]
%define res_cos64	dword [esp+16]
;
;static void m_sincosD_ASM( F64 angle, F64 *s, F64 *c )
;
%ifdef __linux
global m_sincosD_ASM
m_sincosD_ASM:
%else
global _m_sincosD_ASM
_m_sincosD_ASM:
%endif
    mov     eax, res_cos64
    fld     angle64
    fsincos
    fstp    qword [eax]
    mov     eax, res_sin64
    fstp    qword [eax]
	ret



