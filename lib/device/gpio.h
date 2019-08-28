#pragma once
#include "st/stm32f10x_gpio.h"

void gpio_configuration(GPIO_TypeDef *GPIOx, uint16_t Pin, GPIOMode_TypeDef Mode, GPIOSpeed_TypeDef Speed);

void gpio_toggle_bit(GPIO_TypeDef *GPIOx, uint16_t Pin);

void gpio_set_bit(GPIO_TypeDef *GPIOx, uint16_t Pin);

void gpio_reset_bit(GPIO_TypeDef *GPIOx, uint16_t Pin);
