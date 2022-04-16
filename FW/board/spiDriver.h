#ifndef SPIDRIVER_H
#define SPIDRIVER_H

#include "stdint.h"
#include <stddef.h>
#include "stm32f10x.h"

void spiInit(void);
void SPI_SendByte(u8 data);
void SPI_WaitForSendComplete();
#endif