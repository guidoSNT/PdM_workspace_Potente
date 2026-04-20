#include "AD9833_driver.h"
#include "AD9833_port.h"
#include "API_uart.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

static uint8_t fmclk; //! Master clock [MHz] to get the freq correctly
static bool initialized = false; //! Init flag
static uint16_t ctrl_reg;        //! Control register
static uint32_t curr_freq;       //! current frequency

ad9833_st_t ad9833_init(uint8_t mclk) {
  ad9833_st_t res;
  uartInit();
  if (mclk == AD9833_FMCLK_MIN_FREQ || mclk > AD9833_FMCLK_MAX_FREQ)
    return AD_BAD_ARGUMENT;

  fmclk = mclk;
  curr_freq = 0;

  if (spi_init() != AD_SPI_OK)
    return AD_NOT_INIT;

  res = ad9833_set_control(AD9833_CTRL_RESET, false);
  if (res == AD_NO_RESPONSE)
    return AD_FAIL;

  // HERE MAYBE A WAIT SHOULD BE ADDED
  res = ad9833_set_freq(AD9833_INIT_FREQ, REG_0);
  if (res == AD_NO_RESPONSE)
    return AD_FAIL;

  res = ad9833_set_freq(AD9833_INIT_FREQ, REG_1);
  if (res == AD_NO_RESPONSE)
    return AD_FAIL;

  // res = ad9833_set_phase(AD9833_INIT_PHASE, REG_0, dev);
  // if (res == AD_NO_RESPONSE)
  //   return AD_FAIL;

  initialized = true;
  return AD_OK;
}

ad9833_st_t ad9833_set_freq(uint32_t freq, reg_t reg) {
  if (initialized == false)
    return AD_NOT_INIT;

  if (freq > fmclk * AD9833_MHZ_IN_dHZ / 2)
    return AD_BAD_ARGUMENT;

  // To get which register (freq0 or 1) to set
  uint16_t reg_temp = (reg == REG_0) ? AD9833_REG_FREQ0 : AD9833_REG_FREQ1;
  uint16_t word;

  // Equivalent to do f* 2^28 / fmclk
  uint32_t freq_reg = ((uint64_t)freq << AD9833_FREQ_REG_LEN) / (fmclk * AD9833_MHZ_IN_dHZ) + AD9833_CONVERSION_OFFSET;

  // When both LSB and MSB need to be set then B28 bit has to be set for 2
  // consecutive comms (first is LSB and next is MSB). This is tested by looking
  // if the 14 MSB are the same and if the freq_reg has more than 14 bits
  bool full_tx = GET_14_MSB(freq_reg) != GET_14_MSB(curr_freq);
  uint16_t ctrl_temp = (full_tx) ? ctrl_reg | AD9833_CTRL_B28 : ctrl_reg & ~AD9833_CTRL_B28;

  if (ad9833_set_control(ctrl_temp, false) != AD_OK)
    return AD_NO_RESPONSE;

  word = reg_temp | (uint16_t)(freq & AD9833_FREQ_14BIT_MASK);
  if (AD_SPI_OK != write_word(word))
    return AD_FAIL;

  if (full_tx) {
    word = reg_temp | (uint16_t)((freq >> AD9833_FREQ_14BIT_SHIFT) &
                                 AD9833_FREQ_14BIT_MASK);
    if (AD_SPI_OK != write_word(word))
      return AD_FAIL;
  }
  curr_freq = freq_reg;

  return AD_OK;
}

ad9833_st_t ad9833_set_control(uint16_t control, bool keep) {

  if (initialized == false)
    return AD_NOT_INIT;

  // Set the register to point to the control reg
  uint16_t reg = (keep == true) ? control | ctrl_reg : control;
  reg &= AD9833_FREQ_14BIT_MASK;

  ctrl_reg = reg;

  return (write_word(reg) == AD_SPI_OK) ? AD_OK : AD_FAIL;
}
