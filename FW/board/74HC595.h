#ifndef _74HC595_H
#define _74HC595_H

#include "stdint.h"
#include <stddef.h>
#include "stm32f10x.h"
#include "gpioDriver.h"


void init74HC595(void);
int setBuffer74HC595(u32 idx, u8 value);
int setBit74HC595(u32 idx, u8 offset);
int clearBit74HC595(u32 idx, u8 offset);

void writeRegTo74HC595();

#endif
