#ifndef ADC_HANDLER_H
#define ADC_HANDLER_H

#include "stm32f1xx_hal.h"

#define ADC_BUF_SIZE  4

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
extern volatile uint16_t adc_buffer[ADC_BUF_SIZE];

void MX_ADC1_Init(void);
void MX_DMA_Init(void);
void ADC_Start(void);
float ADC_GetCurrent(uint8_t channel);
float ADC_GetCellVoltage(uint8_t cell);

#endif
