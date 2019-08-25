#include "configuration.h"

#include <string.h>
#include <stdio.h>

#include "basic/gpio.h"
#include "basic/nvic.h"
#include "basic/dma.h"
#include "device/usart.h"
#include "device/timer.h"

#include "st/stm32f10x_tim.h"
#include "st/stm32f10x_usart.h"
#include "st/stm32f10x_dma.h"

#define USART2_BUFFER_SIZE    255U
uint8_t usart2_rx_buffer[USART2_BUFFER_SIZE] = {0};
uint8_t usart2_tx_buffer[USART2_BUFFER_SIZE] = {0};

#define CAMERA_FREQUENCE      10
#define TRIGGER_PERIOD        40000
#define DETECTOR_PRESCALER    720
const uint32_t TRIGGER_PRESCALER    = 72000000 / (CAMERA_FREQUENCE * TRIGGER_PERIOD);
const uint32_t MAX_EXPOSURE_TIME    = 72000000 / (CAMERA_FREQUENCE * DETECTOR_PRESCALER);
const uint32_t DELAY_FACTOR         = DETECTOR_PRESCALER * CAMERA_FREQUENCE * TRIGGER_PERIOD / (2 * 72000000);

// rx接收来自GNSS系统的GPRMC信息
// tx发送曝光时长信息
void usart2_configuration()
{
    gpio_configuration(GPIOA, 2, GPIO_Mode_AF_PP, GPIO_Speed_50MHz); // tx
    gpio_configuration(GPIOA, 3, GPIO_Mode_IPU,   GPIO_Speed_50MHz); // rx

    usart_configuration(USART2, 115200, USART_Mode_Rx | USART_Mode_Tx, USART_Parity_No, USART_IT_IDLE);

    dma_configuration(DMA1, DMA1_Channel6, DMA_DIR_PeripheralSRC, (uint32_t)&(USART2->DR), (uint32_t)usart2_rx_buffer, USART2_BUFFER_SIZE, DMA_IT_TC);
    dma_configuration(DMA1, DMA1_Channel7, DMA_DIR_PeripheralDST, (uint32_t)&(USART2->DR), (uint32_t)usart2_tx_buffer, USART2_BUFFER_SIZE, DMA_IT_TC);

    DMA_Cmd(DMA1_Channel6, ENABLE);
    DMA_Cmd(DMA1_Channel7, ENABLE);

    USART_DMACmd(USART2, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);

    USART_Cmd(USART2, ENABLE);
}

// 通过串口发送给定长度的字节流
void usart2_printblock(uint8_t * data, uint8_t length)
{
    memcpy((uint8_t * )usart2_tx_buffer, data, length);

    DMA_Cmd(DMA1_Channel7, DISABLE);

    while (DMA_GetITStatus(DMA1_IT_TC7) == RESET);

    DMA_SetCurrDataCounter(DMA1_Channel7, length);

    DMA_Cmd(DMA1_Channel7, ENABLE);
}

// 使printf可用
int fputc(int ch, FILE *f)
{
    while (USART_GetITStatus(USART2, USART_IT_TC) == RESET);

    USART_SendData(USART2, (uint8_t)ch);

    return ch;
}

// cameras extern trigger signal
void timer1_configuration()
{   
    gpio_configuration(GPIOA, 8, GPIO_Mode_AF_PP, GPIO_Speed_50MHz);  // tim1_ch1 pps1_out(simulate)
    gpio_configuration(GPIOA, 9, GPIO_Mode_AF_PP, GPIO_Speed_50MHz);  // tim1_ch1 pps1_out(simulate)

    // reduce from 72MHz
    timer_base_configuration(TIM1, 8000, 9000, TIM_CKD_DIV1, TIM_CounterMode_Up);

    timer_oc_configuration(TIM1, TIM_Channel_1, 100, TIM_OCMode_PWM1);

    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    TIM_Cmd(TIM1, ENABLE);
}

// cameras extern trigger signal
void timer3_configuration(uint8_t pre, uint8_t sub)
{
    gpio_configuration(GPIOA, 6, GPIO_Mode_AF_PP, GPIO_Speed_50MHz);  // tim3_ch1 pps20_out1
    gpio_configuration(GPIOA, 7, GPIO_Mode_AF_PP, GPIO_Speed_50MHz);  // tim3_ch3 pps20_out2
    gpio_configuration(GPIOB, 0, GPIO_Mode_IPU,   GPIO_Speed_50MHz);  // tim3_ch4 pps1_in

    nvic_configuration(ENABLE, TIM3_IRQn, pre, sub);

    // reduce from 72MHz
    timer_base_configuration(TIM3, TRIGGER_PERIOD, TRIGGER_PRESCALER, TIM_CKD_DIV1, TIM_CounterMode_Up);

    timer_oc_configuration(TIM3, TIM_Channel_1, 30000, TIM_OCMode_PWM2);
    timer_oc_configuration(TIM3, TIM_Channel_2, 30000, TIM_OCMode_PWM2);

    timer_ic_configuration(TIM3, TIM_Channel_3, TIM_ICPolarity_Rising, 0x00, TIM_ICPSC_DIV1, TIM_ICSelection_DirectTI);
    
    TIM_Cmd(TIM3, ENABLE);
}

// camera1 exposure time counter
void timer2_configuration(uint8_t pre, uint8_t sub)
{
    gpio_configuration(GPIOA, 0, GPIO_Mode_IPU, GPIO_Speed_50MHz);  // tim2_ch1 camera1
    gpio_configuration(GPIOA, 1, GPIO_Mode_IPU, GPIO_Speed_50MHz);  // tim2_ch2 camera1

    nvic_configuration(ENABLE, TIM2_IRQn, pre, sub);

    // reduce from 72MHz
    timer_base_configuration(TIM2, 0xFFFF, DETECTOR_PRESCALER, TIM_CKD_DIV1, TIM_CounterMode_Up);

    timer_ic_configuration(TIM2, TIM_Channel_1, TIM_ICPolarity_Rising,  0x00, TIM_ICPSC_DIV1, TIM_ICSelection_DirectTI);
    timer_ic_configuration(TIM2, TIM_Channel_2, TIM_ICPolarity_Falling, 0x00, TIM_ICPSC_DIV1, TIM_ICSelection_DirectTI);
    
    TIM_Cmd(TIM2, ENABLE);
}

// camera2 exposure time counter
void timer4_configuration(uint8_t pre, uint8_t sub)
{
    gpio_configuration(GPIOB, 6, GPIO_Mode_IPU, GPIO_Speed_50MHz);  // tim4_ch1 camera2
    gpio_configuration(GPIOB, 7, GPIO_Mode_IPU, GPIO_Speed_50MHz);  // tim4_ch2 camera2

    nvic_configuration(ENABLE, TIM4_IRQn, pre, sub);

    // reduce to 1MHz from 72MHz at APB1 (dont know why but let it go)
    timer_base_configuration(TIM4, 0xFFFF, DETECTOR_PRESCALER, TIM_CKD_DIV1, TIM_CounterMode_Up);
    
    timer_ic_configuration(TIM4, TIM_Channel_1, TIM_ICPolarity_Rising,  0x00, TIM_ICPSC_DIV1, TIM_ICSelection_DirectTI);
    timer_ic_configuration(TIM4, TIM_Channel_2, TIM_ICPolarity_Falling, 0x00, TIM_ICPSC_DIV1, TIM_ICSelection_DirectTI);
    
    TIM_Cmd(TIM4, ENABLE);
}



// irq
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


uint16_t counter = 0;
uint16_t comper1 = 0;
uint16_t comper2 = 0;
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

        TIM_SetCounter(TIM3, 0);

        TIM_ClearITPendingBit(TIM3, TIM_IT_CC3);
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
        gpio_reset_bit(GPIOB, 12);

        TIM_SetCounter(TIM4, 0);

        TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
    }
    
    if (TIM_GetITStatus(TIM4, TIM_IT_CC2) != RESET)
    {
        gpio_set_bit(GPIOB, 12);

        exposure_time2 = TIM_GetCounter(TIM4);

        // overflow-avoid
        if (exposure_time2 >= MAX_EXPOSURE_TIME)
            exposure_time2 = MAX_EXPOSURE_TIME;
        
        exposure_time2 = (uint32_t)(TRIGGER_PERIOD - exposure_time2 * DELAY_FACTOR);
        TIM_SetCompare2(TIM3, exposure_time2);

        TIM_SetCounter(TIM4, 0);

        TIM_ClearITPendingBit(TIM4, TIM_IT_CC2);
    }
}
