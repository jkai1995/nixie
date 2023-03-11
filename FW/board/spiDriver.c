#include "stm32f10x_gpio.h"
#include "misc.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"
#include "spiDriver.h"

void spiInit(void)          //SPI1配置函数
{
  SPI_InitTypeDef  SPI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  /* 使能 GPIOA 时钟 */  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
  /*SPI1外设时钟开 */  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
 /*引脚配置*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出，复用模式的输入输出由程序决定。
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  /* SPI1 工作方式配置 */
  SPI_Cmd(SPI1, DISABLE);   //配置前先关闭SPI
  SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;//transmit only
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;   //idle状态低电平
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; //时钟相位，数据在第1个跳边沿被采集
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;     //CS引脚为软模式，即通过程序控制片选脚。 
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;  //CRC多项式不设置，默认。
  SPI_Init(SPI1, &SPI_InitStructure);

  SPI_Cmd(SPI1, ENABLE);   /* 使能 SPI1  */
}

void SPI_SendByte(u8 data)
{
    while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI1, data);
}

void SPI_WaitForSendComplete()
{
    while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_BSY) == SET);
}


