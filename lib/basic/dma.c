#include "basic/dma.h"
#include "st/stm32f10x_dma.h"
#include "st/stm32f10x_rcc.h"

void dma_configuration(DMA_TypeDef * DMAx, DMA_Channel_TypeDef * channel, uint32_t direction, uint32_t peripheral, uint32_t memory, uint32_t buffersize, uint32_t it)
{
    DMA_InitTypeDef dma;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 << (((uint32_t)DMAx - DMA1_BASE) / 0x0400), ENABLE);

    DMA_DeInit(channel);

    dma.DMA_PeripheralBaseAddr = peripheral;
    dma.DMA_MemoryBaseAddr = memory;
    dma.DMA_DIR = direction;
    dma.DMA_BufferSize = buffersize;

    dma.DMA_Mode = DMA_Mode_Normal;
    dma.DMA_Priority = DMA_Priority_High;
    dma.DMA_M2M = DMA_M2M_Disable;
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma.DMA_MemoryInc = DMA_MemoryInc_Disable;
    
    DMA_Init(channel, &dma);

    DMA_ITConfig(channel, it, ENABLE);
}
