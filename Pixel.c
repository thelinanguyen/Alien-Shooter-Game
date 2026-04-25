/*
 * Pixel.c
 *
 *  Created on: -----
 *      Author: -----
 */

#include "Pixel.h"



//Table for pixel dots.
//				 dots[X][Y][COLOR]
volatile uint8_t dots[8][8][3]={0};


// Here the setup operations for the LED matrix will be performed
void setup(){
	// Resetting the screen at start is a MUST to operation (Set RSTn to 1).
	CTRL = 0x00;
	CHANNEL = 0x00;

	//Pull RSTn LOW to trigger reset
	CTRL  &= ~(1<<0);
	usleep(500); // wait 500microseconds for reset to complete

	// Pull RSTn HIGH exit reset
	CTRL |= (1 << 0);
	usleep(500);

	//Write code that sets 6-bit values in register of DM163 chip. Recommended that every bit in that register is set to 1. 6-bits and 24 "bytes", so some kind of loop structure could be nice.
	//24*6 bits needs to be transmitted

	// select the 6-bit register (brightness)
	CTRL  &= ~(1 << 2); //SB = 0 (6-bit register selected)

	// set SDA HIGH
	//CTRL  |= ~(1 << 4);
	CTRL  |= (1 << 4);

	// Send 144 bits (all 1's) by pulsing the clock 144 times
	// 8 lEDs * 3 colors * 6 bits each = 144 bits
	for (int i = 0; i < 144 ; i++) {
		// clock low -> prepare for rising edge
		CTRL  &= ~(1 << 3);

		// clock HIGH
		CTRL |= (1 << 3);
	}

	// Leave clock LOW at the end
	CTRL  &= ~(1 << 3);

	//Final thing in this function is to set SB-bit to 1 to enable transmission to 8-bit register.
	CTRL |= (1 << 2); //(8-bit bank selected)
}

//Change value of one pixel at led matrix. This function is only used for changing values of dots array
void SetPixel(uint8_t x,uint8_t y, uint8_t r, uint8_t g, uint8_t b){

	//Hint: you can invert Y-axis quite easily with 7-y
	dots[x][y][0]=b;
	dots[x][y][1]=g;
	dots[x][y][2]=r;
	//Write rest of two lines of code required to make this function work properly (green and red colors to array).
}


//Put new data to led matrix. Hint: This function is supposed to send 24-bytes and parameter x is for channel x-coordinate.
void run(uint8_t x){



	//Write code that writes data to led matrix driver (8-bit data). Use values from dots array
	//Hint: use nested loops (loops inside loops)
	//Hint2: loop iterations are 8,3,8 (pixels,color,8-bitdata)

	// First, LAT should be zero
	CTRL &= ~(1 << 1);

	// Loops: 8 pixels (y), 3 colors (c), 8 bits per color
	for (uint8_t y = 0; y < 8 ; ++y){
		for (uint8_t c = 0; c < 3; ++c){
			uint8_t value = dots[x][y][c]; // one 8-bit color value

			// Send this byte MSB first (bit 7 down to bit 0)
			for (int bit = 7; bit >= 0; --bit){
				if(value & ( 1 << bit)){
					CTRL |= SDA; // Bit = 1 → SDA high

				}else {
					CTRL &= ~SDA; // Bit = 0 → SDA low

				}
				// Clock pulse
				CTRL |= SCK;
				CTRL &= ~SCK;
			}
		}
	}
	// Copy shifted data to the DM163 output registers
	latch();

	// Ensure clock line is low when we exit
	CTRL &= ~SCK;

}

//Latch signal. See colorsshield.pdf or DM163.pdf in project folder on how latching works
void latch(){
	CTRL |= (1 << 1); // (LAT = HIGH)
	CTRL &= ~(1 << 1); // (LAT = LOW)
}

//Set one line (channel) as active, one at a time.
//0x41220000
void open_line(uint8_t x){
	switch(x){
	case 0 :
		CHANNEL = 0b00000001;
		break;
	case 1 :
			CHANNEL = 0b00000010;
			break;
	case 2 :
			CHANNEL = 0b00000100;
			break;
	case 3 :
			CHANNEL = 0b00001000;
			break;
	case 4 :
			CHANNEL = 0b00010000;
			break;
	case 5 :
			CHANNEL = 0b00100000;
			break;
	case 6 :
			CHANNEL = 0b01000000;
			break;
	case 7 :
			CHANNEL = 0b10000000;
			break;
	default:
		// if x is invalid, turn all  columns off
		CHANNEL = 0b00000000;
		break;
	}


}



