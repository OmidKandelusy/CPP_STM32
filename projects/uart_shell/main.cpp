// including the required header files:
#include "uart_shell.hpp"

int main(void){

    // initialize the uart shell subsystem:
    uart_shell_init();

    // start the uart_shell subsystem:
    uart_shell_start();

    return 0;
}