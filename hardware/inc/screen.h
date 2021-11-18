#ifndef __SCREEN__
#define __SCREEN__

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "usart.h"

typedef enum
{
	be = 0xaa,
	cl = 0x52,
	set_bg = 0x40,
	line = 0x56,
	point = 0x51,
	setpen = 0x40,
	fresh = 0xd0,
	ed1 = 0xcc, 
	ed2 = 0x33, 
	ed3 = 0xc3, 
	ed4 = 0x3C,
}screen;

typedef enum
{
	white = 0xffff,
	black = 0x0000,
} pencolor;

void Screen_Init(void);
void Screen_setbeg(uint16_t);
void Screen_draw_point(uint16_t x, uint16_t y);
void Screen_drawline(uint16_t x, uint16_t y, uint16_t x_, uint16_t y_);

#endif
