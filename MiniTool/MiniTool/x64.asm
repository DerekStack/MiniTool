.CODE
PUBLIC WritePCIByte
PUBLIC WritePCIWord
PUBLIC WritePCIDword
PUBLIC ReadPCIByte
PUBLIC ReadPCIWord
PUBLIC ReadPCIDword


;------------------------------------------------------------------------------
;  void
;  WritePCIByte (
;    unsigned int	pci_reg          // rcx
;    unsigned short	cfg_data_port    // rdx
;    unsigned char	byte_value       // r8
;    )
;------------------------------------------------------------------------------
WritePCIByte PROC FRAME
    push rax
    .ALLOCSTACK 8
    push rdx
    .ALLOCSTACK 8
    .endprolog

    cli
    mov rax, rcx  ; pci_reg
    mov dx, 0CF8h
    out dx, rax

    mov rax, r8   ; byte_value
    pop rdx       ; cfg_data_port
    out dx, al
    sti

    pop rax
    ret
WritePCIByte ENDP

;------------------------------------------------------------------------------
;  void
;  WritePCIWord (
;    unsigned int	pci_reg          // rcx
;    unsigned short	cfg_data_port    // rdx
;    unsigned short	word_value       // r8
;    )
;------------------------------------------------------------------------------
WritePCIWord PROC FRAME
    push rax
    .ALLOCSTACK 8
    push rdx
    .ALLOCSTACK 8
    .endprolog

    cli
    mov rax, rcx  ; pci_reg
    mov dx, 0CF8h
    out dx, rax

    mov rax, r8   ; byte_value
    pop rdx       ; cfg_data_port
    out dx, ax
    sti

    pop rax
    ret
WritePCIWord ENDP

;------------------------------------------------------------------------------
;  void
;  WritePCIDword (
;    unsigned int	pci_reg          // rcx
;    unsigned short	cfg_data_port    // rdx
;    unsigned int	dword_value      // r8
;    )
;------------------------------------------------------------------------------
WritePCIDword PROC FRAME
    push rax
    .ALLOCSTACK 8
    push rdx
    .ALLOCSTACK 8
    .endprolog

    cli
    mov rax, rcx  ; pci_reg
    mov dx, 0CF8h
    out dx, rax

    mov rax, r8   ; byte_value
    pop rdx       ; cfg_data_port
    out dx, eax
    sti

    pop rax
    ret
WritePCIDword ENDP



;------------------------------------------------------------------------------
;  unsigned char
;  ReadPCIByte (
;    unsigned int	pci_reg          // rcx
;    unsigned short	cfg_data_port    // rdx
;    )
;------------------------------------------------------------------------------
ReadPCIByte PROC FRAME
    push rdx
    .ALLOCSTACK 8
    .endprolog

    cli
    mov rax, rcx  ; pci_reg
    mov dx, 0CF8h
    out dx, rax

    xor rax, rax
    pop rdx       ; cfg_data_port
    in  al, dx
    sti

    ret
ReadPCIByte ENDP

;------------------------------------------------------------------------------
;  unsigned short
;  ReadPCIWord (
;    unsigned int	pci_reg          // rcx
;    unsigned short	cfg_data_port    // rdx
;    )
;------------------------------------------------------------------------------
ReadPCIWord PROC FRAME
    push rdx
    .ALLOCSTACK 8
    .endprolog

    cli
    mov rax, rcx  ; pci_reg
    mov dx, 0CF8h
    out dx, rax

    xor rax, rax
    pop rdx       ; cfg_data_port
    in  ax, dx
    sti

    ret
ReadPCIWord ENDP

;------------------------------------------------------------------------------
;  unsigned int
;  ReadPCIDword (
;    unsigned int	pci_reg          // rcx
;    unsigned short	cfg_data_port    // rdx
;    )
;------------------------------------------------------------------------------
ReadPCIDword PROC FRAME
    push rdx
    .ALLOCSTACK 8
    .endprolog

    cli
    mov rax, rcx  ; pci_reg
    mov dx, 0CF8h
    out dx, rax

    xor rax, rax
    pop rdx       ; cfg_data_port
    in  eax, dx
    sti

    ret
ReadPCIDword ENDP

END