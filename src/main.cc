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

    // 模拟PPS信号发生器
    timer1_configuration();
    // CAM1曝光时间测算的定时器
    timer2_configuration(0, 2);
    // CAM2曝光时间测算的定时器
    timer4_configuration(0, 2);
    // 产生相机曝光触发信号的定时器
    timer3_configuration(1, 1);

    usart2_configuration(1, 1);

    while (1)
    {
    }
}
