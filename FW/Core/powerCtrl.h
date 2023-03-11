#ifndef POWERCTRL_H
#define POWERCTRL_H

#include "stdint.h"
#include "gpioDriver.h"

enum ePowerCtrl
{
	V170_SHDN,//PB12
	PinMax,
};

void powerCtrlInit (void);
void pwerOn (uint32_t pin);
void pwerOff (uint32_t pin);

#endif