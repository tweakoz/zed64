
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

*= $0000

jmp begin

*= $0400

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
    adc $3fff
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

; minimal init
ldy #$0
ldx #$ff
txs ;; init stack pointer
ldx #$0
lda #$0
sta $3fff
sta $3ffe

; set up interrupt handler
sei
lda #<irq_handler
sta $fffe
lda #>irq_handler
sta $ffff
cli

lda #0
sta $3fff

; Fill in text 
#fillbyte4k table256, $4000
#fillbyte4k table256, $5000
#fillbyte4k table256, $6000
#fillbyte4k table256, $7000

#fillbyte4k table256, $8000
#fillbyte4k table256, $9000
#fillbyte4k table256, $A000
#fillbyte4k table256, $B000

main_loop:

inc $3fff
#fillbyte256 tableRows, $3800


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

jmp main_loop

irq_handler:
pha
txa
pha
tya
pha
inc $3ffe
lda $3ffe
#fillbyte256 tableRows, $2e00
;;#fillbyte256 tableRows, $2f01
pla
tay 
pla
tax 
pla
rti

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

table256:
.byte 0.0+range(256)

tableRows:
.byte range(256)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; FONT
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

*= $e000
.binary "chargen" ;; you need a c64 rom font