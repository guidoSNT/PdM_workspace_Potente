#include "API_debounce.h"
#include "API_delay.h"
#include "stm32f4xx.h"

typedef enum {
  BUTTON_UP,
  BUTTON_FALLING,
  BUTTON_DOWN,
  BUTTON_RAISING,
} debounceState_t;

static debounceState_t cur_state;
static delay_t dly;
static bool_t pressed;

static void buttonPressed() {
  // Here it would use the buttonpresed if not for the main usage for the pwm:
  // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}

static void buttonReleased() {
  // Here it would use the buttonreleased if not for the main usage for the pwm:
  // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
}

bool_t readKey() {
  bool_t prev_pressed = pressed;
  pressed = false;
  return prev_pressed;
}

void debounceFSM_init() {
  cur_state = BUTTON_UP;
  delayInit(&dly, INIT_DELAY_TIME);
}

void debounceFSM_update() {
  GPIO_PinState btn_state = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);

  switch (cur_state) {
  case BUTTON_UP:
    if (btn_state == GPIO_PIN_RESET) {
      if (delayIsRunning(&dly))
        delayStop(&dly);

      delayWrite(&dly, TRANSITORY_STATE_TIME);
      delayRead(&dly);
      cur_state = BUTTON_FALLING;
    }
    break;

  case BUTTON_FALLING:
    if (delayRead(&dly)) {
      cur_state = (btn_state == GPIO_PIN_RESET) ? BUTTON_DOWN : BUTTON_UP;
      if (cur_state == BUTTON_DOWN) {
        pressed = true;
        buttonPressed();
      }
    }
    break;

  case BUTTON_DOWN:
    if (btn_state == GPIO_PIN_SET) {
      cur_state = BUTTON_RAISING;
      if (delayIsRunning(&dly))
        delayStop(&dly);

      delayWrite(&dly, TRANSITORY_STATE_TIME);
      delayRead(&dly);
    }
    break;

  case BUTTON_RAISING:
    if (delayRead(&dly)) {
      cur_state = (btn_state == GPIO_PIN_SET) ? BUTTON_UP : BUTTON_DOWN;
      if (cur_state == BUTTON_UP)
        buttonReleased();
    }
    break;

  default:
    // Here an error handler should be used
    break;
  }
}
