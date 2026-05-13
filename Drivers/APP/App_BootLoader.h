#ifndef __APP_BOOTLOADER_H__
#define __APP_BOOTLOADER_H__

#include "Int_w24c02.h"
#include "Int_BootLoader.h"

// 添加校验的密钥
#define CHECK_KEY_ADDR 0x11
#define CHECK_KEY      0x5A6B

// 存储更新状态
#define CHECK_UPDATE_ADDR   0x10
#define BOOT_UPDATE         0x01
#define BOOT_NOT_UPDATE     0X00

// 判断当前是否需要更新
void App_BootLoader_Check_Update(void);
void App_BootLoader_Update(void);
void App_BootLoader_Jump_to_App(void);


#endif

