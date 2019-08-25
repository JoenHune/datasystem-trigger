#pragma once

#include "st/stm32f10x_usart.h"

#define IS_APB1_PERIPH(PERIPH) ((((uint32_t)PERIPH) != 0x00) && ((((uint32_t)PERIPH) & (uint32_t)0xFFFF0000) == APB1PERIPH_BASE))
#define IS_APB2_PERIPH(PERIPH) ((((uint32_t)PERIPH) != 0x00) && ((((uint32_t)PERIPH) & (uint32_t)0xFFFF0000) == APB2PERIPH_BASE))

void usart_configuration(USART_TypeDef * USARTx, uint32_t baudrate, uint16_t mode, uint16_t parity, uint16_t it);
