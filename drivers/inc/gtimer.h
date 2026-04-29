/**
 * @brief general timer driver
 * 
 * @details this driver enables to congiure a one-shot timeout through the TIM3 general
 *          purpose timer peripheral on the stm32c031c6.
 * 
 * Authore: Omid Kandelusy
 */

#ifndef GTIMER_HEADER_GUARD
#define GTIMER_HEADER_GUARD
// =====================================================================================
// including the required header files

/** standard C header files */
#include <stdint.h>

// =====================================================================================
// typedefs, macros and constants

/* upper level timeout callback type */
typedef void (*gtimer_cb_t)(void);

/* driver error codes */
#define GTIMER_SUCCESS 0
#define GTIMER_NULL_POINTER -1
#define GTIMER_INVALID_TIMEOUT_INPUT -2
#define GTIMER_TIMER_BUSY -3

// =====================================================================================
// exposed APIs

// name mingling fix for c functions:
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief initializes the general purpose timer
 * 
 * @param [in] cb timeout callback that user can register with the driver
 */
void gtimer_init(gtimer_cb_t cb);

/**
 * @brief stops the general purpose timer
 */
void gtimer_stop(void);

/**
 * @brief arms a timeout at ms from now
 * 
 * @param [in] ms target timeout in ms
 * 
 * @return GTIMER_SUCCESS if succeeds, negative error code if fails.
 */
int gtimer_timeout_ms(uint32_t ms);


#ifdef __cplusplus
}
#endif

#endif