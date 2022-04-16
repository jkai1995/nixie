#include "ds3231.h"

#define 	DS3231_ARRD			0x68
#define REG_NUM 0x13

#define		REG_SEC				0x00
#define		REG_MIN				0x01
#define		REG_HOUR			0x02
#define		REG_DAY				0x03
#define		REG_DATA			0x04
#define		REG_MON				0x05
#define		REG_YEAR			0x06
#define 	REG_ALM1_SEC  		0x07
#define 	REG_ALM1_MIN 	  	0x08
#define 	REG_ALM1_HOUR     	0x09
#define 	REG_ALM1_DAY_DATE 	0x0A
#define 	REG_ALM2_MIN  		0x0B
#define 	REG_ALM2_HOUR     	0x0C
#define 	REG_ALM2_DAY_DATE 	0x0D
#define 	REG_CONTROL         0x0E
#define 	REG_STATUS          0x0F
#define 	REG_AGING_OFFSET    0x10
#define 	REG_TEMP_MSB 		0x11
#define 	REG_TEMP_LSB 		0x12

static RtcTime_t m_rtctime;
pfnWiteReadRegs pWriteRegs = NULL;
pfnWiteReadRegs pReadRegs = NULL;

__attribute__( ( always_inline ) ) static inline void DS3231WriteRegs(uint8_t* data,uint8_t num)
{
	pWriteRegs(DS3231_ARRD,data,num);
}
__attribute__( ( always_inline ) ) static inline void DS3231ReadRegs(uint8_t* data,uint8_t num)
{
	pReadRegs(DS3231_ARRD,data,num);
}

typedef struct
{
	uint8_t regAddr;
	uint8_t regData[REG_NUM];
}st_regInfo;

typedef union
{
	uint8_t		regBuf[REG_NUM+1];
	st_regInfo	regInfo;
}un_ds3231Reg;

un_ds3231Reg m_ds3231Reg;


static unsigned char bcd_to_hex(unsigned char data)
{
    unsigned char temp;

    temp = ((data>>4)*10 + (data&0x0f));
    return temp;
}

static unsigned char hex_to_bcd(unsigned char data)
{
    unsigned char temp;

    temp = (((data/10)<<4) + (data%10));
    return temp;
}



void ds3231Init(pfnWiteReadRegs write,pfnWiteReadRegs read)
{
	pWriteRegs = write;
	pReadRegs = read;
}

#define FIRST_YEAR 2021

void getRtcTime(void)
{
	m_ds3231Reg.regInfo.regAddr = REG_SEC;
	DS3231WriteRegs(&m_ds3231Reg.regInfo.regAddr,sizeof(m_ds3231Reg.regInfo.regAddr));

	uint8_t *pRegData = m_ds3231Reg.regInfo.regData;
	DS3231ReadRegs(pRegData,REG_NUM);

	m_rtctime.second = bcd_to_hex(pRegData[REG_SEC]);
	m_rtctime.minute = bcd_to_hex(pRegData[REG_MIN]);
	m_rtctime.hour = bcd_to_hex(pRegData[REG_HOUR]);
	m_rtctime.week = bcd_to_hex(pRegData[REG_DAY]);
	m_rtctime.year = bcd_to_hex(pRegData[REG_YEAR]) + FIRST_YEAR;
	m_rtctime.month = bcd_to_hex(pRegData[REG_MON]);
	m_rtctime.date = bcd_to_hex(pRegData[REG_DATA]);
}

void setRtcTime(RtcTime_t* rtc)
{
	uint8_t *pRegData = m_ds3231Reg.regInfo.regData;

	memcpy(&m_rtctime,rtc,sizeof(RtcTime_t));

	if (m_rtctime.year < FIRST_YEAR)
	{
		m_rtctime.year = FIRST_YEAR;
	}
	pRegData[REG_YEAR]	= hex_to_bcd(m_rtctime.year - FIRST_YEAR);
	pRegData[REG_MON]	= hex_to_bcd(m_rtctime.month);
	pRegData[REG_DATA]	= hex_to_bcd(m_rtctime.date);
	pRegData[REG_DAY]	= hex_to_bcd(m_rtctime.week);
	pRegData[REG_HOUR]	= hex_to_bcd(m_rtctime.hour);
	pRegData[REG_MIN]	= hex_to_bcd(m_rtctime.minute);
	pRegData[REG_SEC]	= hex_to_bcd(m_rtctime.second);

	m_ds3231Reg.regInfo.regAddr = REG_SEC;
	DS3231WriteRegs(m_ds3231Reg.regBuf,sizeof(m_ds3231Reg.regBuf));
}

RtcTime_t* returnRtcTime()
{
	return &m_rtctime;
}

