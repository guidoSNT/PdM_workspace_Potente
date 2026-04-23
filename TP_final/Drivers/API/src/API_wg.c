/**
 * @file API_wg.c
 * @brief Wave generator fsm interface source file.
 *
 * Using 6 debounced buttons, ADC phase input, and AD9833
 * control through a fsm. Outputs status via UART
 * with ANSI escape codes.
 */
#include "API_wg.h"
#include "AD9833_driver.h"
#include "API_debounce.h"
#include "API_uart.h"
#include "API_adc.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint8_t     btns;                                    //! Current state of the buttons
static uint8_t     curr_pos;                                //! Current digit position
static uint16_t    curr_phase;                              //! Current phase
static ad9833_wf_t curr_form;                               //! Current wave form
static char        freq_str[MAX_DIGITS_FREQ + ONE_OFFSET];  //! Str to save the freq
static bool        initialized = false;                     //! To check if initialized
static wg_state_t  wg_st;                                   //! State of the fsm
static button_t    inputs[AMOUNT_BUTTONS];                  //! All buttons pressed

// This has the names for the different waves
// (there is probably a better solution  with stringify but it works )
static const char *wf_names[] = {
    [SINE]       = "SINE\r\n",
    [TRIANGULAR] = "TRIANGULAR\r\n",
    [SQUARE]     = "SQUARE\r\n",
    [SQUARE_DIV] = "SQUARE_DIV\r\n",
};

#define WF_NAMES_COUNT (sizeof(wf_names) / sizeof(wf_names[0]))

/*-------------------------- PRIVATE FUNCTION DECLARATIONS --------------------------*/
/**
 * @brief Formats wave generator state into buffer with ANSI escape codes.
 *
 * Output format:
 *   XXXXXXXX.X Hz
 *   X.XXX rads
 *   WAVEFORM_NAME
 *
 * @param[in]  form     Waveform type
 * @param[in]  phase    Phase in mRads
 * @param[in]  pos      Digit position to blink
 * @param[out] buff     Output buffer
 * @param[in]  buff_len Buffer length
 *
 * @retval Number of bytes written, 0 on error
 */
static uint16_t wg_format_display(ad9833_wf_t form, uint16_t phase, uint8_t pos, char *buff, uint16_t buff_len);

/**
 * @brief Process the btns inputs and change local variables
 *
 * @retval True on success else false
 */
static bool process_input();

/**
 * @brief Returns which buttons are pressed. Each bit represents one button
 * pressed.
 *
 * @retval value with the pressed buttons
 */
static void input_getter();

/**
 * @brief Converts a uint32_t frequency into freq_str.
 *
 * @param[in] freq Frecuency in dHz
 */
void freq_to_digit(uint32_t freq);

/**
 * @brief Converts freq_str into a uint32_t.
 */
uint32_t digit_to_freq(void);

/**
 * @brief Increases a specific digit.
 *
 * @param[in] pos position of the digit to change
 */
void increase_digit(uint8_t pos);

/**
 * @brief Reduces a specific digit.
 * @param[in] pos position of the digit to change
 */
void reduce_digit(uint8_t pos);

/**
 * @brief Check if more than one bit is set.
 *
 * @retval true if only one button
 */
static inline bool check_bits() {
    if (btns == 0) return false;

    return (btns & (btns - ONE_OFFSET)) == 0;
}

/*-------------------------- END PRIVATE FUNCTION DECLARATIONS --------------------------*/

void wg_fsm_init() {
    wg_st = WG_IDLE;

    // Init all the buttons
    for (uint8_t i = 0; i < AMOUNT_BUTTONS; i++) {
        inputs[i].btn_id = i;
        debounceFSM_init(&inputs[i]);
    }

    // Init both interfaces
    uartInit();
    adc_port_init();

    // Setup the ad9833
    if (ad9833_init(INIT_FMCLK_MHZ) != AD_OK) return;

    if (ad9833_set_freq(INIT_FREQ_DHZ, REG_0) != AD_OK) return;

    if (ad9833_set_phase(INIT_PHASE_DHZ, REG_0) != AD_OK) return;

    if (ad9833_set_form(SINE) != AD_OK) return;

    // Clearing the screen allows that the string isn't imposed in the rest of the terminal
    uartSendStringSize((uint8_t *) ANSI_CLEAR, sizeof(ANSI_CLEAR));
    memset(freq_str, EOL, sizeof(freq_str));

    freq_to_digit(INIT_FREQ_DHZ);
    curr_phase  = INIT_PHASE_DHZ;
    curr_form   = SINE;
    initialized = true;
    btns        = 0;
}

bool wg_fsm_update() {
    char buff[FSM_BUFFER_LEN];
    if (initialized != true) return false;

    switch (wg_st) {
        case WG_IDLE:
            input_getter();
            if (btns != NO_PRESSED_BTNS) wg_st = WG_PRC_IN;
            break;

        case WG_PRC_IN:
            wg_st = (check_bits()) ? WG_EXEC : WG_ERROR;
            break;

        case WG_EXEC:
            wg_st = (process_input()) ? WG_CNG_UART : WG_ERROR;
            break;

        case WG_CNG_UART:
            memset(buff, EOL, sizeof(buff));
            if (wg_format_display(curr_form, curr_phase, curr_pos, buff, sizeof(buff)) == 0) {
                wg_st = WG_ERROR;
                break;
            }

            wg_st = WG_IDLE;
            uartSendStringSize((uint8_t *) buff, strnlen(buff, sizeof(buff)));
            break;

        case WG_ERROR:
            // This simple error transmit is because the system may loop the error and remove it from screen.
            // This means than long Error messages may be lost
            uartSendStringSize((uint8_t *) ERROR_STR, strlen(ERROR_STR));
            wg_st = WG_IDLE;
            break;
    }
    return true;
}

/*-------------------------- PRIVATE FUNCTION --------------------------*/

// The idea was to reduce the amount of snprintf (ideally it shouldn't use any), thas why the usage of man strlcat
static uint16_t wg_format_display(ad9833_wf_t form, uint16_t phase, uint8_t pos, char *buff, uint16_t buff_len) {
    if (buff == NULL || buff_len == 0 || pos > MAX_DIGITS_FREQ) return 0;

    // Get the freq string
    uint16_t    freq_len  = strnlen(freq_str, MAX_DIGITS_FREQ) - ONE_OFFSET;
    uint8_t     blink_idx = (pos == 0) ? UINT8_MAX : freq_len - pos;  // to point the correct char
    char       *p         = buff;
    const char *end       = buff + buff_len - ONE_OFFSET;
    const char *wf_str    = ((uint8_t) form < WF_NAMES_COUNT && wf_names[form] != NULL) ? wf_names[form] : "UNKNOWN";

    // To remove anything from the terminal, go to the beginning and remove the cursor, we add the following commands at the start
    strlcat(p, ANSI_CLEAR ANSI_HOME ANSI_HIDE_CURSOR, end - p);
    if (p + strlen(ANSI_CLEAR ANSI_HOME ANSI_HIDE_CURSOR) >= end) return 0;
    p += strlen(ANSI_CLEAR ANSI_HOME ANSI_HIDE_CURSOR);

    // Find the digit and add the highlight
    for (uint16_t i = 0; i < freq_len; i++) {
        if (i == blink_idx && blink_idx >= 0) {
            int16_t written = snprintf(p, (size_t) (end - p), ANSI_HIGHLIGHT "%c" ANSI_HIGHLIGHT_OFF, freq_str[i]);
            if (written < 0 || p + written >= end) return 0;
            p += written;
        } else {
            if (p >= end) return 0;
            *p++ = freq_str[i];
        }
    }

    // Decimal point
    if (p >= end) return 0;
    *p++ = DOT_CHAR;

    // Decimal digit
    if (pos == 0) {
        int16_t written = snprintf(p, (size_t) (end - p), ANSI_HIGHLIGHT "%c" ANSI_HIGHLIGHT_OFF, freq_str[MAX_DIGITS_FREQ - ONE_OFFSET]);
        if (written < 0 || p + written >= end) return 0;
        p += written;
    } else {
        if (p >= end) return 0;
        *p++ = freq_str[MAX_DIGITS_FREQ - ONE_OFFSET];
    }

    // Add end of freq
    strlcat(p, END_FREQ_STR, end - p);
    if (p + strlen(END_FREQ_STR) >= end) return 0;
    p               += strlen(END_FREQ_STR);

    // Print the phase
    int16_t written  = snprintf(p, (size_t) (end - p), "%u.%03u rads\r\n", phase / AD9833_FREQ_SCALE_MRAD_RAD, phase % AD9833_FREQ_SCALE_MRAD_RAD);
    if (written < 0 || p + written >= end) return 0;
    p += written;

    // Append the wave form
    strlcat(p, wf_str, end - p);
    if (p + strlen(wf_str) >= end) return 0;
    p += strlen(wf_str);

    // Append the Reset
    strlcat(p, ANSI_RESET, end - p);
    if (p + strlen(ANSI_RESET) >= end) return 0;
    p  += strlen(ANSI_RESET);
    *p  = EOL;

    return (uint16_t) (p - buff);
}

static void input_getter() {
    uint8_t  state = 0;
    uint16_t adc_val;
    btns = 0;

    if (adc_port_read(&adc_val)) {
        uint16_t new_phase = ((uint32_t) adc_val * AD9833_MAX_PHASE_MRADS) / ADC_RESOLUTION;
        uint16_t diff      = (new_phase > curr_phase) ? (new_phase - curr_phase) : (curr_phase - new_phase);
        if (diff > ADC_PHASE_THRESHOLD) {
            curr_phase = new_phase;
            state      = BTN_PHASE;
        }
    }

    for (uint8_t i = 0; i < AMOUNT_BUTTONS; i++) {
        debounceFSM_update(&inputs[i]);
        state |= (uint8_t) readKey(&inputs[i]) << i;
    }
    btns = state;
}

static bool process_input() {
    switch (btns) {
        case BTN_LOAD:
            ad9833_set_phase(curr_phase, REG_0);
            ad9833_set_freq(digit_to_freq(), REG_0);
            ad9833_set_form(curr_form);
            break;

        case BTN_LEFT:
            curr_pos++;
            curr_pos %= MAX_DIGITS_FREQ;
            break;

        case BTN_RIGHT:
            curr_pos = (curr_pos == 0) ? MAX_DIGITS_FREQ - ONE_OFFSET : curr_pos - ONE_OFFSET;
            break;

        case BTN_UP:
            increase_digit(curr_pos);
            break;

        case BTN_DOWN:
            reduce_digit(curr_pos);
            break;

        case BTN_CHANGE:
            curr_form++;
            curr_form %= SQUARE_DIV + ONE_OFFSET;
            break;

        case BTN_PHASE:
            break;

        default:
            btns = 0;  // Reset the btns for next read
            return false;
    }
    btns = 0;
    return true;
}

void freq_to_digit(uint32_t freq) {
    for (uint8_t i = 0; i < MAX_DIGITS_FREQ; i++) {
        freq_str[MAX_DIGITS_FREQ - ONE_OFFSET - i]  = (freq % DECIMAL_BASE) + ZERO_CHAR;
        freq                                       /= DECIMAL_BASE;
    }
    freq_str[MAX_DIGITS_FREQ] = EOL;
}

uint32_t digit_to_freq(void) {
    uint32_t val = 0;
    for (int i = 0; i < MAX_DIGITS_FREQ; i++) { val = (val * DECIMAL_BASE) + (freq_str[i] - ZERO_CHAR); }
    return val;
}

void increase_digit(uint8_t pos) {
    while (pos < MAX_DIGITS_FREQ) {
        int idx = MAX_DIGITS_FREQ - ONE_OFFSET - pos;

        if (freq_str[idx] == NINE_CHAR) {
            freq_str[idx] = ZERO_CHAR;
            pos++;
        } else {
            freq_str[idx]++;
            break;
        }
    }
}

void reduce_digit(uint8_t pos) {
    bool can_borrow = false;
    for (int i = pos; i < MAX_DIGITS_FREQ; i++) {
        if (freq_str[MAX_DIGITS_FREQ - ONE_OFFSET - i] > ZERO_CHAR) {
            can_borrow = true;
            break;
        }
    }

    // Already 0
    if (!can_borrow) { return; }

    while (pos < MAX_DIGITS_FREQ) {
        int idx = MAX_DIGITS_FREQ - ONE_OFFSET - pos;

        if (freq_str[idx] == ZERO_CHAR) {
            freq_str[idx] = NINE_CHAR;
            pos++;
        } else {
            freq_str[idx]--;
            break;
        }
    }
}
