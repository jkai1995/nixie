#include "powerCtrl.h"
#include "GPIO_STM32F10x.h"
#include "shellPort.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


const static GPIO_PIN_ID Pin_pwCtrl[] = {
  { GPIOB, 12 },
};

#define PWCTRL_COUNT (sizeof(Pin_pwCtrl)/sizeof(GPIO_PIN_ID))

const static uint32_t PowerStatus[] =
{
	0,//V170_SHDN
};

void powerCtrlInit (void)
{
	/* Configure pins: Push-pull Output Mode (2 MHz) */
	for (uint32_t n = 0; n < PWCTRL_COUNT;n++)
	{
    GPIO_PortClock   (Pin_pwCtrl[n].port, true);
    pwerOn(n);
    GPIO_PinConfigure(Pin_pwCtrl[n].port, Pin_pwCtrl[n].num,
                      GPIO_OUT_PUSH_PULL,
                      GPIO_MODE_OUT2MHZ);
	}
}

void pwerOn (uint32_t num)
{
		if (num < PWCTRL_COUNT)
		{
			GPIO_PinWrite(Pin_pwCtrl[num].port, Pin_pwCtrl[num].num, PowerStatus[num]);
		}
}

void pwerOff (uint32_t num)
{
		if (num < PWCTRL_COUNT)
		{
			GPIO_PinWrite(Pin_pwCtrl[num].port, Pin_pwCtrl[num].num, (PowerStatus[num]?0:1));
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


