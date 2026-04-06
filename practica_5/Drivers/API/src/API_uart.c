#include "API_uart.h"
#include "stm32f4xx_hal_uart.h"
#include <string.h> //! for the strlen

static UART_HandleTypeDef uart_handle; //! Handler for the HAL uart comms
static bool_t uart_initialized =
    false; //! to check of the uart has already been initialized

bool_t uartInit() {
  // Config string (it's weird and has to be configured if uart_handle is
  // changed but it's better than using an printf with the raw values)
  char config[] = API_UART_INSTANCE_STR
      "|" MACRO_STR(API_UART_BUADRATE) "|" API_UART_WL_STR "|" API_UART_SB_STR
                                       "|" API_UART_PARITY_STR
                                       "|" API_UART_MODE_STR
                                       "|" API_UART_HWCTL_STR
                                       "|" API_UART_OS_STR;

  uart_handle = (UART_HandleTypeDef){
      .Instance = API_UART_INSTANCE,
      .Init.BaudRate = API_UART_BUADRATE,
      .Init.WordLength = API_UART_WL,
      .Init.StopBits = API_UART_SB,
      .Init.Parity = API_UART_PARITY,
      .Init.Mode = API_UART_MODE,
      .Init.HwFlowCtl = API_UART_HWCTL,
      .Init.OverSampling = API_UART_OS,
  };

  // Init the UART and send the config on success
  if (HAL_UART_Init(&uart_handle) != HAL_OK)
    return false;

  HAL_UART_Transmit(&uart_handle, (uint8_t *)config, strlen(config),
                    HAL_MAX_DELAY);
  return true;
}

void uartSendString(uint8_t *pstring) {
  size_t str_size = 0;
  char *str_cp = (char *)pstring;

  // Check for NULL pointer
  if (pstring == NULL)
    return;

  // If str_cp doesn't end with \0 this will read out of bounds.
  // Using this instead of strlen() is to make clear the issue without the \0.
  while (*str_cp++ != '\0' || str_size >= API_UART_MAX_SIZE)
    str_size++;

  HAL_UART_Transmit(&uart_handle, pstring, str_size, HAL_MAX_DELAY);
}

void uartSendStringSize(uint8_t *pstring, uint16_t size) {
  // This should be separated to use an error handler
  if (pstring == NULL || uart_initialized == false || size >= API_UART_MAX_SIZE)
    return;

  // check if size = actual size (also checks if null-terminated)
  if (strnlen((char *)pstring, size) != size)
    return;

  HAL_UART_Transmit(&uart_handle, pstring, size, HAL_MAX_DELAY);
}
