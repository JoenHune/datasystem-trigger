#pragma once
#include "st/stm32f10x.h"

#define IS_APB1_PERIPH(PERIPH) ((((uint32_t)PERIPH) != 0x00) && ((((uint32_t)PERIPH) & (uint32_t)0xFFFF0000) == APB1PERIPH_BASE))
#define IS_APB2_PERIPH(PERIPH) ((((uint32_t)PERIPH) != 0x00) && ((((uint32_t)PERIPH) & (uint32_t)0xFFFF0000) == APB2PERIPH_BASE))

void timer_base_configuration(TIM_TypeDef *TIMx, uint16_t period, uint16_t prescaler, uint16_t division, uint16_t mode);
void timer_ic_configuration(TIM_TypeDef *TIMx, uint16_t channel, uint16_t polarity, uint16_t filter, uint16_t prescaler, uint16_t selection);
void timer_oc_configuration(TIM_TypeDef *TIMx, uint16_t channel, uint16_t pulse, uint16_t mode);
