#ifndef _DS3231_H_
#define _DS3231_H_

#include "stdint.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


typedef struct
{
    uint16_t year;//年
    uint8_t month;//月
    uint8_t week;//星期
    uint8_t date;//日
    uint8_t hour;//小时
    uint8_t minute;//分钟
    uint8_t second;//秒钟
}RtcTime_t;


typedef void (*pfnWiteReadRegs)(uint8_t slaveAddr,uint8_t* data,uint8_t num);

void ds3231Init(pfnWiteReadRegs write,pfnWiteReadRegs read);

void getRtcTime(void);
void setRtcTime(RtcTime_t* rtc);

RtcTime_t* returnRtcTime();


#endif
