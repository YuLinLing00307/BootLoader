#include "App_BootLoader.h"

uint8_t app_boot_update_flag = BOOT_NOT_UPDATE;

void App_BootLoader_Check_Update(void)
{
    printf("Bootloader Start!\r\n");
    printf("Check update...\r\n");

    // 读取3个字节的数据
    uint8_t data[3] = {0};
    Int_W24C02_Read_Bytes(CHECK_UPDATE_ADDR,data,3);

    // 1.校验密钥
    uint16_t key = data[1] << 8 | data[0];
    if(key != CHECK_KEY)
    {
        printf("Error: Key error\n");
        uint8_t fix_data[3] = {BOOT_NOT_UPDATE,(uint8_t)(CHECK_KEY>>8),(uint8_t)(CHECK_KEY&0xFF)};
        Int_W24C02_Write_Bytes(CHECK_KEY_ADDR,fix_data,3);
        return;
    }

    // 2.判断当前是否需要更新
    app_boot_update_flag = data[0];
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == KEY1_Pin)
    {
        app_boot_update_flag = BOOT_RESET;
    }
}

void App_BootLoader_Check_Default(void)
{
    HAL_Delay(3000); // 主要由EXTI工作,这里只做延时
}

void App_BootLoader_Update(void)
{
    if(app_boot_update_flag == BOOT_UPDATE)
    {
        // 将存储设备中的程序写入到Flash中
        printf("Update app to flash\r\n");
    }
    else if(app_boot_update_flag == BOOT_NOT_UPDATE)
    {
        printf("No update needed\r\n");
    }
    else if(app_boot_update_flag == BOOT_RESET)
    {
        printf("Reset!\r\n");
    }
}

void App_BootLoader_Jump_to_App(void)
{
    // 不管更新与否,最后都要跳转到App
    if (app_boot_update_flag == BOOT_RESET)
    {
        // 跳转到0x800 4000
        Int_BootLoader_Jump_to_App(RESET_START_ADDR);
    }
    else
    {
        // 不需要恢复到出厂设置,跳转到0x800 8000
        Int_BootLoader_Jump_to_App(APP_START_ADDR);
    }
}
