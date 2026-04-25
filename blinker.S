
//ARM INSTRUCTIONS
//Instructions are conditional
//32-bit instruction


//How to load data from memory:
//LDR R0, =variable 	(Load address to register. You can use any global variable defined in main.c, or variable found here)
//LDR R1, [R0] 			(Load data from address, in this case 32-bit data)
//LDRB R1, [R0] 		(Load data from address, in this case 8-bit data)
//******
//LDR R0, =0x123456		(Load address to register, Use only if you know the memory address)
//LDR R1, [R0]			(load data from address, in this case 32-bit data)
//LDRB R1, [R0]			(load data from address, in this case 8-bit data)

//How to store data to memory:
//STR R1,[R0]			(R1 contains the data and R0 contains the address where  to store, 32-bit)
//STRB R1,[R0]			(R1 contains the data and R0 contains the address where  to store, 8-bit)


.data
    blink_dir: .byte 1      // 1 = move right, 0 = move left
    blink_pos: .byte 0      // current LED index: 0..3 (LD0..LD3)

.text
.global blinker

blinker:
    // Save registers we use
    push {r0-r5, lr}

    // r0 = address of button-LEDs (LD0–LD3 at 0x41200000)
    LDR     r0, =0x41200000

    // --- load current position 0..3 ---
    LDR     r1, =blink_pos
    LDRB    r2, [r1]          // r2 = blink_pos

    // mask = 1 << pos  (only one LED on)
    MOV     r3, #1
    LSL     r3, r3, r2        // r3 = (1 << blink_pos)

    // write mask to LEDs
    STRB    r3, [r0]

    // delay so movement is visible
    BL      delay

    // --- load direction: 1 = right, 0 = left ---
    LDR     r1, =blink_dir
    LDRB    r4, [r1]          // r4 = blink_dir

    // decide which way to move
    CMP     r4, #1
    BEQ     move_right
    B       move_left

// Move 0 -> 1 -> 2 -> 3 (right)
move_right:
    ADD     r2, r2, #1        // pos++

    CMP     r2, #3
    BLE     store_and_return  // if 0..3, OK

    // went past 3 → bounce back
    MOV     r2, #2            // next = 2
    MOV     r4, #0            // change dir to left
    B       store_and_return

// Move 3 -> 2 -> 1 -> 0 (left)
move_left:
    SUB     r2, r2, #1        // pos--

    CMP     r2, #0
    BGE     store_and_return  // if 0..3, OK

    // went past 0 → bounce back
    MOV     r2, #1            // next = 1
    MOV     r4, #1            // change dir to right
    B       store_and_return

// Save new pos + dir and return
store_and_return:
    // save pos
    LDR     r1, =blink_pos
    STRB    r2, [r1]

    // save dir
    LDR     r1, =blink_dir
    STRB    r4, [r1]

    // restore and return
    pop     {r0-r5, lr}
    BX      lr


// Simple busy-wait delay
delay:
    MOV     r5, #0
    LDR     r4, =0x4000000      // increase if too fast, decrease if too slow

delay_loop:
    ADD     r5, r5, #1
    CMP     r5, r4
    BLT     delay_loop

    BX      lr




