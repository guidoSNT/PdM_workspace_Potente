#ifndef API_INC_API_DEBOUNCE_H_
#define API_INC_API_DEBOUNCE_H_
#include "API_debounce_port.h"
#include "API_delay.h"
#include "stm32f4xx.h"

#define INIT_DELAY_TIME 0        //! Delay to init the delays
#define TRANSITORY_STATE_TIME 40 //! Transitory wait time

typedef enum {
  BUTTON_UP,
  BUTTON_FALLING,
  BUTTON_DOWN,
  BUTTON_RAISING,
} debounceState_t;

typedef struct {
  debounceState_t cur_state; //! Current state of the fsm
  button_id_t btn_id;        //! Button from the map list
  delay_t dly;               //! Delay to be used
  bool_t pressed;            //! True when pressed
} button_t;

/**
 * @brief Reads if the key has been pressed.
 *
 * @input[in,out] dev pointer to a button structure
 *
 * @retval True if pressed else false.
 */
bool_t readKey(button_t *dev);

/**
 * @brief Initialize the debounce FSM.
 *
 * @input[in,out] dev pointer to a button structure
 *
 * @retval None.
 */
void debounceFSM_init(button_t *dev);

/**
 * @brief runs one step of the debounceFSM.
 *
 * @input[in,out] dev pointer to a button structure
 *
 * @retval None.
 */
void debounceFSM_update(button_t *dev);

#endif /* API_INC_API_DEBOUNCE_H_ */
