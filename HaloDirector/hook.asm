.data
p_DollyFunc qword 0
p_DollyOri qword 0
DollyOri qword 0

p_CameraPointer qword 0
CameraPointer qword 0
p_CameraOri qword 0
CameraOri qword 0

p_UninitFunc qword 0
p_UninitOri qword 0

tick dword 0
.code

GetTeb proc
	mov rax, qword ptr gs:[58h]
	ret
GetTeb endp

SetDolly proc
	mov [p_DollyFunc], rcx
	mov [p_DollyOri], rdx
	ret
SetDolly endp

SetCam proc
	mov [p_CameraPointer], rcx
	mov [p_CameraOri], rdx
	ret
SetCam endp

SetUninit proc
	mov [p_UninitFunc], rcx
	mov [p_UninitOri], rdx
	ret
SetUninit endp

HookDolly proc
	mov tick, ebx
	mov rax, [p_DollyOri]
	mov rax, [rax]
	push rax
	jmp [p_DollyFunc]
HookDolly endp

HookCamera proc
	mov rax, [p_CameraPointer]
	mov [rax], rsi
	mov rax, [p_CameraOri]
	jmp qword ptr [rax]
HookCamera endp

HookUninit proc
	mov rax, [p_UninitOri]
	mov rax, [rax]
	push rax
	jmp [p_UninitFunc]
HookUninit endp

GetTick proc
	mov eax, tick
	ret
GetTick endp

end