#ifndef API_ADC__H_
#define API_ADC__H_
#include <stdint.h>
#include <stdbool.h>

#define ADC_RESOLUTION 4095U  //! 12-bit ADC max value
#define ADC_TIMEOUT_MS 100U //! Timeout for the read

/**
 * @brief Init ADC peripheral.
 * @retval true on success
 */
bool adc_port_init(void);

/**
 * @brief Read raw ADC value (blocking, single conversion).
 * @param[out] value Raw ADC reading
 * @retval true on success
 */
bool adc_port_read(uint16_t *value);
#endif /* API_ADC_H_ */
