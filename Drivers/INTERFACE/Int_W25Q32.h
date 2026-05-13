#ifndef __INT_W25Q32_H__
#define __INT_W25Q32_H__

#include "main.h"
#include "spi.h"

// SPI设备在使用的时候片选引脚需要拉低，平时都拉高

// 命令声明
#define W25Q32_READ_ID              0X9F
#define W25Q32_READ_COMMAND         0X03
#define W25Q32_WRITE_EN             0X06
#define W25Q32_WRITE_COMMAND        0X02
#define W25Q32_ERASE_SECTOR_COMMAND 0X20
#define W25Q32_STATUS_REG           0X05

void Int_W25Q32_Start(void);
void Int_W25Q32_Stop(void);
void Int_W25Q32_Write_Byte(uint8_t data);
uint8_t Int_W25Q32_Read_Byte(void);

// 实际ADDR共有24位 0x000000 -> 0x3FF FFF 一次擦除4096字节即0xFFF，一次写入256字节
//void Int_W25Q32_Read_Data(uint32_t addr,uint8_t* data,uint8_t len);

// 采用把地址拆分的方式,更加直观
void Int_W25Q32_Read_Data(uint8_t Block,uint8_t Sector,uint8_t Page,uint8_t Addr,uint8_t* data,uint16_t len);

void Int_W25Q32_Write_Data(uint8_t Block,uint8_t Sector,uint8_t Page,uint8_t Addr,uint8_t* data,uint16_t len);

void Int_W25Q32_Erase_Sector(uint8_t Block,uint8_t Sector);

#endif
