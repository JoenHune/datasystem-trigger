#include "device/timer.h"

#include "st/stm32f10x_tim.h"
#include "st/stm32f10x_rcc.h"

void timer_base_configuration(TIM_TypeDef *TIMx, uint16_t period, uint16_t prescaler, uint16_t division, uint16_t mode)
{
    if (IS_APB1_PERIPH(TIMx))
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 << (((uint32_t)TIMx - TIM2_BASE) / 0x400), ENABLE);
    }
    else if (IS_APB2_PERIPH(TIMx))
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 << (((uint32_t)TIMx - TIM1_BASE) / 0x400), ENABLE);
    }

    TIM_TimeBaseInitTypeDef timebase;

    timebase.TIM_Period = period;
    timebase.TIM_Prescaler = prescaler - 1U;
    timebase.TIM_ClockDivision = division;
    timebase.TIM_CounterMode = mode;

    TIM_TimeBaseInit(TIMx, &timebase);
}

void timer_ic_configuration(TIM_TypeDef *TIMx, uint16_t channel, uint16_t polarity, uint16_t filter, uint16_t prescaler, uint16_t selection)
{
    TIM_ICInitTypeDef ic;

    ic.TIM_Channel = channel;
    ic.TIM_ICFilter = filter;
    ic.TIM_ICPolarity = polarity;
    ic.TIM_ICPrescaler = prescaler;
    ic.TIM_ICSelection = selection;

    TIM_ICInit(TIMx, &ic);

    TIM_ITConfig(TIMx, TIM_IT_CC1 << (channel / 0x0004), ENABLE);
    TIM_ClearITPendingBit(TIMx, TIM_IT_CC1 << (channel / 0x0004));
}

void timer_oc_configuration(TIM_TypeDef *TIMx, uint16_t channel, uint16_t pulse, uint16_t mode)
{
    TIM_OCInitTypeDef oc;

    oc.TIM_OCMode = mode;
    
    oc.TIM_OutputState  = TIM_OutputState_Enable;
    oc.TIM_OCPolarity   = TIM_OCPolarity_High;
    oc.TIM_OCIdleState  = TIM_OCIdleState_Reset;
    // 互补输出（仅高级定时器有此功能）
    oc.TIM_OutputNState = TIM_OutputState_Disable;
    oc.TIM_OCNPolarity  = TIM_OCPolarity_High;
    oc.TIM_OCNIdleState = TIM_OCIdleState_Set;

    oc.TIM_Pulse = pulse;

    switch (channel)
    {
    case TIM_Channel_1:
        TIM_OC1Init(TIMx, &oc);
        TIM_OC1PreloadConfig(TIMx, TIM_OCPreload_Enable);
        break;
    
    case TIM_Channel_2:
        TIM_OC2Init(TIMx, &oc);
        TIM_OC2PreloadConfig(TIMx, TIM_OCPreload_Enable);
        break;
    
    case TIM_Channel_3:
        TIM_OC3Init(TIMx, &oc);
        TIM_OC3PreloadConfig(TIMx, TIM_OCPreload_Enable);
        break;
    
    case TIM_Channel_4:
        TIM_OC4Init(TIMx, &oc);
        TIM_OC4PreloadConfig(TIMx, TIM_OCPreload_Enable);
        break;
    
    default:
        break;
    }

    TIM_ARRPreloadConfig(TIMx, ENABLE);
}