/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 *
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

// Main program for exercise

//****************************************************
//By default, every output used in this exercise is 0
//****************************************************
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "sleep.h"
#include "xgpiops.h"
#include "xttcps.h"
#include "xscugic.h"
#include "xparameters.h"
#include "Pixel.h"
#include "Interrupt_setup.h"

//********************************************************************
//***************TRY TO READ COMMENTS*********************************
//********************************************************************

//***Hint: Use sleep(x)  or usleep(x) if you want some delays.****
//***To call assembler code found in blinker.S, call it using: blinker();***


//Comment this if you want to disable all interrupts
#define enable_interrupts




/***************************************************************************************
Name:
Student number:

Name:
Student number:

Name:
Student number:

Tick boxes that you have coded

Led-matrix driver		Game		    Assembler
	[]					[]					[]

Brief description:

*****************************************************************************************/

// For screen refresh
volatile uint8_t current_channel = 0;

// For ship movement
volatile int8_t ship_x = 3;

// For alien
volatile int8_t alien_x = 6;
volatile int8_t alien_y = 0;
volatile int8_t alien_dx = 1; // Moving right initially

// For bullet
volatile int8_t bullet_x = 0;
volatile int8_t bullet_y = 0;
volatile int8_t bullet_active = 0;

// Game scoring and state

// how many times alien has been hit
volatile int8_t hits = 0;
// how many bullets fired
volatile int8_t bullets_used = 0;
volatile int8_t game_over = 0; // 0 = running, 1 = finished

// Game design: 8 bullets total, need at least 5 hits to win
const int8_t MAX_BULLETS = 8;
const int8_t WIN_HITS = 3;

// Clear the screen
void clear_screen(void) {
	for (uint8_t x = 0; x < 8; ++x) {
		for (uint8_t y = 0; y < 8; ++y) {
			SetPixel(x,y,0,0,0);
		}
	}
}

// Draw the ship at the current ship_x position
void draw_ship(void){
	SetPixel(ship_x,7,0,255,0);
	SetPixel(ship_x+1,6,0,255,0);
	SetPixel(ship_x+1,7,0,255,0);
	SetPixel(ship_x+2,7,0,255,0);
}

// Draw the alien
void draw_alien(void){
	SetPixel(alien_x, alien_y, 100, 65,0);
}

// Drawing score bar: Draw hits as a bar from the top row (y = 0)
// Number of lit pixels = hits
void draw_score_bar(void) {
	int8_t shown_hits = hits;
	if (shown_hits < 0) shown_hits = 0;
	if (shown_hits > 8) shown_hits = 8;

	for (uint8_t x = 0; x < 8; ++x) {
		// Clear row 0 first
		SetPixel(x,0,0,0,0);

		for (int8_t x = 0; x < shown_hits; ++x) {
			// White bar showing how many hits (score)
			SetPixel(x,0,255,0,255);
		}
	}
}

// Drawing smiley face for winning
void draw_smiley_face(void)
{

    // Eyes (green)
    SetPixel(2, 3, 0, 255, 0);
    SetPixel(5, 3, 0, 255, 0);

    // Smile (green) – a curve
    SetPixel(1, 5, 0, 255, 0);
    SetPixel(2, 6, 0, 255, 0);
    SetPixel(3, 6, 0, 255, 0);
    SetPixel(4, 6, 0, 255, 0);
    SetPixel(5, 6, 0, 255, 0);

    SetPixel(6, 5, 0, 255, 0);

}


void draw_sad_face(void)
{
    // Face outline
    for (uint8_t x = 1; x <= 6; ++x) {
        for (uint8_t y = 1; y <= 7; ++y) {
            SetPixel(x, y, 255, 0, 0);  // red
        }
    }

    // Eyes (black)
    SetPixel(2, 2, 0, 0, 0);
    SetPixel(5, 2, 0, 0, 0);

    // Sad mouth (black) – curve down
    SetPixel(3, 5, 0, 0, 0);
    SetPixel(4, 5, 0, 0, 0);
    SetPixel(2, 6, 0, 0, 0);
    SetPixel(5, 6, 0, 0, 0);
}

// Showing end screen
//  - If hits >= WIN_HITS  -> smiley face
//  - Else                -> sad face
//  - Always draw score bar on top row to show hits
void show_end_screen(void) {
	clear_screen();

	if (hits >= WIN_HITS) {
		draw_smiley_face();
	} else {
		draw_sad_face();
	}

	draw_score_bar();


}

// Restarting the game
void reset_game(void) {
	clear_screen();

	//Reset state
	hits = 0;
	bullets_used = 0;
	game_over = 0;

	ship_x = 3;

	alien_x = 6;
	alien_y = 0;
	alien_dx = 1;

	bullet_active = 0;
	bullet_x = 0;
	bullet_y = 0;

	// Draw intial objects
	draw_ship();
	draw_alien();
}


// Clear the current alien pixel
void clear_alien(void) {
	SetPixel(alien_x, alien_y, 0, 0, 0);
}

// Move ship one step left (if not at left edge)
void move_ship_left(void){
	if (ship_x == 0){return;}

	// erase the old ship position
	SetPixel(ship_x, 7,0,0,0);
	SetPixel(ship_x+1, 6,0,0,0);
	SetPixel(ship_x+1, 7,0,0,0);
	SetPixel(ship_x+2, 7,0,0,0);

	// update position
	ship_x--;

	// draw new ship
	draw_ship();
}

// Move ship one step right (if not too close to right edge)
void move_ship_right(void){
	if (ship_x >= 5){return;}

	// erase the old ship position
	SetPixel(ship_x, 7,0,0,0);
	SetPixel(ship_x+1, 6,0,0,0);
	SetPixel(ship_x+1, 7,0,0,0);
	SetPixel(ship_x+2, 7,0,0,0);

	// update position
	ship_x++;

	// draw new ship
	draw_ship();
}

// Fire bullet from the top-middle of the ship.
// Only allowed if game is running, there is no active bullet,
// and we still have bullets left.
void fire_bullet(void){
	if (game_over) {
		return;
	}

	if ( bullet_active == 1){
		return;
	}

	if (bullets_used >= MAX_BULLETS) {
		return;
	}

	bullets_used++;

	bullet_x = ship_x+1;
	bullet_y = 5;

	bullet_active = 1;

	SetPixel(bullet_x, bullet_y, 0,0,255);
}

// Move the bullet upwards by one row.
// Handles:
//  - Bullet going off-screen (miss)
//  - Bullet hitting alien (hit, score, respawn, win)
//  - Losing when all bullets are used and too few hits
void move_bullet(void){
	if (game_over) {
		return;
	}

	if (bullet_active == 0){
		return;
	}

	// Clear old bullet
	SetPixel(bullet_x, bullet_y, 0,0,0);

	bullet_y--;

	// Bullet went off the top -> miss
	if (bullet_y < 0){
		bullet_active = 0;

		// Check lose condition: used all bullets and not enough hits
		if (bullets_used >= MAX_BULLETS && hits < WIN_HITS) {
			game_over = 1;
			show_end_screen();
		}
		return;
	}

	// Check collision with alien
	if (bullet_x == alien_x && bullet_y == alien_y) {
		bullet_active = 0;
		hits++;

		// Remove alien from old position
		clear_alien();

		// Respawn alien from old position
		alien_x = 6;
		alien_y = 0;
		alien_dx = -1;
		draw_alien();


		// Check win condition
		if (hits >= WIN_HITS) {
			game_over = 1;
			show_end_screen();
			return;
		}

		return;
	}

	// No collision and still on screen -> draw bullet at new position
	SetPixel(bullet_x, bullet_y, 0,0,255);
}


int main()
{
	//**DO NOT REMOVE THIS****
	    init_platform();
	//************************


#ifdef	enable_interrupts
	    init_interrupts();
#endif


	    //setup screen
	    setup();
	    reset_game();





	    Xil_ExceptionEnable();



	    //Try to avoid writing any code in the main loop.
		while(1){
			blinker();

		}


		cleanup_platform();
		return 0;
}



//Timer interrupt handler for led matrix update. Frequency is 800 Hz
void TickHandler(void *CallBackRef){
	//Don't remove this
	uint32_t StatusEvent;

	// Exceptions must be disabled when updating screen
	Xil_ExceptionDisable();



	//****Write code here ****

	open_line(8);
	run(current_channel);
	open_line(current_channel);
	current_channel++;

	if(current_channel >= 8){
		current_channel = 0;
	}


	//****END OF OWN CODE*****************

	//*********clear timer interrupt status. DO NOT REMOVE********
	StatusEvent = XTtcPs_GetInterruptStatus((XTtcPs *)CallBackRef);
	XTtcPs_ClearInterruptStatus((XTtcPs *)CallBackRef, StatusEvent);
	//*************************************************************
	//enable exceptions
	Xil_ExceptionEnable();
}


//Timer interrupt for moving alien, shooting... Frequency is 10 Hz by default
void TickHandler1(void *CallBackRef){

	//Don't remove this
	uint32_t StatusEvent;

	//****Write code here ****

	if (game_over) {
		StatusEvent = XTtcPs_GetInterruptStatus((XTtcPs *)CallBackRef);
		XTtcPs_ClearInterruptStatus((XTtcPs *)CallBackRef, StatusEvent);
		return;
	}

	clear_alien();


	alien_x += alien_dx;
	if(alien_x < 0){
		alien_x = 0;
		alien_dx = 1;

	} else if (alien_x > 7){
		alien_x = 7;
		alien_dx = -1;

	}

	draw_alien();
	move_bullet();




	//****END OF OWN CODE*****************
	//clear timer interrupt status. DO NOT REMOVE
	StatusEvent = XTtcPs_GetInterruptStatus((XTtcPs *)CallBackRef);
	XTtcPs_ClearInterruptStatus((XTtcPs *)CallBackRef, StatusEvent);

}


//Interrupt handler for switches and buttons.
//Reading Status will tell which button or switch was used
//Bank information is useless in this exercise
void ButtonHandler(void *CallBackRef, u32 Bank, u32 Status){
	//****Write code here ****

	//Hint: Status==0x01 ->btn0, Status==0x02->btn1, Status==0x04->btn2, Status==0x08-> btn3, Status==0x10->SW0, Status==0x20 -> SW1

	//If true, btn0 was used to trigger interrupt
	if (!game_over) {
		if (Status & 0x01){ move_ship_right();}
		if (Status & 0x02){ move_ship_left();}
		if (Status & 0x04){ fire_bullet();}
	}

	//Restart game with switch 0
	if (Status & 0x08) {
		reset_game();
	}

	//****END OF OWN CODE*****************
}


