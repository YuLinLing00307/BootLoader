#include "Int_BootLoader.h"

uint8_t uart_rec_buff[BOOTLOADER_UART_REC_BUFF_LEN];
uint16_t uart_rec_len = 0;
uint16_t uart_rec_full_len = 0;

// 记录当前写入程序的偏移量
uint32_t flash_write_offset = 0;

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

        // 使用完数据后清空
        memset(uart_rec_buff,0,BOOTLOADER_UART_REC_BUFF_LEN);

        // 进行下一次接收
        HAL_UARTEx_ReceiveToIdle_IT(&huart1,uart_rec_buff,BOOTLOADER_UART_REC_BUFF_LEN);
    }
}
