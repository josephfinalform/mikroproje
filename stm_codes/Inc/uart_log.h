#ifndef UART_LOG_H
#define UART_LOG_H

#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef huart1;

void MX_USART1_UART_Init(void);
void UART_LogBMSData(void);
void UART_SendString(const char *str);

#endif
