#ifndef __INT_W24C02_H
#define __INT_W24C02_H

#include "i2c.h"
#include "usart.h"

#define W24C02_ADDR_WRITE 0XA0
#define W24C02_ADDR_READ  0XA1

#define W24C02_ADDR_SIZE 8  // W24C02地址总线宽度为8位,总共256个字节
#define W24C02_PAGE_SIZE 16 // W24C02每页16字节

uint8_t Int_W24C02_Read_Byte(uint8_t byte_addr);
void Int_W24C02_Write_Byte(uint8_t byte_addr, uint8_t data);
void Int_W24C02_Read_Bytes(uint8_t byte_addr,uint8_t* data,uint16_t len);
void Int_W24C02_Write_Bytes(uint8_t byte_addr,uint8_t* data,uint16_t len);

#endif
