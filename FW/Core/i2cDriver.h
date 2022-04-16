#ifndef I2CDRIVER_H
#define I2CDRIVER_H

#include "stdint.h"
#include <stddef.h>
#include "stm32f10x.h"

void I2C1_Init(void);
void IIC_WriteBytes(u8 slaveAddr,u8* data,u8 num);
void IIC_ReadBytes(u8 slaveAddr,u8* data,u8 num);
#endif