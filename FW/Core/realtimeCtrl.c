#include "realtimeCtrl.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "shellPort.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "ds3231Port.h"
#include "tubeDriver.h"


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

RtcTime_t* m_pRtcTime;
NixeTubeCtrl_t m_nixeTubeCtrl;


void RealTimeTask(void *param)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(500);
    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();//获取当前tick

    int isDotOn = 0; 
    char buffer[10];

    u8 tube[m_nixeTubeCtrl.numOfTube];

    while(1)
    {
        isDotOn?m_nixeTubeCtrl.setDote():m_nixeTubeCtrl.clearDote();

        if (isDotOn)
        {
            getRtcTime();
            tube[0] = m_pRtcTime->hour / 10;
            tube[1] = m_pRtcTime->hour % 10;
            tube[2] = m_pRtcTime->minute / 10;
            tube[3] = m_pRtcTime->minute % 10;
            for (int i = 0; i < m_nixeTubeCtrl.numOfTube; i++)
            {
                m_nixeTubeCtrl.setTube(i,tube[i]);
            }

            sprintf(buffer,"On : %d\r\n",m_pRtcTime->second);
            //shellWriteString(&g_shell,buffer);
        }
        else
        {
            //shellWriteString(&g_shell,"Off\r\n");
        }

        isDotOn = !isDotOn;

        m_nixeTubeCtrl.refreshTube();
        xTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}

void InitRealtimeTask(void)
{
    RtcTimeInit();

    initNixeTube(&m_nixeTubeCtrl);

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
                    //error = setBuffer74HC595(idx, value);
                    error = 0;
                }
                break;
            case SETTUBE:
                if (argc == 4)
                {
                    idx = atoi(agrv[2]);
                    value = atoi(agrv[3]);
                    error = m_nixeTubeCtrl.setTube(idx,value);
                }
                break;
            case SETDOT:
                if (argc == 3)
                {
                    value = atoi(agrv[2]);
                    error = value?m_nixeTubeCtrl.setDote():m_nixeTubeCtrl.clearDote();
                }
                break;
            case SETDATE:
                if (argc == 6)
                {
                    RtcTime_t rtc;
                    RtcTime_t * pRtc = returnRtcTime();
                    rtc.year    = atoi(agrv[2]);
                    rtc.month    = atoi(agrv[3]);
                    rtc.date    = atoi(agrv[4]);
                    rtc.week    = atoi(agrv[5]);
                    rtc.hour    = pRtc->hour;
                    rtc.minute    = pRtc->minute;
                    rtc.second    = pRtc->second;

                    setRtcTime(&rtc);
                    error = 0;
                }
                break;
            case SETTIME:
                if (argc == 5)
                {
                    RtcTime_t rtc;
                    RtcTime_t * pRtc = returnRtcTime();
                    rtc.year    = pRtc->year;
                    rtc.month    = pRtc->month;
                    rtc.week    = pRtc->week;
                    rtc.date    = pRtc->date;
                    rtc.hour    = atoi(agrv[2]);
                    rtc.minute    = atoi(agrv[3]);
                    rtc.second    = atoi(agrv[4]);

                    setRtcTime(&rtc);
                    error = 0;
                }
                break;
            default:
                break;
        }
        if (0 == error)
        {
            m_nixeTubeCtrl.refreshTube();
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



