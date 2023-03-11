#include "gpioDriver.h"

void regularOutGpioInit (gpioInfo_t *pGpioInfo,uint32_t num)
{
    /* Configure pins: Push-pull Output Mode (2 MHz) */
    GPIO_InitTypeDef GPIO_InitStructure;

    for (int i = 0; i < num; i++)
    {
        GPIO_InitStructure.GPIO_Pin = pGpioInfo[i].pinx;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_Init(pGpioInfo[i].gpiox, &GPIO_InitStructure);
    }
}

