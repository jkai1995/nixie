#ifndef JDY08_H
#define JDY08_H

#include "stdint.h"
#include "usartDriver.h"

#define JDY08WRITE(data,num) USART1_Write(data,num)

void JDY08_Init();

#endif