#include "screen.h"

void Screen_Init(void)
{
	uint8_t temp[] = {be, cl, ed1, ed2, ed3, ed4};
	USARTA0_SendString(temp);
}

void Screen_setbeg(uint16_t color)
{
	uint8_t temp[] = {be, setpen, 0x00, 0x00, (uint8_t)(color >> 8), (uint8_t)color, ed1, ed2, ed3, ed4};
	USARTA0_SendString(temp);
}

void Screen_draw_point(uint16_t x, uint16_t y)
{
	uint8_t temp[10] = {be, point, 0};
	temp[2] = (uint8_t)(x >> 8);
	temp[3] = (uint8_t)(x);
	temp[4] = (uint8_t)(y >> 8);
	temp[5] = (uint8_t)(y);
	temp[6] = ed1;
	temp[7] = ed2;
	temp[8] = ed3;
	temp[9] = ed4;
	USARTA0_SendString(temp);
}

void Screen_drawline(uint16_t x, uint16_t y, uint16_t x_, uint16_t y_)
{
    uint8_t temp[14] = {be, line, 0};
    temp[2] = (uint8_t)(x >> 8);
    temp[3] = (uint8_t)(x);
    temp[4] = (uint8_t)(y >> 8);
    temp[5] = (uint8_t)(y);
    temp[6] = (uint8_t)(x_ >> 8);
    temp[7] = (uint8_t)(x_);
    temp[8] = (uint8_t)(y_ >> 8);
    temp[9] = (uint8_t)(y_);
    temp[10] = ed1;
    temp[11] = ed2;
    temp[12] = ed3;
    temp[13] = ed4;

    USARTA0_SendString(temp);
}
