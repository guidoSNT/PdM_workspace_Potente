#include "AD9833_port.h"
#include "stm32f4xx_hal_def.h"

extern SPI_HandleTypeDef hspi1;

ad9833_spi_st_t spi_init(void) {
  if (hspi1.State != HAL_SPI_STATE_READY)
    return AD_SPI_NOT_READY;

  HAL_GPIO_WritePin(PORT_GPIO, PIN1_GPIO, GPIO_PIN_RESET);

  return AD_SPI_OK;
}

ad9833_spi_st_t write_word(uint16_t word) {
  uint8_t tx[SIZE_OF_WORD];
  tx[0] = (uint8_t)(word >> SHIFT_MSB);
  tx[1] = (uint8_t)(word & MASK_LSB);

  ad9833_spi_st_t status = AD_SPI_FAIL;

  switch (
      HAL_SPI_Transmit(&hspi1, (uint8_t *)&tx, SIZE_OF_WORD, COMM_TIMEOUT_MS)) {
  case HAL_OK:
    status = AD_SPI_OK;
    break;

  case HAL_TIMEOUT:
    status = AD_SPI_TIMEOUT;
    break;

  case HAL_ERROR:
    status = AD_SPI_FAIL;
    break;

  default:
    status = AD_SPI_FAIL;
  }
  return status;
}
