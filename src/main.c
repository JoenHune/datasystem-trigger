#include "main.h"

int main()
{
    SystemInit();

    nvic_set_priority_group(NVIC_PriorityGroup_1);

    gpio_configuration(GPIOB, 12, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);

    gpio_set_bit(GPIOB, 12);

    usart2_configuration();

    timer1_configuration();
    timer2_configuration(1, 2);
    timer3_configuration(1, 1);
    timer4_configuration(1, 2);

    while (1)
    {
        /* empty loop */
    }
}
