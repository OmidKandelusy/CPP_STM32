// including the required header files

/* standard C header files */
#include <stdint.h>

/** driver header files */
#include "gtimer.h"


// ===============================================================================
// main routine

void timeout_callback(void){
    // do nothing yet
}

int main(void){
    int ret = 0;
    
    gtimer_init(timeout_callback);

    ret = gtimer_timeout_ms(500);
    if (ret <0 ) while (1);

    return 0;
}
