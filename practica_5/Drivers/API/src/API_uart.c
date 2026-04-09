#include "API_uart.h"
#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_uart.h"
#include <string.h>                                  //! for the strnlen

static UART_HandleTypeDef uart_handle;               //! Handler for the HAL uart comms
static bool_t             uart_initialized = false;  //! Check the uart has been init

bool_t uartInit() {
    // Config string (it's weird and has to be configured if uart_handle is
    // changed but it's better than using an printf with the raw values)
    char config[] =
        API_UART_INSTANCE_STR "|" MACRO_STR(API_UART_BUADRATE) "|" API_UART_WL_STR "|" API_UART_SB_STR "|" API_UART_PARITY_STR "|" API_UART_MODE_STR "|" API_UART_HWCTL_STR "|" API_UART_OS_STR "\r\n";

    uart_handle = (UART_HandleTypeDef) {
        .Instance          = API_UART_INSTANCE,
        .Init.BaudRate     = API_UART_BUADRATE,
        .Init.WordLength   = API_UART_WL,
        .Init.StopBits     = API_UART_SB,
        .Init.Parity       = API_UART_PARITY,
        .Init.Mode         = API_UART_MODE,
        .Init.HwFlowCtl    = API_UART_HWCTL,
        .Init.OverSampling = API_UART_OS,
    };

    // Init the UART and send the config on success
    if (HAL_UART_Init(&uart_handle) != HAL_OK) return false;

    HAL_UART_Transmit(&uart_handle, (uint8_t *) config, strlen(config), HAL_MAX_DELAY);
    uart_initialized = true;
    return true;
}

void uartSendString(uint8_t *pstring) {
    size_t str_size = 0;
    char  *str_cp   = (char *) pstring;

    // Check for NULL pointer
    if (pstring == NULL) return;

    // If str_cp doesn't end with \0 this will read out of bounds.
    // Using this instead of strlen() is to make clear the issue without the \0.
    while (*str_cp++ != '\0' && str_size < API_UART_MAX_SIZE) str_size++;

    HAL_UART_Transmit(&uart_handle, pstring, str_size, HAL_MAX_DELAY);
}

void uartSendStringSize(uint8_t *pstring, uint16_t size) {
    // This should be separated to use an error handler
    if (pstring == NULL || uart_initialized == false || size >= API_UART_MAX_SIZE) return;

    // check if size = actual size (also checks if null-terminated)
    if (strnlen((char *) pstring, size) != size) return;

    HAL_UART_Transmit(&uart_handle, pstring, size, HAL_MAX_DELAY);
}

bool_t uartReceiveStringSize(uint8_t *pstring, uint16_t size) {
    if (pstring == NULL || uart_initialized == false || size >= API_UART_MAX_SIZE) return false;

    HAL_StatusTypeDef st_comm = HAL_UART_Receive(&uart_handle, pstring, size, HAL_MAX_DELAY);
    return (st_comm == HAL_OK) ? true : false;
}

/*----------Functions implemented for ex. 2---------------*/
bool_t uartReceiveCharNonBlocking(uint8_t *pstring) {
    if (pstring == NULL || uart_initialized == false) return false;

    HAL_StatusTypeDef st_comm = HAL_UART_Receive(&uart_handle, pstring, API_UART_ONE_BIT_RECEIVE, API_UART_NON_BLOCKING_TIMEOUT);
    return (st_comm == HAL_OK) ? true : false;
}

int32_t uartGetBaud() { return uart_handle.Init.BaudRate; }
