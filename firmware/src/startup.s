/*
 * startup.s - minimal Cortex-M3 startup for STM32F103C8T6
 *
 * Sets up the initial stack pointer, copies .data from flash to RAM,
 * zeroes .bss, then jumps into main(). This is the very first code that
 * runs when the chip powers on.
 */
    .syntax unified
    .cpu cortex-m3
    .thumb

    .global Reset_Handler
    .global _estack

    .section .isr_vector, "a", %progbits
    .type g_pfnVectors, %object
g_pfnVectors:
    .word _estack                /* initial stack pointer */
    .word Reset_Handler          /* reset handler */
    .word Default_Handler        /* NMI */
    .word Default_Handler        /* HardFault */
    .word Default_Handler        /* MemManage */
    .word Default_Handler        /* BusFault */
    .word Default_Handler        /* UsageFault */
    .word 0, 0, 0, 0              /* reserved */
    .word Default_Handler        /* SVCall */
    .word Default_Handler        /* DebugMon */
    .word 0                      /* reserved */
    .word Default_Handler        /* PendSV */
    .word Default_Handler        /* SysTick */

    .section .text.Reset_Handler
    .weak Reset_Handler
    .type Reset_Handler, %function
Reset_Handler:
    ldr r0, =_estack
    mov sp, r0

    /* copy .data section from flash to RAM */
    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata
copy_loop:
    cmp r1, r2
    bcs copy_done
    ldr r3, [r0], #4
    str r3, [r1], #4
    b copy_loop
copy_done:

    /* zero .bss section */
    ldr r0, =_sbss
    ldr r1, =_ebss
    movs r2, #0
zero_loop:
    cmp r0, r1
    bcs zero_done
    str r2, [r0], #4
    b zero_loop
zero_done:

    bl main
    b .

    .section .text.Default_Handler
    .type Default_Handler, %function
Default_Handler:
    b .

    .size Default_Handler, . - Default_Handler