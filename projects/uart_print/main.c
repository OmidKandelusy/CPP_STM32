// including the required header files:
#include "uart.h"
#include "gpio.h"

static uart_t uart_obj;

void rx_callback(uint8_t data){
    gpio_toggle(GPIOA, 5);
}

int main(void) {

    int ret = uart_params_init(&uart_obj);
    if (ret != 0){
        return 0;
    }
    uart_obj.rx_cb = rx_callback;

    gpio_init_as_output(GPIOA, 5);

    ret = uart_init(&uart_obj);
    if (ret != 0){
        return 0;
    }

    for (int i=0; i<5; i++){
        uart_write_byte(&uart_obj, 'A' +i);
    }


    return 0;
}