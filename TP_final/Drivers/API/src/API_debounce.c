#include "API_debounce.h"
#include "API_delay.h"

bool_t readKey(button_t *dev) {
  if (dev == NULL)
    return false;

  bool_t prev_pressed = dev->pressed;
  dev->pressed = false;
  return prev_pressed;
}

void debounceFSM_init(button_t *dev) {
  if (dev == NULL)
    return;

  dev->cur_state = BUTTON_UP;
  delayInit(&dev->dly, INIT_DELAY_TIME);
  dev->pressed       = false;
}

void debounceFSM_update(button_t *dev) {
  if (dev == NULL)
    return;

  button_pin_state_t btn_state = debounce_port_read_pin(dev->btn_id);

  switch (dev->cur_state) {
  case BUTTON_UP:
    if (btn_state == BTN_PIN_LOW) {
      if (delayIsRunning(&dev->dly))
        delayStop(&dev->dly);

      delayWrite(&dev->dly, TRANSITORY_STATE_TIME);
      delayRead(&dev->dly);
      dev->cur_state = BUTTON_FALLING;
    }
    break;

  case BUTTON_FALLING:
    if (delayRead(&dev->dly)) {
      dev->cur_state = (btn_state == BTN_PIN_LOW) ? BUTTON_DOWN : BUTTON_UP;
      if (dev->cur_state == BUTTON_DOWN)
        dev->pressed = true;
    }
    break;

  case BUTTON_DOWN:
    if (btn_state == BTN_PIN_HIGH) {
      dev->cur_state = BUTTON_RAISING;
      if (delayIsRunning(&dev->dly))
        delayStop(&dev->dly);

      delayWrite(&dev->dly, TRANSITORY_STATE_TIME);
      delayRead(&dev->dly);
    }
    break;

  case BUTTON_RAISING:
    if (delayRead(&dev->dly)) {
      dev->cur_state = (btn_state == BTN_PIN_HIGH) ? BUTTON_UP : BUTTON_DOWN;
    }
    break;

  default:
    // Here an error handler should be used
    break;
  }
}
