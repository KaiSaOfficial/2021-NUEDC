#include "sysinit.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "fft_calculate.h"

void Init(void);

int main(void)
{
	Init();
	arm_cfft_radix4_instance_f32 scfft;
	arm_cfft_radix4_init_f32(&scfft, sample, 0, 1);
	OLED_ColorTurn(0);
	OLED_DisplayTurn(1);
	Screen_setbeg(white);
	while (1)
	{
		ADC_FFT_calcualtion(&scfft, &basewave);
		show_THD_result(&basewave);
		Screen_show_real_signal(&basewave);
		Bluetooth_THD_result(&basewave);
		Screen_Init();
	}
}

void Init(void)
{
	SysInit();		   		
	uart_init(115200);
	delay_init();
	LED_Init();
	Screen_Init();
	OLED_Init();
	ADC_Init();
	Bluetooth_Init();
	/* 启用浮点运算的FPU */
	MAP_FPU_enableModule();
	MAP_FPU_enableLazyStacking();
}

