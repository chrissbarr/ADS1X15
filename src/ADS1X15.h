/*************************************************** 
 This is a library for the ADS1X15 I2C ADC.

 Written by Chris Barr, 2022.
 ****************************************************/

#ifndef _ADS1X15_H_
#define _ADS1X15_H_

#include <stdint.h>
#include <Arduino.h>

namespace ADS1X15 {

constexpr int ADS1X15_ADDRESS = 0x48;

enum class Rate : uint16_t {
  ADS1015_128SPS = 0x0000,
  ADS1015_250SPS = 0x0020,
  ADS1015_490SPS = 0x0040,
  ADS1015_920SPS = 0x0060,
  ADS1015_1600SPS = 0x0080,
  ADS1015_2500SPS = 0x00A0,
  ADS1015_3300SPS = 0x00C0,
  ADS1115_8SPS = 0x0000,
  ADS1115_16SPS = 0x0020,
  ADS1115_32SPS = 0x0040,
  ADS1115_64SPS = 0x0060,
  ADS1115_128SPS = 0x0080,
  ADS1115_250SPS = 0x00A0,
  ADS1115_475SPS = 0x00C0,
  ADS1115_860SPS = 0x00E0
};

enum class Gain : uint16_t {
  TWOTHIRDS_6144MV = 0x0000,
  ONE_4096MV = 0x0200,
  TWO_2048MV = 0x0400,
  FOUR_1024MV = 0x0600,
  EIGHT_512MV = 0x0800,
  SIXTEEN_256MV = 0x0A00
};

// registers
enum class RegisterAddress : uint8_t {
  CONVERSION  = 0x00,
  CONFIG      = 0x01,
  LOTHRESH    = 0x02,
  HITHRESH    = 0x03
};


constexpr uint16_t ADS1X15_REG_CONFIG_OS_MASK = 0x8000; ///< OS Mask
constexpr uint16_t ADS1X15_REG_CONFIG_OS_SINGLE = 0x8000; ///< Write: Set to start a single-conversion
constexpr uint16_t ADS1X15_REG_CONFIG_OS_BUSY = 0x0000; ///< Read: Bit = 0 when conversion is in progress
constexpr uint16_t ADS1X15_REG_CONFIG_OS_NOTBUSY = 0x8000; ///< Read: Bit = 1 when device is not performing a conversion

constexpr uint16_t ADS1X15_REG_CONFIG_MUX_MASK = 0x7000; ///< Mux Mask
constexpr uint16_t ADS1X15_REG_CONFIG_MUX_DIFF_0_1 = 0x0000; ///< Differential P = AIN0, N = AIN1 (default)
constexpr uint16_t ADS1X15_REG_CONFIG_MUX_DIFF_0_3 = 0x1000; ///< Differential P = AIN0, N = AIN3
constexpr uint16_t ADS1X15_REG_CONFIG_MUX_DIFF_1_3 = 0x2000; ///< Differential P = AIN1, N = AIN3
constexpr uint16_t ADS1X15_REG_CONFIG_MUX_DIFF_2_3 = 0x3000; ///< Differential P = AIN2, N = AIN3
constexpr uint16_t ADS1X15_REG_CONFIG_MUX_SINGLE_0 = 0x4000; ///< Single-ended AIN0
constexpr uint16_t ADS1X15_REG_CONFIG_MUX_SINGLE_1 = 0x5000; ///< Single-ended AIN1
constexpr uint16_t ADS1X15_REG_CONFIG_MUX_SINGLE_2 = 0x6000; ///< Single-ended AIN2
constexpr uint16_t ADS1X15_REG_CONFIG_MUX_SINGLE_3 = 0x7000; ///< Single-ended AIN3

constexpr uint16_t MUX_BY_CHANNEL[] = {
    ADS1X15_REG_CONFIG_MUX_SINGLE_0, ///< Single-ended AIN0
    ADS1X15_REG_CONFIG_MUX_SINGLE_1, ///< Single-ended AIN1
    ADS1X15_REG_CONFIG_MUX_SINGLE_2, ///< Single-ended AIN2
    ADS1X15_REG_CONFIG_MUX_SINGLE_3  ///< Single-ended AIN3
};                                   ///< MUX config by channel

constexpr uint16_t ADS1X15_REG_CONFIG_MODE_MASK = 0x0100;   ///< Mode Mask
constexpr uint16_t ADS1X15_REG_CONFIG_MODE_CONTIN = 0x0000; ///< Continuous conversion mode
constexpr uint16_t ADS1X15_REG_CONFIG_MODE_SINGLE = 0x0100; ///< Power-down single-shot mode (default)

constexpr uint16_t ADS1X15_REG_CONFIG_RATE_MASK = 0x00E0; ///< Data Rate Mask

constexpr uint16_t ADS1X15_REG_CONFIG_CMODE_MASK = 0x0010; ///< CMode Mask
constexpr uint16_t ADS1X15_REG_CONFIG_CMODE_TRAD = 0x0000; ///< Traditional comparator with hysteresis (default)
constexpr uint16_t ADS1X15_REG_CONFIG_CMODE_WINDOW = 0x0010; ///< Window comparator

constexpr uint16_t ADS1X15_REG_CONFIG_CPOL_MASK = 0x0008; ///< CPol Mask
constexpr uint16_t ADS1X15_REG_CONFIG_CPOL_ACTVLOW = 0x0000; ///< ALERT/RDY pin is low when active (default)
constexpr uint16_t ADS1X15_REG_CONFIG_CPOL_ACTVHI = 0x0008; ///< ALERT/RDY pin is high when active

constexpr uint16_t ADS1X15_REG_CONFIG_CLAT_MASK = 0x0004; ///< Determines if ALERT/RDY pin latches once asserted
constexpr uint16_t ADS1X15_REG_CONFIG_CLAT_NONLAT = 0x0000; ///< Non-latching comparator (default)
constexpr uint16_t ADS1X15_REG_CONFIG_CLAT_LATCH = 0x0004; ///< Latching comparator

constexpr uint16_t ADS1X15_REG_CONFIG_CQUE_MASK = 0x0003; ///< CQue Mask
constexpr uint16_t ADS1X15_REG_CONFIG_CQUE_1CONV = 0x0000; ///< Assert ALERT/RDY after one conversions
constexpr uint16_t ADS1X15_REG_CONFIG_CQUE_2CONV = 0x0001; ///< Assert ALERT/RDY after two conversions
constexpr uint16_t ADS1X15_REG_CONFIG_CQUE_4CONV = 0x0002; ///< Assert ALERT/RDY after four conversions
constexpr uint16_t ADS1X15_REG_CONFIG_CQUE_NONE = 0x0003; ///< Disable the comparator and put ALERT/RDY in high state (default)

template <typename WIRE>
class ADS1X15 {
public:

  ADS1X15(WIRE& wire) : mWire(wire) {};

  /**
   * Initializes the PCAL9535A given its HW address, see datasheet for address selection.
   * /param addr Address of PCAL9535A (0 - 7)
   */
  void begin(uint8_t address = ADS1X15_ADDRESS)
  {
    _i2caddr = address;
    mWire.begin();
  }

  void setGain(Gain gain) { _gain = gain; }
  Gain getGain() const { return _gain; }

  void setDataRate(Rate rate) { _rate = rate; }
  Rate getDataRate() const { return _rate; }

  int16_t readADCSingleEnded(uint8_t channel)
  {
    if (channel > 3) { return 0; }

    startADCReading(MUX_BY_CHANNEL[channel], /*continuous=*/false);

    // Wait for the conversion to complete
    while (!conversionComplete());

    // Read the conversion results
    return getLastConversionResults();
  }

  void startADCReading(uint16_t mux, bool continuous)
  {
    // Start with default values
    uint16_t config =
      ADS1X15_REG_CONFIG_CQUE_1CONV |   // Set CQUE to any value other than
                                        // None so we can use it in RDY mode
      ADS1X15_REG_CONFIG_CLAT_NONLAT |  // Non-latching (default val)
      ADS1X15_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
      ADS1X15_REG_CONFIG_CMODE_TRAD;    // Traditional comparator (default val)

    if (continuous) {
      config |= ADS1X15_REG_CONFIG_MODE_CONTIN;
    } else {
      config |= ADS1X15_REG_CONFIG_MODE_SINGLE;
    }

    // Set PGA/voltage range
    config |= static_cast<uint16_t>(_gain);

    // Set data rate
    config |= static_cast<uint16_t>(_rate);

    // Set channels
    config |= mux;

    // Set 'start single-conversion' bit
    config |= ADS1X15_REG_CONFIG_OS_SINGLE;

    // Write config register to the ADC
    writeRegister(RegisterAddress::CONFIG, config);

    // Set ALERT/RDY to RDY mode.
    writeRegister(RegisterAddress::HITHRESH, 0x8000);
    writeRegister(RegisterAddress::LOTHRESH, 0x0000);
  }

  bool conversionComplete() {
    return (readRegister(RegisterAddress::CONFIG) & ADS1X15_REG_CONFIG_OS_NOTBUSY) != 0;
  }

  int16_t getLastConversionResults() {
    // Read the conversion results
    uint16_t res = readRegister(RegisterAddress::CONVERSION) >> _bitshift;
    if (_bitshift == 0) {
      return (int16_t)res;
    } else {
      // Shift 12-bit results right 4 bits for the ADS1015,
      // making sure we keep the sign bit intact
      if (res > 0x07FF) {
        // negative number - extend the sign to 16th bit
        res |= 0xF000;
      }
      return (int16_t)res;
    }
  }

  float computeVolts(int16_t count) 
  { 
    float range;
    switch (_gain) {
      case Gain::TWOTHIRDS_6144MV:
        range = 6.144;
        break;
      case Gain::ONE_4096MV:
        range = 4.096;
        break;
      case Gain::TWO_2048MV:
        range = 2.048;
        break;
      case Gain::FOUR_1024MV:
        range = 1.024;
        break;
      case Gain::EIGHT_512MV:
        range = 0.512;
        break;
      case Gain::SIXTEEN_256MV:
        range = 0.256;
        break;
      default:
        range = 0.0;
        break;
    }
    return count * (range / (32768 >> _bitshift));
  }

protected:
  uint8_t _i2caddr;
  WIRE& mWire;
  uint8_t _bitshift;
  Gain _gain;
  Rate _rate;

private:
  uint8_t buffer[3];

  void writeRegister(RegisterAddress reg, uint16_t value) {
    buffer[0] = static_cast<uint8_t>(reg);
    buffer[1] = value >> 8;
    buffer[2] = value & 0xFF;
    mWire.write(buffer[0]);
    mWire.write(buffer[1]);
    mWire.write(buffer[2]);
  }

  uint16_t readRegister(RegisterAddress reg) {
    mWire.write(static_cast<uint8_t>(reg));
    buffer[0] = mWire.read();
    buffer[1] = mWire.read();
    return ((buffer[0] << 8) | buffer[1]);
  }



};

template <typename WIRE>
class ADS1015 : public ADS1X15<WIRE> {
public:
  ADS1015(WIRE& wire) : ADS1X15<WIRE>(wire) 
  {
    this->_bitshift = 4;
    this->_gain = Gain::TWOTHIRDS_6144MV;
    this->_rate = Rate::ADS1015_1600SPS;
  };
};

template <typename WIRE>
class ADS1115 : public ADS1X15<WIRE> {
public:
  ADS1115(WIRE& wire) : ADS1X15<WIRE>(wire) 
  {
    this->_bitshift = 0;
    this->_gain = Gain::TWOTHIRDS_6144MV;
    this->_rate = Rate::ADS1115_128SPS;
  };
};



} // namespace _ADS1X15_H_

#endif