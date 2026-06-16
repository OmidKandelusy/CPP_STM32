/**
 * @brief this driver enables access to the I2C peripheral on the STM32C031C6
 * 
 * The driver makes use of CMSIS header files to access the registers controlling
 * the peripheral.
 * 
 * @author Omid Kandelusy, June 2026
 */
// =================================================================================
#ifndef I2C_HEADER_GUARD
#define I2C_HEADER_GUARD
// =================================================================================
// including the required header files


/** standard C header files */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/** cmsis chip header file */
#include "stm32c031xx.h"

// name mingling fix for c functions:
#ifdef __cplusplus
extern "C" {
#endif
// =================================================================================



#ifdef __cplusplus
}
#endif

#endif