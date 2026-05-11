#include "App_BootLoader.h"

uint8_t app_rec_start_buff[64] = {0};
uint16_t app_rec_start_len;

// 接收到的数据总长度
uint32_t app_rec_total_len;

uint8_t send_finished_flag = 0;

BootLoader_Status bootloader_status = BOOTLOADER_STATUS_INIT;

void App_BootLoader_Init(void)
{
    printf("BootLoader Start\r\n");
    printf("send 'start:len' to start\r\n");
    bootloader_status = BOOTLOADER_STATUS_WaitForStart;
}
void App_BootLoader_WaitForStart(void)
{
    // 使用阻塞非中断方式接收 -> 区分接收程序
    HAL_UARTEx_ReceiveToIdle(&huart1,app_rec_start_buff,sizeof(app_rec_start_buff),&app_rec_start_len,0xffffffff);
    if(app_rec_start_len > 0)
    {
        // 判断接收到的数据是否包含start:len
        char* start_str = strstr((char*)app_rec_start_buff,"start:");
        if(start_str != NULL)
        {
            app_rec_total_len = atoi((char*)start_str+6);
            printf("app len:%d",app_rec_total_len);
            if(app_rec_total_len > 0)
            {
                Int_BootLoader_Erase_Flash(APP_FLASH_START_ADDR,10); // 准备进入数据接收状态,先提前擦除10页
                Int_BootLoader_Init(); // 准备进入数据接收状态,开启串口接收空闲中断
                printf("ready to receive app\r\n");
                bootloader_status = BOOTLOADER_STATUS_REC_DATA; //直到接收到正确的start:len,则修改状态
            }
            else
            {
                printf("len error\r\n");
            }
        }
        else
        {
            printf("error!\r\n"); // 倘若格式不对,则打印error
        }
    }
    memset(app_rec_start_buff,0,sizeof(app_rec_start_buff));
}

// 等待接收完毕进行状态切换
void App_BootLoader_Rec_Data(void)
{
    // 接收完成后修改状态为检查状态
    // (1) 软件:等待2s.其中last_rec_time不等于0,说明已经开始接收到了第一次数据.如果last_rec_time一直为0,则说明还没开始发送
    if(last_rec_time != 0 && (HAL_GetTick() - last_rec_time > 2000))
    {
        bootloader_status = BOOTLOADER_STATUS_CHECK;
    }

    // (2) 硬件:使用按键,按下跳转
    if(send_finished_flag == 1)
    {
        bootloader_status = BOOTLOADER_STATUS_CHECK;
    }
}

// 检查数据是否合法
void App_BootLoader_Check(void)
{
    if(uart_rec_full_len == app_rec_total_len)
    {
        // 说明长度一致
        printf("app rec ok!\r\n");
        bootloader_status = BOOTLOADER_STATUS_JUMP;
    }
    else
    {
        // 倘若校验失败,则前往失败状态
        printf("app rec error!\r\n");
        bootloader_status = BOOTLOADER_STATUS_FAIL;
    }
}

void App_Bootloader_Jump(void)
{
    printf("jump to app!\r\n");
    uint8_t ret = Int_BootLoader_Jump_to_App(); // 倘若该函数中校验失败,则跳转到错误状态
    if(ret == 1)
    {
        bootloader_status = BOOTLOADER_STATUS_FAIL;
    }
}

void App_Bootloader_Fail(void)
{
    printf("The application is fail!The system will reset!\r\n");
    HAL_NVIC_SystemReset();
}

// 在main中死循环调用的函数,需要死循环的函数直接用main中的死循环即可
void App_BootLoader_Process(void)
{
    switch(bootloader_status)
    {
        case BOOTLOADER_STATUS_WaitForStart:
            App_BootLoader_WaitForStart();
            break;
        case BOOTLOADER_STATUS_REC_DATA:
            App_BootLoader_Rec_Data();
            break;
        case BOOTLOADER_STATUS_CHECK:
            App_BootLoader_Check();
            break;
        case BOOTLOADER_STATUS_JUMP:
            App_Bootloader_Jump();
            break;
        case BOOTLOADER_STATUS_FAIL:
            App_Bootloader_Fail();
            break;
        default:
            break;
    }
}

// 按键中断回调函数 -> 确认发送程序完成
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == KEY1_Pin)
    {
        send_finished_flag = 1;
    }
}
