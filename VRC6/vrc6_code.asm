;version Feb 2023
;credit to Matt Hughson for help



;values 0 or $80, for the $8000 register
;(changes in tileset mapping)

;if invert bit is 0 
;mode 0 changes $0000-$07FF
;mode 1 changes $0800-$0FFF
;mode 2 changes $1000-$13FF
;mode 3 changes $1400-$17FF
;mode 4 changes $1800-$1BFF
;mode 5 changes $1C00-$1FFF

;if invert bit is $80
;mode 0 changes $1000-$17FF
;mode 1 changes $1800-$1FFF
;mode 2 changes $0000-$03FF
;mode 3 changes $0400-$07FF
;mode 4 changes $0800-$0BFF
;mode 5 changes $0C00-$0FFF



.segment "ZEROPAGE"
    BP_BANK_8000: .res 1
;   mmc3_8000_PRG:    .res 1
;    mmc3_8001_PRG:    .res 1
;    mmc3_8000_CHR:    .res 1
;    mmc3_8001_CHR:    .res 1
    mmc3_8000: .res 1 ; backup, just in case interrupt
    vrc6_b000: .res 1
    vrc6_ptr:        .res 2 ; array for the irq parser
    vrc6_index:        .res 1 ; index to this array
    irq_done:        .res 1
    is_vblank_initial_setup: .res 1
    
;   .exportzp mmc3_8000_PRG, mmc3_8001_PRG, mmc3_8000_CHR, mmc3_8001_CHR
    .exportzp BP_BANK_8000, mmc3_8000
    .exportzp vrc6_ptr, vrc6_index, irq_done
    

.segment "STARTUP"
;needs to be mapped to the fixed bank
.export _setup_vrc6
.export _set_prg_8000, _get_prg_8000, _set_prg_c000
.export _set_chr_mode_0, _set_chr_mode_1, _set_chr_mode_2, _set_chr_mode_3
.export _set_chr_mode_4, _set_chr_mode_5, _set_chr_mode_6, _set_chr_mode_7

.export _set_mirroring, _set_wram_mode, _disable_irq
.export _set_irq_ptr, _is_irq_done
.export _disable_irq_timer,_prepare_line0_timer
.export _famistudio_init_wrapper, _famistudio_music_play_wrapper, famistudio_dpcm_bank_callback

; most of these functions only use the A register
; because some of the music code needs X unchanged

_setup_vrc6:
    pha
    lda #%10100000
    sta $b003

    lda #$0
    sta is_vblank_initial_setup
    sta $D000
    lda #$1
    sta $D001
    lda #$2
    sta $D002
    lda #$3
    sta $D003
    lda #$4
    sta $E000
    lda #$5
    sta $E001
    lda #$6
    sta $E002
    lda #$7
    sta $E003
    lda #$0
    sta $f001
    pla
    rts
; sets the bank at $8000-9fff
_set_prg_8000:
    sta BP_BANK_8000
    sta mmc3_8000 ;backup, in case of interrupt
    sta $8000
    rts

;    sta mmc3_8000 ;backup, in case of interrupt
;    sta $8000
;    pla
;    sta $8001
;    rts


; returns the current bank at $8000-9fff    
_get_prg_8000:
    lda BP_BANK_8000
    ldx #0
    rts
    
    
; sets the bank at $a000-bfff
_set_prg_c000:
    sta $c000
    rts
    
;    sta mmc3_8000 ;backup, in case of interrupt
;    sta $8000
;    pla
;    sta $8001
;    rts



; see chart in above for what these do
_set_chr_mode_0:
    sta $D000
    rts
bank_swap_common:    
    sta mmc3_8000 ;backup, in case of interrupt
    sta $8000   
    pla
    sta $8001
    rts
chr_swap_common:
    
_set_chr_mode_1:
    sta $D001
    rts
    
;    sta mmc3_8000 ;backup, in case of interrupt
;    sta $8000   
;    pla
;    sta $8001
;    rts

    
_set_chr_mode_2:
    sta $D002
    rts
    
;    sta mmc3_8000 ;backup, in case of interrupt
;    sta $8000   
;    pla
;    sta $8001
;    rts

    
_set_chr_mode_3:
    sta $D003
    rts
    
;    sta mmc3_8000 ;backup, in case of interrupt
;    sta $8000   
;    pla
;    sta $8001
;    rts

    
_set_chr_mode_4:
    sta $E000
    rts
    
;    sta mmc3_8000 ;backup, in case of interrupt
;    sta $8000   
;    pla
;    sta $8001
;    rts

    
_set_chr_mode_5:
    sta $E001
    rts
    
;    sta mmc3_8000 ;backup, in case of interrupt
;    sta $8000   
;    pla
;    sta $8001
;    rts
_set_chr_mode_6:
    sta $E002
    rts

_set_chr_mode_7:
    sta $E003
    rts
 
    
; MIRROR_VERTICAL 0
; MIRROR_HORIZONTAL 1    
_set_mirroring:
    sta $a000
    rts
    
    
; WRAM_OFF $40
; WRAM_ON $80
; WRAM_READ_ONLY $C0
_set_wram_mode:
    sta $a001
    rts
    

_disable_irq:
    ;sta $e000 ;any value
    lda #<default_array
    ldx #>default_array
    ;jmp _set_irq_ptr ; fall through
    
    
_set_irq_ptr:
;ax = pointer
    sta vrc6_ptr
    stx vrc6_ptr+1
    rts    
    
    
_is_irq_done:
    lda irq_done
    ldx #0
    rts

    
default_array: ;just an eof terminator
.byte $ff



    
irq:
    pha
    txa
    pha
    tya
    pha
    
    ;sta $e000    ; disable mmc3 irq
                ; any value will do
    sta $f002
    jsr irq_parser
    
    pla
    tay
    pla
    tax
    pla
    rti
    
    
;format
;value < 0xf0, it's a scanline count
;zero is valid, it triggers an IRQ at the end of the current line

;if >= 0xf0...
;f0 = 2000 write, next byte is write value
;f1 = 2001 write, next byte is write value
;f2-f4 unused - future TODO ?
;f5 = 2005 write, next byte is H Scroll value
;f6 = 2006 write, next 2 bytes are write values


;f7 = change CHR mode 0, next byte is write value
;f8 = change CHR mode 1, next byte is write value
;f9 = change CHR mode 2, next byte is write value
;fa = change CHR mode 3, next byte is write value
;fb = change CHR mode 4, next byte is write value
;fc = change CHR mode 5, next byte is write value

;fd = very short wait, no following byte 
;fe = short wait, next byte is quick loop value
;(for fine tuning timing of things)

;ff = end of data set

    
irq_parser:
    ldy vrc6_index
;    ldx #0
@loop:
    lda (vrc6_ptr), y ; get value from array
    iny
    cmp #$fd ;very short wait
    beq @loop
    cmp #$f2 ; xy split (to X100 Y y)
    beq @yscroll
    cmp #$fe ;fe-ff wait or exit
    bcs @wait
    
    cmp #$f0
    bcs @1
    ldx is_vblank_initial_setup
    beq @scanline
    ; if we are here, we are in the vblank initial setup, we don't want to trigger the scanline here because it will not be correct
    lda #0
    sta is_vblank_initial_setup
    dey
    sty vrc6_index
    rts
    
@1:    
    
    cmp #$f7
    bcs @chr_change
;f0-f6    
    tax
    lda (vrc6_ptr), y ; get value from array
    iny
    cpx #$f0
    bne @2
    sta $2000 ; f0
    jmp @loop
@yscroll:
    lda $2002
    ldx #$00
    lda #$04
    sta $2006
    lda (vrc6_ptr), y
    iny
    sta $2005
    and #$F8
    asl
    asl
    stx $2005
    sta $2006
    jmp @loop
@2:
    cpx #$f1
    bne @3
    sta $2001 ; f1
    jmp @loop
@3:
    cpx #$f5 
    bne @4
    ldx #4
@better_timing: ; don't change till near the end of the line
    dex
    bne @better_timing
    
    sta $2005 ; f5
    sta $2005 ; second value doesn't matter
    jmp @loop
@4:
    sta $2006 ; f6
    lda (vrc6_ptr), y ; get 2nd value from array
    iny    
    sta $2006
    jmp @loop
    
@wait: ; fe-ff wait or exit
    cmp #$ff
    beq @exit    
    lda (vrc6_ptr), y ; get value from array
    iny
    tax
    beq @loop ; if zero, just exit
@wait_loop: ; the timing of this wait could change if this crosses a page boundary
    dex
    bne @wait_loop        
    jmp @loop    

@chr_change:
;f7-fa change a CHR set
    sec
    sbc #$f7 ;should result in 0-3
    tax
    lda (vrc6_ptr), y ; get next value
    iny
    sta $d000,x
    jmp @loop
    
@scanline:
    nop ;trying to improve stability
    nop
    nop
    nop
    jsr set_scanline_count ;this terminates the set
    sty vrc6_index
    rts
    
@exit:
    sta irq_done ;value 0xff
    dey ; undo the previous iny, keep it pointed to ff
    sty vrc6_index
    rts
    
    
set_scanline_count:
; any value will do for most of these registers
    eor #$FF
    sta $f000 ;new scanline count
    lda #%00000010 
    sta $f001 ;reset and enable the timer
    cli ;make sure irqs are enabled
    rts
_disable_irq_timer:
    lda #$0
    sta $f001
    lda #$1
    sta is_vblank_initial_setup
    rts
_prepare_line0_timer:
    lda #236 ; 14 lines until line 0 more or less (let's hope irq_parser has already been run)
    sta $f000
    lda #%00000010
    sta $f001
    rts

_famistudio_init_wrapper:
    pha
    lda #$0
    sta $8000
    txa
    tay
    pla
    tax
    jsr famistudio_init
    pha
    lda #$6
    sta $8000
    pla
    rts
_famistudio_music_play_wrapper:
    pha
    lda #$0
    sta $8000
    pla
    jsr famistudio_music_play
    pha
    lda #$6
    sta $8000
    pla
    rts

famistudio_dpcm_bank_callback:
    clc
    adc #$A
    sta $C000
    rts

    
    
    