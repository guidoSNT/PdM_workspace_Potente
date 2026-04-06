#include "API_uart.h"

UART_HandleTypeDef uart_handle;

bool_t uartInit() {
  // Config string (it's weird and has to be configured if uart_handle is
  // changed but it's better than using an printf with the raw values)
  char config[] = API_UART_INSTANCE_STR
                  "|" MACRO_STR(API_UART_BUADRATE)
                  "|" API_UART_WL_STR
                  "|" API_UART_SB_STR
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

  HAL_UART_Transmit(&uart_handle, (uint8_t *)config, sizeof(config),
                    HAL_MAX_DELAY);
  return true;
}
