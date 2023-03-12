#include "JDY08.h"
#include "usartDriver.h"
#include "shellPort.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


enum
{
    STAT,//PA11 连接后高电平，未连接低电平
//    DISC,//连接状态下，低电平进入AT指令模式
    PWRC,//PA8 连接以后按下断开连接
    PinMax,
};

const char* m_btPinName[PinMax] =
{
    "STAT",
    "PWRC"
};

static GPIO_TypeDef* m_btGpiox[PinMax] =
{
    GPIOA,
    GPIOA,
};
static u16 m_btPinx[PinMax] =
{
    GPIO_Pin_11,
    GPIO_Pin_8,
};

#define ReadSTAT() GPIO_ReadInputDataBit(m_btGpiox[STAT],m_btPinx[STAT])
#define ReadPWRC() GPIO_ReadInputDataBit(m_btGpiox[PWRC],m_btPinx[PWRC])
#define WriteWRC(x) GPIO_WriteBit(m_btGpiox[PWRC],m_btPinx[PWRC],x)


#define RECEIVE_BUFFER_SIZE 64
//usart 预留buffer，用于DMA接收数据
static u8    m_receiveBuffer[RECEIVE_BUFFER_SIZE];

static QueueHandle_t xQueueReceiveData = NULL;

static u8    m_isATCmdWaiting;

void OnUSART1ReceiveData(u8* data,u32 num,u8 isFromISR)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    UsartReceiveBuffer_t buffer;
    buffer.addr = data;
    buffer.size = num;

    if (isFromISR)
    {
        if (m_isATCmdWaiting)
        {
            xQueueSendFromISR(xQueueReceiveData,
                              &buffer,
                              &xHigherPriorityTaskWoken);

            portYIELD_FROM_ISR (xHigherPriorityTaskWoken);
        }
        else
        {
            g_shell.write = USART1_Write;
            OnShellReceiveData(data,num,isFromISR);
        }

    }
}

void GPIOInitBlueTooth(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);


    GPIO_InitStructure.GPIO_Pin = m_btPinx[STAT];
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(m_btGpiox[STAT], &GPIO_InitStructure);


    GPIO_InitStructure.GPIO_Pin = m_btPinx[PWRC];
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_SetBits(m_btGpiox[PWRC],m_btPinx[PWRC]); 
    GPIO_Init(m_btGpiox[PWRC], &GPIO_InitStructure);
}

void JDY08_Init()
{
    xQueueReceiveData = xQueueCreate(1, sizeof(UsartReceiveBuffer_t));

    UsartReceiveBuffer_t buffer;
    buffer.addr = m_receiveBuffer;
    buffer.size = RECEIVE_BUFFER_SIZE;
    USART1_Init(OnUSART1ReceiveData,&buffer);

    GPIOInitBlueTooth();
}

int ATCommand(int argc, char *agrv[])
{
    char * command = NULL;
    u32    length = 0;
    if (argc == 2)
    {
        BaseType_t result = pdFAIL;
        UsartReceiveBuffer_t buffer;
        buffer.addr = NULL;
        buffer.size = 0;
        command = (char*)agrv[1];
        length = strlen(command);
        m_isATCmdWaiting = 1;
        JDY08WRITE(command,length);

        result = xQueueReceive(xQueueReceiveData,
                    &buffer,
                    pdMS_TO_TICKS(600));
        m_isATCmdWaiting = 0;
        if (result == pdPASS && buffer.addr != NULL && buffer.size > 0)
        {
            g_shell.write((char *)buffer.addr,buffer.size);
        }
        else
        {
            shellWriteString(&g_shell,shellTextCustom[SHELL_TEXT_WAIT_FOR_RESULT_TIEMOUT]);
        }

    }
    else
    {
        shellWriteString(&g_shell,shellTextCustom[SHELL_TEXT_WRONG_NUMBER_OF_PARAMETERS]);
    }

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN, AT, ATCommand, BlueTooth AT Command : AT AT+BOUD);

int BTCommand(int argc, char *agrv[])
{
    char * command = NULL;
    u32    length = 0;
    u32    error = 1;

    if (argc == 2 || argc == 3)
    {
        u32 pinIdx = PinMax;
        u32 value = 0;
        char buffer[6];

        for (u32 i = 0; i < PinMax; i++)
        {
            if (0 == strcmp(m_btPinName[i],agrv[1]))
            {
                pinIdx = i;
            }
        }

        switch (pinIdx)
        {
            case STAT:
                if (argc == 2)
                {
                    value = ReadSTAT();
                    sprintf(buffer,"%d",(value?1:0));
                    shellWriteString(&g_shell,buffer);
                    error = 0;
                }
                break;
            case PWRC:
                if (argc == 2)
                {
                    value = ReadPWRC();
                    sprintf(buffer,"%d",(value?1:0));
                    shellWriteString(&g_shell,buffer);
                    error = 0;
                }
                else if (argc == 3)
                {
                    value = atoi(agrv[2]);
                    value = value?1:0;
                    WriteWRC(value);
                    error = 0;
                }
                break;
            default:
                break;
        }
        if (error)
        {
            shellWriteString(&g_shell,shellTextCustom[SHELL_TEXT_PARAMETERS_FORMAT_ERROR]);
        }

    }
    else
    {
        shellWriteString(&g_shell,shellTextCustom[SHELL_TEXT_WRONG_NUMBER_OF_PARAMETERS]);
    }

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN, BT, BTCommand,BT <STAT|PWRC> [0|1]);



