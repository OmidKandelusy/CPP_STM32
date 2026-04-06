/** driver's header file */
#include "gpio.h"


// ===============================================================================================
// driver's implementations

// Configure a pin as general purpose output
void gpio_init_as_output(GPIO_TypeDef *port, uint8_t pin) {
    // Enable GPIO clock (simplified: here only for GPIOA, extend later)
    if (port == GPIOA) RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

    // Set pin mode (bits per pin in MODER register)
    port->MODER &= ~(3 << (pin * 2));
    port->MODER |=  (1 << (pin * 2));
}

// Set pin high
void gpio_set(GPIO_TypeDef *port, uint8_t pin) {
    port->ODR |= (1 << pin);
}

// Set pin low
void gpio_clear(GPIO_TypeDef *port, uint8_t pin) {
    port->ODR &= ~(1 << pin);
}

// Toggle pin
void gpio_toggle(GPIO_TypeDef *port, uint8_t pin) {
    port->ODR ^= (1 << pin);
}
