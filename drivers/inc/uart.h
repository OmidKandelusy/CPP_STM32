/**
 * @brief this library implements a custom driver for UART peripheral
 * 
 * This driver uses CMSIS register to configure and initialize acees to
 * the uart peripheral on the STM32C031C6. Currently, a simple Tx and Rx
 * mode are supported.
 * 
 * @note This library is particularly intended for use with the NUCLEO
 *       boards and therefore the pin configuration and setup in the
 *       param initializer function should be changed if you plan to 
 *       use this code for a different board.
 * 
 * @attention
 *      The clock running the perihperal by default runs on 12MHz and the
 *      baudrate values higher than the default 9600bps have not been tested
 *      yet. So, it is recommended to be mindful of this note when changing
 *      the baud rate.
 * 
 * Author: Omid Kandelusy
 */

#ifndef UART_HEADER_GUARD
#define UART_HEADER_GUARD
// ==========================================================================
// including the required header files

/** standard C header files */
#include <stdint.h>

/** cmsis chip header file */
#include "stm32c031xx.h"

// name mingling fix for c functions:
#ifdef __cplusplus
extern "C" {
#endif

// ==========================================================================
// typedefs and macros:

// GPIO port's MODER register macro representing Alternate Function
// (another peripheral access)
#define GPIO_MODE_AF  (2U)

// GPIO port's AFR register bit-width per pin
#define AFR_PERPIN_WIDTH (4U)

// Number of pins per AFR register
#define AFR_PIN_COUNT 8

// uart peripheral clock frequency
#define UART_PCLK 12000000UL


// uart rx callback function type
typedef void (*uart_rx_cb_t)(uint8_t data);

/**
 * @brief uart object type used by the driver logic
 * 
 * @param baudrate uart throughput in bit per second on the line
 * @param instance the pointer to the USART peripheral
 * @param tx_port GPIO port used for the tx pin of the uart
 * @param tx_af flag indicating if the tx_mode is enabled
 * @param rx_port GPIO port used for the rx pin of the uart
 * @param rx_af flag indicating if the rx_mode is enabled
 * @param rx_cb the rx callback that use can pass to the driver
 */
typedef struct {   
    uint32_t baudrate;
    USART_TypeDef *instance;

    // Tx
    GPIO_TypeDef *tx_port;
    uint8_t tx_pin;
    uint8_t tx_af;

    // Rx
    GPIO_TypeDef *rx_port;
    uint8_t rx_pin;
    uint8_t rx_af;
    uart_rx_cb_t rx_cb;

} uart_t;

// ==========================================================================
// driver's exposed apis:

/**
 * @brief uart object initializer function
 * 
 * @details this function must be called before initializing the uart peripheral
 *          upon calling this function, the uart object's feilds are populated by
 *          default configurations.
 * 
 * @note If you plan to use the uart in the rx mode as well, the callback must be
 *       registered via updating the uart object after this function call.
 * 
 * @param [in] uart uart object containing the meta data and references used by driver
 * @return 0 on success and negative error code on failure.
 */
int uart_params_init(uart_t *uart);

/**
 * @brief initialized the uart peripheral
 * 
 * @param [in] uart uart object containing the meta data and references used by driver
 * @return 0 on success and negative error code on failure.
 */
int uart_init(uart_t *uart);


/**
 * @brief this function writes one byte to the uart line
 * 
 * @param [in] uart uart object containing the meta data and references used by driver
 * @param [out] data the byte to be written onto the uart line.
 * @return 0 on success and negative error code on failure.
 */
int uart_write_byte(uart_t *uart, uint8_t data);


#ifdef __cplusplus
}
#endif

#endif