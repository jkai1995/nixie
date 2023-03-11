#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "misc.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_dma.h"

#include "usartDriver.h"

#define FALSE                                  ( 0 )
#define TRUE                                   ( 1 )


#define DMABufferNum 2
typedef struct
{
    u8* buffer[DMABufferNum];
    u32 size;
    u32 currIdx;
}DMABuffer_t;


#define USART1BaudRate 115200
#define USART2BaudRate 9600

static DMABuffer_t        m_USART1DMABuffer;
static DMABuffer_t        m_USART2DMABuffer;
static pfnOnReceiveData OnUSART2ReceiveData = NULL;
static pfnOnReceiveData OnUSART1ReceiveData = NULL;

void DMA_SetMemoryBuffer(DMA_Channel_TypeDef* DMAy_Channelx,DMABuffer_t *pBuff)
{
    DMAy_Channelx->CMAR = (u32)pBuff->buffer[pBuff->currIdx];
    DMAy_Channelx->CNDTR = pBuff->size;
}

void USART2_Init(pfnOnReceiveData pfn,UsartReceiveBuffer_t *pBuff)
{
    GPIO_InitTypeDef GPIO_InitStrue;
    USART_InitTypeDef USART_InitStrue;
    NVIC_InitTypeDef NVIC_InitStrue;
    DMA_InitTypeDef DMA_InitStructure;

    m_USART2DMABuffer.size = pBuff->size / DMABufferNum;
    m_USART2DMABuffer.currIdx = 0;
    for (u32 i = 0; i < DMABufferNum; i++)
    {
        m_USART2DMABuffer.buffer[i] = &(pBuff->addr[m_USART2DMABuffer.size * i]);
    }
    // 外设使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    USART_DeInit(USART2);  //复位串口2 -> 可以没有

    // 初始化 串口对应IO口    TX-PA2    RX-PA3
    GPIO_InitStrue.GPIO_Mode=GPIO_Mode_AF_PP;
    GPIO_InitStrue.GPIO_Pin=GPIO_Pin_2;
    GPIO_InitStrue.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&GPIO_InitStrue);
    
    GPIO_InitStrue.GPIO_Mode=GPIO_Mode_IN_FLOATING;
    GPIO_InitStrue.GPIO_Pin=GPIO_Pin_3;
    GPIO_Init(GPIOA,&GPIO_InitStrue);

    // 初始化 串口模式状态
    USART_InitStrue.USART_BaudRate=USART2BaudRate; // 波特率
    USART_InitStrue.USART_HardwareFlowControl=USART_HardwareFlowControl_None; // 硬件流控制
    USART_InitStrue.USART_Mode=USART_Mode_Tx|USART_Mode_Rx; // 发送 接收 模式都使用
    USART_InitStrue.USART_Parity=USART_Parity_No; // 没有奇偶校验
    USART_InitStrue.USART_StopBits=USART_StopBits_1; // 一位停止位
    USART_InitStrue.USART_WordLength=USART_WordLength_8b; // 每次发送数据宽度为8位
    USART_Init(USART2,&USART_InitStrue);
    
    
    // 初始化 中断优先级
    NVIC_InitStrue.NVIC_IRQChannel=USART2_IRQn;
    NVIC_InitStrue.NVIC_IRQChannelCmd=ENABLE;
    NVIC_InitStrue.NVIC_IRQChannelPreemptionPriority=1;
    NVIC_InitStrue.NVIC_IRQChannelSubPriority=1;
    NVIC_Init(&NVIC_InitStrue);

    DMA_Cmd(DMA1_Channel6,DISABLE);
    DMA_DeInit(DMA1_Channel6);

        //外设地址
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART2->DR);
    //内存地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)m_USART2DMABuffer.buffer[m_USART2DMABuffer.currIdx]; 
    //dma传输方向单向
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; 
    //设置DMA在传输时缓冲区的长度
    DMA_InitStructure.DMA_BufferSize = m_USART2DMABuffer.size;
    //设置DMA的外设递增模式，一个外设
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    //设置DMA的内存递增模式
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    //外设数据字长
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    //内存数据字长
    DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;
    //设置DMA的传输模式
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    //设置DMA的优先级别
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    //设置DMA的2个memory中的变量互相访问
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel6,&DMA_InitStructure);


    USART_ITConfig(USART2,USART_IT_IDLE, ENABLE);
    USART_ClearFlag(USART1, USART_FLAG_IDLE);
    USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE); //DMA接收
    USART_DMACmd(USART2,USART_DMAReq_Tx,ENABLE);//DMA发送

    DMA_ClearFlag(DMA1_FLAG_TC6);//清除DMA所以标志
    DMA_ITConfig(DMA1_Channel6,DMA_IT_TE,ENABLE);


    USART_Cmd(USART2,ENABLE);//使能串口
    DMA_Cmd(DMA1_Channel6,ENABLE);


    OnUSART2ReceiveData = pfn;
}



void USART2_IRQHandler(void) // 串口2中断服务函数
{
    u8 res;
    if(USART_GetITStatus(USART2,USART_IT_RXNE)) // 中断标志
    {
        //while (USART_GetFlagStatus(USART2,USART_FLAG_RXNE))
        {
            res= USART_ReceiveData(USART2);  // 串口2 接收
            if (OnUSART2ReceiveData != NULL)
            {
                OnUSART2ReceiveData(&res,1,TRUE);
            }
        }
    }
    else if (USART_GetITStatus(USART2,USART_IT_IDLE))
    {
        DMA_Cmd(DMA1_Channel6,DISABLE);
        DMA_ClearFlag(DMA1_FLAG_TC6);
        USART2->SR;
        USART2->DR;//IDLE中断需要先读SR再读DR才能清掉
        //总计数减去未传输的数据个数，得到已经接收的数据个数
        u32 size = m_USART2DMABuffer.size - DMA_GetCurrDataCounter(DMA1_Channel6);
        u8* data = m_USART2DMABuffer.buffer[m_USART2DMABuffer.currIdx];

        if (OnUSART2ReceiveData != NULL)
        {
            OnUSART2ReceiveData(data,size,TRUE);
        }

        m_USART2DMABuffer.currIdx = (m_USART2DMABuffer.currIdx + 1) % DMABufferNum;
        DMA_SetMemoryBuffer(DMA1_Channel6,&m_USART2DMABuffer);
        DMA_Cmd(DMA1_Channel6,ENABLE);
        //USART_ClearITPendingBit(USART2, USART_IT_IDLE);
    }
}


void USART2_WriteDMA(uint8_t *Data, uint32_t len)
{
    DMA_InitTypeDef DMA_InitStruct;
    DMA_Cmd(DMA1_Channel7,DISABLE);
    DMA_InitStruct.DMA_PeripheralBaseAddr = (u32)(&USART2->DR);
    DMA_InitStruct.DMA_MemoryBaseAddr = (u32)Data;
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStruct.DMA_BufferSize = len;
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStruct.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel7,&DMA_InitStruct);
    DMA_ClearFlag(DMA1_FLAG_TC7);//传输完成标志位
    DMA_Cmd(DMA1_Channel7,ENABLE);
    while(!DMA_GetFlagStatus(DMA1_FLAG_TC7))
    {}
    DMA_ClearFlag(DMA1_FLAG_TC7);

}

s16 USART2_Write(u8 *ch, u16 num)
{

    for (int i = 0; i < num; i++)
    {
        while (USART_GetFlagStatus(USART2,USART_FLAG_TXE) == RESET);
        USART_SendData(USART2,ch[i]);
    }
    return 0;

    //USART2_WriteDMA(ch,num);
    //return 0;
}


void USART1_Init(pfnOnReceiveData pfn,UsartReceiveBuffer_t *pBuff)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;

    m_USART1DMABuffer.size = pBuff->size / DMABufferNum;
    m_USART1DMABuffer.currIdx = 0;
    for (u32 i = 0; i < DMABufferNum; i++)
    {
        m_USART1DMABuffer.buffer[i] = &(pBuff->addr[m_USART1DMABuffer.size * i]);
    }


    /*使能端口时钟*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); 

    /*USART1-TX输出脚配置*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /*USART1-RX接收脚配置*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /*串口参数配置*/
    USART_InitStructure.USART_BaudRate = USART1BaudRate;//设置波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//一帧中传输或者接收的数据位数（8位/9位）
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//定义发送的停止位个数
    USART_InitStructure.USART_Parity = USART_Parity_No ;//设置为不需要校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//不使用流控制模式
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//发送且接收使能
    USART_Init(USART1, &USART_InitStructure);

    /*使能接收中断*/
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
    USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE); //DMA接收
    /*USART1终端优先级的配置*/

    NVIC_InitTypeDef NVIC_InitStructure; 

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//指定IRQ通道     
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;//指定先占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//从优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //定义的IRQ是被使能还是失能
    NVIC_Init(&NVIC_InitStructure);

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);                  // 开启DMA时钟

    DMA_Cmd(DMA1_Channel5,DISABLE);
    DMA_DeInit(DMA1_Channel5);

    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART1->DR);      // 设置DMA源地址：串口数据寄存器地址*/
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)m_USART1DMABuffer.buffer[m_USART1DMABuffer.currIdx]; 
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                  // 方向：外设到内存
    DMA_InitStructure.DMA_BufferSize = m_USART1DMABuffer.size;          // 传输大小    
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;    // 外设地址不增
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;             // 内存地址自增   
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 外设数据单位    
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;     // 内存数据单位 
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                       // DMA一次模式    
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;               // 优先级：中 
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                        // 禁止内存到内存的传输
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);                    // 配置DMA通道DMA1_Channel5


    /*配置好参数之后，使能USART1外设*/
    USART_Cmd(USART1, ENABLE); 
    DMA_Cmd (DMA1_Channel5,ENABLE);                                // 使能DMA


    OnUSART1ReceiveData = pfn;
}


void USART1_IRQHandler(void) // 串口2中断服务函数
{
    u8 res;
    if(USART_GetITStatus(USART1,USART_IT_RXNE)) // 中断标志
    {
        //while (USART_GetFlagStatus(USART1,USART_FLAG_RXNE))
        {
            res= USART_ReceiveData(USART1);  // 串口1 接收
            if (OnUSART1ReceiveData != NULL)
            {
                OnUSART1ReceiveData(&res,1,TRUE);
            }
        }
    }
    else if (USART_GetITStatus(USART1,USART_IT_IDLE))
    {
        DMA_Cmd(DMA1_Channel5,DISABLE);
        DMA_ClearFlag(DMA1_FLAG_TC6);
        USART1->SR;
        USART1->DR;//IDLE中断需要先读SR再都DR才能清掉
        //总计数减去未传输的数据个数，得到已经接收的数据个数
        u32 size = m_USART1DMABuffer.size - DMA_GetCurrDataCounter(DMA1_Channel5);
        u8* data = m_USART1DMABuffer.buffer[m_USART1DMABuffer.currIdx];

        if (OnUSART1ReceiveData != NULL)
        {
            OnUSART1ReceiveData(data,size,TRUE);
        }

        m_USART1DMABuffer.currIdx = (m_USART1DMABuffer.currIdx + 1) % DMABufferNum;
        DMA_SetMemoryBuffer(DMA1_Channel5,&m_USART1DMABuffer);
        DMA_Cmd(DMA1_Channel5,ENABLE);
    }

}

s16 USART1_Write(char *ch, u16 num)
{
    for (int i = 0; i < num; i++)
    {
        while (USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
        USART_SendData(USART1,ch[i]);
    }
    return 0;
}

