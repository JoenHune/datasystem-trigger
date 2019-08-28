#include "configuration.h"

#include <cmath>

#include "basic/common.h"

extern "C"
{
#include <stdio.h>

#include "device/gpio.h"

#include "st/stm32f10x_tim.h"
#include "st/stm32f10x_usart.h"
#include "st/stm32f10x_dma.h"

// usart2 dma发送中断
void DMA1_Channel7_IRQHandler()
{
    if (DMA_GetITStatus(DMA1_IT_TC7) != RESET)
    {
        DMA_ClearITPendingBit(DMA1_IT_TC7);

        DMA_Cmd(DMA1_Channel7, DISABLE);
    }
}

uint32_t receive_msg_size = 0;

void usart2_receive_msg_handler()
{
    DMA_Cmd(DMA1_Channel6, DISABLE);
    receive_msg_size = USART2_RX_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA1_Channel6);

    if (usart2_rx_buffer[receive_msg_size - 3] == '*')
    {
        gpio_toggle_bit(GPIOB, 12);
    }
    
    usart2_printblock(usart2_rx_buffer, receive_msg_size);

    DMA_SetCurrDataCounter(DMA1_Channel6, USART2_RX_BUFFER_SIZE);
    DMA_Cmd(DMA1_Channel6, ENABLE);

    DMA_ClearITPendingBit(DMA1_IT_TC6);
}

// usart2定长接收中断处理函数 基于DMA接收完毕函数
void DMA1_Channel6_IRQHandler()
{
    if (DMA_GetITStatus(DMA1_IT_TC6) != RESET)
    {
        usart2_receive_msg_handler();
    }
}

// usart2不定长接收中断处理函数 基于串口空闲中断
void USART2_IRQHandler()
{
    if (USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
    {
        USART2->SR;
        USART2->DR;

        usart2_receive_msg_handler();
    }
    
}

// 两个相机的曝光时间
uint16_t exposure_time1 = TRIGGER_PERIOD;
uint16_t exposure_time2 = TRIGGER_PERIOD;

int16_t  error_avg2[20] = {0};
uint8_t  index2         = 0;

uint16_t trigger_time(uint16_t exposure_time_tim3)
{
    return static_cast<uint16_t>(TRIGGER_PERIOD - exposure_time_tim3 / 2.f);
}

uint16_t max_trigger_time(uint16_t last_exposure_time_tim4)
{
    return static_cast<uint16_t>((2.f * (MAX_EXPOSURE_TIME - MIN_EXPOSURE_TIME) + last_exposure_time_tim4) / 3.f);
}

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
        limit<uint16_t>(exposure_time1, MIN_EXPOSURE_TIME, MAX_EXPOSURE_TIME);
        
        TIM_SetCompare1(TIM3, trigger_time(TIM_FACTOR_4to3 * exposure_time1));

        TIM_SetCounter(TIM2, 0);

        TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);
    }
}
uint16_t trigger = 0;
// camera2 exposure time calculator
void TIM4_IRQHandler()
{
    if (TIM_GetITStatus(TIM4, TIM_IT_CC1) != RESET)
    {
        gpio_reset_bit(GPIOB, 12);

        TIM_SetCounter(TIM4, 0);

        TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
    }
    
    if (TIM_GetITStatus(TIM4, TIM_IT_CC2) != RESET)
    {
        gpio_set_bit(GPIOB, 12);

        uint16_t last_exposure_time = exposure_time2;
        exposure_time2 = TIM_GetCounter(TIM4);

        // overflow-avoid
        limit<uint16_t>(exposure_time2, MIN_EXPOSURE_TIME, max_trigger_time(last_exposure_time));

        trigger = trigger_time(TIM_FACTOR_4to3 * exposure_time2);

        // error_avg2[index2++] = TIM_GetCapture2(TIM3) - trigger;
        error_avg2[index2++] = exposure_time2;

            
        TIM_SetCompare2(TIM3, trigger);

        TIM_SetCounter(TIM4, 0);

        TIM_ClearITPendingBit(TIM4, TIM_IT_CC2);
    }
}

uint16_t counter = 0;
uint16_t compare1 = 0;
uint16_t compare2 = 0;

uint8_t debug_message[USART2_TX_BUFFER_SIZE] = {0};
uint16_t length = 0;

void TIM3_IRQHandler()
{    
    if (TIM_GetITStatus(TIM3, TIM_IT_CC3) != RESET)
    {
        counter = TIM_GetCounter(TIM3);
        compare1 = TIM_GetCapture1(TIM3);
        compare2 = TIM_GetCapture2(TIM3);
        
        if (compare1 >= counter)
        {
            TIM_SetCompare1(TIM3, compare1 - counter);
        }
        if (compare2 >= counter)
        {
            TIM_SetCompare2(TIM3, compare2 - counter);
        }

        // float  time = DETECTOR_TIME_FACTOR * exposure_time2;

        // float error1 = 0;
        // for (uint8_t i = 0; i < CAMERA_FREQUENCE; i++)
        // {
        //     error1 += abs(error_avg2[i]);
        // }
        // error1 *= TRIGGER_TIME_FACTOR / CAMERA_FREQUENCE;

        // float error2 = TRIGGER_TIME_FACTOR * (TRIGGER_PERIOD - counter) / TRIGGER_PERIOD;

        // length = sprintf((char *)debug_message, "cam2: t=%.1f(ns), e1=%.1f(ns), e2=%f(ns), lost=%d \n", time, error1, error2, CAMERA_FREQUENCE - index2);
        length = sprintf((char *)debug_message, "e0=%7.1f(us) e1=%7.1f(us) e2=%7.1f(us) e3=%7.1f(us) e4=%7.1f(us) e5=%7.1f(us) e6=%7.1f(us) e7=%7.1f(us) e8=%7.1f(us) e9=%7.1f \n",
        DETECTOR_TIME_FACTOR * error_avg2[0], 
        DETECTOR_TIME_FACTOR * error_avg2[1], 
        DETECTOR_TIME_FACTOR * error_avg2[2], 
        DETECTOR_TIME_FACTOR * error_avg2[3], 
        DETECTOR_TIME_FACTOR * error_avg2[4], 
        DETECTOR_TIME_FACTOR * error_avg2[5], 
        DETECTOR_TIME_FACTOR * error_avg2[6], 
        DETECTOR_TIME_FACTOR * error_avg2[7], 
        DETECTOR_TIME_FACTOR * error_avg2[8], 
        DETECTOR_TIME_FACTOR * error_avg2[9]);
        
        usart2_printblock(debug_message, length);

        index2 = 0;

        TIM_SetCounter(TIM3, 0);

        TIM_ClearITPendingBit(TIM3, TIM_IT_CC3);
    }
}

}
