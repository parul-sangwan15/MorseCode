#include "hardware/regs/addressmap.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/timer.h"
#include "hardware/regs/m0plus.h"

.syntax unified                                                 @ Specify unified assembly syntax
.cpu    cortex-m0plus                                           @ Specify CPU type is Cortex M0+
.thumb                                                          @ Specify thumb assembly for RP2040
.global main_asm                                                @ Provide program starting address to the linker
.align 4                                                        @ Specify code alignment

.equ    GPIO_BTN_FALL_MSK, 0x00400000        @ Bit-22 for falling-edge event on GP21
.equ    GPIO_BTN_RISE_MSK, 0x00800000        @ Bit-22 for rising-edge event on GP21

.equ    GPIO_BTN, 21                         @GPIO-21 button 

.equ    NOT,0                                @To set the status  2sec ALARM TO 0 denoting that the loop branch should run
.equ    YES,1                                @TO SET THE STATUS OF THE 2sec ALARM TO 0 for returning to main loop

.equ    GPIO_ISR_OFFSET,0x74 
.equ    ALRM_ISR_OFFSET, 0x40                @ ALARM0 is int #0 (vector table entry 16)

.equ    DASH_TIME,500000
.equ    SECOND,1000000

.equ    DOT,0                                @ 0 is sent to the C function to denote that it is a .
.equ    DASH,1                               @ 1 is sent to the C function to denote that it is a _
.equ    TAB,2                                @ 2 is sent to the C function to denote that it is a _
.equ    END,3
.equ    default,10000000

.equ    STOP_TIME,2000000               

@ Entry point to the ASM portion of the program
main_asm:
    push    {lr}                                
    ldr     r0,=msg_input
    bl      init_btns
    bl      install_gpio_isr
    wfi
    bl      asm_watchdog_reset
    bl      install_alrm_isr
loop:   
    bl set_alarm 

    wfi
    bl      asm_watchdog_reset
    ldr r3,=alrm_status
    ldr r4,[r3]
    cmp r4,#1
    beq out
    b loop                                          @ Infinite loop

out:
    pop {pc}
init_btns:
    push    {lr}
    movs    r0, #GPIO_BTN                            @ This value is the GPIO 21 button on the PI PICO board
    bl      asm_gpio_init                            @ Call the subroutine to initialise the GPIO pin specified by r0
    movs    r0, #GPIO_BTN                            @ GP21 buttons
    bl      asm_gpio_set_irq_fall                    @ Sets the button to a falling-edge interrupt
    movs    r0, #GPIO_BTN                            @ GP21 buttons
    bl      asm_gpio_set_irq_rise                    @ Sets the button to a risin-edge interrupt
    pop     {pc}

install_gpio_isr:
    ldr     r2, =(PPB_BASE + M0PLUS_VTOR_OFFSET)     @ Adds the base address of the CPU in RP2040 to vector offset address to find out where vector table is
    ldr     r1, [r2]                                 @ Hexadecimal address of where the vector table(value stored in r2) should be loaded in r1. r1 now has actual vector table base address
    movs    r2, #GPIO_ISR_OFFSET                     @ Move offset address to r2
    add     r2, r1                                   @ Adds offset address to the base address, stores the whole value in r2. r2 now has the address to the service routine
    ldr     r0, =gpio_isr                            @ Address label gpio_isr is stored in r0
    str     r0, [r2]                                 @ Pushes the address back to r0 and stores where we have the offset address of the table
    
    @ Enable the ALARM 0 IRQ(disabling(clear) and the enabling(set))
    @clear
    ldr     r0,=8192                                 @ Setting the 14th bit to 0 as that is the position of the GPIO interrput 
    ldr     r1,=(PPB_BASE + M0PLUS_NVIC_ICPR_OFFSET) @ Disabling the GPIO IRQ by writing the correct value to (PPB_BASE + M0PLUS_NVIC_ICPR_OFFSET)
    str     r0,[r1]                                  @ storing in the correct postion
    @set
    ldr     r1,=(PPB_BASE + M0PLUS_NVIC_ISER_OFFSET) @Enabling the GPIO IRQ by writing the correct value to (PPB_BASE + M0PLUS_NVIC_ISER_OFFSET)
    str     r0,[r1]                                  @storing in the correct postion
    bx      lr 

install_alrm_isr:
    @Set the IRQ handler to our subroutine
    ldr     r2, =(PPB_BASE + M0PLUS_VTOR_OFFSET)     @ address of the RAM vector table using the (PPB_BASE + M0PLUS_VTOR_OFFSET) register 
    ldr     r1, [r2]                                 @ Hexadecimal address of where the vector table(value stored in r2) should be loaded in r1. r1 now has actual vector table base address
    movs    r2, #ALRM_ISR_OFFSET                     @ Move offset address to r2
    add     r2, r1                                   @ Adds offset address to the base address, stores the whole value in r2. r2 now has the address to the alarm interrupt 
    ldr     r0, =alrm_isr                            @ Address label alrm_isr is stored in r0
    str     r0, [r2]                                 @ Pushes the address back to r0 and stores where we have the offset address of the table
   
    @ Enable the ALARM 0 IRQ(disabling(clear) and the enbling(set))
    @clear
    movs    r0,#1                                       @setting the 0th bit to 1 as that is the position of the alarm 0 interrupt
    ldr     r1,=(PPB_BASE + M0PLUS_NVIC_ICPR_OFFSET)    @Disabling the ALARM0 IRQ by writing the correct value to (PPB_BASE + M0PLUS_NVIC_ICPR_OFFSET)
    str     r0,[r1]                                     @storing in the correct postion 
    @set
    ldr     r1,=(PPB_BASE + M0PLUS_NVIC_ISER_OFFSET)    @ Enabling the ALARM0 IRQ by writing the correct value to (PPB_BASE + M0PLUS_NVIC_ISER_OFFSET)
    str     r0,[r1]                                      @storing in the correct postion
    bx      lr 

set_alarm:
    ldr  r2,=TIMER_BASE                 
    movs r1, #1                          @ for alarm 0 interrupt
    str  r1, [r2, #TIMER_INTE_OFFSET]    @Enable alarm timer interrupts using the (TIMER_BASE + TIMER_INTE_OFFSET)
    ldr  r1, [r2, #TIMER_TIMELR_OFFSET]  @the current timer count from (TIMER_BASE + TIMER_TIMELR_OFFSET)
    ldr  r3,=ltimer                      @loads the address of the value of the timer 
    ldr  r4,[r3]                         @loads the value of the timer in r4
    add  r1, r4                          @ adds r4 to the timer to set the alarm time
    str  r1, [r2, #TIMER_ALARM0_OFFSET]  @Updated value is pushed to (TIMER_BASE + TIMER_ALARM0_OFFSET)
    bx lr                                @exits the subroutine

.thumb_func                              @ Required for all interrupt service routines
alrm_isr:
    push {lr}
    @Acknowledge the interrupt has been recieved
    LDR R2, =TIMER_BASE
    MOVS R1, #1 @ for alarm 0
    STR R1, [R2, #TIMER_INTR_OFFSET]     
    ldr r0,=YES
    ldr r2,=alrm_status             @time after each button event
    str r0,[r2]
    ldr r0,=END
    bl add_input
    pop  {pc}
.thumb_func

gpio_isr:
    push {lr}
    movs r0,#1
    ldr r2,=btn_status
    str r0,[r2]
    ldr r2, =(IO_BANK0_BASE + IO_BANK0_PROC0_INTS2_OFFSET)  @Reads the interrupt status 
    @Interrupt acknowledgement
    ldr r1, [r2]
    ldr r0, =8192
    str r0, [r2]
    @implementing the button functionality
    ldr r0, =GPIO_BTN_FALL_MSK            @loads in the GP21 falling edge event mask value to compare 
    cmp r1, r0                            @and detect if this is the button that has been pressed
    beq btn_fall                          @branches to GP21 button functionality

    ldr r0, =GPIO_BTN_RISE_MSK            @loads in the GP21 rising edge event mask value to compare 
    cmp r1, r0                            @and detect if this is the button that has been pressed
    beq btn_rise                          @branches to GP21 button functionality

btn_fall:
    movs r0,#1
    ldr r2,=btn_status
    str r0,[r2]
    bl  asm_get_time
    ldr r2,=time_rise
    str r0,[r2]
    ldr r3,=time_button_over
    ldr r4,[r3]
    subs r5,r0,r4
    ldr r6,=SECOND
    cmp r5,r6
    blt btn_fall_done
    bgt btn_fall_space
    beq btn_fall_space

btn_fall_space:
    ldr r0,=TAB
    bl add_input
    ldr r2, =(IO_BANK0_BASE + IO_BANK0_INTR2_OFFSET)    @Used to reset the interrupt
    ldr r1, =GPIO_BTN_FALL_MSK                           @Used to reset the GP20 interrupt
    str r1, [r2]                                        @Used to reset the GP20 interrupt
    b   gpio_isr_done

btn_fall_done:
    ldr r2, =(IO_BANK0_BASE + IO_BANK0_INTR2_OFFSET)    @Used to reset the interrupt
    ldr r1, =GPIO_BTN_FALL_MSK                           @Used to reset the GP20 interrupt
    str r1, [r2]                                        @Used to reset the GP20 interrupt
    b   gpio_isr_done

btn_rise:
    movs r0,#0
    ldr r2,=btn_status
    str r0,[r2]
    bl  asm_get_time
    ldr r2,=time_rise
    ldr r1,[r2]
    subs r3,r0,r1
    ldr r4,=DASH_TIME
    cmp r3,r4
    blt btn_rise_dot
    beq btn_rise_dash
    bgt btn_rise_dash

btn_rise_dot:
    ldr r0,=DOT
    bl add_input
    b  btn_rise_stop

btn_rise_dash:
    ldr r0,=DASH
    bl add_input
    b  btn_rise_stop

btn_rise_stop:
    ldr r2, =(IO_BANK0_BASE + IO_BANK0_INTR2_OFFSET)    @Used to reset the interrupt
    ldr r1, =GPIO_BTN_RISE_MSK                           @Used to reset the GP20 interrupt
    str r1, [r2]                                        @Used to reset the GP20 interrupt
    b   gpio_isr_done

gpio_isr_done:
    bl  asm_get_time
    ldr r2,=time_button_over        @time after each button event
    str r0,[r2]
    pop {pc}


@ Set data alignment
.align 4
msg_input:         .asciz "INPUT:"
.data
time_rise:         .word  default
time_button_over:  .word  default
ltimer:            .word  STOP_TIME                       @ alarm for the timer 
alrm_status:       .word  NOT
btn_status:        .word  NOT
start_loop:        .word  NOT