#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "misc.h"
#include "stm32f10x_rcc.h"
#include "generalUsartDriver.h"
#include "shellPort.h"

extern Shell g_shell;

#define RECEIVE_BUFFER_SIZE 64
//usart 预留buffer，用于DMA接收数据
static u8    m_receiveBuffer[RECEIVE_BUFFER_SIZE];

void On_USART2ReceiveData(u8*data,u32 num,u8 isFromISR)
{
    g_shell.write = USART2_Write;
    OnShellReceiveData(data,num,isFromISR);
}

void generalUSARTInit(void)
{
    UsartReceiveBuffer_t buffer;
    buffer.addr = m_receiveBuffer;
    buffer.size = RECEIVE_BUFFER_SIZE;
    USART2_Init(On_USART2ReceiveData,&buffer);

}

