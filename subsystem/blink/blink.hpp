#ifndef BLINK_HEADER_GUARD
#define BLINK_HEADER_GUARD

/** including the required header files */
#include "gpio.h"
#include "wait.h"

// ==========================================================================
// varaible defitnions and constants

typedef enum {
    PATTERN_0 = 0,
    PATTERN_1 = 1,
    PATTERN_2 = 2
} blink_pattern_t;

// ==========================================================================
// typedefs and declarations

class Blink{
    public:
    // Constructor takes GPIO port and pin number
    Blink(GPIO_TypeDef *port, uint8_t pin);

        void init();
        void blink();
        void pattern(blink_pattern_t pattern);

    private:
        GPIO_TypeDef *m_port;
        uint8_t m_pin;
};



#endif