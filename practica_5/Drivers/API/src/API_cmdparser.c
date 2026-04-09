#include "API_cmdparser.h"
#include "API_delay.h"
#include "API_uart.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_uart.h"
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// To call the uart api easier
#define SEND(str) uartSendStringSize((uint8_t *) (str), (uint16_t) (sizeof(str) - 1))

static bool_t     parser_init = false;   //! To check of the init has been called
static cmd_fsm_t  cmd_state;             //! the current state of the FSM
static cmd_result curr_cmd;              //! the current cmd found
static uint8_t    rx_buf[CMD_MAX_LINE];  //! Buffer of the current message
static uint8_t   *rx_buf_p = rx_buf;     //! Pointer to the buffer

/** @brief append value to the buffer while checking for out of bounds and moving the pointer */
static bool_t cmdAppend(uint8_t value);

/** @brief end the rx_buf with \0. */
static bool_t cmdEndStr();

/** @brief reset the buffer. */
static void resetRx();

/** @brief Print the error status. */
static void cmd_err_str(cmd_status_t status);

/** @brief Process the current rx_buf. */
static cmd_result cmdProcessLine();

/** @brief Converts to upper case the rx_buff. */
static void cmd_uppercase(void);

/*--------------- Handler definitions for the commands --------------*/
// TODO: THIS SHOULD BE IN A  DIFFERENT FILE BUT FOR NOW (FOREVER!?) IT WORKS
/** @brief Get the number from the buffer after an '='. */
static int32_t cmdGetNum(char *buffer);

/** @brief Help handler. */
static void cmdHelp(int32_t arg);

/** @brief led on handler. */
static void cmdLedOn(int32_t arg);

/** @brief led off handler. */
static void cmdLedOff(int32_t arg);

/** @brief toggle led handler. */
static void cmdLedToggle(int32_t arg);

/** @brief Print the status of the led. */
static void cmdStatus(int32_t arg);

/** @brief Print the status of the led. */
static void cmdBaudGet(int32_t arg);

static cmd_entry cmds[] = {
    {      "HELP",      cmdHelp},
    {    "LED ON",     cmdLedOn},
    {   "LED OFF",    cmdLedOff},
    {"LED TOGGLE", cmdLedToggle},
    {    "STATUS",    cmdStatus},
    {     "BAUD?",   cmdBaudGet},
    {     "BAUD=",         NULL},
    {        NULL,         NULL}  // THIS NULL IS IMPORTANT TO AVOID GOING OUT OF BOUNDS
};  //! List for the commands (this could be done with a linked list or similar)
/*--------------- Handler definitions for the commands --------------*/

static const char invalid_list[] = {EOL, RETURN_CHAR, NEW_LINE_CHAR, HASHTAG_CHAR, DASH_CHAR, SPACE_CHAR, TAB_CHAR};  //! list for the invalid start chars

void cmdParserInit() {
    cmd_state   = CMD_IDLE;
    parser_init = uartInit();
    cmdPrintHelp();
}

void cmdPrintHelp(void) { cmdHelp(ERROR_CMD_ARG); }

void cmdPoll(void) {
    char   curr_var = EOL;
    bool_t overflow;

    // This should have a return type or an error handler to show the error
    if (!parser_init) return;

    switch (cmd_state) {
        case CMD_IDLE:
            if (uartReceiveCharNonBlocking((uint8_t *) &curr_var) == false) { break; }

            // Test if the first char is correct and handle the error
            bool_t start_condition = true;
            for (uint8_t i = 0; i < sizeof(invalid_list); i++) {
                if (curr_var == invalid_list[i]) {
                    start_condition = false;
                    break;
                }
            }
            if (!start_condition) {
                cmd_state       = CMD_ERROR;
                curr_cmd.status = CMD_ERR_SYNTAX;
                break;
            }

            // If non-null go to receiving and move the pointer to next place
            if ((char) curr_var != EOL) {
                overflow        = !cmdAppend(curr_var);
                cmd_state       = (overflow) ? CMD_ERROR : CMD_RECEIVING;
                curr_cmd.status = (overflow) ? CMD_ERR_OVERFLOW : CMD_OK;
            }
            break;

        case CMD_RECEIVING:
            if (uartReceiveCharNonBlocking((uint8_t *) &curr_var) == false) { break; }

            // End of string
            if ((char) curr_var == RETURN_CHAR || (char) curr_var == NEW_LINE_CHAR) {
                cmdEndStr();
                cmd_state = CMD_PROCESS;
                break;
            }

            // If not end of string the return
            overflow = !cmdAppend(curr_var);
            if (overflow) { cmd_state = CMD_ERROR; }
            curr_cmd.status = (overflow) ? CMD_ERR_OVERFLOW : CMD_OK;

            break;

        case CMD_PROCESS:
            curr_cmd  = cmdProcessLine();
            cmd_state = (curr_cmd.status == CMD_OK) ? CMD_EXEC : CMD_ERROR;
            break;

        case CMD_EXEC:
            curr_cmd.entry.handler(curr_cmd.arg);
            cmd_state = CMD_IDLE;
            resetRx();
            break;

        case CMD_ERROR:
            cmd_state = CMD_IDLE;
            cmd_err_str(curr_cmd.status);
            resetRx();
            break;
    }
}

static bool_t cmdAppend(uint8_t value) {
    if (rx_buf_p - rx_buf >= CMD_MAX_LINE) return false;
    *rx_buf_p = value;
    rx_buf_p++;
    return true;
}

static bool_t cmdEndStr() {
    if (rx_buf_p - rx_buf >= CMD_MAX_LINE) rx_buf_p = rx_buf + CMD_MAX_LINE - 1;
    *rx_buf_p = EOL;
    return true;
}

static void resetRx() {
    rx_buf_p = rx_buf;
    memset(rx_buf, 0, CMD_MAX_LINE);
}

static void cmd_uppercase() {
    for (uint8_t *p = rx_buf; *p != EOL; p++) {
        if (p - rx_buf >= CMD_MAX_LINE) return;
        *p = (uint8_t) toupper(*p);
    }
}

static int32_t cmdGetNum(char *buffer) {
    char *end = NULL;

    // Fail if empty or if the buffer isn't pointing to the separator
    if (buffer == NULL || *buffer != '=') return ERROR_CMD_ARG;
    buffer++;

    // if empty after separator
    if (*buffer == EOL || *buffer == RETURN_CHAR || *buffer == NEW_LINE_CHAR) return ERROR_CMD_ARG;

    int32_t result = strtol(buffer, &end, BASE_GET_BAUD);

    if (*end != EOL && *end != RETURN_CHAR && *end != NEW_LINE_CHAR) return ERROR_CMD_ARG;
    return result;
}

static cmd_result cmdProcessLine() {
    cmd_result res = {
        CMD_ERR_UNKNOWN, {.key = NULL, .handler = NULL},
         ERROR_CMD_ARG
    };
    int32_t arg = ERROR_CMD_ARG;
    char   *eq;

    if (rx_buf[0] == EOL) {
        res.status = CMD_EMPTY_CMD;
        return res;
    }

    if (strnlen((char *) rx_buf, CMD_MAX_LINE) == CMD_MAX_LINE) {
        res.status = CMD_ERR_OVERFLOW;
        return res;
    }

    cmd_uppercase();

    // Get the case where = is used
    eq = strchr((char *) rx_buf, SETTER_CHAR);
    if (eq != NULL) {
        arg = cmdGetNum(eq);
        if (arg == ERROR_CMD_ARG) {
            res.status = CMD_ERR_ARG;
            return res;
        }

        res.arg = arg;
    }

    // Get the cmd
    for (cmd_entry *cmd = cmds; cmd->key != NULL; cmd++) {
        if (strncmp((char *) rx_buf, cmd->key, CMD_MAX_LINE) == 0) {
            res.status = CMD_OK;
            res.entry  = *cmd;
            res.arg    = arg;
            return res;
        }
    }

    // Unknown command
    res.status = CMD_ERR_UNKNOWN;
    return res;
}

static void cmd_err_str(cmd_status_t status) {
    const char *err_str[] = {
        [CMD_OK]           = "OK",
        [CMD_ERR_OVERFLOW] = "OVERFLOW",
        [CMD_ERR_SYNTAX]   = "SYNTAX",
        [CMD_ERR_UNKNOWN]  = "UNKNOWN CMD",
        [CMD_ERR_ARG]      = "BAD ARG",
        [CMD_EMPTY_CMD]    = "EMPTY CMD",
    };

    char     buf[HANDLER_BUFFERS_LEN];
    uint16_t len = (uint16_t) snprintf(buf, sizeof(buf), "[ERR] %s\r\n", err_str[status]);
    uartSendStringSize((uint8_t *) buf, len);
}

/*--------------- Handler for the commands --------------*/
// TODO: THIS SHOULD BE IN A  DIFFERENT FILE BUT FOR NOW (FOREVER!?) IT WORKS
static void cmdHelp(int32_t arg) {
    SEND("Available commands:\r\n");
    SEND("  HELP        -> Show this help message\r\n");
    SEND("  LED ON      -> Turn the LED on\r\n");
    SEND("  LED OFF     -> Turn the LED off\r\n");
    SEND("  LED TOGGLE  -> Toggle the LED state\r\n");
    SEND("  STATUS      -> Print the current LED state\r\n");
    SEND("  BAUD?       -> Print the current UART baud rate\r\n");
    SEND("  BAUD=<n>    -> Set the UART baud rate to <n>\r\n");
}

static void cmdLedOn(int32_t arg) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
    SEND("LED turned on\r\n");
}

static void cmdLedOff(int32_t arg) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    SEND("LED turned off\r\n");
}

static void cmdLedToggle(int32_t arg) {
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    SEND("LED toggled\r\n");
}

static void cmdStatus(int32_t arg) {
    GPIO_PinState st = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5);
    char          str[HANDLER_BUFFERS_LEN];
    uint16_t      len = (uint16_t) snprintf(str, sizeof(str), "LED is %s\r\n", (st == GPIO_PIN_RESET) ? "off" : "on");
    uartSendStringSize((uint8_t *) str, len);
}

static void cmdBaudGet(int32_t arg) {
    char     str[HANDLER_BUFFERS_LEN];
    uint16_t len = (uint16_t) snprintf(str, sizeof(str), "[BAUD] %lu\r\n", uartGetBaud());
    uartSendStringSize((uint8_t *) str, len);
}
