#ifndef GPIO_DRIVER_HEADER_G
#define GPIO_DRIVER_HEADER_G

/**
 * @biref This file implements a simple driver for settin up the gpio pins on nucleo-stm32c310c6
 * 
 * As of now, this driver uses the CMSIS registers namely RCC and IOPENR and the gpio port object
 * to set up and acees the gpio pin of the port where
 *      RCC is used to make sure the clock is enable for the GPIO peripheral
 *      IOPENR is used to make sure that the clock is hooked up the GPIO pin
 *  
 * Author: Omid Kandelusy
*/
// ===============================================================================================
// including the required header files:

/** standard C header files */
#include <stdint.h>

/** CMSIS header files */
#include "stm32c031xx.h"


// ===============================================================================================
// exposed driver apis

// name mingling fix for c functions:
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief configures a pin from a port in output mode
 * 
 * @param[in] port the gpio port that pin blongs to
 * @param[in] pin the pin number
 */
void gpio_init_as_output(GPIO_TypeDef *port, uint8_t pin);

/**
 * @brief sets a pin to logic high
 * 
 * @param[in] port the gpio port that pin blongs to
 * @param[in] pin the pin number
 */
void gpio_set(GPIO_TypeDef *port, uint8_t pin);

/**
 * @brief sets a pin to loigc low
 * 
 * @param[in] port the gpio port that pin blongs to
 * @param[in] pin the pin number
 */
void gpio_clear(GPIO_TypeDef *port, uint8_t pin);

/**
 * @brief toggles a pin logic state
 * 
 * @param[in] port the gpio port that pin blongs to
 * @param[in] pin the pin number
 */
void gpio_toggle(GPIO_TypeDef *port, uint8_t pin);


#ifdef __cplusplus
}
#endif //__CPLUSPLUS



#endif