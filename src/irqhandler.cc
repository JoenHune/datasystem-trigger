#include "configuration.h"

extern "C"
{
    
#include "basic/gpio.h"

#include "st/stm32f10x_tim.h"
#include "st/stm32f10x_usart.h"
#include "st/stm32f10x_dma.h"

// usart2 dma发送中断
void DMA1_Channel7_IRQHandler()
{
    if (DMA_GetITStatus(DMA1_IT_TC7) != RESET)
    {
        DMA_ClearITPendingBit(DMA1_IT_TC7);
    }
}

uint32_t receive_msg_size = 0;

// usart2 dma接收中断
void DMA1_Channel6_IRQHandler()
{
    if (DMA_GetITStatus(DMA1_IT_TC6) != RESET)
    {
        DMA_Cmd(DMA1_Channel6, DISABLE);
        receive_msg_size = USART2_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA1_Channel6);

        for (uint16_t i = 0; i < receive_msg_size; i++)
        {

        }

        DMA_SetCurrDataCounter(DMA1_Channel6, USART2_BUFFER_SIZE);
        DMA_Cmd(DMA1_Channel6, ENABLE);

        DMA_ClearITPendingBit(DMA1_IT_TC6);
    }
}

// usart2接收中断
void USART2_IRQHandler()
{
    if (USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
    {
        gpio_toggle_bit(GPIOB, 12);
        
        USART2->SR;
        USART2->DR;

        DMA_Cmd(DMA1_Channel6, DISABLE);
        receive_msg_size = USART2_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA1_Channel6);

        for (uint16_t i = 0; i < receive_msg_size; i++)
        {

        }

        DMA_SetCurrDataCounter(DMA1_Channel6, USART2_BUFFER_SIZE);
        DMA_Cmd(DMA1_Channel6, ENABLE);
    }
    
}

// 两个相机的曝光时间
uint32_t exposure_time1 = TRIGGER_PERIOD;
uint32_t exposure_time2 = TRIGGER_PERIOD;

// camera1 exposure time calculator
void TIM2_IRQHandler()
{
    if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET)
    {
        TIM_SetCounter(TIM2, 0);

        TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);
    }
    
    if (TIM_GetITStatus(TIM2, TIM_IT_CC2) != RESET)
    {
        exposure_time1 = TIM_GetCounter(TIM2);

        // overflow-avoid
        if (exposure_time1 >= MAX_EXPOSURE_TIME)
            exposure_time1 = MAX_EXPOSURE_TIME;
        else if (exposure_time1 <= MIN_EXPOSURE_TIME)
            exposure_time1 = MIN_EXPOSURE_TIME;

        exposure_time1 = (uint32_t)(TRIGGER_PERIOD - exposure_time1 * DELAY_FACTOR);
        TIM_SetCompare1(TIM3, exposure_time1);

        TIM_SetCounter(TIM2, 0);

        TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);
    }
}

// camera2 exposure time calculator
void TIM4_IRQHandler()
{
    if (TIM_GetITStatus(TIM4, TIM_IT_CC1) != RESET)
    {
        // gpio_reset_bit(GPIOB, 12);

        TIM_SetCounter(TIM4, 0);

        TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
    }
    
    if (TIM_GetITStatus(TIM4, TIM_IT_CC2) != RESET)
    {
        // gpio_set_bit(GPIOB, 12);

        exposure_time2 = TIM_GetCounter(TIM4);

        // overflow-avoid
        if (exposure_time2 >= MAX_EXPOSURE_TIME)
            exposure_time2 = MAX_EXPOSURE_TIME;
        else if (exposure_time2 <= MIN_EXPOSURE_TIME)
            exposure_time2 = MIN_EXPOSURE_TIME;
        
        exposure_time2 = (uint32_t)(TRIGGER_PERIOD - exposure_time2 * DELAY_FACTOR);
        TIM_SetCompare2(TIM3, exposure_time2);

        TIM_SetCounter(TIM4, 0);

        TIM_ClearITPendingBit(TIM4, TIM_IT_CC2);
    }
}

uint16_t counter = 0;
uint16_t comper1 = 0;
uint16_t comper2 = 0;

uint8_t test_arr[3] = {'a', 'b', '\n'};

void TIM3_IRQHandler()
{    
    if (TIM_GetITStatus(TIM3, TIM_IT_CC3) != RESET)
    {
        counter = TIM_GetCounter(TIM3);
        comper1 = TIM_GetCapture1(TIM3);
        comper2 = TIM_GetCapture2(TIM3);
        
        if (comper1 >= counter)
        {
            TIM_SetCompare1(TIM3, comper1 - counter);
        }
        if (comper2 >= counter)
        {
            TIM_SetCompare2(TIM3, comper2 - counter);
        }

        // printf("cam1: t=%ld, e=%d | cam2: t=%ld, e=%d\n", exposure_time1, comper1 - counter, exposure_time2, comper2 - counter);

        // usart2_printblock(test_arr, 3);

        TIM_SetCounter(TIM3, 0);

        TIM_ClearITPendingBit(TIM3, TIM_IT_CC3);
    }
}

}