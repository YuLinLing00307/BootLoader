#include "Int_BootLoader.h"

uint8_t uart_rec_buff[BOOTLOADER_UART_REC_BUFF_LEN];
uint16_t uart_rec_len = 0;
uint16_t uart_rec_full_len = 0;

// 记录当前写入程序的偏移量
uint32_t flash_write_offset = 0;
// 末尾可能出现的单独字节,用于下一次写入的时候放在开头
uint8_t last_end_byte = 0;
uint8_t last_end_byte_flag = 0;

void Int_BootLoader_Init(void)
{
    // 清空掉串口初始化使用之前的所有问题
    __HAL_UART_CLEAR_OREFLAG(&huart1);
    __HAL_UART_CLEAR_IDLEFLAG(&huart1);

    // 带有中断的串口接收函数
    HAL_UARTEx_ReceiveToIdle_IT(&huart1,uart_rec_buff,BOOTLOADER_UART_REC_BUFF_LEN);
}

// 触发空闲中断时回调函数
// HAL串口代码比较繁琐,如果在串口中进行printf串口打印,会占用很多资源
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if(huart->Instance == USART1)
    {
        // 保存接收到的数据长度
        uart_rec_len = Size;
        uart_rec_full_len += Size;
        
        // 1.解锁Flash
        HAL_FLASH_Unlock();

        // 2.判断当前写入的地址是否需要擦除.
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

        //3. 完成flash内容写入 -> 使用16位写入(半字)
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

        // 4. 重新上锁
        HAL_FLASH_Lock();

        // 使用完数据后清空
        memset(uart_rec_buff,0,BOOTLOADER_UART_REC_BUFF_LEN);

        // 进行下一次接收
        HAL_UARTEx_ReceiveToIdle_IT(&huart1,uart_rec_buff,BOOTLOADER_UART_REC_BUFF_LEN);
    }
}
