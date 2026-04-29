
/**
 * @details the clocking system in stm32c031c6 has its own specific congifuration
 *       and architetur which must be taken into account when designing a timer
 *       driver. The following clarifies the clocking system:
 *
 * Clock Sources:
 *   HSI48 = 48 MHz internal RC oscillator (no PLL available on this device)
 *
 * HSISYS Divider (RCC->CR, HSIDIV bits):
 *   Reset default = /4  →  SYSCLK = 12 MHz on startup
 *   Set to /1           →  SYSCLK = 48 MHz (must also set FLASH latency)
 *
 * Default clock chain (out of reset):
 *
 *   HSI48 (48 MHz)
 *       └─> HSIDIV /4        → SYSCLK = 12 MHz   (RCC->CR HSIDIV[2:0] = 010)
 *               └─> HPRE /1  → HCLK   = 12 MHz   (RCC->CFGR HPRE  = 0000)
 *                       └─> PPRE /1  → PCLK = 12 MHz   (RCC->CFGR PPRE  = 000)
 *
 * Peripheral clocks at reset default (all fed from PCLK):
 *   TIM3   = 12 MHz
 *   USART1 = 12 MHz  (can also be sourced directly from HSI48 via USART1SEL)
 *   USART2 = 12 MHz
 *
 * Note: There is only ONE APB bus on the C031 (no APB1/APB2 split).
 *       All peripherals share a single PCLK.
 *
 * To run at full 48 MHz:
 *   RCC->CR  = (RCC->CR & ~RCC_CR_HSIDIV) | (0 << RCC_CR_HSIDIV_Pos); // HSIDIV = /1
 *   FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY) | FLASH_ACR_LATENCY_1;
 *   SystemCoreClock = 48000000UL;
 *
 * Timer PSC/ARR formula:
 *   f_update = PCLK / ((PSC + 1) * (ARR + 1))
 *
 * UART BRR formula:
 *   BRR = PCLK / baud_rate
 */
// ===============================================================================================
// including the required header files:

/** standard C header files */
#include <stdint.h>
#include <stddef.h>

/** CMSIS header files */
#include "stm32c031xx.h"

/** driver header  */
#include "gtimer.h"

// ===============================================================================================
// driver definitions, variables and macros

#define GTIMER_PCLK_HZ    12000000UL
#define GTIMER_DRIVER_PSC  ((GTIMER_PCLK_HZ / 1000000UL) - 1)
#define GTIMER_DRIVER_ARR  999

#define GTIMER_MAX_MS  65UL

static volatile uint8_t timer_busy = 0;
static gtimer_cb_t registered_cb = NULL;
// ===============================================================================================
// driver implementations

// linker expected ISR
void TIM3_IRQHandler(void)
{
    if (TIM3->SR & TIM_SR_UIF) {
        TIM3->SR &= ~TIM_SR_UIF;  // clear flag

        // dispatch the isr to higher level cb

        timer_busy = 0;
    }
}

void gtimer_init(gtimer_cb_t cb){
    // enabling the peripheral clock source
    RCC->APBENR1 |= RCC_APBENR1_TIM3EN;

    // configuring the timer peripheral
    TIM3->CR1 |= TIM_CR1_OPM; // one-shot mode
    TIM3->PSC = GTIMER_DRIVER_PSC;
    TIM3->ARR = GTIMER_DRIVER_ARR;
    TIM3->EGR |= TIM_EGR_UG;
    // clear the UIF that EGR_UG just triggered
    TIM3->SR &= ~TIM_SR_UIF;

    // enabling the interrupt
    TIM3->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM3_IRQn);

    // regiser the user callback
    registered_cb = cb;
}


void gtimer_stop(void){
    // Note: order matters, don't change.

    // stop the counter
    TIM3->CR1 &= ~TIM_CR1_CEN;
    // clear any pending status flag
    TIM3->SR &= ~TIM_SR_UIF;
    // clear any pending interrupt that already made it to the NVIC
    NVIC_ClearPendingIRQ(TIM3_IRQn);

    timer_busy = 0;
}


int gtimer_timeout_ms(uint32_t ms) {
    if (ms>GTIMER_MAX_MS || ms == 0) return GTIMER_INVALID_TIMEOUT_INPUT;
    if (timer_busy != 0) return GTIMER_TIMER_BUSY;

    // reset before loading the new params:
    gtimer_stop();
    // taking the peripheral:
    timer_busy = 1;

    // ARR = number of ticks - 1 (1 tick = 1us at 1MHz, so 1ms = 1000 ticks)
    TIM3->ARR = (ms * 1000) - 1;
    // force register reload before starting
    TIM3->EGR |= TIM_EGR_UG;
    // clear the UIF that EGR_UG just triggered
    TIM3->SR &= ~TIM_SR_UIF;
    // start the timer
    TIM3->CR1 |= TIM_CR1_CEN;

    return 0;
}