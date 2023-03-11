#ifndef USARTDRIVER_H
#define USARTDRIVER_H

#include "stdint.h"
#include <stddef.h>
#include "stm32f10x.h"

typedef void (*pfnOnReceiveData)(u8*,u32,u8);

typedef struct
{
    u8* addr;
    u32 size;
}UsartReceiveBuffer_t;

void USART2_Init(pfnOnReceiveData pfn,UsartReceiveBuffer_t *pBuff);
int16_t USART2_Write(u8 *ch, uint16_t num);

void USART1_Init(pfnOnReceiveData pfn,UsartReceiveBuffer_t *pBuff);
s16 USART1_Write(char *ch, u16 num);
#endif