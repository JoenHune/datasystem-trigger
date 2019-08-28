#pragma once 
#include "st/stm32f10x.h"

template <class T>
void limit(T &origin, const T &min, const T &max)
{
    if (min <= max && origin < min)
        origin = min;

    else if (min <= max && origin > max)
        origin = max;
    
    else
        origin = origin;
};
