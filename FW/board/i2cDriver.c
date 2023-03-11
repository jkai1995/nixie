#include "stm32f10x_gpio.h"
#include "misc.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_i2c.h"
#include "i2cDriver.h"

enum
{
    SCL,//PA11 连接后高电平，未连接低电平
    SDA,//PA8 连接以后按下断开连接
    PinMax,
};

static GPIO_TypeDef* m_btGpiox[PinMax] =
{
    GPIOB,
    GPIOB,
};
static u16 m_btPinx[PinMax] =
{
    GPIO_Pin_6,
    GPIO_Pin_7,
};

void I2C1_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitStruct;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin =  m_btPinx[SCL];
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD; 
    GPIO_Init(m_btGpiox[SCL], &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin =  m_btPinx[SDA];
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(m_btGpiox[SDA], &GPIO_InitStructure);


    I2C_InitStruct.I2C_Ack=I2C_Ack_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress=I2C_AcknowledgedAddress_7bit;
    I2C_InitStruct.I2C_ClockSpeed=400000;
    I2C_InitStruct.I2C_DutyCycle=I2C_DutyCycle_2;
    I2C_InitStruct.I2C_Mode=I2C_Mode_I2C;
    I2C_InitStruct.I2C_OwnAddress1=0x55;
    I2C_Init(I2C1,&I2C_InitStruct);

    I2C_Cmd(I2C1,ENABLE);
}


/*写入一个字节*/
void IIC_WriteBytes(u8 slaveAddr,u8* data,u8 num)
{
    I2C_GenerateSTART(I2C1,ENABLE);
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT))//检测    EV5事件
    {
    }

    slaveAddr = (slaveAddr << 1);
    I2C_Send7bitAddress(I2C1,slaveAddr,I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))//检测EV6事件
    {
    }

    for (int i = 0; i < num; i++)
    {
        I2C_SendData(I2C1,data[i]);//发送操作的内存地址
        while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTED))//检测EV8事件
        {
        }
    }    
    I2C_GenerateSTOP(I2C1,ENABLE);//发送结束信号
    
}
 
/*读取一个字节*/
void IIC_ReadBytes(u8 slaveAddr,u8* data,u8 num)
{
    I2C_GenerateSTART(I2C1,ENABLE);
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT))
    {
    }

    slaveAddr = (slaveAddr << 1);
    I2C_Send7bitAddress(I2C1,slaveAddr,I2C_Direction_Receiver);
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    {
    }
    for (int i = 0; i < num; i++)
    {
        if (i == (num - 1))
        {
            I2C_AcknowledgeConfig(I2C1,DISABLE);
        }
        else
        {
            I2C_AcknowledgeConfig(I2C1,ENABLE);
        }
     
        /* 检测 EV7 事件 */
        while(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_RECEIVED) != SUCCESS )
        {
        }
      
      /* 读取接收数据 */
      data[i] = I2C_ReceiveData(I2C1);
    }
  /* 停止信号 */
  I2C_GenerateSTOP(I2C1,ENABLE);
}
