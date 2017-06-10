.arm.little

.create "build/svcInitHooks.bin", 0
.arm
    b skip_vars
vars:
    ; SVCs that are susceptible to be used by section0 modules very early on
    ControlMemory: .word 0
    SleepThread: .word 0
    ConnectToPort: .word 0
    KernelSetState: .word 0
skip_vars:
    push {r0-r4, lr}
    mov r4, #0xa0000000

    loop:
        ldrb r12, [r4, #-0x10]
        tst r12, #(1 << (2 + 2))
        bne loop_end

        ldr r12, [SleepThread]
        ldr r0, =(10 * 1000 * 1000)
        mov r1, #0
        blx r12
        b loop

    loop_end:
    pop {r0-r4, lr}
    mov r12, #0x40000000
    add r12, #0x14
    bx r12

.pool

.close
