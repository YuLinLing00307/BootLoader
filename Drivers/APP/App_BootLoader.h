#ifndef __APP_BOOTLOADER_H__
#define __APP_BOOTLOADER_H__

#include "Int_w24c02.h"
#include "Int_BootLoader.h"
#include "Int_W25Q32.h"

// 添加校验的密钥
#define CHECK_KEY_ADDR 0x11
#define CHECK_KEY      0x5A6B

// 存储更新状态
#define CHECK_UPDATE_ADDR   0x10
#define BOOT_UPDATE         0x00
#define BOOT_NOT_UPDATE     0X01
#define BOOT_RESET          0X02


// W25Q32中存储元数据信息的地址
#define W25Q32_META_DATA_ADDR_BLOCK          0X00
#define W25Q32_META_DATA_ADDR_SECTOR         0X00
#define W25Q32_META_DATA_ADDR_PAGE           0X00
#define W25Q32_META_DATA_ADDR_PAGE_ADDR      0X00
#define APP_START_ADDR_IN_W25Q32_MIN         0X001000
#define APP_CODE_SIZE_MIN                    500
#define APP_CODE_SIZE_MAX                    0X78000

// 判断当前是否需要更新
void App_BootLoader_Check_Update(void);
void App_BootLoader_Check_Default(void);
void App_BootLoader_Update(void);
void App_BootLoader_Jump_to_App(void);

#endif

