#ifndef __FFT_CALCULATE__
#define __FFT_CALCULATE__

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <arm_math.h>
#include "adc.h"
#include "oled.h"
#include "screen.h"
#include "bluetooth.h"

#define srate (uint16_t)(48000000 / (timerA_PWM.timerPeriod + 1))
#define resolution ((float)srate / sample)
#define harfre (uint16_t)(1000.f / resolution)

typedef struct
{
	float DC_float;
	float Harmo[cou];
	bool state;
}Wave;

extern Wave basewave;

void ADC_FFT_calcualtion(arm_cfft_radix4_instance_f32*, Wave*);
float get_THD(Wave *);
void show_THD_result(Wave* wptr);
void Screen_show_real_signal(Wave* wptr);
void Bluetooth_THD_result(Wave* wptr);

#endif

