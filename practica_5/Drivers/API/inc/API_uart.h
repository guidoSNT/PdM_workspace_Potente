#ifndef API_INC_API_UART_H_
#define API_INC_API_UART_H_
#include "API_delay.h" // for the bool_t
#include "stm32f4xx_hal.h"

#define API_UART_MAX_SIZE 256 //! Max size for transmisions

#define API_UART_INSTANCE USART2 //! USART instance to use
#define API_UART_INSTANCE_STR "USART2"
#define API_UART_BUADRATE 115200       //! Buadrate in bd of the USART
#define API_UART_WL UART_WORDLENGTH_8B //! Wordlength of 8 bits
#define API_UART_WL_STR "UART_WORDLENGTH_8B"
#define API_UART_SB UART_STOPBITS_1 //! one stop bit
#define API_UART_SB_STR "UART_STOPBITS_1"
#define API_UART_PARITY UART_PARITY_NONE //! No parity bit
#define API_UART_PARITY_STR "UART_PARITY_NONE"
#define API_UART_MODE UART_MODE_TX_RX //! Mode to send and receive
#define API_UART_MODE_STR "UART_MODE_TX_RX"
#define API_UART_HWCTL UART_HWCONTROL_NONE //! No control flow
#define API_UART_HWCTL_STR "UART_HWCONTROL_NONE"
#define API_UART_OS UART_OVERSAMPLING_16 //! Oversampling enabled
#define API_UART_OS_STR "UART_OVERSAMPLING_16"

// This macros are needed to convert an macro value to string
// e.g.: (MACRO_STR(BAUDRATE) -> _MACRO_STR(115200) -> "115200")
#define MACRO_STR(M) #M
#define _MACRO_STR(M) MACRO_STR(M)

/**
 * @brief Initializes the UART and sends the config on success
 *
 * @retval true on success else false
 */
bool_t uartInit();

/**
 * @brief Sends NULL terminated string through UART (unsecure).
 *
 * Sends the NULL terminated string without checking for size. This means that
 * none NULL terminated strings will do an out-of-bounds read (undefined
 * behaviour).
 *
 * @param[in] pstring Pointer to the NULL terminated string to send.
 *
 * @retval None
 */
void uartSendString(uint8_t *pstring);

/**
 * @brief Sends NULL terminated string through UART.
 *
 * @param[in] pstring Pointer to the NULL terminated string to send.
 * @param[in] size Size of the string to send without the NULL.
 *
 * @retval None
 */
void uartSendStringSize(uint8_t *pstring, uint16_t size);
void uartReceiveStringSize(uint8_t *pstring, uint16_t size);
#endif /* API_INC_API_UART_H_ */
