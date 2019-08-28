#include "configuration.h"

extern "C"
{
#include "device/gpio.h"
#include "device/nvic.h"

#include "st/stm32f10x_gpio.h"
#include "st/misc.h"
}

int main()
{
    SystemInit();

    nvic_set_priority_group(NVIC_PriorityGroup_1);

    gpio_configuration(GPIOB, 12, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);

    gpio_set_bit(GPIOB, 12);

    timer1_configuration();
    timer2_configuration(0, 2);
    timer3_configuration(0, 1);
    timer4_configuration(0, 2);

    usart2_configuration(1, 1);

    while (1)
    {
    }
}
