#include "bms.h"
#include "adc_handler.h"
#include "uart_log.h"

BMS_HandleTypeDef bms;

static const float ocv_soc[5]  = {0.0f, 20.0f, 50.0f, 80.0f, 100.0f};
static const float ocv_discharge[5] = {3.00f, 3.55f, 3.75f, 3.95f, 4.20f};
static const float ocv_charge[5]    = {3.10f, 3.65f, 3.85f, 4.05f, 4.20f};

void BMS_Init(void) {
    bms.cell_voltage[0] = 0.0f;
    bms.cell_voltage[1] = 0.0f;
    bms.current[0] = 0.0f;
    bms.current[1] = 0.0f;
    bms.soc = 100.0f;
    bms.state = BMS_STATE_IDLE;
    bms.load_connected = 1;
}

float BMS_VoltageToSoC(float v_cell) {
    if (v_cell <= ocv_discharge[0]) return 0.0f;
    if (v_cell >= ocv_discharge[4]) return 100.0f;
    for (int i = 0; i < 4; i++) {
        if (v_cell >= ocv_discharge[i] && v_cell < ocv_discharge[i+1]) {
            float ratio = (v_cell - ocv_discharge[i])
                        / (ocv_discharge[i+1] - ocv_discharge[i]);
            return ocv_soc[i] + ratio * (ocv_soc[i+1] - ocv_soc[i]);
        }
    }
    return 0.0f;
}

float BMS_SoCToOCV(float soc, uint8_t charging) {
    const float *table = charging ? ocv_charge : ocv_discharge;
    if (soc <= 0.0f) return table[0];
    if (soc >= 100.0f) return table[4];
    for (int i = 0; i < 4; i++) {
        if (soc >= ocv_soc[i] && soc < ocv_soc[i+1]) {
            float ratio = (soc - ocv_soc[i])
                        / (ocv_soc[i+1] - ocv_soc[i]);
            return table[i] + ratio * (table[i+1] - table[i]);
        }
    }
    return table[0];
}

uint8_t BMS_CheckVoltageDrop(void) {
    for (int i = 0; i < BMS_CELL_COUNT; i++) {
        if (bms.cell_voltage[i] < BMS_VOLTAGE_THRESH && bms.cell_voltage[i] > 0.1f) {
            return 1;
        }
    }
    return 0;
}

void BMS_SetRelay(uint8_t on) {
    bms.load_connected = on;
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void BMS_SetLEDs(void) {
    switch (bms.state) {
        case BMS_STATE_CHARGE:
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
            break;
        case BMS_STATE_DISCHARGE:
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
            break;
        case BMS_STATE_FAULT:
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
            break;
        default:
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
            break;
    }
}

void BMS_Update(void) {
    bms.current[0] = ADC_GetCurrent(0);
    bms.current[1] = ADC_GetCurrent(1);
    bms.cell_voltage[0] = ADC_GetCellVoltage(0);
    bms.cell_voltage[1] = ADC_GetCellVoltage(1);

    float avg_voltage = (bms.cell_voltage[0] + bms.cell_voltage[1]) / 2.0f;
    if (avg_voltage > 0.1f) {
        bms.soc = BMS_VoltageToSoC(avg_voltage);
    }

    if (BMS_CheckVoltageDrop()) {
        bms.state = BMS_STATE_FAULT;
        BMS_SetRelay(0);
    } else {
        BMS_SetRelay(1);
        if (bms.soc > 50.0f) {
            bms.state = BMS_STATE_CHARGE;
        } else {
            bms.state = BMS_STATE_DISCHARGE;
        }
    }

    BMS_SetLEDs();
    UART_LogBMSData();
}
