#ifndef _74HC595_H
#define _74HC595_H

#include "stdint.h"
#include <stddef.h>
#include "stm32f10x.h"

void init74HC595(void);
int setBuffer(u32 idx, u8 value);
int set74HC595Bit(u32 idx, u8 offset);
int clear74HC595Bit(u32 idx, u8 offset);

void updateReg();

#endif
