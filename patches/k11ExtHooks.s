.arm.little

.create "build/k11ExtHooks.bin", 0
.arm

bindSGI0:
    ; hook __kernel_main to bind SGI0 for own purposes
    push {r0-r4, lr}
    sub sp, #16                  ; 3 args passed through the stack + alignment
    adr r0, parameters
    ldr r0, [r0, #4]
    adr r1, interruptEvent
    mov r2, #0
    mrc p15, 0, r3, c0, c0, 5
    and r3, #3
    mov r4, #0
    str r4, [sp]
    str r4, [sp, #4]
    str r4, [sp, #8]

    ldr r12, [InterruptManager_MapInterrupt]
    blx r12
    cmp r0, #0
    blt .

    add sp, #16
    pop {r0-r4, pc}

executeCustomHandler:
    push {r4, lr}
    mrs r4, cpsr
    adr r0, parameters
    bl convertVAToPA
    orr r0, #(1 << 31)
    ldr r12, [r0]

    blx r12

    mov r0, #0
    msr cpsr_cx, r4
    pop {r4, pc}

convertVAToPA:
    mov r1, #0x1000
    sub r1, #1
    and r2, r0, r1
    bic r0, r1
    mcr p15, 0, r0, c7, c8, 0    ; VA to PA translation with privileged read permission check
    mrc p15, 0, r0, c7, c4, 0    ; read PA register
    tst r0, #1                   ; failure bit
    bic r0, r1
    addeq r0, r2
    movne r0, #0
    bx lr

undefinedInstructionVeneer:
    .word 0xf57ff01f            ; clrex
    mov sp, #0xa0000000
    ldrb sp, [sp, #-0x10]       ; normally 3 on init
    tst sp, #(1 << (1 + 2))
    ldreq pc, [undefinedInstructionHandler]
    ldrne pc, [undefinedInstructionHandlerAlt]

svcVeneer:
    .word 0xf57ff01f            ; clrex
    push {r0}
    mov r0, #0xa0000000
    ldrb r0, [r0, #-0x10]
    tst r0, #(1 << (2 + 2))
    pop {r0}
    ldreq pc, [svcHandler]
    ldrne pc, [svcHandlerAlt]

prefetchAbortVeneer:
    .word 0xf57ff01f            ; clrex
    mov sp, #0xa0000000
    ldrb sp, [sp, #-0x10]
    tst sp, #(1 << (3 + 2))
    ldreq pc, [prefetchAbortHandler]
    ldrne pc, [prefetchAbortHandlerAlt]

dataAbortVeneer:
    .word 0xf57ff01f            ; clrex
    mov sp, #0xa0000000
    ldrb sp, [sp, #-0x10]
    tst sp, #(1 << (4 + 2))
    ldreq pc, [dataAbortHandler]
    ldrne pc, [dataAbortHandlerAlt]
.pool

.ascii "vner"
.word undefinedInstructionVeneer
.word svcVeneer
.word prefetchAbortVeneer
.word dataAbortVeneer

.ascii "exch"
undefinedInstructionHandler: .word 0
svcHandler: .word 0
prefetchAbortHandler: .word 0
dataAbortHandler: .word 0

undefinedInstructionHandlerAlt: .word 0x40000004
svcHandlerAlt: .word 0x40000008
prefetchAbortHandlerAlt: .word 0x4000000C
dataAbortHandlerAlt: .word 0x40000010

; Result InterruptManager::MapInterrupt(InterruptManager *this, InterruptEvent *iEvent, u32 interruptID, u32 coreID, s32 priority, bool willBeMasked, bool isLevelHighActive);
InterruptManager_MapInterrupt: .ascii "bind"

_vtable: .word executeCustomHandler
interruptEvent: .word _vtable

parameters:

.close
