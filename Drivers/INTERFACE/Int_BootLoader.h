#ifndef __INT_BOOTLOADER_H
#define __INT_BOOTLOADER_H

// STD库
#include "stdlib.h"
#include "string.h"

#include "main.h"
#include "usart.h"

#define BOOTLOADER_UART_REC_BUFF_LEN 512

// 程序写入的起始位置 -> A区起始位置
#define APP_FLASH_START_ADDR 0X8008000
#define APP_FLASH_END_ADDR 0X807FFFF
#define APP_STACK_START_ADDR 0X20000000
#define APP_STACK_SIZE 0X10000

extern uint16_t uart_rec_full_len;
extern uint32_t last_rec_time;

// 串口接收 -> 准备接收A程序
void Int_BootLoader_Init(void);
uint8_t Int_BootLoader_Jump_to_App(void);
void Int_BootLoader_Erase_Flash(uint32_t page_addr,uint16_t pages);

#endif
