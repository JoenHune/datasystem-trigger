#pragma once
#include "st/stm32f10x.h"

void usart2_configuration();

void timer1_configuration();
void timer2_configuration(uint8_t pre, uint8_t sub);
void timer3_configuration(uint8_t pre, uint8_t sub);
void timer4_configuration(uint8_t pre, uint8_t sub);

void usart2_printblock(uint8_t * data, uint8_t length);

// APB1 0x40000000
// APB2 0x40010000
// AHB  0x40020000
// #define IS_APB1_PERIPH(PERIPH) ((((uint32_t)PERIPH) != 0x00) && ((((uint32_t)PERIPH) & (uint32_t)0xFFFF0000) == APB1PERIPH_BASE))
// #define IS_APB2_PERIPH(PERIPH) ((((uint32_t)PERIPH) != 0x00) && ((((uint32_t)PERIPH) & (uint32_t)0xFFFF0000) == APB2PERIPH_BASE))
// #define IS_AHB_PERIPH(PERIPH)  (((PERIPH) != 0x00) && (((PERIPH) & (uint32_t)0xFFFF0000) == AHBPERIPH_BASE))