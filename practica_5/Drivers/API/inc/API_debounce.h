#ifndef API_INC_API_DEBOUNCE_H_
#define API_INC_API_DEBOUNCE_H_
#include "API_delay.h"

#define INIT_DELAY_TIME 0 //! Delay to init the internal delay in debounce API
#define TRANSITORY_STATE_TIME 40 //! Transitory wait time 

/**
 * @brief Reads if the key has been pressed.
 *
 * @retval True if pressed else false.
 */
bool_t readKey();

/**
 * @brief Initialize the debounce FSM.
 *
 * @retval None.
 */
void debounceFSM_init();

/**
 * @brief runs one step of the debounceFSM.
 *
 * @retval None.
 */
void debounceFSM_update();

#endif /* API_INC_API_DEBOUNCE_H_ */
