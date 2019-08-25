#pragma once

#include "st/stm32f10x_dma.h"

void dma_configuration(DMA_TypeDef * DMAx, DMA_Channel_TypeDef * channel, uint32_t direction, uint32_t peripheral, uint32_t memory, uint32_t buffersize, uint32_t it);
