#include "fft_calculate.h"
#include "usart.h"

static float fftinput[sample * 2];
static float fftoutput[sample];
static uint16_t fftarry_count[cou];
static float har_base[cou];
Wave basewave = {0, {0, 0, 0, 0, 0}, false};

static void get_real_volt(volatile uint16_t* oldarr, float* newarr)
{
	for(uint16_t i = 0; i < sample; i++)
	{
		newarr[2 * i] = (Vrefp - Vrefn) * oldarr[i] / bits;
		newarr[2 * i + 1] = 0.0f;
	}
}

static void get_Har_Base(Wave* wptr)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		har_base[i] = (wptr->Harmo)[i] / (wptr->Harmo)[0];
	}
}

void ADC_FFT_calcualtion(arm_cfft_radix4_instance_f32* fftptr, Wave* wptr)
{
	if (ADC_State)
	{
		uint16_t har_count = 0;
		double sum = 0.0;
		get_real_volt(resultsBufferAlternate, fftinput);
		arm_cfft_radix4_f32(fftptr, fftinput);                  //FFT基4
		arm_cmplx_mag_f32(fftinput, fftoutput, sample);

		float max = fftoutput[1];
		for (uint16_t i = 1; i < sample / 2; i++)
		{
			if (fftoutput[i] > 1.5f * fftoutput[i - 1] && fftoutput[i] > 1.5f * fftoutput[i + 1])
			{
				if (fftoutput[i] > max)
				{
					har_count = i;
					max = fftoutput[i];
				}
			}
		}
		
		if (har_count < (sample / 10))
		{
			for (uint8_t i = 1; i <= cou; i++)
			{
				fftarry_count[i] = har_count * i;
			}
		}
		else 
		{
			if (ADC_samplerate < chioce)
			{
				ADC_State = false;
				wptr->state = false;
				ADC_samplerate++;
				ADC_Init();
				return;
			}
		}
		
		wptr->DC_float = fftoutput[0] / sample;
		for(uint8_t i = 1; i <= cou; i++)
		{
			if (fftarry_count[i] - fftarry_count[i - 1] > 5)
			{
				sum = (fftoutput[har_count * i] * fftoutput[har_count * i]) + 
				(fftoutput[har_count * i - 1] * fftoutput[har_count * i - 1]) + 
				(fftoutput[har_count * i + 1] * fftoutput[har_count * i + 1]) +
				(fftoutput[har_count * i - 2] * fftoutput[har_count * i - 2]) + 
				(fftoutput[har_count * i + 2] * fftoutput[har_count * i + 2]);
				(wptr->Harmo)[i - 1] = sqrt(sum) * 2.0f / sample;
			}
			else
			{
				sum = fftoutput[har_count * i] * 2.0f / sample;
				(wptr->Harmo)[i - 1] = sum;
			}
		}
		
		get_Har_Base(wptr);
		wptr->state = true;
		
		ADC_State = false;
		MAP_DMA_enableChannel(7);
		MAP_Timer_A_generatePWM(TIMER_A0_BASE, &(timerA_PWM[ADC_samplerate]));
	}
}

float get_THD(Wave* wptr)
{
    double thd = 0;
    for (uint8_t i = 1; i < 5; i++)
    {
			thd += (wptr->Harmo[i] * wptr->Harmo[i]);
		}
		thd = sqrt(thd) / wptr->Harmo[0];
    return thd;
}

void show_THD_result(Wave* wptr)
{
	if (wptr->state)
	{
		float temp = get_THD(wptr);
		uint16_t in = temp * 100;
		float fl = (temp * 100) - in;
		OLED_ShowString(10, 0, (uint8_t*)"THD:", 8);
		OLED_ShowNum(46, 0, in, 3, 8);
		OLED_ShowChar(64, 0, '.', 8);
		OLED_ShowNum(70, 0, (uint16_t)(fl * 100), 2, 8);
		OLED_ShowChar(82, 0, '%', 8);
		
		OLED_ShowString(10, 1, (uint8_t*)"Har1:", 8);
		OLED_ShowString(10, 2, (uint8_t*)"Har2:", 8);
		OLED_ShowString(10, 3, (uint8_t*)"Har3:", 8);
		OLED_ShowString(10, 4, (uint8_t*)"Har4:", 8);
		OLED_ShowString(10, 5, (uint8_t*)"Har5:", 8);
		
		for (uint8_t i = 1; i <= cou; i++)
		{
			fl = har_base[i - 1];
			in = fl * 100;
			fl = (fl * 100) - in;
			OLED_ShowNum(46, i, in, 3, 8);
			OLED_ShowChar(64, i, '.', 8);
			OLED_ShowNum(70, i, (uint16_t)(fl * 100), 2, 8);
			OLED_ShowChar(82, i, '%', 8);
		}
	}
}

void Screen_show_real_signal(Wave* wptr)
{
	if (wptr->state)
	{
		float point = 0;
		for(uint16_t i = 0; i < 480; i++)
		{
			point = -30.0f * ((wptr->Harmo)[0] * sin (i / 16.0) + (wptr->Harmo)[1] * sin (i * 2 / 16.0) + 
			(wptr->Harmo)[2] * sin (i * 3 / 16.0)  + (wptr->Harmo)[3] * sin (i * 4 / 16.0) + 
			(wptr->Harmo)[4] * sin (i * 5 / 16.0)) + 68;
			Screen_draw_point(i, point);
		}
		
		for(uint16_t i = 20; i < 400; i++)
		{
			Screen_drawline(i, (uint16_t)(-fftoutput[i] * 0.25f + 264), i + 1, (uint16_t)(-fftoutput[i + 1] * 0.25f + 264));
		}
	}
}

void Bluetooth_THD_result(Wave* wptr)
{
	if (wptr->state)
	{
		int16_t point;
		Bluetooth_printf("THD:%lf\n", get_THD(wptr));
		for(uint8_t i = 0; i < 5; i++)
		{
			Bluetooth_printf("Har%d:%lf\n\r", i + 1, har_base[i]);
		}
		
		for(uint16_t i = 0; i < 76; i++)
		{
			MAP_UART_transmitData(EUSCI_A1_BASE, 0xa5);
			point = (int16_t)(-100.0f * ((wptr->Harmo)[0] * sin (i / 4.0) + (wptr->Harmo)[1] * sin (i * 2 / 4.0) + 
			(wptr->Harmo)[2] * sin (i * 3 / 4.0)  + (wptr->Harmo)[3] * sin (i * 4 / 4.0) + 
			(wptr->Harmo)[4] * sin (i * 5 / 4.0)));
			MAP_UART_transmitData(EUSCI_A1_BASE, (int8_t)(point >> 8));
			MAP_UART_transmitData(EUSCI_A1_BASE, (uint8_t)point);
			MAP_UART_transmitData(EUSCI_A1_BASE, (uint8_t)((uint8_t)point + (uint8_t)(point >> 8)));
			MAP_UART_transmitData(EUSCI_A1_BASE, 0x5a);
		}		
	}
}

/*
	for (uint16_t i = 0; i < sample; i++)
	{
		printf("%d:%lf\r\n", i, fftoutput[i]);
	}
*/
