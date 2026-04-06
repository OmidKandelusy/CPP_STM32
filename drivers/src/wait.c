#include "wait.h"

volatile uint32_t isr_counter = 0;

/** systick handler (weakly defined in the vector table) */
void SysTick_Handler(void) {
    isr_counter++;
}

void sys_wait_ms(uint32_t wait_ms) {

    // reset the isr_counter
    isr_counter = 0;

    // configure a 1-ms recurring isr routine: (ticks per 1ms)
    uint32_t ticks = SystemCoreClock / 1000;

    // set the systic register
    SysTick->LOAD = ticks - 1;
    SysTick->VAL  = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;

    while (isr_counter < wait_ms) {
        __WFI();
    }

    // reset the systick register
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
}