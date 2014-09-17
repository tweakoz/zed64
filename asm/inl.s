
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

*= $0000

jmp begin

*= $1000

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

push_addr16 .macro
  lda <\1
  pha
  lda >\1
  pha
.endm

pop_addr16_to .macro
  pla
  sta \1+1
  pla
  sta \1
.endm

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  ;.for ad=0,ad<4,ad=ad+1
  ;  pla
  ;  sta $40-ad
  ;.next

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
fillbyte256 .macro

  ldx #0
  ;
  fillbyte256_loopB:
    lda (\1),x
    adc $4000
    sta (\2),x
    inx
    cpx #0
    bne fillbyte256_loopB
.endm

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

fillbyte1k .macro

  #fillbyte256 \1, \2
  #fillbyte256 \1, \2+256
  #fillbyte256 \1, \2+512
  #fillbyte256 \1, \2+768

.endm

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

fillbyte4k .macro

  #fillbyte1k \1, \2
  #fillbyte1k \1, \2+1024
  #fillbyte1k \1, \2+2048
  #fillbyte1k \1, \2+3072

.endm

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

begin:

lda #0
sta $4000

main_loop:

; Fill in text 
#fillbyte4k table256, $8000
#fillbyte4k table256, $9000
#fillbyte4k table256, $a000

inc $4000

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

jmp main_loop

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

table256:
.byte 0.0+range(256)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; FONT
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

*= $c000
.binary "chargen" ;; you need a c64 rom font