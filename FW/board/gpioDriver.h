#ifndef _GPIODRIVER_H
#define _GPIODRIVER_H

#include "stdint.h"
#include <stddef.h>
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "misc.h"
#include "stm32f10x_rcc.h"

typedef struct
{
    char*           pinName;
    GPIO_TypeDef*   gpiox;
    u16             pinx;
}gpioInfo_t;

void regularOutGpioInit (gpioInfo_t *pGpioInfo,uint32_t num);

#endif