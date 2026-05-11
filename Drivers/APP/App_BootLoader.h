#ifndef _APP_BOOTLOADER_H_
#define _APP_BOOTLOADER_H_

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "usart.h"

#include "Int_BootLoader.h"

typedef enum
{
    BOOTLOADER_STATUS_INIT,
    BOOTLOADER_STATUS_WaitForStart,
    BOOTLOADER_STATUS_REC_DATA,
    BOOTLOADER_STATUS_CHECK,
    BOOTLOADER_STATUS_FAIL,
    BOOTLOADER_STATUS_JUMP,
}BootLoader_Status;

// 初始化->打印日志启动
void App_BootLoader_Init(void);
void App_BootLoader_Run(void);
void App_BootLoader_Rec_Data(void);
void App_BootLoader_Check(void);
void App_BootLoader_Jump(void);
void App_BootLoader_Process(void);

#endif
