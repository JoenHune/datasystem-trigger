#include "basic/gpio.h"
#include "st/stm32f10x_rcc.h"

void gpio_configuration(GPIO_TypeDef *GPIOx, uint16_t Pin, GPIOMode_TypeDef Mode, GPIOSpeed_TypeDef Speed)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA << (((uint32_t)GPIOx - GPIOA_BASE) / 0x0400), ENABLE);

    GPIO_InitTypeDef gpio;

    gpio.GPIO_Pin   = GPIO_Pin_0 << Pin;
    gpio.GPIO_Mode  = Mode;
    gpio.GPIO_Speed = Speed;

    GPIO_Init(GPIOx, &gpio);
}

void gpio_toggle_bit(GPIO_TypeDef *GPIOx, uint16_t Pin)
{
    assert_param(IS_GPIO_ALL_PERIPH(GPIOx));
    assert_param(IS_GPIO_PIN(GPIO_Pin_0 << Pin));

    GPIOx->ODR ^= (GPIO_Pin_0 << Pin);
}

void gpio_set_bit(GPIO_TypeDef *GPIOx, uint16_t Pin)
{
    assert_param(IS_GPIO_ALL_PERIPH(GPIOx));
    assert_param(IS_GPIO_PIN(GPIO_Pin_0 << Pin));

    GPIOx->BSRR = (GPIO_Pin_0 << Pin);
}

void gpio_reset_bit(GPIO_TypeDef *GPIOx, uint16_t Pin)
{
    assert_param(IS_GPIO_ALL_PERIPH(GPIOx));
    assert_param(IS_GPIO_PIN(GPIO_Pin_0 << Pin));

    GPIOx->BRR = (GPIO_Pin_0 << Pin);
}
