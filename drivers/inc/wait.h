#ifndef WAIT_HEADER_GUARD
#define WAIT_HEADER_GUARD
// ===================================================================================

/** standard C header files */
#include <stdint.h>

/** CMSIS header files */
#include "stm32c031xx.h"


#ifdef __cplusplus
extern "C" {
#endif

void sys_wait_ms(uint32_t wait_ms);

#ifdef __cplusplus
}
#endif

#endif