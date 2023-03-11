#include "stm32f10x.h"
#include "powerCtrl.h"
#include "shellPort.h"
#include "bluetooth.h"
#include "misc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "realtimeCtrl.h"

void createTasks(void *pvParameters )
{

    powerCtrlInit();
    //UserShellInit();
    JDY08_Init();
    
    createShellTask();
    createRealtimeTask();
    vTaskSuspend(NULL);
}


int main (void)
{
    /*设置优先级分组*/
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    TaskHandle_t xHandle;

    xTaskCreate(createTasks,
                "createTasks",
                64,
                NULL,
                1,
                &xHandle);
    /*启动任务调度器，任务开始执行*/
    vTaskStartScheduler();


    for(;;);
}



