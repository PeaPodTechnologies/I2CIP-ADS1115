#include <ADS1115.h>

using namespace I2CIP;

I2CIP_DEVICE_INIT_STATIC_ID(ADS1115);
I2CIP_INPUT_INIT_RESET(ADS1115, float, NAN, i2cip_ads1115_chsel_t, ADS1115_CHANNEL_0);
// I2CIP_INPUT_INIT_RESET(ADS1115, float, NAN, i2cip_ads1115_chsel_t, ADS1115_CHANNEL_M);

ADS1115::ADS1115(i2cip_fqa_t fqa, const i2cip_id_t& id) : Device(fqa, id), InputInterface<float, i2cip_ads1115_chsel_t>((Device*)this) { }

float ADS1115::computeVolts(int16_t counts) {
  // see data sheet Table 3
  float fsRange;
  switch (I2CIP_ADS1115_GAIN) {
    case ADS1115_REG_CONFIG_PGA_6_144V:
      fsRange = 6.144f;
      break;
    case ADS1115_REG_CONFIG_PGA_4_096V:
      fsRange = 4.096f;
      break;
    case ADS1115_REG_CONFIG_PGA_2_048V:
      fsRange = 2.048f;
      break;
    case ADS1115_REG_CONFIG_PGA_1_024V:
      fsRange = 1.024f;
      break;
    case ADS1115_REG_CONFIG_PGA_0_512V:
      fsRange = 0.512f;
      break;
    case ADS1115_REG_CONFIG_PGA_0_256V:
      fsRange = 0.256f;
      break;
    default:
      fsRange = 0.0f;
  }
  return counts * (fsRange / (32768 >> 4));
}

i2cip_errorlevel_t ADS1115::get(float& dest, const i2cip_ads1115_chsel_t& args) {
  // 0. Check args
  if(args == ADS1115_CHANNEL_NULL) {
    return I2CIP_ERR_SOFT;
  }

  // Set config register values
  uint16_t config =
    ADS1115_REG_CONFIG_CQUE_1CONV   | // Set CQUE to any value other than none so we can use it in RDY mode
    ADS1115_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
    ADS1115_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low (default val)
    ADS1115_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
    ADS1115_REG_CONFIG_MODE_SINGLE  | // Single-ended discrete reading
    I2CIP_ADS1115_GAIN                    | // Gain setting
    I2CIP_ADS1115_SPS                     | // Sampling rate setting
    ADS1115_CHANNEL_TO_MUX(args)    | // Set MUX from channel #
    ADS1115_REG_CONFIG_OS_SINGLE;     // Set 'start single-conversion' bit

  // Overwrite config register
  uint8_t instr[2] = { (ADS1115_REG_POINTER_CONFIG >> 8) & 0xFF, ADS1115_REG_POINTER_CONFIG & 0xFF };
  i2cip_errorlevel_t errlev = this->writeRegister(ADS1115_REG_POINTER_CONFIG, instr, 2, false);
  I2CIP_ERR_BREAK(errlev);

  // Write threshold registers [0x8000, 0x0000]
  instr[0] = 0x80; instr[1] = 0x00;
  errlev = this->writeRegister(ADS1115_REG_POINTER_HITHRESH, instr, 2, false);
  I2CIP_ERR_BREAK(errlev);

  instr[0] = 0x00; instr[1] = 0x00;
  errlev = this->writeRegister(ADS1115_REG_POINTER_LOWTHRESH, instr, 2, false);
  I2CIP_ERR_BREAK(errlev);

  // Wait for the conversion to complete
  uint8_t timeout = 0;
  uint16_t ready = 0;
  do {
    errlev = this->readRegisterWord(ADS1115_REG_POINTER_CONFIG, ready, false, false);
    timeout++;
    if(timeout >= I2CIP_ADS1115_TIMEOUT) {
      errlev = I2CIP_ERR_SOFT;
      break;
    }
    delayMicroseconds(200);
  } while(((ready & 0x8000) == 0) && (errlev == I2CIP_ERR_NONE));
  I2CIP_ERR_BREAK(errlev);

  // Read the conversion results
  uint16_t result;
  errlev = this->readRegisterWord(ADS1115_REG_POINTER_CONVERT, result, false, false);
  I2CIP_ERR_BREAK(errlev);
  uint8_t buf [2] = { 0 };
  size_t readlen = 2;
  result &= this->read(buf, readlen, false, false, false);

  // Shift 12-bit results right 4 bits for the ADS1015, making sure we keep the sign bit intact
  uint16_t res = (((uint16_t)buf[0] << 8) | buf[1]) >> 4;
  if (res > 0x07FF) {
    // negative number - extend the sign to 16th bit
    res |= 0xF000;
  }
  dest = ADS1115::computeVolts((int16_t)res);
  return errlev;
}