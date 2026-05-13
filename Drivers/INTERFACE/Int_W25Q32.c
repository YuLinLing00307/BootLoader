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

