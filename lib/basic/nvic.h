#include "st/misc.h"

void nvic_set_priority_group(uint32_t PriorityGroup);

void nvic_configuration(FunctionalState Command, uint8_t Channel, uint8_t PPriority, uint8_t SPriority);
