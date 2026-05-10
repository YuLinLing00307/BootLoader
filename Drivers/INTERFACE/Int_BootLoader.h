#ifndef __INT_BOOTLOADER_H
#define __INT_BOOTLOADER_H

#include "stdlib.h"
#include "string.h"
#include "main.h"
#include "usart.h"

#define BOOTLOADER_UART_REC_BUFF_LEN 512

// 程序写入的起始位置 -> A区起始位置 假设B区16K=0x4000 A区有512-16K=0x7C000
#define APP_START_ADDR 0X8004000

extern uint16_t uart_rec_full_len;

// 串口接收 -> 准备接收A程序
void Int_BootLoader_Init(void);

#endif
