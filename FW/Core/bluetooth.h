#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "stdint.h"

#define USART_BLUETOOTH(ch,num) USART1_Write(ch,num)

void JDY08_Init();

#endif