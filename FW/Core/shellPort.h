#ifndef SHELLPORT_H
#define SHELLPORT_H

#include "stdint.h"
#include "shell.h"

/* 将shell定义为外部变量，在串口中断回调函数中还要使用 */
extern Shell g_shell;

enum shellTextCustomEnum
{
    SHELL_TEXT_WRONG_NUMBER_OF_PARAMETERS,
    SHELL_TEXT_PARAMETERS_FORMAT_ERROR,
    SHELL_TEXT_WAIT_FOR_RESULT_TIEMOUT,
};
extern const char *shellTextCustom[];

void UserShellInit(void);
void createShellTask (void);
void OnShellReceiveData(uint8_t*data,uint32_t num,uint8_t isFromISR);
#endif