#ifndef GENERALUSARTDRIVER_H
#define GENERALUSARTDRIVER_H

#include "stdint.h"
#include "usartDriver.h"

#define GENERALUSART(data,num) USART2_Write(data,num)

void generalUSARTInit(void);
#endif