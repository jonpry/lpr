    .sect   ".text:main"
    .clink
    .global ||foo||

CONST_INTC .set	c0
    
    ;r0 - 0
    ;r1 - 32 bit shift content
    ;r2 - temp shift register
    ;r3 - mask constant
    ;r4 - neg mask content
    ;r5 - output shadow 
    ;r6 - temp, can clobber, will be clobbered
    ;r7 - loop count
    ;r8 - control register address
    ;r9 - can use
    ;r10 - initial cycle count
    ;r11 - shared ram address
    ;r12 - can use
    ;r13 - address of loopcount
    ;r14 - can use
    ;r15 - maximum loopcount

||foo||:
    ;init,
    ldi r0, 0
    ldi r3, 0x10
    not r4, r3
    mov r5, r30
listen:
    ldi r7, 0
    LDI32     r8, 0x00022000
    LDI32     r13, 0x00012000 ; Loop counter
    LDI32     r11, 0x00010000 ; Shared RAM
    LDI32     r15, 6144000 ;maximum loops

    ;spin here until pru1 notifies us of run command
    WBS       r31, 30
    ldi      r12, 16
    SBCO     &r12, CONST_INTC, 0x24, 4  ;clear the interrupt

    ;disable, then enable cycle counter
    LBBO      &r10, r8, 0, 4   ; load CYCLE register - extra delay from here
    CLR       r10, r10, 3
    SBBO      &r10, r8, 0, 4    
    SET       r10, r10, 3
    SBBO      &r10, r8, 0, 4  
    LBBO      &r10, r8, 0xc, 4 ; Get starting cyclecount

    LBBO      &r1, r11, 0, 4 ; //Fetch first word 
l1:
    ;bit 0
    lsl     r2, r1, 4 ; output is gpo7
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    add     r7, r7, 1 ;increment loop counter
    nop 
    nop  
    nop
    nop
    nop

    ;bit 1
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 2
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 3
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 4
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 5
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 6
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 7
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 8
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 9
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 10
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 11
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 12
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 13
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 14
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 15
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 16
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 17
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 18
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 19
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 20
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 21
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 22
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 23
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 24
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

   ;bit 25
    lsr     r2,r2,1 
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 26
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    nop
    nop
    nop
    nop
    nop

    ;bit 27
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    LBBO    &r12, r8, 0xc, 4 ; //TODO: here for testing CPI
    ;nop
    ;nop
    ;nop - 3 nops for control register access
    ldi     r12, 0x7F    ;write status every 0x80 words
    and     r12, r12, r7 ;r7==0 when loop%0x80==0

    ;bit 28
    lsr     r2,r1,24 ; x>>28<<4
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    qbne    no_int, r12, 0  ;equal time conditional write
    sbbo    &r7, r13, 0, 4  ;transfer loop count
    ;nop
    ldi     r31, (1<<5)+17-16  ;system event 17
    nop
    jmp     no_inte
no_int:
    nop
    nop
    nop
    nop
    nop
no_inte:

    ;bit 29
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    nop
    and     r12, r7, 0xFF  ;Address computation, SMEM[(loop&0xFF)*4]
    lsl     r12, r12, 2
    LBBO    &r1, r11, r12, 4 ; //Fetch first word 
    ;nop - stall
    ;nop - stall

    ;bit 30
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    LBBO    &r12, r8, 0xc, 4 ; Get cyclecount
    ;nop - stall
    ;nop - stall
    ;nop - stall
    sub     r14, r12, r10
    nop

    ;bit 31
    lsr     r2,r2,1
    and     r6, r2, r3
    and     r5, r30, r4
    or      r30, r6, r5

    sbbo    &r14, r13, 4, 4
    ;nop - stall
    mov     r10, r12
    nop
    qbeq   listen, r7, r15
    jmp l1 ;do it again

    HALT
