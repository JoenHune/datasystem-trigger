#include "device/nvic.h"

void nvic_set_priority_group(uint32_t PriorityGroup)
{
    NVIC_SetPriorityGrouping(PriorityGroup);
}

void nvic_configuration(FunctionalState Command, uint8_t Channel, uint8_t PPriority, uint8_t SPriority)
{
    NVIC_InitTypeDef nvic;

    nvic.NVIC_IRQChannel    = Channel;
    nvic.NVIC_IRQChannelCmd = Command;
    nvic.NVIC_IRQChannelPreemptionPriority  = PPriority;
    nvic.NVIC_IRQChannelSubPriority         = SPriority;

    NVIC_Init(&nvic);
}
