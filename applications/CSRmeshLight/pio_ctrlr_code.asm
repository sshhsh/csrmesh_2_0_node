;/******************************************************************************
; *  Copyright 2015 Qualcomm Technologies International, Ltd.
; *  Bluetooth Low Energy CSRmesh 2.0
; *  Application version 2.0
; *
; * FILE
; *    pio_ctrlr_code.asm
; *
; *  DESCRIPTION
; *    This file contains the low level 8051 assembly code for fast PWM
; *
; *  NOTICE
; *
; *****************************************************************************/
; 
; Local variables
.equ TEMP, 0x3e

; Shared memory from 0x40
; 0~7 BRIGHT duty cycles
; 8~15 DULL duty cycles
; 16 Initial states of outputs
; 18 BRIGHT period
; 20 DULL period
; 22 RESET

.equ SHARED_MEM, 0x40
.equ INIT_STATE, SHARED_MEM+16
.equ BRIGHT_PERIOD, SHARED_MEM+18
.equ DULL_PERIOD, SHARED_MEM+20
.equ PWM_RESET, SHARED_MEM+22

; HW registers
.equ P0_DRIVE_EN, 0xc0
.equ P1_DRIVE_EN, 0xc8
.equ P2_DRIVE_EN, 0xd8
.equ P3_DRIVE_EN, 0xe8

; IDLE LOOP COUNT to make each step 4 microseconds
.equ IDLE_COUNT, 6

; R1 (LSB) and R2 (MSB) are used to count the number of pulses in a given phase

START:
    
; Set the stack up
    mov SP, #30H
    
; Adjust to the used PWM pins. This case is for all 8 pins on Port1.
; If needed apply only to required pins
    mov P1_DRIVE_EN, #0xFF

;****************************************************************************
;   BRIGHT phase
;****************************************************************************

RESET:

    mov  R1, #0
    mov  R2, #0

BRIGHT_START:

;   Check if we should start this pulse or jump to dull phase

    mov  A, R1
    cjne A, BRIGHT_PERIOD, LSB_NE
    mov  A, R2
    cjne A, BRIGHT_PERIOD+1, MSB_NE
    mov  R1, #0
    mov  R2, #0
    ajmp DULL_START
   
MSB_NE:

    mov  A, R1
    
LSB_NE:

    ; A is R1
    add  A, #1
    mov  R1, A
    mov  A, R2
    addc A, #0
    mov  R2, A
    
START_PULSE:

    ; A is now the step number
    mov  A, #0
    mov  TEMP, INIT_STATE

BIT0:
    cjne A, SHARED_MEM, BIT1
    xrl  TEMP, #1
BIT1:
    cjne A, SHARED_MEM+1, BIT2
    xrl  TEMP, #2
BIT2:
    cjne A, SHARED_MEM+2, BIT3
    xrl  TEMP, #4
BIT3:
    cjne A, SHARED_MEM+3, BIT4
    xrl  TEMP, #8
BIT4:
    cjne A, SHARED_MEM+4, BIT5
    xrl  TEMP, #16
BIT5:
    cjne A, SHARED_MEM+5, BIT6
    xrl  TEMP, #32
BIT6:
    cjne A, SHARED_MEM+6, BIT7
    xrl  TEMP, #64
BIT7:
    cjne A, SHARED_MEM+7, DONE
    xrl  TEMP, #128
DONE:

; Output the resulted state

    mov  P1, TEMP

; Filling the rest of the step with a loop

    mov  R0, #0
IDLE:
    inc  R0
    cjne R0, #IDLE_COUNT, IDLE   ; Change loop count for corse step size adjustment

; Finer adjustment of step size. Add or remove NOPs below
    nop
;    nop
;    nop


; Next pulse after 255 steps     
    inc  A
    cjne A, #255, BIT0

; Even finer adjustment of pulse frequency can be done by adding NOPs here
;   nop

; Check RESET at the end of each pulse

    mov  A, PWM_RESET
    cjne A, #0, DO_RESET
    ajmp NO_RESET

DO_RESET:

    mov  PWM_RESET, #0
    ajmp RESET

NO_RESET:

    ajmp BRIGHT_START

;****************************************************************************
; DULL phase
;****************************************************************************

DULL_START:

;   Check if we should start this pulse

    mov  A, R1
    cjne A, DULL_PERIOD, LSB_NE2
    mov  A, R2
    cjne A, DULL_PERIOD+1, MSB_NE2
    ajmp RESET
    
MSB_NE2:

    mov  A, R1
    
LSB_NE2:

    ; A is R1
    add  A, #1
    mov  R1, A
    mov  A, R2
    addc A, #0
    mov  R2, A

START_PULSE2:

    ; A is now the step number
    mov  A, #0
    mov  TEMP, INIT_STATE
BIT0_2:
    cjne A, SHARED_MEM+8, BIT1_2
    xrl  TEMP, #1
BIT1_2:
    cjne A, SHARED_MEM+9, BIT2_2
    xrl  TEMP, #2
BIT2_2:
    cjne A, SHARED_MEM+10, BIT3_2
    xrl  TEMP, #4
BIT3_2:
    cjne A, SHARED_MEM+11, BIT4_2
    xrl  TEMP, #8
BIT4_2:
    cjne A, SHARED_MEM+12, BIT5_2
    xrl  TEMP, #16
BIT5_2:
    cjne A, SHARED_MEM+13, BIT6_2
    xrl  TEMP, #32
BIT6_2:
    cjne A, SHARED_MEM+14, BIT7_2
    xrl  TEMP, #64
BIT7_2:
    cjne A, SHARED_MEM+15, DONE2
    xrl  TEMP, #128
DONE2:

; Output the resulted state
    mov  P1, TEMP

; Filling the rest of the step
    mov  R0, #0
IDLE2:
    inc  R0
    cjne R0, #IDLE_COUNT, IDLE2   ; Change loop count for corse step size adjustment

; Finer adjustment of step size. Add or remove NOPs below
    nop
;    nop
;    nop

; Next pulse after 255 steps     
    inc    A
    cjne   A, #255, BIT0_2

; Even finer adjustment of pulse frequency can be done by adding NOPs here
;   nop

; Check RESET at the end of each pulse

    mov  A, PWM_RESET
    cjne A, #0, DO_RESET2
    ajmp NO_RESET2
    
DO_RESET2:

    mov  PWM_RESET, #0
    ajmp RESET

NO_RESET2:

    ajmp     DULL_START
