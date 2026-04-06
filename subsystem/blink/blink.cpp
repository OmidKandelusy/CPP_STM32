/** including the library's header file */
#include "blink.hpp"

// ==========================================================================
// varaible defitnions and constants


#define PATTERN_0_COUNT 5
#define PATTERN_1_COUNT 10
#define PATTERN_2_COUNT 15

/** class definition */
Blink::Blink(GPIO_TypeDef* port, uint8_t pin):m_port(port), m_pin(pin) { };



void Blink::init(){

    /** initialize the pin as the output: */
    gpio_init_as_output(m_port, m_pin);

    /** turn off the LED or the pin for clean start */
    gpio_clear(m_port, m_pin);

}

void Blink::blink(){

    gpio_toggle(m_port, m_pin);
    sys_wait_ms(30);
    gpio_toggle(m_port, m_pin);

}

void Blink::pattern(blink_pattern_t pattern){
    switch (pattern){
        case PATTERN_0:
            for (int i=0; i<PATTERN_0_COUNT;i++){
                gpio_toggle(m_port, m_pin);
                sys_wait_ms(10);
            }
            break;

        case PATTERN_1:
            for(int i=0; i<PATTERN_1_COUNT;i++){
                gpio_toggle(m_port, m_pin);
                sys_wait_ms(10*(i+1));
            }

        case PATTERN_2:
            for(int i=0; i<PATTERN_2_COUNT;i++){
                if (i < PATTERN_2_COUNT/2 ){
                    gpio_toggle(m_port, m_pin);
                    sys_wait_ms(10*(i+1));
                } else {
                    gpio_toggle(m_port, m_pin);
                    sys_wait_ms(100 - (i+1)*10);
                }
            }

        default:
            break;

    }
}