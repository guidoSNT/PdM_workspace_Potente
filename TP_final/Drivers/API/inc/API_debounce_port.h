#ifndef API_DEBOUNCE_PORT_H_
#define API_DEBOUNCE_PORT_H_

#include <stdbool.h>
#include <stdint.h>

typedef uint8_t button_id_t; //! up to 256 buttons
typedef enum {
  BTN_PIN_HIGH,
  BTN_PIN_LOW
} button_pin_state_t; //! generic enum for pressed
                      //
/**
 * @brief Read raw pin state for given button id.
 *
 * @param[in] id Button id
 *
 * @retval BTN_PIN_HIGH or BTN_PIN_LOW
 */
button_pin_state_t debounce_port_read_pin(button_id_t id);
#endif /* API_DEBOUNCE_PORT_H_ */
