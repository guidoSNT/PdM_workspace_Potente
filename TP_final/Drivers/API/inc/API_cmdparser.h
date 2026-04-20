#ifndef API_INC_API_CMDPARSER_H_
#define API_INC_API_CMDPARSER_H_
#include <stdint.h>

#define CMD_MAX_LINE        64             //! Includes the \0
#define CMD_MAX_TOKENS      3              //! COMANDO + maximo de argumentos
#define ERROR_CMD_ARG       -1             //! Return for the parser when the number getter fails
#define HANDLER_BUFFERS_LEN 32             //! For the local buffers
#define BASE_GET_BAUD       10
#define SETTER_CHAR         '='            //! When setting the buad, the separator is this

typedef enum {
    CMD_OK = 0,                            //! On success
    CMD_ERR_OVERFLOW,                      //! Overflow in the command string
    CMD_ERR_SYNTAX,                        //! Wrong syntax
    CMD_ERR_UNKNOWN,                       //! Unknown command
    CMD_ERR_ARG,                           //! Wrong argument
    CMD_EMPTY_CMD                          //! If buffer is empty
} cmd_status_t;

typedef enum {
    CMD_IDLE = 0,                          //! Waits none NULL char
    CMD_RECEIVING,                         //! Saves char up to \r, \n or CMD_ERROR
    CMD_PROCESS,                           //! Process buffer
    CMD_EXEC,                              //! Executes action
    CMD_ERROR                              //! Print error msg and goes back to CMD_IDLE
} cmd_fsm_t;

typedef void (*cmd_handler)(int32_t arg);  //! Handler type for all commands

typedef struct {
    const char* key;                       //! Cmd string to call the command
    cmd_handler handler;                   //! Function for the command
} cmd_entry;

typedef struct {
    cmd_status_t status;                   //! Status of the cmd process
    cmd_entry    entry;                    //! Function for the command
    int32_t      arg;                      //! Argument for the handler
} cmd_result;

/**
 * @brief Init the command parser
 *
 * @retval None
 */
void cmdParserInit(void);

/**
 * @brief Parser FSM.
 *
 * Should be called recursively and can process up to 16 bytes per call.
 *
 * @retval None
 */
void cmdPoll(void);

/**
 * @brief Prints the list of available commands.
 *
 * @retval None
 */
void cmdPrintHelp(void);
#endif
