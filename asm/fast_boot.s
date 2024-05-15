SECTION "main", ROM0[0]

; Set Stack Pointer
ld sp, $FFFE
; Jump to $F9
jp $00F9

; Align
rept $F9 - $06
    db $00
endr

; Enable LCD
ld a, $80
ldh [$40], a
; Disable Boot-ROM
ld a, $01
ldh [$50], a