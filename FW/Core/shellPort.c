#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "misc.h"
#include "stm32f10x_rcc.h"
#include "usartDriver.h"
#include "shellPort.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


/* 1. 创建shell对象，开辟shell缓冲区 */
#define SHELLBUFFERSIZE 512
Shell g_shell;
//letter-shell预留buffer用于存储输入的命令和命令记录
char shell_buffer[SHELLBUFFERSIZE];

#define RECEIVE_BUFFER_SIZE 64
//usart 预留buffer，用于DMA接收数据
static u8	m_receiveBuffer[RECEIVE_BUFFER_SIZE];
//usart将接收的数据存储在m_receiveBuffer[]中的某一段
//m_receivedData表明起始地址和大小
static UsartReceiveBuffer_t m_receivedData;

const char *shellTextCustom[] =
{
    [SHELL_TEXT_WRONG_NUMBER_OF_PARAMETERS] = 
        "\r\nERROR: Wrong number of parameters!\r\n",
    [SHELL_TEXT_PARAMETERS_FORMAT_ERROR] = 
        "\r\nERROR: parameters format error!\r\n",
    [SHELL_TEXT_WAIT_FOR_RESULT_TIEMOUT] = 
        "\r\nERROR: Wait for result Timeout\r\n",

};



TaskHandle_t m_shellHandle;
#define ShellTaskPriority 4

static QueueHandle_t xQueueReceiveData = NULL;

u32 *pSizeofQueue = NULL;

s16 shellWrite(char* data,u16 num)
{
	return USART2_Write((u8*)data,num);
}

u32 height = 0;
s16 shellRead(char *data, u16 num)
{
	if (m_receivedData.size == 0 || m_receivedData.addr == NULL)
	{
		height = uxTaskGetStackHighWaterMark(NULL);
		xQueueReceive(xQueueReceiveData,
				&m_receivedData,
				portMAX_DELAY);
	}

	if (data != NULL && m_receivedData.addr != NULL && m_receivedData.size > 0)
	{
		*data = *(m_receivedData.addr);
		m_receivedData.addr++;
		m_receivedData.size--;
	}

	return 1;
}

void On_USART2ReceiveData(u8*data,u32 num,u8 isFromISR)
{
	g_shell.write = shellWrite;
	OnShellReceiveData(data,num,isFromISR);
}


void OnShellReceiveData(u8*data,u32 num,u8 isFromISR)
{
	if (isFromISR)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		UsartReceiveBuffer_t buffer;
		buffer.addr = data;
		buffer.size = num;

		xQueueSendFromISR(xQueueReceiveData,
						  &buffer,
						  &xHigherPriorityTaskWoken);

		portYIELD_FROM_ISR (xHigherPriorityTaskWoken);
	}
}


void UserShellInit(void)
{
	m_receivedData.addr = NULL;
	m_receivedData.size = 0;

	UsartReceiveBuffer_t buffer;
	buffer.addr = m_receiveBuffer;
	buffer.size = RECEIVE_BUFFER_SIZE;
	USART2_Init(On_USART2ReceiveData,&buffer);


	g_shell.write = shellWrite;
	g_shell.read = shellRead;

	//调用shell初始化函数
	shellInit(&g_shell, shell_buffer, SHELLBUFFERSIZE);
	
	
}



void createShellTask (void)
{

	xQueueReceiveData = xQueueCreate(1, sizeof(UsartReceiveBuffer_t));

	UserShellInit();

	xTaskCreate(shellTask,
				"shell task",
				128,
				&g_shell,
				ShellTaskPriority,
				&m_shellHandle);

}


