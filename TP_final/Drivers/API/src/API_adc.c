#include "API_adc.h"
#include "stm32f4xx_hal.h"

extern ADC_HandleTypeDef hadc1;

bool adc_port_init(void) {
    return (hadc1.State != HAL_ADC_STATE_RESET);
}

bool adc_port_read(uint16_t *value) {
    if (value == NULL) return false;

    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, ADC_TIMEOUT_MS) != HAL_OK) {
        HAL_ADC_Stop(&hadc1);
        return false;
    }

    *value = (uint16_t) HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return true;
}
