// including the required header files

/* standard C header files */
#include <stdint.h>

/** driver header files */
#include "gtimer.h"
#include "wait.h"

/** subsystem header files */
#include "blink.hpp"

#define PIN_NUMBER 5

static volatile uint8_t timeout_flag = 0;
// ===============================================================================
// main routine

void timeout_callback(void){
    timeout_flag = 1;
}

int main(void){
    int ret = 0;

    // creating a blink object:
    Blink Blink(GPIOA, PIN_NUMBER);

    // initialize the object:
    Blink.init();
    
    gtimer_init(timeout_callback);

    ret = gtimer_timeout_ms(500);
    if (ret <0 ){
        Blink.blink();
    }

    while(1){
        sys_wait_ms(400);

        if (timeout_flag){
            Blink.pattern(PATTERN_1);
        }
    }

    return 0;
}
