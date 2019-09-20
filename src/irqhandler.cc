#include "configuration.h"

#include "basic/common.h"

#include "cameras.h"

extern "C"
{
#include <stdio.h>

#include "device/gpio.h"

#include "st/stm32f10x_tim.h"
#include "st/stm32f10x_usart.h"
#include "st/stm32f10x_dma.h"

uint8_t test_m[] = "test"; 

// usart2接收处理函数
void usart2_receive_msg_handler()
{
    DMA_Cmd(DMA1_Channel6, DISABLE);

    // uint32_t length = USART2_RX_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA1_Channel6);

    // gpio_toggle_bit(GPIOB, 12);

    // usart2_printblock(test_m, 4);

    DMA_SetCurrDataCounter(DMA1_Channel6, USART2_RX_BUFFER_SIZE);
    DMA_Cmd(DMA1_Channel6, ENABLE);
}

// usart3接收处理函数
void usart3_receive_msg_handler()
{
    DMA_Cmd(DMA1_Channel3, DISABLE);

    // uint32_t length = USART3_RX_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA1_Channel3);

    DMA_SetCurrDataCounter(DMA1_Channel3, USART3_RX_BUFFER_SIZE);
    DMA_Cmd(DMA1_Channel3, ENABLE);
}

// ============================ USART2/3 发送 ============================ //

// usart2 dma发送中断
void DMA1_Channel7_IRQHandler()
{
    if (DMA_GetITStatus(DMA1_IT_TC7) != RESET)
    {
        DMA_ClearITPendingBit(DMA1_IT_TC7);

        DMA_Cmd(DMA1_Channel7, DISABLE);
    }
}

// usart3 dma发送中断
void DMA1_Channel2_IRQHandler()
{
    if (DMA_GetITStatus(DMA1_IT_TC2) != RESET)
    {
        DMA_ClearITPendingBit(DMA1_IT_TC2);
    
        DMA_Cmd(DMA1_Channel2, DISABLE);
    } 
}

// ============================ USART2 接收 ============================ //

// usart2定长接收中断处理函数 基于DMA接收完毕函数
void DMA1_Channel6_IRQHandler()
{
    if (DMA_GetITStatus(DMA1_IT_TC6) != RESET)
    {
        usart2_receive_msg_handler();

        DMA_ClearITPendingBit(DMA1_IT_TC6);
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

// ============================ USART3 接收 ============================ //

// usart3定长接收中断处理函数 基于DMA接收完毕函数
void DMA1_Channel3_IRQHandler()
{
    if (DMA_GetITStatus(DMA1_IT_TC3) != RESET)
    {
        usart3_receive_msg_handler();

        DMA_ClearITPendingBit(DMA1_IT_TC3);
    }
}
// usart3不定长接收中断处理函数 基于串口空闲中断
void USART3_IRQHandler()
{
    if (USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
    {
        USART3->SR;
        USART3->DR;

        usart3_receive_msg_handler();
    }
}

// ============================ TIMers ============================ //

// 获取camera1的曝光时间和预测下一帧曝光时间
void TIM2_IRQHandler()
{
    if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET)
    {
        // gpio_reset_bit(GPIOB, 12);  // debug用途

        TIM_SetCounter(TIM2, 0);

        TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);
    }

    if (TIM_GetITStatus(TIM2, TIM_IT_CC2) != RESET)
    {
        // gpio_set_bit(GPIOB, 12);    // debug用途

        update_camera_trigger_time(CAM1, TIM2, TIM3, TIM_Channel_1);

        TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);
    }
}

// 获取camera2的曝光时间和预测下一帧曝光时间
void TIM4_IRQHandler()
{
    if (TIM_GetITStatus(TIM4, TIM_IT_CC1) != RESET)
    {
        TIM_SetCounter(TIM4, 0);

        TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
    }

    if (TIM_GetITStatus(TIM4, TIM_IT_CC2) != RESET)
    {
        update_camera_trigger_time(CAM2, TIM4, TIM3, TIM_Channel_2);

        TIM_ClearITPendingBit(TIM4, TIM_IT_CC2);
    }
}

void TIM3_IRQHandler()
{
    // 捕获PPS信号上升沿
    if (TIM_GetITStatus(TIM3, TIM_IT_CC3) != RESET)
    {
        // 只有TIM3的误差很大的时候才会有用？
        if (TIM_GetCapture1(TIM3) >= TIM_GetCounter(TIM3))
        {
            TIM_SetCompare1(TIM3, TIM_GetCapture1(TIM3) - TIM_GetCounter(TIM3));
        }
        if (TIM_GetCapture2(TIM3) >= TIM_GetCounter(TIM3))
        {
            TIM_SetCompare2(TIM3, TIM_GetCapture2(TIM3) - TIM_GetCounter(TIM3));
        }

        message_show_all_errors(CAM2);

        TIM_SetCounter(TIM3, 0);

        TIM_ClearITPendingBit(TIM3, TIM_IT_CC3);
    }

    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        update_predict_error_index();

        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}

}
