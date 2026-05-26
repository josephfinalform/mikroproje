#ifndef BMS_H
#define BMS_H

#include "stm32f1xx_hal.h"

#define BMS_CELL_COUNT      2
#define BMS_VOLTAGE_THRESH  3.0f
#define BATTERY_FULL        4.2f
#define BATTERY_EMPTY       3.0f
#define VOLTAGE_DIVIDER_R1  10000.0f
#define VOLTAGE_DIVIDER_R2  7500.0f
#define ADC_VREF            3.3f
#define ADC_RESOLUTION      4095.0f

typedef enum {
    BMS_STATE_IDLE      = 0,
    BMS_STATE_CHARGE    = 1,
    BMS_STATE_DISCHARGE = 2,
    BMS_STATE_FAULT     = 3,
} BMS_StateTypeDef;

typedef struct {
    float cell_voltage[BMS_CELL_COUNT];
    float current[2];
    float soc;
    BMS_StateTypeDef state;
    uint8_t load_connected;
} BMS_HandleTypeDef;

extern BMS_HandleTypeDef bms;

void BMS_Init(void);
void BMS_Update(void);
float BMS_VoltageToSoC(float v_cell);
float BMS_SoCToOCV(float soc, uint8_t charging);
uint8_t BMS_CheckVoltageDrop(void);
void BMS_SetRelay(uint8_t on);
void BMS_SetLEDs(void);

#endif
