#ifndef __AD9833_PORT__
#define __AD9833_PORT__
#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

#define COMM_TIMEOUT_MS 1000 //! Timeout on comm
#define SIZE_OF_WORD 1       //! Size of the word
#define PORT_GPIO GPIOA      //! GPIO port for CS
#define PIN1_GPIO GPIO_PIN_5 //! Pin to the first device
#define SHIFT_MSB 8          //! Shift to get the MSB
#define MASK_LSB 0xFF        //! MASK to get the LSB

typedef enum {
  AD_SPI_OK,        //! If the write was successfull
  AD_SPI_NOT_READY, //! If not ready
  AD_SPI_FAIL,      //! If failed
  AD_SPI_TIMEOUT    //! If failed
} ad9833_spi_st_t;

/**
 * @brief writes a 16-bit word to the device.
 *
 * @param[in] word 16-bit word to send.
 *
 * @retval AD_SPI_OK on success else depends on the error code.
 */
ad9833_spi_st_t write_word(uint16_t word);

/**
 * @brief Inits the spi comms.
 *
 * @retval AD_SPI_OK on success else depends on the error code.
 */
ad9833_spi_st_t spi_init(void);
#endif // __AD9833_PORT__
