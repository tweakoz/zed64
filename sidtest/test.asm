# test.asm

$pc=0

start:

    ld.0 r0     
    ld.0 r1     
    ld.0 r2     
    ld.0 r3     
    ld.0 r4     
    ld.0 r5     
    ld.0 r6     
    ld.i18 r7 7    
    ld.i18 r1 0x5

lp1:

    ld.i18 r6 1 
    sub r1 r1 r6
    call iter
    jz r1 fini
    jnz r1 fini
    #jmp lp1

iter: # accum *= r0
    
    mul.async r7 r1
    ld.product.lo r2
    st.mem r7 0
    #div.async r2 r1 
    #ld.quotient.hi r3
    ret
    
fini:

    ld.mem r3 0
    ld.x36 r0 32.23
    ld.x36 r1 16.23
    add r2 r0 r1
    ld.x36 r4 4.07
    add r5 r2 r4
    push r2
    pop r7
    shr r0 18
    ld.l24 0x456
    wait 0
    ld.memr r0

    ld.i18 r0 12
    call interval2ratio
    ld.i18 r0 0x18 
    ld.i18 r1 15 
    call poke_sid
    ld.i18 r0 0x18 
    ld.i18 r1 0 
    call poke_sid
    ld.i18 r0 0x18 
    ld.i18 r1 7 
    call poke_sid
    ld.i18 r0 500
    call delay

#####################################

main_loop:

    jmp start

#####################################

interval2ratio: # r0 semi r1 cents
                # result = r2

    #10025de = 1 cent (25bit)
    #10f38f9 = 1 semi (25bit)
    #1000011
    ld.u18 r2 0x43
    ld.l18 r2 0x338f9
    nop nop nop nop
    mul.async r2 r0
    nop nop nop nop
    ld.prod.ab.hi r3
    ld.prod.ab.lo r4
    nop nop nop nop
    shr r3 29
    ret

#####################################

delay: # r0:uSec
    wait 0
    wait 1
    ld.i18 r1 1
    nop nop nop nop
    sub r0 r0 r1
    nop nop nop nop
    jnz r0 delay
    ret

#####################################

poke_sid: # r0:addr r1:data
    ld.i18 r2 255 
    ld.i18 r3 0 
    ld.i18 r4 1 
    wait 0
    wait 1
    wait 0
    st.io r0 r1
    wait 1
    wait 0
    st.io r2 r3
    wait 1
    ret



 