#ifndef WIIPOD_ADS1115_H
#define WIIPOD_ADS1115_H

#include <Arduino.h>

// #include <guarantee.h>
#include <I2CIP.h>

// REGISTERS

// Pointer register
#define ADS1115_REG_POINTER_MASK        (uint8_t)0x03 // Point mask
#define ADS1115_REG_POINTER_CONVERT     (uint8_t)0x00 // Conversion
#define ADS1115_REG_POINTER_CONFIG      (uint8_t)0x01 // Configuration
#define ADS1115_REG_POINTER_LOWTHRESH   (uint8_t)0x02 // Low threshold
#define ADS1115_REG_POINTER_HITHRESH    (uint8_t)0x03 // High threshold

// Config register: OS bit
#define ADS1115_REG_CONFIG_OS_MASK      (uint16_t)0x8000 // OS Mask
#define ADS1115_REG_CONFIG_OS_SINGLE    (uint16_t)0x8000 // Write: Set to start a single-conversion
#define ADS1115_REG_CONFIG_OS_BUSY      (uint16_t)0x0000 // Read: Bit = 0 when conversion is in progress
#define ADS1115_REG_CONFIG_OS_NOTBUSY   (uint16_t)0x8000 // Read: Bit = 1 when device is not performing a conversion

// Config register: MUX bits
#define ADS1115_REG_CONFIG_MUX_MASK     (uint16_t)0x7000 // Mux Mask
#define ADS1115_REG_CONFIG_MUX_DIFF_0_1 (uint16_t)0x0000 // Differential P = AIN0, N = AIN1 (default)
#define ADS1115_REG_CONFIG_MUX_DIFF_0_3 (uint16_t)0x1000 // Differential P = AIN0, N = AIN3
#define ADS1115_REG_CONFIG_MUX_DIFF_1_3 (uint16_t)0x2000 // Differential P = AIN1, N = AIN3
#define ADS1115_REG_CONFIG_MUX_DIFF_2_3 (uint16_t)0x3000 // Differential P = AIN2, N = AIN3
#define ADS1115_REG_CONFIG_MUX_SINGLE_0 (uint16_t)0x4000 // Single-ended AIN0
#define ADS1115_REG_CONFIG_MUX_SINGLE_1 (uint16_t)0x5000 // Single-ended AIN1
#define ADS1115_REG_CONFIG_MUX_SINGLE_2 (uint16_t)0x6000 // Single-ended AIN2
#define ADS1115_REG_CONFIG_MUX_SINGLE_3 (uint16_t)0x7000 // Single-ended AIN3

// Config register: Gain bits
#define ADS1115_REG_CONFIG_PGA_MASK     (uint16_t)0x0E00 // PGA Mask
#define ADS1115_REG_CONFIG_PGA_6_144V   (uint16_t)0x0000 // +/-6.144V range = Gain 2/3
#define ADS1115_REG_CONFIG_PGA_4_096V   (uint16_t)0x0200 // +/-4.096V range = Gain 1
#define ADS1115_REG_CONFIG_PGA_2_048V   (uint16_t)0x0400 // +/-2.048V range = Gain 2 (default)
#define ADS1115_REG_CONFIG_PGA_1_024V   (uint16_t)0x0600 // +/-1.024V range = Gain 4
#define ADS1115_REG_CONFIG_PGA_0_512V   (uint16_t)0x0800 // +/-0.512V range = Gain 8
#define ADS1115_REG_CONFIG_PGA_0_256V   (uint16_t)0x0A00 // +/-0.256V range = Gain 16

// Config register: ADC mode bit
#define ADS1115_REG_CONFIG_MODE_MASK    (uint16_t)0x0100 // Mode Mask
#define ADS1115_REG_CONFIG_MODE_CONTIN  (uint16_t)0x0000 // Continuous conversion mode
#define ADS1115_REG_CONFIG_MODE_SINGLE  (uint16_t)0x0100 // Power-down single-shot mode (default)

// Config register: sample rate bits
#define ADS1115_REG_CONFIG_RATE_MASK    (uint16_t)0x00E0 // Data Rate Mask
#define ADS1115_RATE_128SPS             (uint16_t)0x0000 // 128 samples per second
#define ADS1115_RATE_250SPS             (uint16_t)0x0020 // 250 samples per second
#define ADS1115_RATE_490SPS             (uint16_t)0x0040 // 490 samples per second
#define ADS1115_RATE_920SPS             (uint16_t)0x0060 // 920 samples per second
#define ADS1115_RATE_1600SPS            (uint16_t)0x0080 // 1600 samples per second (default)
#define ADS1115_RATE_2400SPS            (uint16_t)0x00A0 // 2400 samples per second
#define ADS1115_RATE_3300SPS            (uint16_t)0x00C0 // 3300 samples per second

// Config register: Comparator mode bit
#define ADS1115_REG_CONFIG_CMODE_MASK   (uint16_t)0x0010 // CMode Mask
#define ADS1115_REG_CONFIG_CMODE_TRAD   (uint16_t)0x0000 // Traditional comparator with hysteresis (default)
#define ADS1115_REG_CONFIG_CMODE_WINDOW (uint16_t)0x0010 // Window comparator

// Config register: Comparator polarity bit
#define ADS1115_REG_CONFIG_CPOL_MASK    (uint16_t)0x0008 // CPol Mask
#define ADS1115_REG_CONFIG_CPOL_ACTVLOW (uint16_t)0x0000 // ALERT/RDY pin is low when active (default)
#define ADS1115_REG_CONFIG_CPOL_ACTVHI  (uint16_t)0x0008 // ALERT/RDY pin is high when active

// Config register: Comparator latching bit
#define ADS1115_REG_CONFIG_CLAT_MASK    (uint16_t)0x0004 // Determines if ALERT/RDY pin latches once asserted
#define ADS1115_REG_CONFIG_CLAT_NONLAT  (uint16_t)0x0000 // Non-latching comparator (default)
#define ADS1115_REG_CONFIG_CLAT_LATCH   (uint16_t)0x0004 // Latching comparator

// Config register: Comparator queue bits
#define ADS1115_REG_CONFIG_CQUE_MASK    (uint16_t)0x0003 // CQue Mask
#define ADS1115_REG_CONFIG_CQUE_1CONV   (uint16_t)0x0000 // Assert ALERT/RDY after one conversions
#define ADS1115_REG_CONFIG_CQUE_2CONV   (uint16_t)0x0001 // Assert ALERT/RDY after two conversions
#define ADS1115_REG_CONFIG_CQUE_4CONV   (uint16_t)0x0002 // Assert ALERT/RDY after four conversions
#define ADS1115_REG_CONFIG_CQUE_NONE    (uint16_t)0x0003 // Disable the comparator and put ALERT/RDY in high state (default)

// MACROS

#define ADS1115_CHANNEL_TO_MUX(channel) (0x4000 + (channel) * 0x1000)

// SETTINGS

#define I2CIP_ADS1115_GAIN    ADS1115_REG_CONFIG_PGA_6_144V // +/-6.144V for 0-5V single-ended analogRead
#define I2CIP_ADS1115_SPS     ADS1115_RATE_1600SPS
#define I2CIP_ADS1115_TIMEOUT 100 // Number of "ready" read attempts

#define I2CIP_ADS1115_ADDRESS 0x48
#define I2CIP_ADS1115_DELAY 10
#ifdef I2CIP_USE_GUARANTEES
#define I2CIP_GUARANTEE_ADS1115 1115
#endif

#ifdef I2CIP_USE_GUARANTEES
class ADS1115;
I2CIP_GUARANTEE_DEFINE(ADS1115, I2CIP_GUARANTEE_ADS1115);
#endif

// const char i2cip_ads1115_id_progmem[] PROGMEM = {"ADS1115"};

typedef enum {
  ADS1115_CHANNEL_0 = 0x0,
  ADS1115_CHANNEL_1,
  ADS1115_CHANNEL_2,
  ADS1115_CHANNEL_3,
  ADS1115_CHANNEL_NULL = 0xFF
} i2cip_ads1115_chsel_t;

// Get VOOOLTAGE

class ADS1115 : public I2CIP::Device, public I2CIP::InputInterface<float, i2cip_ads1115_chsel_t>
  #ifdef I2CIP_USE_GUARANTEES
  , public I2CIP::Guarantee<ADS1115>
  #endif
  {
  I2CIP_DEVICE_CLASS_BUNDLE(ADS1115);
  I2CIP_INPUT_USE_TOSTRING(float, "%.2f");
  I2CIP_INPUT_ADD_PRINTCACHE(float, "%.2f V");
  #ifdef I2CIP_USE_GUARANTEES
  I2CIP_CLASS_USE_GUARANTEE(ADS1115, I2CIP_GUARANTEE_ADS1115);
  #endif

  private:
      bool initialized = false;

      // const i2cip_ads1115_chsel_t _default_args = ADS1115_CHANNEL_NULL; // Uncomment if you want to disable the ADS1115 unless you explicitly set a channel :(
      const i2cip_ads1115_chsel_t _default_args = ADS1115_CHANNEL_0; // Uncomment if you want to enable the ADS1115 by default using channel 0 :)
      const float _default_cache = NAN;

      static float computeVolts(int16_t counts);

      #ifdef MAIN_CLASS_NAME
      friend class MAIN_CLASS_NAME;
      #endif
    public:
      ADS1115(i2cip_fqa_t fqa, const i2cip_id_t& id);
      // ADS1115(i2cip_fqa_t fqa) : Device(fqa, i2cip_ads1115_id_progmem, _id), InputInterface<float, i2cip_ads1115_chsel_t>((Device*)this) { }

      virtual ~ADS1115() { }

      /**
       * Read from the Nunchuck.
       * @param dest Destination heap (pointer reassigned, not overwritten)
       * @param args Number of bytes to read
       **/
      i2cip_errorlevel_t get(float& dest, const i2cip_ads1115_chsel_t& args = ADS1115_CHANNEL_NULL) override;

      void clearCache(void) override { this->setCache(this->_default_cache); }
      const i2cip_ads1115_chsel_t& getDefaultA(void) const override { return this->_default_args; };
};

#endif