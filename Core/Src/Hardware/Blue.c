#include "Blue.h"
#include "main.h"
#include "usart.h"

#define BLUE_RX_BUF_SIZE 128
uint8_t rx_buffer[BLUE_RX_BUF_SIZE];
void Blue_Init(void)
{ 
    // DMA空闲中断
   HAL_UARTEx_ReceiveToIdle_DMA(&huart3, (uint8_t *)rx_buffer, BLUE_RX_BUF_SIZE);
}


/// DMA空闲中断回调函数
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART3)
    {
        // 回显接收数据
        HAL_UART_Transmit(&huart3, (uint8_t *)rx_buffer, Size, 1000);
         HAL_UARTEx_ReceiveToIdle_DMA(&huart3, (uint8_t *)rx_buffer, BLUE_RX_BUF_SIZE);
    }

}
