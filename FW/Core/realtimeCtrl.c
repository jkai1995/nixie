#include "realtimeCtrl.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "shellPort.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "ds3231Port.h"
#include "74HC595.h"

enum
{
	SETBUFFER = 0,
	SETTUBE,
	SETDOT,
	SETDATE,
	SETTIME,
	RTCmdMax,
};

const char* m_RTCmdName[RTCmdMax] =
{
	"SETBUFFER",
	"SETTUBE",
	"SETDOT",
	"SETDATE",
	"SETTIME",
};

// num    1    2    4    8    16    32    64    128
//buf[0]  7    6    5    4     3    2      1     0      四
//buf[1]                                   9     8
//buf[1]  5    4    3    2     1    0                   三
//buf[2]                       9    8      7     6
//buf[2]  3    2    1    0                              二
//buf[3]            9    8     7    6      5     4
//buf[3]  1    0                                        一
//buf[4]  9    8    7    6     5    4      3     2

#define NUM_OF_TUBE 4  //we have 4 nixie tube
#define NUM_OF_NUMBER 10 //we have 10 numbers(0-9) to show per tube 
#define NUM_OF_DOT 2

typedef struct
{
	u8 idx;
	u8 offset;
}st_BufferOf74HC595;

st_BufferOf74HC595 m_tubeBuffer[NUM_OF_TUBE][NUM_OF_NUMBER] = 
{
//tube 0
//     0     1     2     3     4     5     6     7     8     9
	{{3,1},{3,0},{4,7},{4,6},{4,5},{4,4},{4,3},{4,2},{4,1},{4,0}},
//tube 1
//	   0	 1	   2	 3	   4	 5	   6	 7	   8	 9
	{{2,3},{2,2},{2,1},{2,0},{3,7},{3,6},{3,5},{3,4},{3,3},{3,2}},
//tube 2
//	   0	 1	   2	 3	   4	 5	   6	 7	   8	 9
	{{1,5},{1,4},{1,3},{1,2},{1,1},{1,0},{2,7},{2,6},{2,5},{2,4}},
//tube 3
//	   0	 1	   2	 3	   4	 5	   6	 7	   8	 9
	{{0,7},{0,6},{0,5},{0,4},{0,3},{0,2},{0,1},{0,0},{1,7},{1,6}}
};

st_BufferOf74HC595 m_dotBuffer[NUM_OF_DOT] =
{
	{5,6},{5,7}
};

int clearTube(u32 idx)
{
	for (int i = 0; i < NUM_OF_NUMBER; i++)
	{
		clear74HC595Bit(m_tubeBuffer[idx][i].idx,m_tubeBuffer[idx][i].offset);
	}
	return 0;
}

int setTube(u32 idx, s8 value)
{
	if (idx >= 0 && idx < NUM_OF_TUBE)
	{
		if (value >= 0 && value < NUM_OF_NUMBER)
		{
			clearTube(idx);
			return set74HC595Bit(m_tubeBuffer[idx][value].idx,m_tubeBuffer[idx][value].offset);
		}
		else if (value == -1)
		{
			return clearTube(idx);
		}
	}
	return -1;
}

int setDote()
{
	for (int i = 0; i < NUM_OF_DOT; i++)
	{
		set74HC595Bit(m_dotBuffer[i].idx,m_dotBuffer[i].offset);
	}
	return 0;
}

int clearDote()
{
	for (int i = 0; i < NUM_OF_DOT; i++)
	{
		clear74HC595Bit(m_dotBuffer[i].idx,m_dotBuffer[i].offset);
	}
	return 0;
}

RtcTime_t* m_pRtcTime;


void RealTimeTask(void *param)
{
	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(500);
	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();//获取当前tick

	int isDotOn = 0; 
	char buffer[10];

	u8 tube[NUM_OF_TUBE];

	while(1)
	{
		isDotOn?setDote():clearDote();

		if (isDotOn)
		{
			getRtcTime();
			tube[0] = m_pRtcTime->hour / 10;
			tube[1] = m_pRtcTime->hour % 10;
			tube[2] = m_pRtcTime->minute / 10;
			tube[3] = m_pRtcTime->minute % 10;
			for (int i = 0; i < NUM_OF_TUBE; i++)
			{
				setTube(i,tube[i]);
			}

			sprintf(buffer,"On : %d\r\n",m_pRtcTime->second);
			//shellWriteString(&g_shell,buffer);
		}
		else
		{
			//shellWriteString(&g_shell,"Off\r\n");
		}

		isDotOn = !isDotOn;

	

		updateReg();
		xTaskDelayUntil( &xLastWakeTime, xFrequency );
	}
}

void InitRealtimeTask(void)
{
	RtcTimeInit();

	init74HC595();
	updateReg();

	m_pRtcTime = returnRtcTime();

}

#define RealtimeTaskPriority 2
TaskHandle_t m_realtimeHandle;

void createRealtimeTask (void)
{

	InitRealtimeTask();

	xTaskCreate(RealTimeTask,
				"realtime task",
				128,
				NULL,
				RealtimeTaskPriority,
				&m_realtimeHandle);

}


int RealTimeCommand(int argc, char *agrv[])
{
	char * command = NULL;
	u32    length = 0;
	u32    error = 1;
	u32    cmd = RTCmdMax;

	if (argc == 4 || argc == 3 || argc == 5 || argc == 6)
	{
		u32 idx = 0;
		s8 value = 0;

		for (u32 i = 0; i < RTCmdMax; i++)
		{
			if (0 == strcmp(m_RTCmdName[i],agrv[1]))
			{
				cmd = i;
			}
		}

		switch (cmd)
		{
			case SETBUFFER:
				if (argc == 4)
				{
					idx = atoi(agrv[2]);
					value = atoi(agrv[3]);
					error = setBuffer(idx, value);
				}
				break;
			case SETTUBE:
				if (argc == 4)
				{
					idx = atoi(agrv[2]);
					value = atoi(agrv[3]);
					error = setTube(idx,value);
				}
				break;
			case SETDOT:
				if (argc == 3)
				{
					value = atoi(agrv[2]);
					error = value?setDote():clearDote();
				}
				break;
			case SETDATE:
				if (argc == 6)
				{
					RtcTime_t rtc;
					RtcTime_t * pRtc = returnRtcTime();
					rtc.year	= atoi(agrv[2]);
					rtc.month	= atoi(agrv[3]);
					rtc.date	= atoi(agrv[4]);
					rtc.week	= atoi(agrv[5]);
					rtc.hour	= pRtc->hour;
					rtc.minute	= pRtc->minute;
					rtc.second	= pRtc->second;

					setRtcTime(&rtc);
					error = 0;
				}
				break;
			case SETTIME:
				if (argc == 5)
				{
					RtcTime_t rtc;
					RtcTime_t * pRtc = returnRtcTime();
					rtc.year	= pRtc->year;
					rtc.month	= pRtc->month;
					rtc.week	= pRtc->week;
					rtc.date	= pRtc->date;
					rtc.hour	= atoi(agrv[2]);
					rtc.minute	= atoi(agrv[3]);
					rtc.second	= atoi(agrv[4]);

					setRtcTime(&rtc);
					error = 0;
				}
				break;
			default:
				break;
		}
		if (0 == error)
		{
			updateReg();
		}
		else
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
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN, RTIME, RealTimeCommand,RTIME <SETBUFFER|SETTUBE|SETDOT> <idx> <value>);



