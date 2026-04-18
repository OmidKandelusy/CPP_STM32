// including the required header files:

/** standard c header file */
#include <stdbool.h>


/** driver's header file */
#include "uart.h"

// ==================================================================================
// definitions:

static bool uart_initialized = false;
// ==================================================================================
// driver functions:

// isr registery:
static uart_rx_cb_t rx_callback;

int uart_params_init(uart_t *uart){
    if (!uart) return -1;
    
    // defualt configs:
    uart->tx_pin = 2;
    uart->rx_pin = 3;
    uart->baudrate = 9600;
    uart->tx_port = GPIOA;
    uart->rx_port = GPIOA;
    uart->instance = USART2;
    uart->tx_af = 1;
    uart->rx_af = 1;

    return 0;
}


int uart_init(uart_t *uart) {

    if (!uart || !uart->tx_port || !uart->rx_port) return -1;
    if (uart_initialized) return 0;

    // Enable GPIO clocks for the port:
    if (uart->tx_port == GPIOA || uart->rx_port == GPIOA){
        RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    } else if (uart->tx_port == GPIOB || uart->rx_port == GPIOB){
        RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    } else {
        return -2;
    }

    // activate the USART clock
    // (Advanced Peripheral Bus Enable Register)
    if (uart->instance == USART1){
        RCC->APBENR2 |= RCC_APBENR2_USART1EN;
    }else if (uart->instance == USART2){
        RCC->APBENR1 |= RCC_APBENR1_USART2EN;
    } else {
        return -3;
    }


    // set the gpio port mode to AF (another peripheral uses this port)
    // (clear & set)
    uart->tx_port->MODER &= ~(3U << (uart->tx_pin * 2));
    uart->tx_port->MODER |=  (GPIO_MODE_AF << (uart->tx_pin * 2));
    uart->rx_port->MODER &= ~(3U << (uart->rx_pin * 2));
    uart->rx_port->MODER |=  (GPIO_MODE_AF << (uart->rx_pin * 2));


    // Tx and Rx AFR registers:
    uart->tx_port->AFR[uart->tx_pin / AFR_PIN_COUNT] &= ~(0xFU << ((uart->tx_pin % AFR_PIN_COUNT) * AFR_PERPIN_WIDTH));
    uart->tx_port->AFR[uart->tx_pin / AFR_PIN_COUNT] |=  (uart->tx_af << ((uart->tx_pin % AFR_PIN_COUNT) * AFR_PERPIN_WIDTH));
    uart->rx_port->AFR[uart->rx_pin / AFR_PIN_COUNT] &= ~(0xFU << ((uart->rx_pin % AFR_PIN_COUNT) * AFR_PERPIN_WIDTH));
    uart->rx_port->AFR[uart->rx_pin / AFR_PIN_COUNT] |=  (uart->rx_af << ((uart->rx_pin % AFR_PIN_COUNT) * AFR_PERPIN_WIDTH));

    // forcing 8-bit uart
    uart->instance->CR1 &= ~USART_CR1_M0;
    uart->instance->CR1 &= ~USART_CR1_M1;

    // Disable USART to avoid unwanted peripheral behavior
    uart->instance->CR1 &= ~USART_CR1_UE;

    // Set baud rate
    uart->instance->BRR = UART_PCLK / uart->baudrate;

    // Enable TX and RX
    uart->instance->CR1 |= USART_CR1_TE | USART_CR1_RE;

    // Enable RXNE interrupt
    uart->instance->CR1 |= USART_CR1_RXNEIE_RXFNEIE;

    // Enable USART2 interrupt in NVIC
    NVIC_EnableIRQ(USART2_IRQn);

    // register the callback to the driver
    rx_callback = uart->rx_cb;

    // Enable back the USART
    uart->instance->CR1 |= USART_CR1_UE;

    // setting the flag to indicate this uart instance is initialized:
    uart_initialized = true;

    return 0;
}

void USART2_IRQHandler(void) {
    if (USART2->ISR & USART_ISR_RXNE_RXFNE) {
        uint8_t data = (uint8_t)(USART2->RDR & 0xFF);
        if (rx_callback) {
            rx_callback(data);
        }
    }
    if (USART2->ISR & USART_ISR_ORE) {
        USART2->ICR |= USART_ICR_ORECF;
    }
}


int uart_write_byte(uart_t *uart, uint8_t data){
    if (!uart) return -1;

    // wait until transmit buffer is acepting:
    uint32_t timeout_counter = 2500000;
    while (!(uart->instance->ISR & USART_ISR_TXE_TXFNF)){
        if (--timeout_counter == 0){
            return -5;
        }
    };

    // write data to transmit register
    uart->instance->TDR = data;

    return 0;
}