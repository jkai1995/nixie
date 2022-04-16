#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "misc.h"
#include "stm32f10x_rcc.h"
#include "usartDriver.h"
#include "shellPort.h"
#include "i2cDriver.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "ds3231Port.h"

void RtcTimeInit(void)
{
	I2C1_Init();
	ds3231Init(IIC_WriteBytes,IIC_ReadBytes);

	getRtcTime();
}
