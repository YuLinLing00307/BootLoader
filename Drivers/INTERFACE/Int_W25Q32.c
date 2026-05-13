#include "Int_W25Q32.h"

void Int_W25Q32_Start(void)
{
    HAL_GPIO_WritePin(W25Q32_CS_GPIO_Port,W25Q32_CS_Pin,GPIO_PIN_RESET)
}

void Int_W25Q32_Stop(void)
{
    HAL_GPIO_WritePin(W25Q32_CS_GPIO_Port,W25Q32_CS_Pin,GPIO_PIN_SET);
}

void Int_W25Q32_Write_Byte(uint8_t data)
{
    HAL_SPI_Transmit(&hspi1,&data,1,1000);
}

uint8_t Int_W25Q32_Read_Byte(void)
{
    uint8_t data;
    HAL_SPI_Receive(&hspi1,&data,1,1000);
    return data;
}

void Int_W25Q32_Read_ID(uint8_t* mf_id,uint16_t* device_id)
{
    // 1.拉低片选
    Int_W25Q32_Start();

    // 2.发送读取ID指令
    Int_W25Q32_Write_Byte(W25Q32_READ_ID);

    // 3.接收SPI数据
    *mf_id = Int_W25Q32_Read_Byte();
    uint8_t high = Int_W25Q32_Read_Byte();
    uint8_t low  = Int_W25Q32_Read_Byte();
    *device_id = high << 8 | low;

    // 4.拉高片选
    Int_W25Q32_Stop();
}

// 静态方法: 等待芯片忙状态
static void Int_W25Q32_Wait_Busy(void)
{
    // 1.拉低片选
    Int_W25Q32_Start();

    // 2.读取状态寄存器
    while(1)
    {
        Int_W25Q32_Write_Byte(W25Q32_STATUS_REG);
        uint8_t status = Int_W25Q32_Read_Byte();
        if((status & 0x01) == 0) // 判断最后一位即忙位是否为0
        {
            break;
        }
    }

    // 3.拉高片选
    Int_W25Q32_Stop();
}

static void Int_W25Q32_Write_Enable(void)
{
    // 1.等待忙状态
    Int_W25Q32_Wait_Busy();

    // 2.拉低片选
    Int_W25Q32_Start();

    // 3.发送写使能
    Int_W25Q32_Write_Byte(W25Q32_WRITE_EN);

    // 4.拉高片选
    Int_W25Q32_Stop();
}

void Int_W25Q32_Read_Data(uint8_t Block,uint8_t Sector,uint8_t Page,uint8_t Addr,uint8_t* data,uint16_t len)
{
    // 1.等待忙状态结束
    Int_W25Q32_Wait_Busy();

    // 2.拉低片选
    Int_W25Q32_Start();

    // 3.发送读取指令,接收返回的数据
    Int_W25Q32_Write_Byte(W25Q32_READ_COMMAND);
    uint32_t addr_32 = Block << 16 | Sector << 12 | Page << 8 | Addr;
    Int_W25Q32_Write_Byte((uint8_t)((addr_32 >> 16) & 0XFF));
    Int_W25Q32_Write_Byte((uint8_t)((addr_32 >> 8 ) & 0XFF));
    Int_W25Q32_Write_Byte((uint8_t)((addr_32 >> 0 ) & 0XFF));

    for(uint32_t i;i<len;i++)
    {
        data[i] = Int_W25Q32_Read_Byte();
    }
    
    // 4.拉高片选
    Int_W25Q32_Stop();
}

void Int_W25Q32_Write_Data(uint8_t Block,uint8_t Sector,uint8_t Page,uint8_t Addr,uint8_t* data,uint16_t len)
{
    // 写使能
    Int_W25Q32_Write_Enable();

    // 1.等待忙状态
    Int_W25Q32_Wait_Busy();

    // 2.拉低片选
    Int_W25Q32_Start();

    // 3.写入数据
    uint32_t addr_32 = Block << 16 | Sector << 12 | Page << 8 | Addr;
    Int_W25Q32_Write_Byte(W25Q32_WRITE_COMMAND);
    Int_W25Q32_Write_Byte((uint8_t)((addr_32 >> 16) & 0XFF));
    Int_W25Q32_Write_Byte((uint8_t)((addr_32 >> 8 ) & 0XFF));
    Int_W25Q32_Write_Byte((uint8_t)((addr_32 >> 0 ) & 0XFF));

    for (uint16_t i = 0; i < len; i++)
    {
        Int_W25Q32_Write_Byte(data[i]);
    }
    
    // 4.拉高片选
    Int_W25Q32_Stop();
}

void Int_W25Q32_Erase_Sector(uint8_t Block,uint8_t Sector)
{
    // 写使能
    Int_W25Q32_Write_Enable();

    // 1.等待忙状态
    Int_W25Q32_Wait_Busy();
    
    // 2.拉低片选
    Int_W25Q32_Start();
    
    // 3.发送擦除指令
    Int_W25Q32_Write_Byte(W25Q32_ERASE_SECTOR_COMMAND);
    uint32_t addr_32 = Block << 16 | Sector << 12;
    Int_W25Q32_Write_Byte((uint8_t)((addr_32 >> 16) & 0XFF));
    Int_W25Q32_Write_Byte((uint8_t)((addr_32 >> 8 ) & 0XFF));
    Int_W25Q32_Write_Byte((uint8_t)((addr_32 >> 0 ) & 0XFF));
    
    // 4.拉高片选
    Int_W25Q32_Stop();
}

