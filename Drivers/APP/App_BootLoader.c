#include "App_BootLoader.h"

uint8_t app_boot_update_flag = BOOT_NOT_UPDATE;
uint8_t meta_data_buf[8];

static uint8_t App_BootLoader_Check_Code(void)
{
// 1.读取元数据信息 -> 描述后续的程序.前4个字节拼成App程序的起始地址,后4个字节拼成App程序的长度
    Int_W25Q32_Read_Data(W25Q32_META_DATA_ADDR_BLOCK,W25Q32_META_DATA_ADDR_SECTOR,W25Q32_META_DATA_ADDR_PAGE,W25Q32_META_DATA_ADDR_PAGE_ADDR,meta_data_buf,8);
    uint32_t App_Start_Addr_In_W25Q32 = meta_data_buf[0] | meta_data_buf[1] << 8 | meta_data_buf[2] << 16 | meta_data_buf[3] << 24;// 低位在前
    uint32_t App_Code_Size  = meta_data_buf[4] | meta_data_buf[5] << 8 | meta_data_buf[6] << 16 | meta_data_buf[7] << 24;

    // 2. 校验程序
    if(App_Start_Addr_In_W25Q32 < APP_START_ADDR_IN_W25Q32_MIN)
    {
        printf("Error: App_Start_Addr_In_W25Q32 error\n");
        return 1;
    }
    if (App_Code_Size < APP_CODE_SIZE_MIN || App_Code_Size > APP_CODE_SIZE_MAX)
    {
        printf("Error: App_Code_Size error\n");
        return 1;
    }

    // 读取程序判断程序头两个uint32_t值是否符合要求
    uint8_t app_head_buf[8];
    Int_W25Q32_Read_Data_Uint32(App_Start_Addr_In_W25Q32,app_head_buf,2);

    uint32_t app_stack_ptr = app_head_buf[0] | app_head_buf[1] << 8 | app_head_buf[2] << 16 | app_head_buf[3] << 24; // 在之前的代码中,要校验的程序已经在Flash里面了,所以使用的是指针解引用的方式获取值.这里去读的是W25Q32里面的值,他们就是Flash中各地址上面的值
    if(app_stack_ptr < STACK_START_ADDR || app_stack_ptr > STACK_START_ADDR+STACK_SIZE)
    {
        return 1;
    }

    // 1.2判断复位中断处理函数的位置，向量表是一张“跳转地址表”，每个表项存的是 ISR 的入口地址.这里获取的是复位中断向量所指向的值，即复位中断处理函数的入口地址
    uint32_t app_reset_handle = app_head_buf[4] | app_head_buf[5] << 8 | app_head_buf[6] << 16 | app_head_buf[7] << 24;
    if(app_reset_handle < APP_START_ADDR || app_reset_handle > FLASH_END_ADDR)
    {
        return 1;
    }

    return 0;
}

static void App_BootLoader_Write_Flash(void)
{
    // 1.进行数据校验
    if(App_BootLoader_Check_Code() != 0)
    {
        return;
    }
    
    // 2. 写入程序到Flash
}

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
        App_BootLoader_Write_Flash();
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
