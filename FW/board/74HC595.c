#include "stm32f10x_spi.h"
#include "spiDriver.h"
#include "74HC595.h"


#define ICNUMBER 6 //6个芯片串联

u8 m_buffer[ICNUMBER];

enum
{
	OE,//PA6 output enable,低电平有效
	RCLK,//PB0 输出锁存器更新 上升沿执行
	SRCLR,//PA4  清空移位寄存器 低电平生效
	PinMax,
};

gpioInfo_t m_gpio74HC244[PinMax] = 
{
	{"OE",		GPIOA,GPIO_Pin_6},
	{"RCLK",	GPIOB,GPIO_Pin_0},
	{"SRCLR",	GPIOA,GPIO_Pin_4},
};

#define WriteGpio(p,x) GPIO_WriteBit(m_gpio74HC244[p].gpiox,m_gpio74HC244[p].pinx,x)

#define outputEnable()		WriteGpio(OE,0)
#define outputDisable()		WriteGpio(OE,1)

#define shiftRegClear()		WriteGpio(SRCLR,0)
#define shiftRegKeep()		WriteGpio(SRCLR,1)

#define outputRegCLKIdle()	WriteGpio(RCLK,0)
#define outputRegUpdate()	WriteGpio(RCLK,1)

static const u8  fac_us=72;
void delay_us(u32 nus)
{
	u32 ticks;
	u32 told,tnow,tcnt=0;


	u32 reload=SysTick->LOAD;
	ticks=nus*fac_us;

	told=SysTick->VAL;

	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;
			else tcnt+=reload-tnow+told;
			told=tnow;
			if(tcnt>=ticks)break;
		}  
	};

}

#define DELAY() delay_us(1)

void gpio74HC595Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	regularOutGpioInit(m_gpio74HC244,PinMax);

	outputDisable();
	shiftRegClear();
	outputRegCLKIdle();

	for (int i = 0;i < ICNUMBER; i++)
	{
		m_buffer[i] = 0;
	}
	DELAY();
}

//输入380ma  输出2.28ma

void init74HC595(void)
{
	gpio74HC595Init();
	spiInit();
}

int setBuffer74HC595(u32 idx, u8 value)
{
	if (idx >= 0 && idx < ICNUMBER)
	{
		m_buffer[idx] = value;
	}
	else
	{
		return -1;
	}
	return 0;
}

int setBit74HC595(u32 idx, u8 offset)
{
	if (idx >= 0 && idx < ICNUMBER)
	{
		m_buffer[idx] |= (1 << offset);
	}
	else
	{
		return -1;
	}
	return 0;
}

int clearBit74HC595(u32 idx, u8 offset)
{
	if (idx >= 0 && idx < ICNUMBER)
	{
		m_buffer[idx] &= ~(1 << offset);
	}
	else
	{
		return -1;
	}
	return 0;
}


void writeRegTo74HC595()
{
	shiftRegKeep();
	DELAY();
	outputRegCLKIdle();
	DELAY();

	for (int i = 0; i < ICNUMBER; i++)
	{
		SPI_SendByte(m_buffer[i]);
		DELAY();
	}
	SPI_WaitForSendComplete();

	DELAY();
	outputRegUpdate();
	DELAY();
	outputEnable();
	DELAY();
}
