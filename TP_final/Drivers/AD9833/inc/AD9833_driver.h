/**
 * @file AD9833_driver.h
 * @brief AD9833 driver header file.
 *
 * Has the functions to interact with the IC without having to manually calculate registers.
 */
#ifndef __AD9833_DRIVER__
#define __AD9833_DRIVER__
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

/* ---- Control register bit positions ---- */
#define AD9833_CTRL_B28_BIT        13U  //! Consecutive writes for 28-bit write
#define AD9833_CTRL_HLB_BIT        12U  //! Write on the 14-bit MSB or LSB
#define AD9833_CTRL_FSELECT_BIT    11U  //! Select freq. reg
#define AD9833_CTRL_PSELECT_BIT    10U  //! Select phase reg
#define AD9833_CTRL_RESERVED_BIT   9U   //! Reserved bit
#define AD9833_CTRL_RESET_BIT      8U   //! Reset internal regs
#define AD9833_CTRL_SLEEP1_BIT     7U   //! Disable internal CLK
#define AD9833_CTRL_SLEEP12_BIT    6U   //! Disable DAC
#define AD9833_CTRL_OPBITEN_BIT    5U   //! Output DACs MSB
#define AD9833_CTRL_DIV2_BIT       3U   //! DIV2 the MSB
#define AD9833_CTRL_MODE_BIT       1U   //! Sine or tri select

/* ---- Control register bit masks ---- */
#define AD9833_CTRL_B28            1U << AD9833_CTRL_B28_BIT
#define AD9833_CTRL_HLB            1U << AD9833_CTRL_HLB_BIT
#define AD9833_CTRL_FSELECT        1U << AD9833_CTRL_FSELECT_BIT
#define AD9833_CTRL_PSELECT        1U << AD9833_CTRL_PSELECT_BIT
#define AD9833_CTRL_RESERVED       1U << AD9833_CTRL_RESERVED_BIT
#define AD9833_CTRL_RESET          1U << AD9833_CTRL_RESET_BIT
#define AD9833_CTRL_SLEEP1         1U << AD9833_CTRL_SLEEP1_BIT
#define AD9833_CTRL_SLEEP12        1U << AD9833_CTRL_SLEEP12_BIT
#define AD9833_CTRL_OPBITEN        1U << AD9833_CTRL_OPBITEN_BIT
#define AD9833_CTRL_DIV2           1U << AD9833_CTRL_DIV2_BIT
#define AD9833_CTRL_MODE           1U << AD9833_CTRL_MODE_BIT

/* ---- Register address bits (D15:D14) ---- */
#define AD9833_REG_CONTROL         0x0000U  //! Control reg dir
#define AD9833_REG_FREQ0           0x4000U  //! freq0 dir
#define AD9833_REG_FREQ1           0x8000U  //! freq1 dir
#define AD9833_REG_PHASE0          0xC000U  //! phase0 dir
#define AD9833_REG_PHASE1          0xE000U  //! phase1 dir

/* ---- Masks ---- */
#define AD9833_FREQ_28BIT_MASK     0x0FFFFFFFU  //! Get the 28-bit freq
#define AD9833_FREQ_14BIT_MASK     0x3FFFU      //! Get the 14-bit freq
#define AD9833_PHASE_WORD_MASK     0x0FFFU      //! Get the phase
#define AD9833_REG_ADDR_MASK       0xC000U      //! Get the reg dir

/* ---- Init values ---- */
#define AD9833_INIT_FREQ           0U  //! Freq in the init function
#define AD9833_INIT_PHASE          0U  //! Phase in the init function
#define AD9833_INIT_CTRL           0U  //! Init of the ctrl reg

/* ---- Limit values ---- */
#define AD9833_FMCLK_MIN_FREQ      0U         //! Minimun freq for the FMCLK
#define AD9833_FMCLK_MAX_FREQ      25U        //! Max. freq for the FMCLK
#define AD9833_FREQ_REG_LEN        28U        //! Lenght of freq regs
#define AD9833_PHASE_REG_LEN       12U        //! Lenght of phase regs
#define AD9833_FREQ_14BIT_SHIFT    14U        //! m is the current fmclk [MHz]
#define AD9833_FREQ_14BIT_LIMIT    (1 << 15)  //! To check if one or two regs

/* ---- Usefl values ---- */
#define AD9833_MHZ_IN_dHZ          10000000U  //! 1 MHz in dHZ
#define AD9833_CONVERSION_OFFSET   1U         //! Used when truncated value
#define AD9833_PI                  3142       //! PI in mRADs
#define AD9833_MAX_PHASE_MRADS     6283       //! Max phase in mRADs
#define AD9833_FREQ_SCALE_HZ_DHZ   10U        //! Factor from HZ to dHZ
#define AD9833_FREQ_SCALE_MRAD_RAD 1000U      //! Factor from mRads to RADs

/* ---- Macros ---- */
#define HZ_TO_dHZ(x)               x *AD9833_FREQ_SCALE_HZ_DHZ  //! Convert from HZ to dHZ
#define GET_14_MSB(x)              ((x >> 14U) & 0x3FFFU)       //! Get the 14 MSBs
#define DEFAULT_DEV()              {.ctrl_reg = 0, .init = false, .fmclk = 0xFF, .freq0 = 0xFFFF, .freq1 = 0xFFFF, .phase0 = 0xFFFF, .phase1 = 0xFFFF}

typedef enum { AD_OK, AD_FAIL, AD_NOT_INIT, AD_NO_RESPONSE, AD_BAD_ARGUMENT } ad9833_st_t;

typedef enum { REG_0, REG_1 } reg_t;

typedef enum {
    AD_ON,              //! No sleep mode activated
    AD_DAC_OFF,         //! Used when using the square wave
    AD_NO_CLK,          //! Stops the internal CLK (output turns constant)
    AD_FULL_SLEEP       //! Both DAC and CLK off
} ad9833_sleep_t;

typedef enum {
    SINE,               //! Sine wave form (goes through the SIN ROM)
    TRIANGULAR,         //! Triangular wave form (no SIN ROM)
    SQUARE,             //! Square wave (MSB of DAC)
    SQUARE_DIV          //! Square wave device by 2
} ad9833_wf_t;

typedef struct {
    uint16_t ctrl_reg;  //! Current ctrl
    uint32_t freq0;     //! Reg. loaded in f0
    uint32_t freq1;     //! Reg. loaded in f1
    uint16_t phase0;    //! Reg. loaded in p0
    uint16_t phase1;    //! Reg. loaded in p1
    bool     init;      //! If init
    uint8_t  fmclk;     //! fmclk in MHZ (less than 25)
} ad_dev_t;

/**
 * @brief Initializes the ad9833 and test if it reachable.
 *
 * @param[in] mclk Master clock in MHz (has to be between 0-25).
 *
 * @retval AD_OK on success else depends on the error code
 */
ad9833_st_t ad9833_init(uint8_t mclk);

/**
 * @brief Sets the frequency on one of the two 28-bit registers.
 *
 * Be aware that doing this won't select it, as this is done using the control
 * register.j
 *
 * @param[in] freq Frequency to set in dHz.
 * @param[in] reg Select one of the two registers to set.
 *
 * @retval AD_OK on success else depends on the error code
 */
ad9833_st_t ad9833_set_freq(uint32_t freq, reg_t reg);

/**
 * @brief Sets the phase on one of the two 12-bit registers.
 *
 * Be aware that doing this won't select it, as this is done using the control
 * register.
 *
 * @param[in] phase Phase offset in tenth of a degree.
 * @param[in] reg Select one of the two registers to set.
 *
 * @retval AD_OK on success else depends on the error code
 */
ad9833_st_t ad9833_set_phase(uint16_t phase, reg_t reg);

/**
 * @brief Sets the control register.
 *
 * @param[in] control Control register configuration.
 *
 * @retval AD_OK on success else depends on the error code
 */
ad9833_st_t ad9833_set_control(uint16_t control);

/**
 * @brief Sets the form.
 *
 * @param[in] form Variable to select the form to output.
 *
 * @retval AD_OK on success else depends on the error code.
 */
ad9833_st_t ad9833_set_form(ad9833_wf_t form);
#endif  // __AD9833_DRIVER__
