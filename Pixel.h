/*
 * Pixel.h
 *
 *  Created on: ------
 *      Author: ------
 */

#ifndef PIXEL_H_
#define PIXEL_H_

// Memory-mapped hardware registers

// control signals: RSTn, LAT, SB, SCK, SDA
#define CTRL *(uint8_t*)0x41220008

// channel bits C0..C7
#define CHANNEL *(uint8_t*)0x41220000

// Bit masks
#define RSTn (1 << 0) // reset
#define LAT  (1 << 1) // latch
#define SB   (1 << 2) // select 6-bit / 8-bit bank
#define SCK  (1 << 3) // serial clock
#define SDA  (1 << 4) // serial data

#include <stdint.h>
#include "platform.h"
#include "xil_printf.h"
#include "sleep.h"
#include "xgpiops.h"
#include "xttcps.h"
#include "xscugic.h"
#include "xparameters.h"

// uint8_t *name = 0x1234;


void setup();
void SetPixel(uint8_t x,uint8_t y, uint8_t r, uint8_t g, uint8_t b);
void run(uint8_t x);
void latch();
void open_line(uint8_t x);

#endif /* PIXEL_H_ */


