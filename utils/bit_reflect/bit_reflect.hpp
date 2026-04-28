/**
 * @brief this utility enables to reverse an integer's bit order
 * 
 * @details It is very useful when changing LSB to MSB is needed.
 * 
 * Author: Omid Kandelusy
 */
#ifndef BIT_REFLECT_HEADER_G
#define BIT_REFLECT_HEADER_G
// ==========================================================================
// including the required header files

#include <cstdint>


class Bit_reflect{
    public:
    static constexpr uint8_t reflect_8(uint8_t value){
        return reflect<uint8_t, 8>(value);
    }

    static constexpr uint16_t reflect_16(uint16_t value){
        return reflect<uint16_t, 16>(value);
    }

   static constexpr uint32_t reflect_32(uint32_t value){
        return reflect<uint32_t, 32>(value);
    }

    private:
    template<typename T, uint8_t width>
    static constexpr T reflect(T value){
        T reflected =  0;
        for (int i=0; i< width; i++){
            T ruler = static_cast<T>(1) << i;
            // we only need to reverse the ones:
            if (value & ruler){
                reflected |= (static_cast<T>(1) << (width - 1 - i));
            }
        }

        return reflected;
    }

};

// compile time tests:
static_assert(Bit_reflect::reflect_8(0x01) == 0x80, "lsb to msb failed");
static_assert(Bit_reflect::reflect_8(0xA3) == 0xC5, "reflect_8 failed");
static_assert(Bit_reflect::reflect_8(0x00) == 0x00, "zero failed");
static_assert(Bit_reflect::reflect_8(0xFF) == 0xFF, "all ones failed");

static_assert(Bit_reflect::reflect_16(0x0001) == 0x8000, "reflect_16 failed");
static_assert(Bit_reflect::reflect_32(0x00000001) == 0x80000000, "reflect_32 failed");

#endif