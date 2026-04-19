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
// driver apis:


int uart_params_init(uart_t *uart);

int uart_init(uart_t *uart);


int uart_write_byte(uart_t *uart, uint8_t data);


#ifdef __cplusplus
}
#endif

#endif