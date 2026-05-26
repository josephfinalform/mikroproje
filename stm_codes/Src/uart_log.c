#include "main.h"
#include "uart_log.h"
#include "bms.h"
#include <stdio.h>
#include <string.h>

UART_HandleTypeDef huart1;

void MX_USART1_UART_Init(void) {
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 9600;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK) {
        Error_Handler();
    }
}

void UART_SendString(const char *str) {
    HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

static int fmt_float(char *buf, float val, int dec) {
    int mul = 1; for (int i = 0; i < dec; i++) mul *= 10;
    int ip = (int)val;
    int dp = (int)((val - ip) * mul);
    if (dp < 0) dp = -dp;
    return snprintf(buf, 16, "%d.%0*d", ip, dec, dp);
}

void UART_LogBMSData(void) {
    char buf[128], *p = buf;
    const char *state_str;
    switch (bms.state) {
        case BMS_STATE_CHARGE:    state_str = "CHARGE";    break;
        case BMS_STATE_DISCHARGE: state_str = "DISCHARGE"; break;
        case BMS_STATE_FAULT:     state_str = "FAULT";     break;
        default:                  state_str = "IDLE";      break;
    }
    p += sprintf(p, "V1=");  p += fmt_float(p, bms.cell_voltage[0], 2);  p += sprintf(p, "V ");
    p += sprintf(p, "V2=");  p += fmt_float(p, bms.cell_voltage[1], 2);  p += sprintf(p, "V ");
    p += sprintf(p, "I1=");  p += fmt_float(p, bms.current[0], 2);       p += sprintf(p, "A ");
    p += sprintf(p, "I2=");  p += fmt_float(p, bms.current[1], 2);       p += sprintf(p, "A ");
    p += sprintf(p, "SoC=%d%% ", (int)(bms.soc + 0.5f));
    p += sprintf(p, "State=%s Relay=%s\r\n", state_str, bms.load_connected ? "ON" : "OFF");
    HAL_UART_Transmit(&huart1, (uint8_t *)buf, p - buf, HAL_MAX_DELAY);
}
