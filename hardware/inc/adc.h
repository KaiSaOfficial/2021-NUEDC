/****************************************************/
//MSP432P401R
//ADC采集
//Bilibili：m-RNA
//E-mail:m-RNA@qq.com
//创建日期:2021/9/13
/****************************************************/

#ifndef __ADC_H
#define __ADC_H
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#define ADC_TRIGGER_TA0_C1 ADC_TRIGGER_SOURCE1
#define ADC_TRIGGER_TA0_C2 ADC_TRIGGER_SOURCE2
#define ADC_TRIGGER_TA1_C1 ADC_TRIGGER_SOURCE3
#define ADC_TRIGGER_TA1_C2 ADC_TRIGGER_SOURCE4
#define ADC_TRIGGER_TA2_C1 ADC_TRIGGER_SOURCE5
#define ADC_TRIGGER_TA2_C2 ADC_TRIGGER_SOURCE6
#define ADC_TRIGGER_TA3_C1 ADC_TRIGGER_SOURCE7

//M*N*3.56us
#define Vrefp (3.3f)
#define Vrefn (0.0f)
#define bits (1 << 14)
#define chioce (8)
//1024 smaple
#define sample (1024)
//chancel
#define channel 1   
#define cou 5

extern uint8_t ADC_samplerate;
extern bool ADC_State;
extern volatile uint16_t resultsBufferPrimary[sample];
extern volatile uint16_t resultsBufferAlternate[sample];
extern const Timer_A_PWMConfig timerA_PWM[];

void ADC_Init(void);
#endif
