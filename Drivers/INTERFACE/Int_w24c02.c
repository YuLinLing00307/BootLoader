#include "Int_w24c02.h"

uint8_t Int_W24C02_Read_Byte(uint8_t byte_addr)
{
    uint8_t data = 0;
    HAL_I2C_Mem_Read(&hi2c2,W24C02_ADDR_READ,byte_addr,I2C_MEMADD_SIZE_8BIT,&data,1,1000);
    return data;
}

void Int_W24C02_Write_Byte(uint8_t byte_addr, uint8_t data)
{
    HAL_I2C_Mem_Write(&hi2c2, W24C02_ADDR_WRITE, byte_addr, I2C_MEMADD_SIZE_8BIT, &data, 1, 1000);
}

void Int_W24C02_Read_Bytes(uint8_t byte_addr,uint8_t* data,uint16_t len)
{
    HAL_I2C_Mem_Read(&hi2c2,W24C02_ADDR_READ,byte_addr,I2C_MEMADD_SIZE_8BIT,data,len,1000);
}

// EEPROM一次只能写一页(最多16个字节) 比如0x00->0x10 但从0x05也只能到0x10 
void Int_W24C02_Write_Bytes(uint8_t byte_addr,uint8_t* data,uint16_t len)
{
    // // 对写入的长度进行限制
    // uint8_t remain_len = 0x10 - (byte_addr % 0x10);
    // if(len > remain_len)
    //     return;
        
    // HAL_I2C_Mem_Write(&hi2c2, W24C02_ADDR_WRITE, byte_addr, I2C_MEMADD_SIZE_8BIT,data,len,1000);

    // 或者采用多段写入
    // 方法一:循环单字节写入->效率低

    // 方法二:算法判断
    if(byte_addr + len > 0xFF)
    {
        printf("Error: Write out of range");
        return;
    }

    uint8_t now_offset = 0;
    while(len > 0)
    {
        uint8_t remain_byte = (0x10) - ((byte_addr + now_offset)%0x10);
        uint8_t write_byte = 0;
        if(len > remain_byte)
        {
            write_byte = remain_byte;
        }
        else
        {
            write_byte = len;
        }
        HAL_I2C_Mem_Write(&hi2c2, W24C02_ADDR_WRITE, byte_addr + now_offset, I2C_MEMADD_SIZE_8BIT,data+now_offset,write_byte,1000);
        HAL_Delay(5);
        now_offset += write_byte;
        len -= write_byte;
    }
}
