#include "AD9833_driver.h"
#include "AD9833_port.h"
#include "API_uart.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

static ad_dev_t device = DEFAULT_DEV(); //! Device struct

/**
 * @brief this function returns the freq_reg from a freq in dHz. Equivalent: f*
 * 2^28 / fmclk.
 */
static inline uint32_t get_freq_reg(uint32_t freq) {
  return ((uint64_t)freq << AD9833_FREQ_REG_LEN) / (device.fmclk * AD9833_MHZ_IN_dHZ) + AD9833_CONVERSION_OFFSET;
}

ad9833_st_t ad9833_init(uint8_t mclk) {
  ad9833_st_t res;
  // uartInit();
  if (mclk == AD9833_FMCLK_MIN_FREQ || mclk > AD9833_FMCLK_MAX_FREQ)
    return AD_BAD_ARGUMENT;

  device.fmclk = mclk;

  if (spi_init() != AD_SPI_OK)
    return AD_NOT_INIT;

  res = ad9833_set_control(AD9833_CTRL_RESET, false);
  if (res == AD_NO_RESPONSE)
    return AD_FAIL;

  res = ad9833_set_freq(AD9833_INIT_FREQ, REG_0);
  if (res == AD_NO_RESPONSE)
    return AD_FAIL;

  res = ad9833_set_freq(AD9833_INIT_FREQ, REG_1);
  if (res == AD_NO_RESPONSE)
    return AD_FAIL;

  res = ad9833_set_phase(AD9833_INIT_PHASE, REG_0);
  if (res == AD_NO_RESPONSE)
    return AD_FAIL;

  res = ad9833_set_phase(AD9833_INIT_PHASE, REG_1);
  if (res == AD_NO_RESPONSE)
    return AD_FAIL;

  device.init = true;
  return AD_OK;
}

ad9833_st_t ad9833_set_form(ad9833_wf_t form) {
  if (device.init == false)
    return AD_NOT_INIT;

  ad9833_st_t res;
  uint16_t reg_temp;

  switch (form) {
  case SINE:
    reg_temp = device.ctrl_reg & ~(AD9833_CTRL_OPBITEN | AD9833_CTRL_MODE);
    break;

  case TRIANGULAR:
    reg_temp = (device.ctrl_reg & ~AD9833_CTRL_OPBITEN) | AD9833_CTRL_MODE;
    break;

  case SQUARE:
    reg_temp = device.ctrl_reg | AD9833_CTRL_OPBITEN;
    break;

  case SQUARE_DIV:
    reg_temp = (device.ctrl_reg & ~AD9833_CTRL_MODE) | AD9833_CTRL_OPBITEN |
               AD9833_CTRL_DIV2;
    break;

  default:
    return AD_BAD_ARGUMENT;
  }
  res = ad9833_set_control(reg_temp, false);

  return res;
}

ad9833_st_t ad9833_set_phase(uint16_t phase, reg_t reg) {
  if (device.init == false)
    return AD_NOT_INIT;

  if (phase > AD9833_PHASE_WORD_MASK)
    return AD_BAD_ARGUMENT;

  //
  uint16_t ctrl_temp = device.ctrl_reg & ~AD9833_CTRL_B28;
  uint16_t phase_reg = ((uint32_t)phase << AD9833_PHASE_REG_LEN) / (2 * AD9833_PI) - AD9833_CONVERSION_OFFSET;
  uint16_t reg_temp = (reg == REG_0) ? AD9833_REG_PHASE0 : AD9833_REG_PHASE1;
  phase_reg &= AD9833_PHASE_WORD_MASK;

  if (ad9833_set_control(ctrl_temp, false) != AD_OK)
    return AD_NO_RESPONSE;

  if (AD_SPI_OK != write_word(reg_temp | phase_reg))
    return AD_FAIL;

  if (reg == REG_0)
    device.phase0 = phase_reg;
  else
    device.phase1 = phase_reg;

  return AD_OK;
}

ad9833_st_t ad9833_set_freq(uint32_t freq, reg_t reg) {
  if (device.init == false)
    return AD_NOT_INIT;

  // Max freq is the nyquist frec at fs/2
  if (freq > device.fmclk * AD9833_MHZ_IN_dHZ / 2)
    return AD_BAD_ARGUMENT;

  // To properly set the freq, we need to check which register (reg_temp) and
  // get the 28-bit register. curr_freq is to check new and old register.
  uint32_t freq_reg = get_freq_reg(freq);
  uint32_t curr_freq = (reg == REG_0) ? device.freq0 : device.freq1;
  uint16_t reg_temp = (reg == REG_0) ? AD9833_REG_FREQ0 : AD9833_REG_FREQ1;

  // When both LSB and MSB need to be set then B28 bit has to be set for 2
  // consecutive comms (first is LSB and next is MSB). This is tested by looking
  // if the 14 MSB are the same and if the freq_reg has more than 14 bits
  bool full_tx = GET_14_MSB(freq_reg) != GET_14_MSB(curr_freq);
  uint16_t ctrl_temp = (full_tx) ? device.ctrl_reg | AD9833_CTRL_B28
                                 : device.ctrl_reg & ~AD9833_CTRL_B28;

  if (ad9833_set_control(ctrl_temp, false) != AD_OK)
    return AD_NO_RESPONSE;

  if (AD_SPI_OK !=
      write_word(reg_temp | (uint16_t)(freq & AD9833_FREQ_14BIT_MASK)))
    return AD_FAIL;

  if (full_tx) {
    if (AD_SPI_OK !=
        write_word(reg_temp | (uint16_t)((freq >> AD9833_FREQ_14BIT_SHIFT) &
                                         AD9833_FREQ_14BIT_MASK)))
      return AD_FAIL;
  }

  if (reg == REG_0)
    device.freq0 = freq_reg;
  else
    device.freq1 = freq_reg;

  return AD_OK;
}

ad9833_st_t ad9833_set_control(uint16_t control, bool keep) {
  if (device.init == false)
    return AD_NOT_INIT;

  // Set the register to point to the control reg
  uint16_t reg = (keep == true) ? control | device.ctrl_reg : control;
  reg &= AD9833_FREQ_14BIT_MASK;

  device.ctrl_reg = reg;

  return (write_word(reg) == AD_SPI_OK) ? AD_OK : AD_FAIL;
}
