#ifndef __API_WG__
#define __API_WG__

#include <stdbool.h>

/*------- Constants ----------*/
#define AMOUNT_BUTTONS     6U  //! Amount of buttons used
#define NO_PRESSED_BTNS    0U  //! result when no presses
#define MAX_DIGITS_FREQ    9   //! Max amount of digits in (CAN'T HAVE THE U)
#define INIT_FMCLK_MHZ     25U
#define INIT_FREQ_DHZ      10000U
#define INIT_PHASE_DHZ     0U
#define ONE_OFFSET         1U
#define ZERO_CHAR          '0'
#define NINE_CHAR          '9'
#define DOT_CHAR           '.'
#define DECIMAL_BASE       10U
#define ADC_PHASE_THRESHOLD 100U

/*------- Buttons ----------*/
#define BTN_LOAD           1 << 0  //! Loads current config
#define BTN_LEFT           1 << 1  //! Moves to the left
#define BTN_RIGHT          1 << 2  //! Moves to the right
#define BTN_UP             1 << 3  //! Increases the digit
#define BTN_DOWN           1 << 4  //! Decreases the digit
#define BTN_CHANGE         1 << 5  //! Change form
#define BTN_PHASE          1 << 6  //! Phase change

/*------- ANSI escape codes ----------*/
#define ANSI_HIGHLIGHT     "\033[5;7m"    //! Contrast and blinking
#define ANSI_HIGHLIGHT_OFF "\033[25;27m"  //! Contrast and blinking off
#define ANSI_RESET         "\033[0m"      //! Removes all format
#define ANSI_CLEAR         "\033[2J"      //! Clear screen
#define ANSI_HOME          "\033[H"       //! Go to first element
#define ANSI_HIDE_CURSOR   "\033[?25l"    //! hide the cursor

/*------- Useful macros ----------*/
// The next 2 macros work as way to print a define number useful to set the len
// inside printf statements
#define MACRO_TO_STR(x)    _MACRO_TO_STR(x)
#define _MACRO_TO_STR(x)   #x

/*------- To make the string ----------*/
#define END_FREQ_STR       " Hz\r\n"

typedef enum {
    WG_IDLE,      //! When it has nothing
    WG_PRC_IN,    //! Process the inputs
    WG_CNG_UART,  //! Send with UART
    WG_EXEC,      //! Exec changes
    WG_ERROR      //! Error state
} wg_state_t;

/**
 * @brief inits the fsm for the wave generator.
 */
void wg_fsm_init();

/**
 * @brief updates the state machine.
 *
 * @retval true on success else false
 */
bool wg_fsm_update();
#endif  // __API_WG__
