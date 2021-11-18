/*********************
 *
 * 最大采集电压 3.3V
 *
 * ADC采集引脚：
 * 单路 为 P5.5
 ************************/

#include "adc.h"
#include "string.h"

/* DMA Control Table */
#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_ALIGN(MSP_EXP432P401RLP_DMAControlTable, 1024)
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment=1024
#elif defined(__GNUC__)
__attribute__ ((aligned (1024)))
#elif defined(__CC_ARM)
__align(1024)
#endif

uint8_t ADC_samplerate = 0;
static DMA_ControlTable MSP_EXP432P401RLP_DMAControlTable[16];
bool ADC_State = false;
volatile uint16_t resultsBufferPrimary[sample];
volatile uint16_t resultsBufferAlternate[sample];

/*
	799,  //30KHz
	399,  //60KHz
	239,  //100KHz
	99,   //240KHz
	75    //320KHz
	49,   //480KHz
	39	  //600KHz
	29    //800KHz
	23,   //1MHz
*/

/*
 * Timer_A Compare Configuration Parameter
 * CCR1 is used to trigger the ADC14, conversion time
 * is defined by the resolution
 * 14bit -> 16 cycles + 1 cycle (SLAU356d, 20.2.8.3)
 * 12bit -> 14 cycles + 1 cycle
 * 10bit -> 11 cycles + 1 cycle
 *  8bit ->  9 cycles + 1 cycle
 *
 *  In this example, 14-bit resolution at 24Mhz ~708ns conversion time
 *  Sample time is defined by 4 ADC clocks
 *  Sample period is 24/24Mhz = 1us
 */
 
const Timer_A_PWMConfig timerA_PWM[] =
    {
        {
            .clockSource = TIMER_A_CLOCKSOURCE_SMCLK,
            .clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1,
            .timerPeriod = 799,
            .compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1,
            .compareOutputMode = TIMER_A_OUTPUTMODE_SET_RESET,
            .dutyCycle = 399,
        },
        {
            .clockSource = TIMER_A_CLOCKSOURCE_SMCLK,
            .clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1,
            .timerPeriod = 399,
            .compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1,
            .compareOutputMode = TIMER_A_OUTPUTMODE_SET_RESET,
            .dutyCycle = 199,
        },
        {
            .clockSource = TIMER_A_CLOCKSOURCE_SMCLK,
            .clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1,
            .timerPeriod = 239,
            .compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1,
            .compareOutputMode = TIMER_A_OUTPUTMODE_SET_RESET,
            .dutyCycle = 119,
        },
        {
            .clockSource = TIMER_A_CLOCKSOURCE_SMCLK,
            .clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1,
            .timerPeriod = 99,
            .compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1,
            .compareOutputMode = TIMER_A_OUTPUTMODE_SET_RESET,
            .dutyCycle = 49,
        },
				{
            .clockSource = TIMER_A_CLOCKSOURCE_SMCLK,
            .clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1,
            .timerPeriod = 59,
            .compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1,
            .compareOutputMode = TIMER_A_OUTPUTMODE_SET_RESET,
            .dutyCycle = 29,
        },
        {
            .clockSource = TIMER_A_CLOCKSOURCE_SMCLK,
            .clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1,
            .timerPeriod = 39,
            .compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1,
            .compareOutputMode = TIMER_A_OUTPUTMODE_SET_RESET,
            .dutyCycle = 19,
        },
				{
            .clockSource = TIMER_A_CLOCKSOURCE_SMCLK,
            .clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1,
            .timerPeriod = 39,
            .compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1,
            .compareOutputMode = TIMER_A_OUTPUTMODE_SET_RESET,
            .dutyCycle = 19,
        },
        {
            .clockSource = TIMER_A_CLOCKSOURCE_SMCLK,
            .clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1,
            .timerPeriod = 23,
            .compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1,
            .compareOutputMode = TIMER_A_OUTPUTMODE_SET_RESET,
            .dutyCycle = 11,
        },
};

/*
const Timer_A_PWMConfig timerA_PWM =
{
    .clockSource = TIMER_A_CLOCKSOURCE_SMCLK,
    .clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1,
    .timerPeriod = 39,
    .compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1,
    .compareOutputMode = TIMER_A_OUTPUTMODE_SET_RESET,
    .dutyCycle = 19
};
*/

void ADC_Init(void)
{
    memset((void *)resultsBufferPrimary, 0x00, sample);
    memset((void *)resultsBufferAlternate, 0x00, sample);

    /* Initializing ADC (MCLK/1/1) */
    /* Initializing ADC (SMCLK/1/1) */
    MAP_ADC14_enableModule();
    MAP_ADC14_initModule(ADC_CLOCKSOURCE_SMCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_3, 0);
	
		/*
     * Debug
     * Configuring P1.0 as output
     */
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    P1OUT &= ~BIT0;

    /*
     * Configuring GPIOs (5.5 A0)
     */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN5,
    GPIO_TERTIARY_MODULE_FUNCTION);
    /*
     * Configuring ADC Memory, repeat-single-channel, A0
     */
    MAP_ADC14_configureSingleSampleMode(ADC_MEM0, true);
    /*
     * Configuring ADC Memory, reference, and single ended conversion
     * A0 goes to mem0, AVcc is the reference, and the conversion is
     * single-ended
     */
    MAP_ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS,
    ADC_INPUT_A0, false);
    /*
     * Configuring the sample trigger to be sourced from Timer_A0 CCR1 and on the
     * rising edge, default samplemode is extended (SHP=0)
     */
    MAP_ADC14_setSampleHoldTrigger(ADC_TRIGGER_SOURCE1, false);

    /* Enabling the interrupt when a conversion on channel 1 is complete and
     * enabling conversions */
    MAP_ADC14_enableInterrupt(ADC_INT0);
    MAP_ADC14_enableConversion();

    /* Configuring DMA module */
    MAP_DMA_enableModule();
    MAP_DMA_setControlBase(MSP_EXP432P401RLP_DMAControlTable);

    /*
     * Setup the DMA + ADC14 interface
     */
    MAP_DMA_disableChannelAttribute(DMA_CH7_ADC14,
                                 UDMA_ATTR_ALTSELECT | UDMA_ATTR_USEBURST |
                                 UDMA_ATTR_HIGH_PRIORITY |
                                 UDMA_ATTR_REQMASK);

    /*
     * Setting Control Indexes. In this case we will set the source of the
     * DMA transfer to ADC14 Memory 0 and the destination to the destination
     * data array.
     */
    MAP_DMA_setChannelControl(UDMA_PRI_SELECT | DMA_CH7_ADC14,
    		UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
    MAP_DMA_setChannelTransfer(UDMA_PRI_SELECT | DMA_CH7_ADC14,
    		UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[0],
			(void*)resultsBufferPrimary, sample);
    MAP_DMA_setChannelControl(UDMA_ALT_SELECT | DMA_CH7_ADC14,
    		UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
    MAP_DMA_setChannelTransfer(UDMA_ALT_SELECT | DMA_CH7_ADC14,
    		UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[0],
			(void*)resultsBufferAlternate, sample);

    /* Assigning/Enabling Interrupts */
    DMA_assignInterrupt(DMA_INT1, 7);
    MAP_DMA_assignChannel(DMA_CH7_ADC14);
    MAP_DMA_clearInterruptFlag(7);

    /* Enabling Interrupts */
    MAP_Interrupt_enableInterrupt(INT_DMA_INT1);
    MAP_Interrupt_enableInterrupt(INT_PORT1);
    MAP_Interrupt_enableMaster();
				
    MAP_DMA_enableChannel(7);
    MAP_Timer_A_generatePWM(TIMER_A0_BASE, &(timerA_PWM[ADC_samplerate]));
}

void DMA_INT1_IRQHandler(void)
{
	MAP_Timer_A_stopTimer(TIMER_A0_BASE);
    MAP_DMA_disableChannel(7);
    P1->OUT |= BIT0;
    /*
     * Switch between primary and alternate bufferes with DMA's PingPong mode
     */
    if (MAP_DMA_getChannelAttribute(7) & UDMA_ATTR_ALTSELECT)
    {
//        MAP_DMA_setChannelControl(UDMA_PRI_SELECT | DMA_CH7_ADC14,
//        		UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
//        MAP_DMA_setChannelTransfer(UDMA_PRI_SELECT | DMA_CH7_ADC14,
//        		UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[0],
//    			resultsBufferPrimary, ARRAY_LENGTH);
MSP_EXP432P401RLP_DMAControlTable[7].control =
    (MSP_EXP432P401RLP_DMAControlTable[7].control & 0xff000000) |
    (((sample)-1) << 4) | 0x03;
    }
    else
    {
//        MAP_DMA_setChannelControl(UDMA_ALT_SELECT | DMA_CH7_ADC14,
//        		UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
//        MAP_DMA_setChannelTransfer(UDMA_ALT_SELECT | DMA_CH7_ADC14,
//        		UDMA_MODE_PINGPONG, (void*) &ADC14->MEM[0],
//    			resultsBufferAlternate, ARRAY_LENGTH);
MSP_EXP432P401RLP_DMAControlTable[15].control =
    (MSP_EXP432P401RLP_DMAControlTable[15].control & 0xff000000) |
    (((sample)-1) << 4) | 0x03;
    }
		ADC_State = true;
    P1->OUT &= ~BIT0;
}


