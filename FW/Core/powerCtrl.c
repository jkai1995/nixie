#include "powerCtrl.h"
#include "shellPort.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

gpioInfo_t m_gpioPowerCtrl[PinMax] = 
{
    {"V170_SHDN",    GPIOB,GPIO_Pin_12},
};

const static uint32_t PowerOnStatus[PinMax] =
{
    0,//V170_SHDN
};

const static uint32_t PowerOffStatus[PinMax] =
{
    1,//V170_SHDN
};

#define WriteGpio(p,x) GPIO_WriteBit(m_gpioPowerCtrl[p].gpiox,m_gpioPowerCtrl[p].pinx,x)

void powerCtrlInit (void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    regularOutGpioInit(m_gpioPowerCtrl,PinMax);

    pwerOn(V170_SHDN);
}

void pwerOn (uint32_t pin)
{
    if (pin < PinMax)
    {
        WriteGpio(pin,PowerOnStatus[pin]);
    }
}

void pwerOff (uint32_t pin)
{
    if (pin < PinMax)
    {
        WriteGpio(pin,PowerOffStatus[pin]);
    }
}

int PowerCommand(int argc, char *agrv[])
{
    char * command = NULL;
    u32    length = 0;

    if (argc == 2)
    {
        u32 value = 0;
        value = atoi(agrv[1]);

        if (value > 0)
        {
            pwerOn(0);
        }
        else
        {
            pwerOff(0);
        }

    }
    else
    {
        shellWriteString(&g_shell,shellTextCustom[SHELL_TEXT_WRONG_NUMBER_OF_PARAMETERS]);
    }


    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN)|SHELL_CMD_DISABLE_RETURN, boost, PowerCommand, enable|disable 12v-170v boost);


