#ifndef ENDIAN_HEADER_G
#define ENDIAN_HEADER_G
// ==========================================================================
// including the required header files
#include <cstdint>

class Endian {
public:
    // 32-bit conversions
    static constexpr uint32_t to_big_endian(uint32_t value) {
        return swap32(value);
    }

    static constexpr uint32_t to_little_endian(uint32_t value) {
        return swap32(value);
    }

    // 16-bit conversions
    static constexpr uint16_t to_big_endian(uint16_t value) {
        return swap16(value);
    }

    static constexpr uint16_t to_little_endian(uint16_t value) {
        return swap16(value);
    }

private:
    static constexpr uint32_t swap32(uint32_t value) {
        return ((value & 0xFF000000) >> 24) |
               ((value & 0x00FF0000) >> 8)  |
               ((value & 0x0000FF00) << 8)  |
               ((value & 0x000000FF) << 24);
    }

    static constexpr uint16_t swap16(uint16_t value) {
        return ((value & 0xFF00) >> 8) |
               ((value & 0x00FF) << 8);
    }
};

// compile-time tests
static_assert(Endian::to_big_endian(uint32_t(0x11223344)) == 0x44332211, "swap32 failed");
static_assert(Endian::to_big_endian(uint16_t(0x1122))     == 0x2211,     "swap16 failed");
static_assert(Endian::to_little_endian(uint32_t(0x44332211)) == 0x11223344, "swap32 roundtrip failed");

#endif