#include "Int_BootLoader.h"

uint8_t uart_rec_buff[BOOTLOADER_UART_REC_BUFF_LEN];
uint16_t uart_rec_len = 0;
uint16_t uart_rec_full_len = 0;

// 记录当前写入程序的偏移量
uint32_t flash_write_offset = 0;
// 末尾可能出现的单独字节,用于下一次写入的时候放在开头
uint8_t last_end_byte = 0;
uint8_t last_end_byte_flag = 0;

// 记录本次数据接收的时间,用来判断是否接收完成
uint32_t last_rec_time = 0;

void Int_BootLoader_Erase_Flash(uint32_t page_addr,uint16_t pages)
{
    HAL_FLASH_Unlock();
    FLASH_EraseInitTypeDef erase_init;
    erase_init.TypeErase    = FLASH_TYPEERASE_PAGES;  // 擦除类型 页擦除
    erase_init.Banks        = FLASH_BANK_1;           // 擦除哪个FLASH BANK.只有BANK1
    erase_init.NbPages      = pages;                  // 擦除页的数量,这里只需要擦除一页
    erase_init.PageAddress  = page_addr;              // 页起始地址
    HAL_FLASHEx_Erase(&erase_init,NULL);              // 比较耗费性能
    HAL_FLASH_Lock();
}

static void Int_BootLoader_Check_Erase_Flash(void)
{
    // 2.1 遍历需要写入的地址 长度为当前接收的数据长度 如果全部内容都是0xFF 则说明已经擦除过了
    uint8_t need_erased = 0;
    uint32_t erase_page_start_addr = 0;

    for(uint16_t i=0;i<uart_rec_len;i++)
    {
        // 读取对应flash地址上每一个位置的值,倘若有一个不是0xFF,则需要擦除
        uint8_t data = *(volatile uint8_t*)(APP_START_ADDR+flash_write_offset+i); 
        if(data != 0xFF)
        {
            // 根据这个data在哪一页记录要擦除的页起始地址
            erase_page_start_addr = (APP_START_ADDR+flash_write_offset+i) - 
                                    (APP_START_ADDR+flash_write_offset+i)%(0x800U); // 0x800U 为一个页的长度,取模再减去余数得到页起始地址
            need_erased = 1;
            break;
        }
    }

    // 2.2 如果需要擦除 则擦除当前页
    if(need_erased)
    {
        FLASH_EraseInitTypeDef erase_init;
        erase_init.TypeErase  = FLASH_TYPEERASE_PAGES; // 擦除类型 页擦除
        erase_init.Banks     = FLASH_BANK_1; // 擦除哪个FLASH BANK.只有BANK1
        erase_init.NbPages = 1;  // 擦除页的数量,这里只需要擦除一页
        erase_init.PageAddress = erase_page_start_addr;  // 页起始地址
        HAL_FLASHEx_Erase(&erase_init,NULL); // 比较耗费性能
    }
}

static void Int_BootLoader_Write_Flash(void)
{
    //3.1 判断当前内容是否为偶数
    if((uart_rec_len + last_end_byte_flag)%2 == 0) // 这次内容+上次遗留 = 偶数
    {
        if(last_end_byte_flag)
        {
            // 这次收到的内容是个奇数,奇+1=偶数 上次遗留有一个字节,这次作为第一个字节进行写入
            for(uint16_t i=0;i<uart_rec_len;i+=2)
            {
                uint16_t data = 0;
                uint32_t addr = APP_START_ADDR+flash_write_offset+i;

                if(i==0) // 单独处理上次遗留
                {
                    data = uart_rec_buff[i]<<8 | last_end_byte;
                }
                else
                {
                    data = uart_rec_buff[i]<<8 | uart_rec_buff[i-1];
                }
                HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,addr,data);
            }
        }
        else
        {
            // 这次收到的内容是个偶数,偶+0=偶数 上次无遗留字节,直接写本次内容就好
            for(uint16_t i=0;i<uart_rec_len;i+=2)
            {
                uint16_t data = 0;
                uint32_t addr = APP_START_ADDR+flash_write_offset+i;

                data = uart_rec_buff[i+1] << 8 | uart_rec_buff[i];
                HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,addr,data);
            }
        }
        last_end_byte_flag = 0;
        last_end_byte = 0;
        flash_write_offset += uart_rec_len+last_end_byte_flag; // 更新写入偏移量
    }
    else // 这次内容+上次遗留 = 奇数
    {
        if(last_end_byte_flag)
        {
            // 这次收到的内容是个偶数,偶+1=奇数
            for(uint16_t i=0;i<uart_rec_len;i+=2)
            {
                uint16_t data = 0;
                uint32_t addr = APP_START_ADDR+flash_write_offset+i;

                if(i==0) // 单独处理上次遗留
                {
                    data = uart_rec_buff[i]<<8 | last_end_byte;
                }
                else
                {
                    data = uart_rec_buff[i]<<8 | uart_rec_buff[i-1]; // 这样写恰好会忽略本次接收到的最后一个字节
                }
                HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,addr,data);
            }
        }
        else
        {
            // 这次收到的内容是个奇数,奇+0=奇数
            for(uint16_t i=0;i<uart_rec_len;i+=2)
            {
                if (i==uart_rec_len-1)
                {
                    break;
                }
                uint16_t data = 0;
                uint32_t addr = APP_START_ADDR+flash_write_offset+i;

                data = uart_rec_buff[i+1] << 8 | uart_rec_buff[i];
                HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,addr,data);
            }
        }
        last_end_byte_flag = 1;
        last_end_byte = uart_rec_buff[uart_rec_len-1];
        flash_write_offset += uart_rec_len+last_end_byte_flag-1; // 更新写入偏移量
    }
}


/*-----------------------------------------------------------------------------------------*/
uint8_t Int_BootLoader_Jump_to_App(uint32_t start_addr)
{
    // 1.健壮性判断
    // 1.1校验栈顶指针->Keil中IRAM1 Start是0x2000 0000 Size是0x10000.
    uint32_t app_stack_ptr = *(volatile uint32_t*)(start_addr);
    if(app_stack_ptr < STACK_START_ADDR || app_stack_ptr > STACK_START_ADDR+STACK_SIZE)
    {
        return 1;
    }

    // 1.2判断复位中断处理函数的位置，向量表是一张“跳转地址表”，每个表项存的是 ISR 的入口地址.这里获取的是复位中断向量所指向的值，即复位中断处理函数的入口地址
    uint32_t app_reset_handle = *(volatile uint32_t*)(start_addr+4); // 栈顶地址后面紧跟着中断向量表,表里面存储着ISR的入口地址
    if(app_reset_handle < start_addr || app_reset_handle > FLASH_END_ADDR)
    {
        return 1;
    }

    // 2.注销BootLoader程序
    NVIC_DisableIRQ(USART1_IRQn);

    // 关闭Systick系统滴答定时器
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
    
    // 注销HAL库的设置
    HAL_DeInit(); // 注销对外设的配置,不会注销内核

    // 2.1 关闭B程序中断.
    __disable_irq();

    // 2.2 重设置堆栈指针
    __set_MSP(app_stack_ptr);

    // 2.3 重定向中断向量表
    SCB->VTOR = start_addr;

    // 3.跳转到A程序的复位中断处理函数
    // 3.1声明一种函数指针
    typedef void (*pFunc)(void);

    // 3.2完成强制类型转换
    pFunc App_Reset_Handle = (pFunc)(volatile uint32_t*)(app_reset_handle);

    // 3.3调用跳转
    App_Reset_Handle();

    return 0;
}

