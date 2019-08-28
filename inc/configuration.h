#pragma once
#include "st/stm32f10x.h"

const uint8_t  USART2_TX_BUFFER_SIZE   = 255U;
const uint8_t  USART2_RX_BUFFER_SIZE   = 255U;

const uint16_t CAMERA_FREQUENCE     = 10U;

const uint16_t TRIGGER_PERIOD       = 40000U;
const uint16_t TRIGGER_PRESCALER    = 72000000U / (CAMERA_FREQUENCE * TRIGGER_PERIOD);

const uint16_t DETECTOR_PRESCALER   = 720U;
const uint16_t MAX_EXPOSURE_TIME    = 100000U * 720U / (CAMERA_FREQUENCE * DETECTOR_PRESCALER);
const uint16_t MIN_EXPOSURE_TIME    =   1000U * 720U / (CAMERA_FREQUENCE * DETECTOR_PRESCALER);
const uint16_t C0                   =    500U * 720U / (CAMERA_FREQUENCE * DETECTOR_PRESCALER);

const float TIM_FACTOR_4to3          = 1.f * DETECTOR_PRESCALER / TRIGGER_PRESCALER;
const float TIM_FACTOR_3to4          = 1.f * TRIGGER_PRESCALER / DETECTOR_PRESCALER;
const float TRIGGER_TIME_FACTOR     = TRIGGER_PRESCALER / 72.f;
const float DETECTOR_TIME_FACTOR    = DETECTOR_PRESCALER / 72.f;

void timer1_configuration();
void timer2_configuration(uint8_t pre, uint8_t sub);
void timer3_configuration(uint8_t pre, uint8_t sub);
void timer4_configuration(uint8_t pre, uint8_t sub);

void usart2_configuration(uint8_t pre, uint8_t sub);

void usart2_printblock(uint8_t * data, uint8_t length);

extern uint8_t usart2_rx_buffer[USART2_RX_BUFFER_SIZE];
extern uint8_t usart2_tx_buffer[USART2_TX_BUFFER_SIZE];
