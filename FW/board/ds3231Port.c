#include "i2cDriver.h"

#include "ds3231Port.h"

void RtcTimeInit(void)
{
	I2C1_Init();
	ds3231Init(IIC_WriteBytes,IIC_ReadBytes);

	getRtcTime();
}
