#include "configuration.h"

#include <string.h>

extern "C"
{
#include "device/gpio.h"
#include "device/nvic.h"
#include "device/dma.h"
#include "device/usart.h"
#include "device/timer.h"

#include "st/stm32f10x_tim.h"
#include "st/stm32f10x_usart.h"
#include "st/stm32f10x_dma.h"
}

uint8_t usart2_rx_buffer[USART2_RX_BUFFER_SIZE] = {0};
uint8_t usart2_tx_buffer[USART2_TX_BUFFER_SIZE] = {0};

uint8_t usart3_rx_buffer[USART3_RX_BUFFER_SIZE] = {0};
uint8_t usart3_tx_buffer[USART3_TX_BUFFER_SIZE] = {0};

// rx接收来自GNSS系统的GPRMC信息
// tx发送GPRMC信息
void usart2_configuration(uint8_t pre, uint8_t sub)
{
    gpio_configuration(GPIOA, 2, GPIO_Mode_AF_PP, GPIO_Speed_50MHz); // tx
    gpio_configuration(GPIOA, 3, GPIO_Mode_IPU, GPIO_Speed_50MHz);   // rx

    usart_configuration(USART2, 115200, USART_Mode_Rx | USART_Mode_Tx, USART_Parity_No, USART_IT_IDLE);

    dma_configuration(DMA1, DMA1_Channel6, DMA_DIR_PeripheralSRC, (uint32_t) & (USART2->DR), (uint32_t)usart2_rx_buffer, USART2_RX_BUFFER_SIZE, DMA_IT_TC);
    dma_configuration(DMA1, DMA1_Channel7, DMA_DIR_PeripheralDST, (uint32_t) & (USART2->DR), (uint32_t)usart2_tx_buffer, USART2_TX_BUFFER_SIZE, DMA_IT_TC);

    nvic_configuration(ENABLE, USART2_IRQn, pre, sub);
    nvic_configuration(ENABLE, DMA1_Channel6_IRQn, pre, sub);
    nvic_configuration(ENABLE, DMA1_Channel7_IRQn, pre, sub);

    DMA_Cmd(DMA1_Channel6, ENABLE);
    // DMA_Cmd(DMA1_Channel7, ENABLE); // 默认不开启，开启即会传输

    USART_DMACmd(USART2, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);

    USART_Cmd(USART2, ENABLE);
}

// rx接收调试信息
// tx发送曝光时长信息
void usart3_configuration(uint8_t pre, uint8_t sub)
{
    gpio_configuration(GPIOB, 10, GPIO_Mode_AF_PP, GPIO_Speed_50MHz); // tx
    gpio_configuration(GPIOB, 11, GPIO_Mode_IPU, GPIO_Speed_50MHz);   // rx

    usart_configuration(USART3, 115200, USART_Mode_Rx | USART_Mode_Tx, USART_Parity_No, USART_IT_IDLE);

    dma_configuration(DMA1, DMA1_Channel3, DMA_DIR_PeripheralSRC, (uint32_t) & (USART3->DR), (uint32_t)usart3_rx_buffer, USART3_RX_BUFFER_SIZE, DMA_IT_TC);
    dma_configuration(DMA1, DMA1_Channel2, DMA_DIR_PeripheralDST, (uint32_t) & (USART3->DR), (uint32_t)usart3_tx_buffer, USART3_TX_BUFFER_SIZE, DMA_IT_TC);

    nvic_configuration(ENABLE, USART3_IRQn, pre, sub);
    nvic_configuration(ENABLE, DMA1_Channel3_IRQn, pre, sub);
    nvic_configuration(ENABLE, DMA1_Channel2_IRQn, pre, sub);

    DMA_Cmd(DMA1_Channel3, ENABLE);
    // DMA_Cmd(DMA1_Channel2, ENABLE); // 默认不开启，开启即会传输

    USART_DMACmd(USART3, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);

    USART_Cmd(USART3, ENABLE);
}

// 通过串口发送给定长度的字节流
void usart2_printblock(uint8_t *data, uint8_t length)
{
    if (length == 0)
        return;

    DMA_Cmd(DMA1_Channel7, DISABLE);

    if (data != usart2_tx_buffer)
        memcpy(usart2_tx_buffer, data, length);

    DMA_SetCurrDataCounter(DMA1_Channel7, length);
    DMA_Cmd(DMA1_Channel7, ENABLE);
}

// 通过串口发送给定长度的字节流
void usart3_printblock(uint8_t *data, uint8_t length)
{
    if (length == 0)
        return;

    DMA_Cmd(DMA1_Channel2, DISABLE);
    
    if (data != usart3_tx_buffer)
    {
        memcpy(usart3_tx_buffer, data, length);
    }

    DMA_SetCurrDataCounter(DMA1_Channel2, length);
    DMA_Cmd(DMA1_Channel2, ENABLE);
}

// 模拟PPS输出
// 定时器频率：1Hz
void timer1_configuration()
{
    gpio_configuration(GPIOA, 8, GPIO_Mode_AF_PP, GPIO_Speed_50MHz); // tim1_ch1 pps1_out(simulate)
    gpio_configuration(GPIOA, 9, GPIO_Mode_AF_PP, GPIO_Speed_50MHz); // tim1_ch1 pps1_out(simulate)

    // reduce from 72MHz
    timer_base_configuration(TIM1, 8000, 9000, TIM_CKD_DIV1, TIM_CounterMode_Up);

    timer_oc_configuration(TIM1, TIM_Channel_1, 100, TIM_OCMode_PWM1);

    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    TIM_Cmd(TIM1, ENABLE);
}

// 相机触发信号
// 定时器频率：与相机期望频率相同的PWM输出和100KHz的输入捕获
void timer3_configuration(uint8_t pre, uint8_t sub)
{
    gpio_configuration(GPIOA, 6, GPIO_Mode_AF_PP, GPIO_Speed_50MHz); // tim3_ch1 pps20_out1
    gpio_configuration(GPIOA, 7, GPIO_Mode_AF_PP, GPIO_Speed_50MHz); // tim3_ch2 pps20_out2
    gpio_configuration(GPIOB, 0, GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz);   // tim3_ch3 pps1_in

    nvic_configuration(ENABLE, TIM3_IRQn, pre, sub);

    // reduce from 72MHz
    timer_base_configuration(TIM3, TRIGGER_PERIOD, TRIGGER_PRESCALER, TIM_CKD_DIV1, TIM_CounterMode_Up);

    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    timer_oc_configuration(TIM3, TIM_Channel_1, TRIGGER_PERIOD, TIM_OCMode_PWM2);
    timer_oc_configuration(TIM3, TIM_Channel_2, TRIGGER_PERIOD, TIM_OCMode_PWM2);

    timer_ic_configuration(TIM3, TIM_Channel_3, TIM_ICPolarity_Rising, 0x00, TIM_ICPSC_DIV1, TIM_ICSelection_DirectTI);

    TIM_Cmd(TIM3, ENABLE);
}

// camera1曝光时间捕获
// 定时器频率：100KHz的输入捕获
void timer2_configuration(uint8_t pre, uint8_t sub)
{
    gpio_configuration(GPIOA, 0, GPIO_Mode_IPU, GPIO_Speed_50MHz); // tim2_ch1 camera1
    gpio_configuration(GPIOA, 1, GPIO_Mode_IPU, GPIO_Speed_50MHz); // tim2_ch2 camera1

    nvic_configuration(ENABLE, TIM2_IRQn, pre, sub);

    // reduce from 72MHz
    timer_base_configuration(TIM2, 0xFFFF, DETECTOR_PRESCALER, TIM_CKD_DIV1, TIM_CounterMode_Up);

    timer_ic_configuration(TIM2, TIM_Channel_1, TIM_ICPolarity_Rising, 0x00, TIM_ICPSC_DIV1, TIM_ICSelection_DirectTI);
    timer_ic_configuration(TIM2, TIM_Channel_2, TIM_ICPolarity_Falling, 0x00, TIM_ICPSC_DIV1, TIM_ICSelection_DirectTI);

    TIM_Cmd(TIM2, ENABLE);
}

// camera2曝光时间捕获
// 定时器频率：100KHz的输入捕获
void timer4_configuration(uint8_t pre, uint8_t sub)
{
    gpio_configuration(GPIOB, 6, GPIO_Mode_IPU, GPIO_Speed_50MHz); // tim4_ch1 camera2
    gpio_configuration(GPIOB, 7, GPIO_Mode_IPU, GPIO_Speed_50MHz); // tim4_ch2 camera2

    nvic_configuration(ENABLE, TIM4_IRQn, pre, sub);

    timer_base_configuration(TIM4, 0xFFFF, DETECTOR_PRESCALER, TIM_CKD_DIV1, TIM_CounterMode_Up);

    timer_ic_configuration(TIM4, TIM_Channel_1, TIM_ICPolarity_Rising, 0x00, TIM_ICPSC_DIV1, TIM_ICSelection_DirectTI);
    timer_ic_configuration(TIM4, TIM_Channel_2, TIM_ICPolarity_Falling, 0x00, TIM_ICPSC_DIV1, TIM_ICSelection_DirectTI);

    TIM_Cmd(TIM4, ENABLE);
}
