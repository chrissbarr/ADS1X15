/**
 * Unit tests for ADS1X15 library.
 *
 * Uses a MockWire struct to simulate I2C without hardware.
 */

#include <cstdint>
#include <deque>
#include <vector>

#include "ADS1X15.h"
#include "gtest/gtest.h"

// ===========================================================================
// MockWire — minimal I2C mock for host-native testing
// ===========================================================================

struct MockWire {
    std::vector<uint8_t> written;           // bytes from every write() call
    std::vector<uint8_t> transmitted_addrs; // addr from each beginTransmission()
    std::deque<uint8_t> read_queue;         // bytes served by read()
    int end_transmission_count = 0;
    int begin_count = 0;

    void begin() { ++begin_count; }
    void beginTransmission(uint8_t addr) { transmitted_addrs.push_back(addr); }
    void write(uint8_t byte) { written.push_back(byte); }
    void endTransmission() { ++end_transmission_count; }
    void requestFrom(uint8_t /*addr*/, uint8_t /*count*/) {}

    uint8_t read() {
        if (read_queue.empty()) return 0xFF;
        uint8_t v = read_queue.front();
        read_queue.pop_front();
        return v;
    }

    /// Push a 16-bit value as two big-endian bytes (matches ADS1X15 register format).
    void queueWord(uint16_t value) {
        read_queue.push_back(static_cast<uint8_t>(value >> 8));
        read_queue.push_back(static_cast<uint8_t>(value & 0xFF));
    }

    void reset() {
        written.clear();
        transmitted_addrs.clear();
        read_queue.clear();
        end_transmission_count = 0;
        begin_count = 0;
    }
};

// ===========================================================================
// Section 1: computeVolts
//
// Formula: count * (gainToRange() / (32768 >> _bitshift))
// ADS1015: _bitshift=4, divisor=2048. ADS1115: _bitshift=0, divisor=32768.
// Default gain: TWOTHIRDS_6144MV → range=6.144V.
// ===========================================================================

TEST(ComputeVolts, ADS1015_DefaultGain_ZeroCount) {
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    EXPECT_FLOAT_EQ(ads.computeVolts(0), 0.0f);
}

TEST(ComputeVolts, ADS1015_DefaultGain_PositiveFullScale) {
    // 2047 * (6.144 / 2048) ≈ 6.141
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    EXPECT_NEAR(ads.computeVolts(2047), 6.141f, 0.001f);
}

TEST(ComputeVolts, ADS1015_DefaultGain_NegativeFullScale) {
    // -2048 * (6.144 / 2048) = -6.144
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    EXPECT_FLOAT_EQ(ads.computeVolts(-2048), -6.144f);
}

TEST(ComputeVolts, ADS1015_GainOne_MidScale) {
    // Gain ONE_4096MV: 1024 * (4.096 / 2048) = 2.048
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.setGain(ADS1X15::Gain::ONE_4096MV);
    EXPECT_FLOAT_EQ(ads.computeVolts(1024), 2.048f);
}

TEST(ComputeVolts, ADS1115_DefaultGain_PositiveFullScale) {
    // 32767 * (6.144 / 32768) ≈ 6.1438
    MockWire wire;
    ADS1X15::ADS1115<MockWire> ads(wire);
    EXPECT_NEAR(ads.computeVolts(32767), 6.1438f, 0.001f);
}

TEST(ComputeVolts, ADS1115_DefaultGain_NegativeFullScale) {
    // -32768 * (6.144 / 32768) = -6.144
    MockWire wire;
    ADS1X15::ADS1115<MockWire> ads(wire);
    EXPECT_FLOAT_EQ(ads.computeVolts(-32768), -6.144f);
}

// ===========================================================================
// Section 2: computeCount
//
// Formula: int16_t(volts * (32768 >> _bitshift) / gainToRange())
// ===========================================================================

TEST(ComputeCount, ADS1015_DefaultGain_ZeroVolts) {
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    EXPECT_EQ(ads.computeCount(0.0f), 0);
}

TEST(ComputeCount, ADS1015_DefaultGain_HalfScale) {
    // 3.072 * (2048 / 6.144) = 1024.0
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    EXPECT_EQ(ads.computeCount(3.072f), 1024);
}

TEST(ComputeCount, ADS1015_RoundTrip) {
    // computeCount(computeVolts(N)) should return N for representable values.
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    EXPECT_EQ(ads.computeCount(ads.computeVolts(1000)), 1000);
    EXPECT_EQ(ads.computeCount(ads.computeVolts(-500)), -500);
    EXPECT_EQ(ads.computeCount(ads.computeVolts(0)), 0);
}

TEST(ComputeCount, ADS1115_FullScale_ShouldClamp) {
    // BUG: 6.144 * (32768 / 6.144) = 32768.0, which overflows int16_t to -32768.
    // Correct behavior: should clamp to 32767 (max positive 16-bit value).
    MockWire wire;
    ADS1X15::ADS1115<MockWire> ads(wire);
    int16_t result = ads.computeCount(6.144f);
    EXPECT_EQ(result, int16_t(32767));
}

// ===========================================================================
// Section 3: getLastConversionResults — sign extension
//
// ADS1015: raw register >> 4, then if result > 0x07FF: result |= 0xF000.
// ADS1115: raw register cast directly to int16_t (no shift, no extension).
// ===========================================================================

TEST(GetLastConversionResults, ADS1015_PositiveMax_2047) {
    // 0x7FF0 >> 4 = 0x07FF = 2047. Not > 0x07FF, so no sign extension.
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    wire.queueWord(0x7FF0);
    EXPECT_EQ(ads.getLastConversionResults(), 2047);
}

TEST(GetLastConversionResults, ADS1015_Zero) {
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    wire.queueWord(0x0000);
    EXPECT_EQ(ads.getLastConversionResults(), 0);
}

TEST(GetLastConversionResults, ADS1015_NegativeOne) {
    // 0xFFF0 >> 4 = 0x0FFF. > 0x07FF, so |= 0xF000 → 0xFFFF = -1.
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    wire.queueWord(0xFFF0);
    EXPECT_EQ(ads.getLastConversionResults(), -1);
}

TEST(GetLastConversionResults, ADS1015_NegativeMin_Minus2048) {
    // 0x8000 >> 4 = 0x0800. > 0x07FF, so |= 0xF000 → 0xF800 = -2048.
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    wire.queueWord(0x8000);
    EXPECT_EQ(ads.getLastConversionResults(), -2048);
}

TEST(GetLastConversionResults, ADS1015_SmallNegative_Minus2047) {
    // 0x8010 >> 4 = 0x0801. |= 0xF000 → 0xF801 = -2047.
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    wire.queueWord(0x8010);
    EXPECT_EQ(ads.getLastConversionResults(), -2047);
}

TEST(GetLastConversionResults, ADS1115_PositiveMax) {
    // No shift, no sign extension. 0x7FFF cast to int16_t = 32767.
    MockWire wire;
    ADS1X15::ADS1115<MockWire> ads(wire);
    ads.begin();
    wire.queueWord(0x7FFF);
    EXPECT_EQ(ads.getLastConversionResults(), 32767);
}

TEST(GetLastConversionResults, ADS1115_NegativeMin) {
    // 0x8000 cast to int16_t = -32768.
    MockWire wire;
    ADS1X15::ADS1115<MockWire> ads(wire);
    ads.begin();
    wire.queueWord(0x8000);
    EXPECT_EQ(ads.getLastConversionResults(), -32768);
}

TEST(GetLastConversionResults, ADS1115_NegativeOne) {
    MockWire wire;
    ADS1X15::ADS1115<MockWire> ads(wire);
    ads.begin();
    wire.queueWord(0xFFFF);
    EXPECT_EQ(ads.getLastConversionResults(), -1);
}

// ===========================================================================
// Section 4: I2C write/read protocol
//
// writeRegister: beginTransmission(addr), write(reg), write(MSB), write(LSB),
//                endTransmission().
// readRegister:  beginTransmission(addr), write(reg), endTransmission(),
//                requestFrom(addr, 2), read() << 8 | read().
//
// Tested indirectly via startSingleEndedReading (3× writeRegister) and
// conversionComplete (1× readRegister).
// ===========================================================================

TEST(I2CProtocol, WriteRegister_NineBytesForThreeRegisters) {
    // startSingleEndedReading calls writeRegister 3 times = 9 bytes.
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    wire.reset();
    ads.startSingleEndedReading(0, false);
    EXPECT_EQ(wire.written.size(), 9u);
}

TEST(I2CProtocol, WriteRegister_MSBBeforeLSB) {
    // HITHRESH (first register written by startADCReading) gets value 0x8000.
    // Bytes: reg_addr(0x03), MSB(0x80), LSB(0x00).
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    wire.reset();
    ads.startSingleEndedReading(0, false);
    EXPECT_EQ(wire.written[1], 0x80); // MSB of 0x8000
    EXPECT_EQ(wire.written[2], 0x00); // LSB of 0x8000
}

TEST(I2CProtocol, WriteRegister_UsesDefaultAddress) {
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    wire.reset();
    ads.startSingleEndedReading(0, false);
    // 3 writeRegister calls = 3 beginTransmission calls, all to 0x48.
    EXPECT_EQ(wire.transmitted_addrs.size(), 3u);
    for (auto addr : wire.transmitted_addrs) {
        EXPECT_EQ(addr, 0x48);
    }
}

TEST(I2CProtocol, ReadRegister_BigEndianAssembly) {
    // Verify byte order matters: 0x8000 has bit 15 set (true), but if bytes
    // were swapped to 0x0080, bit 15 would be clear (false).
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    wire.queueWord(0x8000);
    EXPECT_TRUE(ads.conversionComplete());  // correct: (0x80 << 8) | 0x00 = 0x8000
    wire.queueWord(0x0080);
    EXPECT_FALSE(ads.conversionComplete()); // correct: (0x00 << 8) | 0x80 = 0x0080
}

TEST(I2CProtocol, ReadRegister_Bit15Clear_NotComplete) {
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    wire.queueWord(0x0000);
    EXPECT_FALSE(ads.conversionComplete());
}

// ===========================================================================
// Section 5: Config register assembly (startADCReading)
//
// startSingleEndedReading(ch, continuous) calls startADCReading internally.
// Register write order: HITHRESH(0x03), LOTHRESH(0x02), CONFIG(0x01).
//
// For ADS1015, channel 0, single-shot, default gain/rate:
//   CONFIG = OS_SINGLE(0x8000) | MUX_SINGLE_0(0x4000) | MODE_SINGLE(0x0100)
//          | ADS1015_1600SPS(0x0080) | TWOTHIRDS(0x0000) | CQUE_1CONV(0x0000)
//          | CLAT_NONLAT(0x0000) | CPOL_ACTVLOW(0x0000) | CMODE_TRAD(0x0000)
//        = 0xC180
// ===========================================================================

TEST(ConfigRegister, RegisterWriteOrder_HITHRESH_LOTHRESH_CONFIG) {
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    wire.reset();
    ads.startSingleEndedReading(0, false);
    // Register addresses at byte indices 0, 3, 6.
    EXPECT_EQ(wire.written[0], 0x03); // HITHRESH
    EXPECT_EQ(wire.written[3], 0x02); // LOTHRESH
    EXPECT_EQ(wire.written[6], 0x01); // CONFIG
}

TEST(ConfigRegister, SingleShot_Channel0_DefaultSettings) {
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    wire.reset();
    ads.startSingleEndedReading(0, false);
    uint16_t config = (static_cast<uint16_t>(wire.written[7]) << 8) | wire.written[8];
    EXPECT_EQ(config, 0xC180);
}

TEST(ConfigRegister, ContinuousMode_ModeBitCleared) {
    // Continuous mode: MODE_CONTIN(0x0000) instead of MODE_SINGLE(0x0100).
    // CONFIG = 0xC180 - 0x0100 = 0xC080.
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    wire.reset();
    ads.startSingleEndedReading(0, true);
    uint16_t config = (static_cast<uint16_t>(wire.written[7]) << 8) | wire.written[8];
    EXPECT_EQ(config, 0xC080);
}

TEST(ConfigRegister, Channel3_MuxBits) {
    // MUX_SINGLE_3 = 0x7000. CONFIG = 0x8000|0x7000|0x0100|0x0080 = 0xF180.
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    wire.reset();
    ads.startSingleEndedReading(3, false);
    uint16_t config = (static_cast<uint16_t>(wire.written[7]) << 8) | wire.written[8];
    EXPECT_EQ(config, 0xF180);
}

// ===========================================================================
// Section 6: Channel guard (readADCSingleEnded)
//
// Channel > 3 returns 0 without any I2C transactions.
// ===========================================================================

TEST(ChannelGuard, InvalidChannel4_ReturnsZero_NoI2C) {
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    wire.reset();
    EXPECT_EQ(ads.readADCSingleEnded(4), 0);
    EXPECT_TRUE(wire.written.empty());
}

TEST(ChannelGuard, InvalidChannel255_ReturnsZero_NoI2C) {
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    wire.reset();
    EXPECT_EQ(ads.readADCSingleEnded(255), 0);
    EXPECT_TRUE(wire.written.empty());
}

TEST(ChannelGuard, ValidChannel0_ReturnsConversionResult) {
    // Queue: conversionComplete needs CONFIG with OS bit set, then
    // getLastConversionResults needs the CONVERSION register value.
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    wire.reset();
    wire.queueWord(0x8000); // conversionComplete → OS bit set → true
    wire.queueWord(0x7FF0); // CONVERSION register → 0x7FF0 >> 4 = 2047
    EXPECT_EQ(ads.readADCSingleEnded(0), 2047);
    EXPECT_FALSE(wire.written.empty());
}

// ===========================================================================
// Section 7: Comparator threshold (startComparatorSingleEnded)
//
// Register write order: LOTHRESH(0x02), HITHRESH(0x03), CONFIG(0x01).
// HITHRESH = static_cast<uint16_t>(threshold) << _bitshift.
// ===========================================================================

TEST(Comparator, RegisterWriteOrder_LOTHRESH_HITHRESH_CONFIG) {
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    wire.reset();
    ads.startComparatorSingleEnded(0, 0);
    EXPECT_EQ(wire.written[0], 0x02); // LOTHRESH
    EXPECT_EQ(wire.written[3], 0x03); // HITHRESH
    EXPECT_EQ(wire.written[6], 0x01); // CONFIG
}

TEST(Comparator, ADS1015_ThresholdShiftedBy4) {
    // threshold=100, shifted: 100 << 4 = 0x0640.
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    wire.reset();
    ads.startComparatorSingleEnded(0, 100);
    // HITHRESH value bytes at indices 4 (MSB) and 5 (LSB).
    EXPECT_EQ(wire.written[4], 0x06); // MSB of 0x0640
    EXPECT_EQ(wire.written[5], 0x40); // LSB of 0x0640
}

TEST(Comparator, ADS1115_ThresholdNotShifted) {
    // threshold=1000 = 0x03E8, no shift.
    MockWire wire;
    ADS1X15::ADS1115<MockWire> ads(wire);
    ads.begin();
    wire.reset();
    ads.startComparatorSingleEnded(0, 1000);
    EXPECT_EQ(wire.written[4], 0x03); // MSB of 0x03E8
    EXPECT_EQ(wire.written[5], 0xE8); // LSB of 0x03E8
}

TEST(Comparator, ADS1015_ThresholdOverflow_ShouldClamp) {
    // BUG: threshold=2048 on ADS1015 overflows: 2048 << 4 = 0x8000, which the
    // chip interprets as a large negative threshold. The comparator will never
    // trigger as intended.
    // Correct behavior: should clamp to the maximum valid ADS1015 threshold
    // (2047), writing 2047 << 4 = 0x7FF0 to HITHRESH.
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    wire.reset();
    ads.startComparatorSingleEnded(0, 2048);
    uint16_t hithresh = (static_cast<uint16_t>(wire.written[4]) << 8) | wire.written[5];
    EXPECT_EQ(hithresh, 0x7FF0);
}

TEST(Comparator, InvalidChannel_NoI2C) {
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    wire.reset();
    ads.startComparatorSingleEnded(4, 100);
    EXPECT_TRUE(wire.written.empty());
}

// ===========================================================================
// Section 8: Differential reading (startDifferentialReading)
//
// DifferentialPair enum values map to MUX bits:
//   PAIR_01=0x0000, PAIR_03=0x1000, PAIR_13=0x2000, PAIR_23=0x3000.
// ===========================================================================

TEST(DifferentialReading, Pair01_MuxBits) {
    // PAIR_01 = 0x0000. CONFIG = 0x8000|0x0000|0x0100|0x0080 = 0x8180.
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    wire.reset();
    ads.startDifferentialReading(ADS1X15::DifferentialPair::PAIR_01, false);
    uint16_t config = (static_cast<uint16_t>(wire.written[7]) << 8) | wire.written[8];
    EXPECT_EQ(config, 0x8180);
}

TEST(DifferentialReading, Pair23_MuxBits) {
    // PAIR_23 = 0x3000. CONFIG = 0x8000|0x3000|0x0100|0x0080 = 0xB180.
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    wire.reset();
    ads.startDifferentialReading(ADS1X15::DifferentialPair::PAIR_23, false);
    uint16_t config = (static_cast<uint16_t>(wire.written[7]) << 8) | wire.written[8];
    EXPECT_EQ(config, 0xB180);
}

// ===========================================================================
// Section 9: setGain and setDataRate effect on config register
// ===========================================================================

TEST(ConfigRegister, SetGain_AppearsInConfigBits) {
    // Gain ONE_4096MV = 0x0200. With channel 0 single-shot:
    // CONFIG = 0x8000|0x4000|0x0200|0x0100|0x0080 = 0xC380.
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    ads.setGain(ADS1X15::Gain::ONE_4096MV);
    wire.reset();
    ads.startSingleEndedReading(0, false);
    uint16_t config = (static_cast<uint16_t>(wire.written[7]) << 8) | wire.written[8];
    EXPECT_EQ(config, 0xC380);
}

TEST(ConfigRegister, SetDataRate_AppearsInConfigBits) {
    // ADS1015_250SPS = 0x0020. With channel 0 single-shot, default gain:
    // CONFIG = 0x8000|0x4000|0x0100|0x0020 = 0xC120.
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin();
    ads.setDataRate(ADS1X15::Rate::ADS1015_250SPS);
    wire.reset();
    ads.startSingleEndedReading(0, false);
    uint16_t config = (static_cast<uint16_t>(wire.written[7]) << 8) | wire.written[8];
    EXPECT_EQ(config, 0xC120);
}

// ===========================================================================
// Section 10: Custom I2C address
// ===========================================================================

TEST(I2CProtocol, CustomAddress_UsedInTransmissions) {
    MockWire wire;
    ADS1X15::ADS1015<MockWire> ads(wire);
    ads.begin(0x49);
    wire.reset();
    ads.startSingleEndedReading(0, false);
    for (auto addr : wire.transmitted_addrs) {
        EXPECT_EQ(addr, 0x49);
    }
}

// ===========================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
