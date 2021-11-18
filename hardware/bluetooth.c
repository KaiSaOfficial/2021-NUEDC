#include "bluetooth.h"

#define USART3_MAX_SEND_LEN		400
__align(8) uint8_t USART3_TX_BUF[USART3_MAX_SEND_LEN]; 	//发送缓冲,最大USART3_MAX_SEND_LEN字节
#include <string.h>
#include "stdarg.h"

//与PC通信的串口初始化
void Bluetooth_Init(void)
{
  //P2.2 RX   P2.3 TX
  const eUSCI_UART_ConfigV1 uartConfig =
      {
          EUSCI_A_UART_CLOCKSOURCE_SMCLK,                // SMCLK Clock Source
          26,                                            // BRDIV = 26
          0,                                             // UCxBRF = 0
          111,                                           // UCxBRS = 111
          EUSCI_A_UART_NO_PARITY,                        // No Parity
          EUSCI_A_UART_LSB_FIRST,                        // MSB First
          EUSCI_A_UART_ONE_STOP_BIT,                     // One stop bit
          EUSCI_A_UART_MODE,                             // UART mode
          EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION, // Oversampling
          EUSCI_A_UART_8_BIT_LEN                         // 8 bit data length
      };
		eusci_calcBaudDividers((eUSCI_UART_ConfigV1 *)&uartConfig, 9600); //配置波特率

		MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P2, 
								GPIO_PIN2 |GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

		MAP_UART_initModule(EUSCI_A1_BASE, &uartConfig);

		/* Enabling interrupts */
		//MAP_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
		//MAP_Interrupt_enableInterrupt(INT_EUSCIA0);
		MAP_UART_enableModule(EUSCI_A1_BASE);
	}


//确保一次发送数据不超过USART3_MAX_SEND_LEN字节
void Bluetooth_printf(char* fmt,...)  
{  
	va_list ap;
	va_start(ap, fmt);
	vsprintf((char*)USART3_TX_BUF, fmt, ap);
	va_end(ap);
	uint16_t i = strlen((const char*)USART3_TX_BUF);//此次发送数据的长度
	for(uint16_t j = 0; j < i; j++)//循环发送数据
	{
		MAP_UART_transmitData(EUSCI_A1_BASE, USART3_TX_BUF[j]);
	}
}
