#ifndef __INT_W25Q32_H__
#define __INT_W25Q32_H__

#include "main.h"
#include "spi.h"

// SPI设备在使用的时候片选引脚需要拉低，平时都拉高

// 命令声明
#define W25Q32_READ_ID 0X9F

void Int_W25Q32_Start(void);
void Int_W25Q32_Stop(void);
void Int_W25Q32_Write_Byte(uint8_t data);
uint8_t Int_W25Q32_Read_Byte(void);


#endif
