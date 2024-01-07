;-----------------------------------------------------------------------------
; V12 Engine
;
; Copyright (c) 2001 GarageGames.Com
; Portions Copyright (c) 2001 by Sierra Online, Inc.
;-----------------------------------------------------------------------------


segment .text

; this is a nice macro Rick
; syntax: export_fn <function name>
%macro export_fn 1
   %ifdef LINUX
   ; No underscore needed for ELF object files
   global %1
   %1:
   %else
   global _%1
   _%1:
   %endif
%endmacro

; so is this
;%define arg(x) [esp+(x*4)]

; void isCPUIDSupported(char *vendor, U32 *properties, U32 *processor);
export_fn isCPUIDSupported

;   mov          ebp, esp
   push         ebp
   push         ebx
   push         edx
   push         ecx
;   mov           ecx, arg(1)
;   mov           edx, arg(2)
;   mov           eax, arg(3)
   pushfd
   pushfd                        ; save EFLAGS to stack
   pop          eax              ; move EFLAGS into EAX
   mov          ebx, eax
   xor          eax, 0x200000    ; flip bit 21
   push         eax
   popfd                         ; restore EFLAGS
   pushfd
   pop          eax
   cmp          eax, ebx
   jz           EXIT             ; doesn't support CPUID instruction

   ;
   ; get vendor information using CPUID eax == 0
   xor          eax, eax
   cpuid

;   mov          dword [vendor], ebx
;   mov          dword [vendor+4], edx
;   mov          dword [vendor+8], ecx
   mov          dword [esp+8], ebx
   mov          dword [esp+12], edx
   mov          dword [esp+16], ecx
   popfd
   pop          ecx
   pop          edx
   pop          ebx
   leave
;   pop          ebp
;   mov          esp, ebp
   ret
;jz EXIT
   ; get generic extended CPUID info
   mov          eax, 1
   cpuid                         ; eax=1, so cpuid queries feature information

   and          eax, 0x0FF0
   mov          [esp+20], eax    ; just store the model bits
   mov          [esp+16], edx

   ; want to check for 3DNow(tm).  need to see if extended cpuid functions present.
   mov          eax, 0x80000000
   cpuid
   cmp          eax, 0x80000000
   jbe          MAYBE_3DLATER
   mov          eax, 0x80000001
   cpuid
   and          edx, 0x80000000  ; 3DNow if bit 31 set -> put bit in our properties
   or           [esp+16], edx
MAYBE_3DLATER:


EXIT:
   popfd
   pop          ecx
   pop          edx
   pop          ebx
   ret

