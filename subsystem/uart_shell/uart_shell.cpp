#include "uart_shell.hpp"
#include "wait.h"


#define SHELL_ISR_BUFFER_CAPACITY 64
#define SHELL_SERVICE_DELAY_MS 150

static uart_t uart_obj;


Isr_buffer<SHELL_ISR_BUFFER_CAPACITY> shell_isr_buffer;



void rx_callback(uint8_t data){
    shell_isr_buffer.push(data);
    uart_write_byte(&uart_obj, data);
}


void print_shell_promt(void){

    uart_write_byte(&uart_obj, '\n');
    uart_write_byte(&uart_obj, '\r');


    uart_write_byte(&uart_obj, 0x1B);
    uart_write_byte(&uart_obj, '[');
    uart_write_byte(&uart_obj, '3');
    uart_write_byte(&uart_obj, '2');
    uart_write_byte(&uart_obj, 'm');

    uart_write_byte(&uart_obj, 'U');
    uart_write_byte(&uart_obj, 'A');
    uart_write_byte(&uart_obj, 'R');
    uart_write_byte(&uart_obj, 'T');
    uart_write_byte(&uart_obj, ' ');
    uart_write_byte(&uart_obj, '$');
    uart_write_byte(&uart_obj, ':');

    uart_write_byte(&uart_obj, 0x1B);
    uart_write_byte(&uart_obj, '[');
    uart_write_byte(&uart_obj, '0');
    uart_write_byte(&uart_obj, 'm');
    uart_write_byte(&uart_obj, ' ');

}

void print_shell_error(int err_code){

    uart_write_byte(&uart_obj, '\n');
    uart_write_byte(&uart_obj, '\r');
    uart_write_byte(&uart_obj, '[');
    uart_write_byte(&uart_obj, 'E');
    uart_write_byte(&uart_obj, 'r');
    uart_write_byte(&uart_obj, 'r');
    uart_write_byte(&uart_obj, 'o');
    uart_write_byte(&uart_obj, 'r');
    uart_write_byte(&uart_obj, ']');
    uart_write_byte(&uart_obj, '\n');
    uart_write_byte(&uart_obj, '\r');

}


void print_help(void){
    uart_write_byte(&uart_obj, '\n');
    uart_write_byte(&uart_obj, 'H');
    uart_write_byte(&uart_obj, 'e');
    uart_write_byte(&uart_obj, 'l');
    uart_write_byte(&uart_obj, 'p');
    uart_write_byte(&uart_obj, ' ');
    uart_write_byte(&uart_obj, 'i');
    uart_write_byte(&uart_obj, 's');
    uart_write_byte(&uart_obj, ' ');
    uart_write_byte(&uart_obj, 'h');
    uart_write_byte(&uart_obj, 'e');
    uart_write_byte(&uart_obj, 'r');
    uart_write_byte(&uart_obj, 'e');
    uart_write_byte(&uart_obj, '\n');
}


int uart_shell_process(uint8_t *data, uint16_t data_len){
    if (!data) return -1;
    if (data_len <=0 || data_len >SHELL_ISR_BUFFER_CAPACITY) return -2;
   
    // shell command check:
    uint8_t literal_index = 0;
    const char* msg_literal_1 = "help";
    for (int i=0; i<data_len-1; i++){
        if (data[i] == msg_literal_1[i]){
            literal_index = 1;
        } else {
            literal_index = 0;
            break;
        }
    }

    if (literal_index == 1){
        print_help();
    }

    return 0;
}



int uart_shell_init(void){

    int ret = uart_params_init(&uart_obj);
    if (ret != 0){
        return 0;
    }
    uart_obj.rx_cb = rx_callback;

    ret = uart_init(&uart_obj);
    if (ret != 0){
        return 0;
    }

    shell_isr_buffer.init();

    return 0;

}


int uart_shell_start(void){

    uint8_t msg_p[128] = {0};
    uint16_t msg_len = 0;

    print_shell_promt();

    // surveilance loop:
    while(1){

        // waiting
        sys_wait_ms(SHELL_SERVICE_DELAY_MS);

        int ret = shell_isr_buffer.get_msg(msg_p, &msg_len);
        if (ret == 0){
            ret = uart_shell_process(msg_p, msg_len);
            if (ret == 0){
                print_shell_promt();
            } else {
                print_shell_error(ret);
            }
        }
    }

    return 0;

}