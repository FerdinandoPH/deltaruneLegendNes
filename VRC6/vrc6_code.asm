; This code is an adaptation of nesdoug's MMC3 code. I (FerdinandoPH) made some changes for it to work with the VRC6

.segment "ZEROPAGE"
    vrc6_ptr:        .res 2 ; array for the irq parser
    vrc6_index:        .res 1 ; index to this array
    irq_done:        .res 1; Flag to indicate that the IRQ array has been fully parsed
    is_vblank_initial_setup: .res 1 ; Flag to indicate that the parser is executing the commands meant to be run at Vblank
    

    .exportzp vrc6_ptr, vrc6_index, irq_done, is_vblank_initial_setup
    

.segment "STARTUP"
;needs to be mapped to the fixed bank
.export _setup_vrc6
.export _set_prg_8000, _set_prg_c000
.export _set_chr_mode_0, _set_chr_mode_1, _set_chr_mode_2, _set_chr_mode_3
.export _set_chr_mode_4, _set_chr_mode_5, _set_chr_mode_6, _set_chr_mode_7

.export  _disable_irq, _set_irq_ptr, _is_irq_done
.export _disable_irq_timer,_prepare_line0_timer
.export _famistudio_init_wrapper, _famistudio_music_play_wrapper, famistudio_dpcm_bank_callback

; most of these functions only use the A register
; because some of the music code needs X unchanged

_setup_vrc6:
    pha
    ; Right now, the supported mode is:
    ; RAM ON, CHR subject to rules, nametables from CIRAM, Vertical mirroring, Mode 0
    lda #%10100000
    sta $b003

    lda #$0
    sta $f001 ; disable the scanline IRQ
    ; Set the CHR banks to the default values
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
    pla
    rts
; sets the bank at $8000-9fff
_set_prg_8000:
    sta $8000
    rts


; sets the bank at $a000-bfff
_set_prg_c000:
    sta $c000
    rts



; set_chr_mode_X: Changes a CHR section to a specific bank
; Each tilemap is divided in 4 sections
; For example: set_chr_mode_2 with A=7 will set the tiles of tilemap0, 128-192 (3rd section) to the bank 7
_set_chr_mode_0:
    sta $D000
    rts

_set_chr_mode_1:
    sta $D001
    rts
    
_set_chr_mode_2:
    sta $D002
    rts
    

_set_chr_mode_3:
    sta $D003
    rts
    
_set_chr_mode_4:
    sta $E000
    rts
    
_set_chr_mode_5:
    sta $E001
    rts
    
_set_chr_mode_6:
    sta $E002
    rts

_set_chr_mode_7:
    sta $E003
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

; How the IRQ works in VRC6:
; In VRC6, the scanline IRQ is actually a timer IRQ that counts CPU cycles.
; However, it has a mode called "scanline mode" that only increases the timer when the amount of CPU cycles passed is equivalent to a scanline.
; Currently, only the scanline mode is supported.
; There are 3 registers:
; $f000: The latch value, it increases by 1 every scanline, and triggers an IRQ when it reaches FF.
; $f001: The control register, it has a mode bit (cycle or scanline), an enable bit, and a "behavior after IRQ" bit (1 = reset and enable, 0 = disable).
; $f002: The IRQ ACK register, before re-enabling the IRQ, it must be written to.
; Since the scanline IRQ is actually a timer, it keeps counting even during VBlank, or when the PPU is off, so I needed to add some workarounds

; How to program the IRQ:
; You need to make an array of commands (see irq_parser below). The structure should be [<scanline (<0xF0)>, <command (>=0xF0)>,<value (if applicable)>]... <0xFF>
; If there are commands before the first scanline, they will be run at VBlank.
; Keep in mind that if you have multiple scanline triggers, the <scanline> values are relative (p.ex: [50,Fx, 25, Fy] will trigger at line 50 and line 75)

; IRQ routine
; 1: At VBlank, the IRQ is disabled, and the array index is set to 0
; 2: If rendering is enabled, prepare_line0_timer is called. Its function is to trigger after VBlank is over. When this happens, the value for the first scanline trigger will be written. This prevents the mapper from counting the VBlank cycles as scanlines.
; 3: The IRQ parser is called with is_vblank_initial_setup set to 1. This will run the commands that are meant to be run at VBlank, and leave the index in such a way that, when the line 0 IRQ triggers, the next command will be the first scanline set.


    
irq:
    pha
    txa
    pha
    tya
    pha
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
;f2 = mid-frame y scroll, next byte is Y value, X value is always $100
;f3-f4 unused - future TODO ?
;f5 = 2005 write, next byte is H Scroll value
;f6 = 2006 write, next 2 bytes are write values


;f7 = change CHR mode 0, next byte is write value
;f8 = change CHR mode 1, next byte is write value
;f9 = change CHR mode 2, next byte is write value
;fa = change CHR mode 3, next byte is write value

; currently, changes to the chr map 1 have not been implemented, but it wouldn't be too hard

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
    cmp #$f2 ; xy split (to X$100 Y y)
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
    lda #236 ; 14 lines until line 0 more or less
    sta $f000
    lda #%00000010
    sta $f001
    rts

; The following functions are made specifically for my game

; Keep in mind that, in my game, music code = bank 0, game code = bank 6, dpcm 0 = bank 10 and dpcm 1 = bank 11
; For more information, check the comments at VRC6.cfg
; Basically, these functions consist in changing the game code to the music code, calling the famistudio functions, and then putting the game code back
_famistudio_init_wrapper:
    pha
    lda #$0
    sta $8000
    ;cc65 stores 3 arguments in A,X, stack, but famistudio_init expects A,X,Y
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
    ; Required by famistudio
    clc
    adc #$A
    sta $C000
    rts

    
    
    