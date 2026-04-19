#ifndef UART_SHELL_HEADER_GUARD
#define UART_SHELL_HEADER_GUARD
// ===================================================================================
// including the required header files

/** standard C header file */
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/** driver header files */
#include "uart.h"
// ===================================================================================
// type defintions and macros



template <size_t capacity>
class Isr_buffer{
    public:
    // class methods
    void init(void){
        memset(isr_buff, 0, capacity);
        counter = 0;
        ready = false;
    }

    void push(uint8_t c){

        if (ready) return;
        isr_buff[counter++] = c;

        if (c == '\n' || c=='\r'){
            ready = true;
        }
        if (counter >= capacity){
            memset(isr_buff, 0, capacity);
        }
    }

    int get_msg(uint8_t* msg, uint16_t* msg_len){
        if (!msg || !msg_len) return -1;
        if (!ready) return -2;
        
        memcpy(msg, isr_buff, counter);
        memcpy(msg_len, &counter, sizeof(counter));

        // clear the buffer
        ready = false;
        counter = 0;
        memset(isr_buff, 0, capacity);

        return 0;
    }

    private:
    bool ready;
    uint8_t isr_buff[capacity];
    uint16_t counter;
};
// ===================================================================================
// subsystems publicy exposes APIs


int uart_shell_init(void);


int uart_shell_start(void);

#endif // UART_SHELL_HEADER_GUARD