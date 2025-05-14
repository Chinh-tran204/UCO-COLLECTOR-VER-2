//DMA mode normal and NVIC interrupt turn on
#include "stm32f1xx_hal_uart.h"

uint8_t rxData[4096];       //buffer where the data will be save in
uint16_t indx = 0;          //number of byte have been receive
int count  = 0;

HAL_UARTEx_ReceiveToIdle_IT(&huart, rxData, 30);

void HAL_UARTEx_RxEventCallback('UART_HandleTypeDef *huart', uint16_t Size)