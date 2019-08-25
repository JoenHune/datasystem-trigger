#include "device/usart.h"

#include "st/stm32f10x_usart.h"
#include "st/stm32f10x_rcc.h"

void usart_configuration(USART_TypeDef * USARTx, uint32_t baudrate, uint16_t mode, uint16_t parity, uint16_t it)
{
    if (IS_APB1_PERIPH(USARTx))
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 << (((uint32_t)USARTx - USART2_BASE) / 0x400), ENABLE);
    }
    else if (IS_APB2_PERIPH(USARTx))
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 << (((uint32_t)USARTx - USART1_BASE) / 0x400), ENABLE);
    }

    USART_InitTypeDef usart;

    usart.USART_BaudRate = baudrate;
    usart.USART_Mode = mode;
    usart.USART_Parity = parity;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

    USART_Init(USARTx, &usart);

    USART_ITConfig(USARTx, it, ENABLE);
}
