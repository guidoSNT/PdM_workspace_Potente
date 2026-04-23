/**
 * @file API_debounce_port.c
 * @brief Source file to use STM32 HAL with the debounce fsm.
 */
#include "API_debounce_port.h"
#include "stm32f4xx_hal.h"

typedef struct {
    GPIO_TypeDef *port;
    uint16_t      pin;
} gpio_map_t;  //! Struct to map the HAL buttons

/** @brief All ported buttons are in this list */
static const gpio_map_t button_map[] = {
    {GPIOC, GPIO_PIN_13},
    {GPIOB, GPIO_PIN_13},
    {GPIOB, GPIO_PIN_14},
    {GPIOB, GPIO_PIN_15},
    {GPIOB,  GPIO_PIN_1},
    {GPIOB,  GPIO_PIN_2},
};

#define BUTTON_MAP_SIZE (sizeof(button_map) / sizeof(button_map[0]))

button_pin_state_t debounce_port_read_pin(button_id_t id) {
    if (id >= BUTTON_MAP_SIZE) return BTN_PIN_HIGH;

    GPIO_PinState st = HAL_GPIO_ReadPin(button_map[id].port, button_map[id].pin);

    return (st == GPIO_PIN_RESET) ? BTN_PIN_LOW : BTN_PIN_HIGH;
}
