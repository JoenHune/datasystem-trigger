#pragma once
#include "st/stm32f10x.h"

const uint8_t  USART2_TX_BUFFER_SIZE = 255u;
const uint8_t  USART2_RX_BUFFER_SIZE = 255u;
const uint8_t  USART3_TX_BUFFER_SIZE = 255u;
const uint8_t  USART3_RX_BUFFER_SIZE = 255u;

// GPRMC数据接收
extern uint8_t usart2_rx_buffer[USART2_RX_BUFFER_SIZE];
// GPRMC数据转发
extern uint8_t usart2_tx_buffer[USART2_TX_BUFFER_SIZE];
// debug信息或控制命令接收
extern uint8_t usart3_rx_buffer[USART3_RX_BUFFER_SIZE];
// debug信息发送（如每帧误差、周期均帧误差等）
extern uint8_t usart3_tx_buffer[USART3_TX_BUFFER_SIZE];

// 定时器的基础时钟频率，单位：赫兹(Hz)
const float RCC_FREQNENCE           = 72.f * 1000.f * 1000.f;
// 时间换算倍数，1秒对应的微秒的数量
const float FACTOR_TIME_UNIT_S_TO_US        =  1.f * 1000.f * 1000.f;

// 相机触发频率，单位：赫兹(Hz)
const uint16_t CAMERA_FREQUENCE     = 20u;
// 最长允许曝光时间，单位：微秒(us)
const uint32_t MAX_EXPOSURE_TIME_US = FACTOR_TIME_UNIT_S_TO_US / CAMERA_FREQUENCE;
// 最短允许曝光时间，单位：微秒(us)
const uint32_t MIN_EXPOSURE_TIME_US = 100u;
// 两帧图像曝光最小间隔时间，单位：微秒(us)
const uint16_t C0                   = 5u;

// 触发信号频率，单位：赫兹(Hz)
const uint16_t TRIGGER_FREQUENCE    = CAMERA_FREQUENCE;
// 触发信号计时器每个计数单位的时间精度，单位：微秒(us)
const float TRIGGER_ACCURANCY       = 0.1f;
// 曝光时长检测计时器每个计数单位的时间精度，单位：微秒(us)
const float DETECTOR_ACCURANCY      = 0.1f;

// 每微秒(us)对应到触发信号定时器的计数单位个数，单位：次/微秒(us)
const float FACTOR_US_TO_TRIGGER_CNT     = 1.f / TRIGGER_ACCURANCY;
// 每微秒(us)对应到曝光时长检测信号定时器的计数单位个数，单位：次/微秒(us)
const float FACTOR_US_TO_DETECTOR_CNT    = 1.f / DETECTOR_ACCURANCY;
// 触发信号定时器的每个计数单位对应的时间，单位：微秒(us)/次
const float FACTOR_TRIGGER_CNT_TO_US     = TRIGGER_ACCURANCY;
// 相机曝光检测计时器的每个计数单位对应的时间，单位：微秒(us)/次
const float FACTOR_DETECTOR_CNT_TO_US    = DETECTOR_ACCURANCY;

// 触发信号计时器的更新周期
const uint16_t TRIGGER_PERIOD       = static_cast<uint16_t>(FACTOR_TIME_UNIT_S_TO_US / (TRIGGER_FREQUENCE * FACTOR_US_TO_TRIGGER_CNT));
// 触发信号计时器的预分频数
const uint16_t TRIGGER_PRESCALER    = static_cast<uint16_t>(RCC_FREQNENCE / (TRIGGER_FREQUENCE * TRIGGER_PERIOD));
// 曝光时长检测计时器的预分频数
const uint16_t DETECTOR_PRESCALER   = static_cast<uint16_t>(RCC_FREQNENCE / FACTOR_TIME_UNIT_S_TO_US * FACTOR_US_TO_DETECTOR_CNT);

// 定时器3的计时数量换算到定时器4的计时数量
const float FACTOR_TIM3to4          = 1.f * TRIGGER_PRESCALER / DETECTOR_PRESCALER;
// 定时器4的计时数量换算到定时器3的计时数量
const float FACTOR_TIM4to3          = 1.f / FACTOR_TIM3to4;

void timer1_configuration();
void timer2_configuration(uint8_t pre, uint8_t sub);
void timer3_configuration(uint8_t pre, uint8_t sub);
void timer4_configuration(uint8_t pre, uint8_t sub);

void usart2_configuration(uint8_t pre, uint8_t sub);
void usart3_configuration(uint8_t pre, uint8_t sub);

void usart2_printblock(uint8_t * data, uint8_t length);
void usart3_printblock(uint8_t * data, uint8_t length);

