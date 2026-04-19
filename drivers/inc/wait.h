/**
 * @brief this library anables a  wait mechanism driver
 * 
 * @details this driver enables the user to enforce ms-level wait withing which the
 *          cpu is put into standby mode recurrently until the timeout is reached.
 *          This driver enforces a blocking wait and within the wait, the cpu only
 *          wakes up for a few cycles of checking some flags.
 *         
 * Author: Omid Kandelusy
 */

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