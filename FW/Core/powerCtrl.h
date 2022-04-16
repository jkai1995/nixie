#ifndef POWERCTRL_H
#define POWERCTRL_H

#include "stdint.h"

enum ePowerCtrl
{
		V170_SHDN,
};

void powerCtrlInit (void);
void pwerOn (uint32_t num);
void pwerOff (uint32_t num);

#endif